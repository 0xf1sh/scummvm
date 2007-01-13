/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 * Original ADL-Player source Copyright (C) 2004 by Patrick Combet aka Dorian Gray
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"
#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/music.h"
#include "gob/gob.h"
#include "gob/game.h"
#include "gob/util.h"

namespace Gob {

const char *Music::_tracks[][2] = {
	{"avt00.tot",  "mine"},
	{"avt001.tot", "nuit"},
	{"avt002.tot", "campagne"},
	{"avt003.tot", "extsor1"},
	{"avt004.tot", "interieure"},
	{"avt005.tot", "zombie"},
	{"avt006.tot", "zombie"},
	{"avt007.tot", "campagne"},
	{"avt008.tot", "campagne"},
	{"avt009.tot", "extsor1"},
	{"avt010.tot", "extsor1"},
	{"avt011.tot", "interieure"},
	{"avt012.tot", "zombie"},
	{"avt014.tot", "nuit"},
	{"avt015.tot", "interieure"},
	{"avt016.tot", "statue"},
	{"avt017.tot", "zombie"},
	{"avt018.tot", "statue"},
	{"avt019.tot", "mine"},
	{"avt020.tot", "statue"},
	{"avt021.tot", "mine"},
	{"avt022.tot", "zombie"}
};

const char *Music::_trackFiles[] = {
//	"musmac1.adl", // TODO: This track isn't played correctly at all yet
	"musmac2.adl",
	"musmac3.adl",
	"musmac4.adl",
	"musmac5.adl",
	"musmac6.adl"
};

const unsigned char Music::_operators[] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
const unsigned char Music::_volRegNums[] = { 
	3,  4,  5,
	11, 12, 13,
	19, 20, 21
};

Music::Music(GobEngine *vm) : _vm(vm) {
	int i;

	_index = -1;
	_data = 0;
	_playPos = 0;
	_dataSize = 0;
	_rate = _vm->_mixer->getOutputRate();
	_opl = makeAdlibOPL(_rate);
	_vm->_mixer->setupPremix(this, Audio::Mixer::kMusicSoundType);
	_first = true;
	_ended = false;
	_playing = false;
	_needFree = false;
	_repCount = -1;
	_samplesTillPoll = 0;

	for (i = 0; i < 16; i ++)
		_pollNotes[i] = 0;

	setFreqs();
}

Music::~Music(void) {
	OPLDestroy(_opl);
	if (_data && _needFree)
		delete[] _data;
	_vm->_mixer->setupPremix(0);
}

void Music::premixerCall(int16 *buf, uint len) {
	_mutex.lock();
	if (!_playing) {
		memset(buf, 0, 2 * len * sizeof(int16));
		_mutex.unlock();
		return;
	}
	else {
		if (_first) {
			memset(buf, 0, 2 * len * sizeof(int16));
			pollMusic();
			_mutex.unlock();
			return;
		}
		else {
			uint32 render;
			int16 *data = buf;
			uint datalen = len;
			while (datalen && _playing) {
				if (_samplesTillPoll) {
					render = (datalen > _samplesTillPoll) ? (_samplesTillPoll) : (datalen);
					datalen -= render;
					_samplesTillPoll -= render;
					YM3812UpdateOne(_opl, data, render);
					data += render;
				} else {
					pollMusic();
					if (_ended) {
						memset(data, 0, datalen * sizeof(int16));
						datalen = 0;
					}
				}
			}
		}
		if (_ended) {
			_first = true;
			_ended = false;
			_playPos = _data + 3 + (_data[1] + 1) * 0x38;
			_samplesTillPoll = 0;
			if (_repCount == -1) {
				reset();
				setVoices();
			} else if (_repCount > 0) {
				_repCount--;
				reset();
				setVoices();
			}
			else
				_playing = false;
		}
		// Convert mono data to stereo
		for (int i = (len - 1); i >= 0; i--) {
			buf[2 * i] = buf[2 * i + 1] = buf[i];
		}
	}
	_mutex.unlock();
}

void Music::writeOPL(byte reg, byte val) {
	debugC(6, kDebugMusic, "writeOPL(%02X, %02X)", reg, val);
	OPLWriteReg(_opl, reg, val);
}

void Music::setFreqs(void) {
	byte lin;
	byte col;
	long val = 0;

	// Run through the 11 channels
	for (lin = 0; lin < 11; lin ++) {
		_notes[lin] = 0;
		_notCol[lin] = 0;
		_notLin[lin] = 0;
		_notOn[lin] = false;
	} 
		
	// Run through the 25 lines
	for (lin = 0; lin < 25; lin ++) {
		// Run through the 12 columns
		for (col = 0; col < 12; col ++) {
			if (!col)
				val = (((0x2710L + lin * 0x18) * 0xCB78 / 0x3D090) << 0xE) * 9 / 0x1B503;
			_freqs[lin][col] = (short)((val + 4) >> 3);
			val = val * 0x6A / 0x64;
	//      val = val *  392 / 370;
		} 
	}
}

void Music::reset() {
	OPLResetChip(_opl);
	_samplesTillPoll = 0;

	setFreqs();
	// Set frequencies and octave to 0; notes off
	for (int i = 0; i < 9; i++) {
		writeOPL(0xA0 | i, 0);
		writeOPL(0xB0 | i, 0);
		writeOPL(0xE0 | _operators[i]    , 0);
		writeOPL(0xE0 | _operators[i] + 3, 0);
	}

	// Authorize the control of the waveformes
	writeOPL(0x01, 0x20);
}

void Music::setVoices() {
	// Definitions of the 9 instruments
	for (int i = 0; i < 9; i++)
		setVoice(i, i, true);
}

void Music::setVoice(byte voice, byte instr, bool set) {
	int i;
	int j;
	uint16 strct[27];
	byte channel;
	byte *dataPtr;

	// i = 0 :  0  1  2  3  4  5  6  7  8  9 10 11 12 26
	// i = 1 : 13 14 15 16 17 18 19 20 21 22 23 24 25 27
	for (i = 0; i < 2; i++) {
		dataPtr = _data + 3 + instr * 0x38 + i * 0x1A;
		for (j = 0; j < 27; j++) {
			strct[j] = READ_LE_UINT16(dataPtr);
			dataPtr += 2;
		}
		channel = _operators[voice] + i * 3;
		writeOPL(0xBD, 0x00);
		writeOPL(0x08, 0x00);
		writeOPL(0x40 | channel, ((strct[0] & 3) << 6) | (strct[8] & 0x3F));
		if (!i)
			writeOPL(0xC0 | voice  , ((strct[2] & 7) << 1) | (1 - (strct[12] & 1)));
		writeOPL(0x60 | channel, ((strct[3] & 0xF) << 4) | (strct[6] & 0xF));
		writeOPL(0x80 | channel, ((strct[4] & 0xF) << 4) | (strct[7] & 0xF));
		writeOPL(0x20 | channel, ((strct[9] & 1) << 7) |
			((strct[10] & 1) << 6) | ((strct[5] & 1) << 5) |
			((strct[11] & 1) << 4) |  (strct[1] & 0xF));
		if (!i)
			writeOPL(0xE0 | channel, (strct[26] & 3));
		else
			writeOPL(0xE0 | channel, (strct[14] & 3));
		if (i && set)
			writeOPL(0x40 | channel, 0);
	}
}

void Music::setKey(byte voice, byte note, bool on, bool spec) {
	short freq = 0;
	short octa = 0;

	// Instruction AX
	if (spec) {
		// 0x7F donne 0x16B;
		//     7F
		// <<   7 =  3F80
		// + E000 = 11F80
		// & FFFF =  1F80
		// *   19 = 31380
		// / 2000 =    18 => Ligne 18h, colonne  0 => freq 16B

		// 0x3A donne 0x2AF;
		//     3A
		// <<   7 =  1D00
		// + E000 =  FD00 n�gatif
		// *   19 = xB500
		// / 2000 =    -2 => Ligne 17h, colonne -1

		//     2E
		// <<   7 =  1700
		// + E000 =  F700 n�gatif
		// *   19 = x1F00
		// / 2000 =
		short a;
		short lin;
		short col;

		a = (note << 7) + 0xE000; // Volontairement tronqu�
		a = (short)((long)a * 25 / 0x2000);
		if (a < 0) {
			col = - ((24 - a) / 25);
			lin = (-a % 25);
			if (lin)
				lin = 25 - lin;
		}
		else {
			col = a / 25;
			lin = a % 25;
		}

		_notCol[voice] = col;
		_notLin[voice] = lin;
		note = _notes[voice];
	}
	// Instructions 0X 9X 8X
	else {
		note -= 12;
		_notOn[voice] = on;
	}

	_notes[voice] = note;
	note += _notCol[voice];
	note = MIN(0x5F, (int)note);
	octa = note / 12;
	freq = _freqs[_notLin[voice]][note - octa * 12];

	writeOPL(0xA0 + voice,  freq & 0xff);
	writeOPL(0xB0 + voice, (freq >> 8) | (octa << 2) | 0x20 * on);

	if (!freq)
		warning("Voice %d, note %02X unknown\n", voice, note);
}

void Music::setVolume(byte voice, byte volume) {
	volume = 0x3F - (volume * 0x7E + 0x7F) / 0xFE;
	writeOPL(0x40 + _volRegNums[voice], volume);
}

void Music::pollMusic(void) {
	unsigned char instr;
	byte channel;
	byte note;
	byte volume;
	uint16 tempo;

	if ((_playPos > (_data + _dataSize)) && (_dataSize != (uint32) -1)) {
		_ended = true;
		return;
	}

	// First tempo, we'll ignore it...
	if (_first) {
		tempo = *(_playPos++);
		// Tempo on 2 bytes
		if (tempo & 0x80)
			tempo = ((tempo & 3) << 8) | *(_playPos++);
	}
	_first = false;

	// Instruction
	instr = *(_playPos++);
	channel = instr & 0x0F;

	switch (instr & 0xF0) {
		// Note on + Volume
		case 0x00:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setVolume(channel, *(_playPos++));
			setKey(channel, note, true, false);
			break;
		case 0x10:
			warning("GOB2 Stub! ADL command 0x10");
			break;
		case 0x50:
			warning("GOB2 Stub! ADL command 0x50");
			break;
		// Note on
		case 0x90:
			note = *(_playPos++);
			_pollNotes[channel] = note;
			setKey(channel, note, true, false);
			break;
		case 0x60:
			warning("GOB2 Stub! ADL command 0x60");
			break;
		// Last note off
		case 0x80:
			note = _pollNotes[channel];
			setKey(channel, note, false, false);
			break;
		// Frequency on/off
		case 0xA0:
			note = *(_playPos++);
			setKey(channel, note, _notOn[channel], true);
			break;
		// Volume
		case 0xB0:
			volume = *(_playPos++);
			setVolume(channel, volume);
			break;
		// Program change
		case 0xC0:
			setVoice(channel, *(_playPos++), false);
			break;
		// Special
		case 0xF0:
			switch (instr & 0x0F) {
			case 0xF: // End instruction
				_ended = true;
				_samplesTillPoll = 0;
				return;
			default:
				warning("Unknown special command in ADL, stopping playback: %X", instr & 0x0F);
				_repCount = 0;
				_ended = true;
				break;
			}
			break;
		default:
			warning("Unknown command in ADL, stopping playback: %X", instr & 0xF0);
			_repCount = 0;
			_ended = true;
			break;
	}

	// Temporization
	tempo = *(_playPos++);
	// End tempo
	if (tempo == 0xFF) {
		_ended = true;
		return;
	}
	// Tempo on 2 bytes
	if (tempo & 0x80)
		tempo = ((tempo & 3) << 8) | *(_playPos++);
	if (!tempo)
		tempo ++;

	_samplesTillPoll = tempo * (_rate / 1000);
}

void Music::startPlay(void) {
	if (!_data)
		return;
	
	_playing = true;
}

void Music::playBgMusic(void) {
	for (int i = 0; i < ARRAYSIZE(_tracks); i++)
		if (!scumm_stricmp(_vm->_game->_curTotFile, _tracks[i][0])) {
			playTrack(_tracks[i][1]);
			break;
		}
}

void Music::playTrack(const char *trackname) {
	if (_playing) return;
	
	debugC(1, kDebugMusic, "Music::playTrack(%s)", trackname);
	unloadMusic();
	loadMusic(_trackFiles[_vm->_util->getRandom(ARRAYSIZE(_trackFiles))]);
	startPlay();
}

bool Music::loadMusic(const char *filename) {
	Common::File song;

	unloadMusic();
	song.open(filename);
	if (!song.isOpen())
		return false;

	_needFree = true;
	_dataSize = song.size();
	_data = new byte[_dataSize];
	song.read(_data, _dataSize);
	song.close();

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;
	
	return true;
}

void Music::loadFromMemory(byte *data, int index) {
	unloadMusic();
	_repCount = 0;

	_dataSize = (uint32) -1;
	_data = data;
	_index = index;

	reset();
	setVoices();
	_playPos = _data + 3 + (_data[1] + 1) * 0x38;
}

void Music::unloadMusic(void) {
	_playing = false;
	_index = -1;

	if (_data && _needFree)
		delete[] _data;

	_needFree = false;
}

} // End of namespace Gob
