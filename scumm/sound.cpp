/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "sound.h"
#include "sound/mididrv.h"
#include "imuse.h"
#include "actor.h"
#include "bundle.h"
#include "common/config-file.h"
#include "common/util.h"

#ifdef _WIN32_WCE
extern void *bsearch(const void *, const void *, size_t,
										 size_t, int (*x) (const void *, const void *));
#endif

Sound::Sound(Scumm *parent) {
	_scumm = parent;
	_nameBundleMusic = NULL;
	_musicBundleBufFinal = NULL;
	_musicBundleBufOutput = NULL;
	_talkChannel = -1;
}

Sound::~Sound() {
}

void Sound::addSoundToQueue(int sound) {
	if (!(_scumm->_features & GF_AFTER_V7)) {
		_scumm->_vars[_scumm->VAR_LAST_SOUND] = sound;
		_scumm->ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	} else {
		// WARNING ! This may break something, maybe this sould be put inside if(_gameID == GID_FT) ? 
		// But why addSoundToQueue should not queue sound ?
		_scumm->ensureResourceLoaded(rtSound, sound);
		addSoundToQueue2(sound);
	}

//	if (_features & GF_AUDIOTRACKS)
//		warning("Requesting audio track: %d", sound);
}

void Sound::addSoundToQueue2(int sound) {
	if (_soundQue2Pos < 10) {
		_soundQue2[_soundQue2Pos++] = sound;
	}
}

void Sound::processSoundQues() {
	int d;
	int i, j;
	int num;
	int16 data[16];
	IMuse *se;

	processSfxQueues();

	while (_soundQue2Pos) {
		d = _soundQue2[--_soundQue2Pos];
		if (d)
			playSound(d);
	}

	for (i = 0; i < _soundQuePos;) {
		num = _soundQue[i++];
		if (i + num > _soundQuePos) {
			warning("processSoundQues: invalid num value");
			break;
		}
		for (j = 0; j < 16; j++)
			data[j] = 0;
		if (num > 0) {
			for (j = 0; j < num; j++)
				data[j] = _soundQue[i + j];
			i += num;

			se = _scumm->_imuse;

#if 0
			debug(1, "processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
						data[0] >> 8,
						data[0] & 0xFF,
						data[1], data[2], data[3], data[4], data[5], data[6], data[7]
				);
#endif
			
			if (!(_scumm->_features & GF_AFTER_V7)) {
				if (se)
					_scumm->_vars[_scumm->VAR_SOUNDRESULT] =
						(short)se->do_command(data[0], data[1], data[2], data[3], data[4],
																	data[5], data[6], data[7]);
			} else {
				if (_scumm->_imuseDigital)
					_scumm->_vars[_scumm->VAR_SOUNDRESULT] =
						(short)_scumm->_imuseDigital->doCommand(data[0], data[1], data[2], data[3], data[4],
																										data[5], data[6], data[7]);
			}
		}
	}
	_soundQuePos = 0;
}

byte * Sound::readCreativeVocFile(byte * ptr, uint32 & size, uint32 & rate, uint32 & loops) {
	assert(strncmp((char*)ptr, "Creative Voice File\x1A", 20) == 0);
	int32 offset = READ_LE_UINT16(ptr + 20);
	int16 version = READ_LE_UINT16(ptr + 22);
	int16 code = READ_LE_UINT16(ptr + 24);
	assert(version == 0x010A || version == 0x0114);
	assert(code == ~version + 0x1234);
	bool quit = 0;
	byte * ret_sound = 0; size = 0, loops = 0;
	while(!quit) {
		int len = READ_LE_UINT32(ptr + offset);
		offset += 4;
		code = len & 0xFF;
		len >>= 8;
		switch(code) {
			case 0: quit = 1; break;
			case 1: {
				int time_constant = ptr[offset++];
				int packing = ptr[offset++];
				len -= 2;
				rate = 1000000L / (256L - time_constant);
				debug(9, "VOC Data Bloc : %d, %d, %d", rate, packing, len);
				if(packing == 0) {
					if(size) {
						ret_sound = (byte*)realloc(ret_sound, size + len);
					} else {
						ret_sound = (byte*)malloc(len);
					}
					memcpy(ret_sound + size, ptr + offset, len);
					size += len;
				} else {
					warning("VOC file packing %d unsupported", packing);
				}
				} break;
			case 6:	// begin of loop
				loops = len + 1;
				break;
			case 7:	// end of loop
				break;
			default:
				warning("Invalid code in VOC file : %d", code);
				quit = 1;
				break;
		}
		// FIXME some FT samples (ex. 362) has bad length, 2 bytes too short
		offset += len;
	}
	debug(9, "VOC Data Size : %d", size);
	return ret_sound;
}

void Sound::playSound(int soundID) {
	byte *ptr;
	char *sound;
	int size;
	int rate;
	
	debug(3,"playSound #%d (room %d)",
		soundID, _scumm->getResourceRoomNr(rtSound, soundID));
	ptr = _scumm->getResourceAddress(rtSound, soundID);
	if (ptr) {
		if (READ_UINT32_UNALIGNED(ptr) == MKID('iMUS')){
			if (_scumm->_imuseDigital)
				_scumm->_imuseDigital->startSound(soundID);
			return;
		}
		else if (READ_UINT32_UNALIGNED(ptr) == MKID('SOUN')) {
			ptr += 8;
			_scumm->_vars[_scumm->VAR_MI1_TIMER] = 0;
			playCDTrack(ptr[16], ptr[17] == 0xff ? -1 : ptr[17],
							(ptr[18] * 60 + ptr[19]) * 75 + ptr[20], 0);
	
			_scumm->current_cd_sound = soundID;
			return;
		}
		// Support for SFX in Monkey Island 1, Mac version
		// This is rather hackish right now, but works OK. SFX are not sounding
		// 100% correct, though, not sure right now what is causing this.
		else if (READ_UINT32_UNALIGNED(ptr) == MKID('Mac1')) {
	
			// Read info from the header
			size = READ_UINT32_UNALIGNED(ptr+0x60);
			rate = READ_UINT32_UNALIGNED(ptr+0x64) >> 16;
	
			// Skip over the header (fixed size)
			ptr += 0x72;
			
			// Allocate a sound buffer, copy the data into it, and play
			sound = (char*)malloc(size);
			memcpy(sound, ptr, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			return;
		}
		// Support for Putt-Putt sounds - very hackish, too 8-)
		else if (READ_UINT32_UNALIGNED(ptr) == MKID('DIGI')) {
			// TODO - discover what data the first chunk, HSHD, contains
			// it might be useful here.
			ptr += 8 + READ_BE_UINT32_UNALIGNED(ptr+12);
			if (READ_UINT32_UNALIGNED(ptr) != MKID('SDAT'))
				return;	// abort
	
			size = READ_BE_UINT32_UNALIGNED(ptr+4);
			rate = 8000;	// FIXME - what value here ?!? 8000 is just a guess
			
			// Allocate a sound buffer, copy the data into it, and play
			sound = (char*)malloc(size);
			memcpy(sound, ptr + 8, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			return;
		}
		else if (READ_UINT32_UNALIGNED(ptr) == MKID('Crea')) {
			_scumm->_imuseDigital->startSound(soundID);
			return;
		}
		else if (READ_UINT32_UNALIGNED(ptr) == MKID('ADL ')) {
			// played as MIDI, just to make perhaps the later use
			// of WA possible (see "else if" with GF_OLD256 below)
		}
		// Support for sampled sound effects in Monkey1 and Monkey2
		else if (READ_UINT32_UNALIGNED(ptr) == MKID('SBL ')) {
			debug(2, "Using SBL sound effect");
	
			// TODO - Figuring out how the SBL chunk works. Here's
			// an example:
			//
			// 53 42 4c 20 00 00 11 ae  |SBL ....|
			// 41 55 68 64 00 00 00 03  |AUhd....|
			// 00 00 80 41 55 64 74 00  |...AUdt.|
			// 00 11 9b 01 96 11 00 a6  |........|
			// 00 7f 7f 7e 7e 7e 7e 7e  |...~~~~~|
			// 7e 7f 7f 80 80 7f 7f 7f  |~.......|
			// 7f 80 80 7f 7e 7d 7d 7e  |....~}}~|
			// 7e 7e 7e 7e 7e 7e 7e 7f  |~~~~~~~.|
			//
			// The length of the AUhd chunk always seems to be 3
			// bytes. Let's skip that for now.
			//
			// The starting offset, length and sample rate is all
			// pure guesswork. The result sounds reasonable to me,
			// but I've never heard the original.
			//
			// I've since discovered that the non-interactive
			// Sam & Max demo also uses SBL sound effects, but
			// with different sub-chunks. Example:
			//
			// 53 42 4c 20 00 01 15 6e  |SBL ...n|
			// 57 56 68 64 00 00 00 03  |WVhd....|
			// 00 00 80 57 56 64 74 00  |...WVdt.|
			// 01 15 5b 01 56 15 01 a6  |..[.V...|
			// 00 80 80 80 80 80 80 80  |........|
			// 80 80 80 80 80 80 80 80  |........|
			// 80 80 80 80 80 80 80 80  |........|
			// 80 80 80 80 80 80 80 80  |........|
			//
			// I'm going to assume that the sample frequency is
			// the only important difference between the two.

			if (READ_UINT32_UNALIGNED(ptr + 8) == MKID('WVhd'))
				rate = 11025;
			else
				rate = 8000;

			size = READ_BE_UINT32_UNALIGNED(ptr + 4) - 27;
	
			// Allocate a sound buffer, copy the data into it, and play
			sound = (char*)malloc(size);
			memcpy(sound, ptr + 33, size);
			_scumm->_mixer->playRaw(NULL, sound, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			return;
		} else if (_scumm->_features & GF_OLD256) {
			size = READ_LE_UINT32(ptr);
			
	#if 0
			// FIXME - this is just some debug output for Zak256
			if (size != 30) {
				char name[9];
				memcpy(name, ptr+22, 8);
				name[8] = 0;
				printf("Going to play Zak256 sound '%s':\n", name);
				hexdump(ptr, 0x40);
			}
			/*
			There seems to be some pattern in the Zak256 sound data. Two typical
			examples are these:
			
			d7 10 00 00 53 4f d1 10  |....SO..|
			00 00 00 00 04 00 ff 00  |........|
			64 00 00 00 01 00 64 6f  |d.....do|
			6f 72 6f 70 65 6e 40 a8  |oropen@.|
			57 14 a1 10 00 00 50 08  |W.....P.|
			00 00 00 00 00 00 b3 07  |........|
			00 00 3c 00 00 00 04 80  |..<.....|
			03 02 0a 01 8c 82 87 81  |........|
	
			5b 07 00 00 53 4f 55 07  |[...SOU.|
			00 00 00 00 04 00 ff 00  |........|
			64 00 00 00 01 00 64 72  |d.....dr|
			77 6f 70 65 6e 00 53 a8  |wopen.S.|
			57 14 25 07 00 00 92 03  |W.%.....|
			00 00 00 00 00 00 88 03  |........|
			00 00 3c 00 00 00 82 82  |..<.....|
			83 84 86 88 89 8b 89 89  |........|
			
			As you can see, there are quite some patterns, e.g.
			the 00 00 00 3c - the sound data seems to start at
			offset 54.
			
			Indy 3 seems to use a different format. The very first sound played
			in Indy 3 looks as follows:
			5a 25 00 00 53 4f 54 25  |Z%..SOT%|
			00 00 53 4f db 0a 00 00  |..SO....|
			57 41 c8 00 18 00 00 00  |WA......|
			00 00 00 00 9e 05 00 00  |........|
			00 00 00 00 fd 32 00 f8  |.....2..|
			02 f9 08 ff 22 08 00 ff  |...."...|
			20 5c 00 ff 10 03 00 fd  | \......|
			64 00 f8 00 f9 00 ff 22  |d......"|

			Indy 3, opening a door:
			d1 00 00 00 53 4f cb 00  |....SO..|
			00 00 53 4f a2 00 00 00  |..SO....|
			57 41 64 00 18 00 00 00  |WAd.....|
			00 00 00 00 00 00 00 00  |........|
			00 00 7e 00 f9 0c ff 20  |..~.... |
			90 01 ff 22 c2 01 ff 0a  |..."....|
			03 00 ff 04 57 06 ff 00  |....W...|
			04 00 ff 0a 00 00 ff 00  |........|
			
			So there seems to be a "SO" chunk which contains again a SO chunk and a WA chunk.
			WA probably again contains audio data?
			*/
	#endif
			rate = 11000;
			
			ptr += 0x16;
			if (size == 30) {
				int track = *ptr;
	
				if (track == _scumm->current_cd_sound)
					if (pollCD() == 1) return;

				playCDTrack(track, 1, 0, 0);
				_scumm->current_cd_sound = track;
				return;
			}
	
			size -= 0x36;
			sound = (char*)malloc(size);
			for (int x = 0; x < size; x++) {
				int bit = *ptr++;
				if (_scumm->_gameId == GID_INDY3_256) {
					// FIXME - this is an (obviously incorrect, just listen to it)
					// test hack for the Indy3 music format.... it doesn't work better
					// but at least the generated data "looks" somewhat OK :-)
					sound[x] = bit ^ 0x80;
				} else {
					if (bit < 0x80)
						sound[x] = 0x7F-bit;
					else
						sound[x] = bit;
				}
			}
	
			// FIXME: Maybe something in the header signifies looping? Need to
			// track it down and add a mixer flag or something (see also bug .
			_scumm->_mixer->playRaw(NULL, sound, size, 11000, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
			return;
		}
	
	}

	IMuse *se = _scumm->_imuse;
	if (se) {
		_scumm->getResourceAddress(rtSound, soundID);
		se->start_sound(soundID);
	}
}

void Sound::processSfxQueues() {
	Actor *a;
	int act;
	bool b, finished;

	if (_talk_sound_mode != 0) {
		if (_talk_sound_mode & 1)
			startTalkSound(_talk_sound_a1, _talk_sound_b1, 1);
		if (_talk_sound_mode & 2)
			_talkChannel = startTalkSound(_talk_sound_a2, _talk_sound_b2, 2);
		_talk_sound_mode = 0;
	}

	if (_scumm->_vars[_scumm->VAR_TALK_ACTOR]) { //_sfxMode & 2) {
		act = _scumm->_vars[_scumm->VAR_TALK_ACTOR];
		if (_talkChannel < 0)
			finished = false;
		else if (_scumm->_mixer->_channels[_talkChannel] == NULL) {
			finished = true;
		} else
			finished = false;
		

		if (act != 0 && (uint) act < 0x80 && !_scumm->_string[0].no_talk_anim) {
			a = _scumm->derefActorSafe(act, "processSfxQueues");
			if (a->room == _scumm->_currentRoom && (finished || !_endOfMouthSync)) {
				b = true;
				if (!finished)
					b = isMouthSyncOff(_curSoundPos);
				if (_mouthSyncMode != b) {
					_mouthSyncMode = b;
					if (_talk_sound_frame != -1) {
						a->startAnimActor(_talk_sound_frame);
						_talk_sound_frame = -1;
					} else
						a->startAnimActor(b ? a->talkFrame2 : a->talkFrame1);
				}
			}
		}
		
		if (finished  && _scumm->_talkDelay == 0) {
			_scumm->stopTalk();
			_sfxMode &= ~2;
			_talkChannel = -1;
		}
	}
		
	if (_sfxMode & 1) {
		if (isSfxFinished()) {
			_sfxMode &= ~1;
		}
	}
}

static int compar(const void *a, const void *b)
{
	return ((MP3OffsetTable *) a)->org_offset -
		((MP3OffsetTable *) b)->org_offset;
}

int Sound::startTalkSound(uint32 offset, uint32 b, int mode) {
	int num = 0, i;
	byte file_byte, file_byte_2;
	int size;

	if (_sfxFile->isOpen() == false) {
		warning("startTalkSound: SFX file is not open");
		return -1;
	}

	// Some games frequently assume that starting one sound effect will
	// automatically stop any other that may be playing at that time. So
	// that is what we do here, but we make an exception for speech.
	//
	// Do any other games than these need this hack?
	//
	// HACK: Checking for script 99 in Sam & Max is to keep Conroy's song
	// from being interrupted.

	if (mode == 1 && (_scumm->_gameId == GID_TENTACLE
		|| (_scumm->_gameId == GID_SAMNMAX && !_scumm->isScriptRunning(99)))) {
		for (i = 0; i < _scumm->_mixer->NUM_CHANNELS; i++) {
			if (i != _talkChannel)
				_scumm->_mixer->stop(i);
		}
	}

	if (b > 8) {
		num = (b - 8) >> 1;
	}

	if (offset_table != NULL) {
		MP3OffsetTable *result = NULL, key;

		key.org_offset = offset;
		result = (MP3OffsetTable *) bsearch(&key, offset_table, num_sound_effects,
		                                    sizeof(MP3OffsetTable), compar);

		if (result == NULL) {
			warning("startTalkSound: did not find sound at offset %d !", offset);
			return -1;
		}
		if (2 * num != result->num_tags) {
			warning("startTalkSound: number of tags do not match (%d - %d) !", b,
							result->num_tags);
			num = result->num_tags;
		}
		offset = result->new_offset;
		size = result->compressed_size;
	} else {
		offset += 8;
		size = -1;
	}

	_sfxFile->seek(offset, SEEK_SET);
	i = 0;
	while (num > 0) {
		_sfxFile->read(&file_byte, sizeof(file_byte));
		_sfxFile->read(&file_byte_2, sizeof(file_byte_2));
		assert(i < (int)ARRAYSIZE(_mouthSyncTimes));
		_mouthSyncTimes[i++] = file_byte | (file_byte_2 << 8);
		num--;
	}
	assert(i < (int)ARRAYSIZE(_mouthSyncTimes));
	_mouthSyncTimes[i] = 0xFFFF;
	_sfxMode |= mode;
	_curSoundPos = 0;
	_mouthSyncMode = true;

	return startSfxSound(_sfxFile, size);
}

void Sound::stopTalkSound() {
	if (_sfxMode & 2) {
		if (_talkChannel != -1)
			_scumm->_mixer->stop(_talkChannel);
		_sfxMode &= ~2;
	}
}

bool Sound::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;

	_endOfMouthSync = false;
	do {
		val ^= 1;
		j = *ms++;
		if (j == 0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos > j);
	return val;
}


int Sound::isSoundRunning(int sound) {
	IMuse *se;
	int i;

	if (sound == _scumm->current_cd_sound)
		return pollCD();

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i] == sound)
			return 1;
	}

	if (isSoundInQueue(sound))
		return 1;

	if (!_scumm->isResourceLoaded(rtSound, sound))
		return 0;

	if (_scumm->_imuseDigital) {
		return _scumm->_imuseDigital->getSoundStatus(sound);
	}

	se = _scumm->_imuse;
	if (!se)
		return 0;
	return se->get_sound_status(sound);
}

// This is exactly the same as isSoundRunning except that it
// calls IMuse::get_sound_active() instead of IMuse::get_sound_status().
// This is necessary when determining what resources to
// expire from memory.
bool Sound::isSoundActive(int sound) {
	IMuse *se;
	int i;

	if (sound == _scumm->current_cd_sound)
		return pollCD() != 0;

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i] == sound)
			return true;
	}

	if (isSoundInQueue(sound))
		return true;

	if (!_scumm->isResourceLoaded(rtSound, sound))
		return false;

	if (_scumm->_imuseDigital) {
		return _scumm->_imuseDigital->getSoundStatus(sound) != 0;
	}

	se = _scumm->_imuse;
	if (!se)
		return false;
	return se->get_sound_active(sound);
}

bool Sound::isSoundInQueue(int sound) {
	int i = 0, j, num;
	int16 table[16];

	while (i < _soundQuePos) {
		num = _soundQue[i++];

		memset(table, 0, sizeof(table));

		if (num > 0) {
			for (j = 0; j < num; j++)
				table[j] = _soundQue[i + j];
			i += num;
			if (table[0] == 0x10F && table[1] == 8 && table[2] == sound)
				return 1;
		}
	}
	return 0;
}

void Sound::stopSound(int a) {
	IMuse *se;
	int i;

	if (a != 0 && a == _scumm->current_cd_sound) {
		_scumm->current_cd_sound = 0;
		stopCD();
	}

	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->stopSound(a);
	} else {
		se = _scumm->_imuse;
		if (se)
			se->stop_sound(a);
	}

	for (i = 0; i < 10; i++)
		if (_soundQue2[i] == a)
			_soundQue2[i] = 0;
}

void Sound::stopAllSounds()
{
	IMuse *se = _scumm->_imuse;

	if (_scumm->current_cd_sound != 0) {
		_scumm->current_cd_sound = 0;
		stopCD();
	}

	if (se) {
		se->stop_all_sounds();
		se->clear_queue();
	}
	clearSoundQue();
	stopSfxSound();
}

void Sound::clearSoundQue() {
	_soundQue2Pos = 0;
	memset(_soundQue2, 0, sizeof(_soundQue2));
}

void Sound::soundKludge(int *list, int num) {
	int16 *ptr;
	int i;

	if (list[0] == -1) {
		processSoundQues();
		return;
	}
	_soundQue[_soundQuePos++] = 8;

	ptr = _soundQue + _soundQuePos;
	_soundQuePos += 8;

	for (i = 0; i < 8; i++)
		*ptr++ = list[i];
	if (_soundQuePos > 0x100)
		error("Sound que buffer overflow");
}

void Sound::talkSound(uint32 a, uint32 b, int mode, int frame) {
	if (mode == 1) {
		_talk_sound_a1 = a;
		_talk_sound_b1 = b;
	} else {
		_talk_sound_a2 = a;
		_talk_sound_b2 = b;
	}

	_talk_sound_frame = frame;
	_talk_sound_mode |= mode;
}

/* The sound code currently only supports General Midi.
 * General Midi is used in Day Of The Tentacle.
 * Roland music is also playable, but doesn't sound well.
 * A mapping between roland instruments and GM instruments
 * is needed.
 */

void Sound::setupSound() {
	if (_scumm->_imuse) {
		_scumm->_imuse->setBase(_scumm->res.address[rtSound]);

		_sound_volume_music = g_config->getInt("music_volume", kDefaultMusicVolume);
		_sound_volume_master = g_config->getInt("master_volume", kDefaultMasterVolume);
		_sound_volume_sfx = g_config->getInt("sfx_volume", kDefaultSFXVolume);

		_scumm->_imuse->set_master_volume(_sound_volume_master);
		_scumm->_imuse->set_music_volume(_sound_volume_music);
		_scumm->_mixer->setVolume(_sound_volume_sfx * _sound_volume_master / 255);
		_scumm->_mixer->setMusicVolume(_sound_volume_music);
	}
	_sfxFile = openSfxFile();
}

void Sound::pauseSounds(bool pause) {
	IMuse *se = _scumm->_imuse;
	if (se)
		se->pause(pause);

	_soundsPaused = pause;
	_scumm->_mixer->pause(pause);	

	_scumm->_sound->pauseBundleMusic(pause);

	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->pause(pause);
	}

	if ((_scumm->_features & GF_AUDIOTRACKS) && _scumm->_vars[_scumm->VAR_MI1_TIMER] > 0) {
		if (pause)
			stopCDTimer();
		else
			startCDTimer();
	}
		
}

int Sound::startSfxSound(File *file, int file_size) {
	char ident[8];
	int block_type;
	byte work[8];
	uint size = 0;
	int rate, comp;
	byte *data;

	if (file_size > 0) {
		int alloc_size = file_size;
#ifdef USE_MAD
		if (!_vorbis_mode)
			alloc_size += MAD_BUFFER_GUARD;
#endif
		data = (byte *)calloc(alloc_size, 1);

		if (file->read(data, file_size) != (uint)file_size) {
			/* no need to free the memory since error will shut down */
			error("startSfxSound: cannot read %d bytes", size);
			return -1;
		}
		if (_vorbis_mode)
			return playSfxSound_Vorbis(data, file_size);
		else
			return playSfxSound_MP3(data, file_size);
	}

	if (file->read(ident, 8) != 8)
		goto invalid;

	if (!memcmp(ident, "VTLK", 4)) {
		file->seek(SOUND_HEADER_BIG_SIZE - 8, SEEK_CUR);
	} else if (!memcmp(ident, "Creative", 8)) {
		file->seek(SOUND_HEADER_SIZE - 8, SEEK_CUR);
	} else {
	invalid:;
		warning("startSfxSound: invalid header");
		return -1;
	}

	block_type = file->readByte();
	if (block_type != 1) {
		warning("startSfxSound: Expecting block_type == 1, got %d", block_type);
		return -1;
	}

	file->read(work, 3);

	size = (work[0] | (work[1] << 8) | (work[2] << 16)) - 2;
	rate = file->readByte();
	comp = file->readByte();

	if (comp != 0) {
		warning("startSfxSound: Unsupported compression type %d", comp);
		return -1;
	}

	data = (byte *)malloc(size);
	if (data == NULL) {
		error("startSfxSound: out of memory");
		return -1;
	}

	if (file->read(data, size) != size) {
		/* no need to free the memory since error will shut down */
		error("startSfxSound: cannot read %d bytes", size);
		return -1;
	}

	return playSfxSound(data, size, 1000000 / (256 - rate), true);
}

File * Sound::openSfxFile() {
	char buf[256];
	File * file = new File();

	/* Try opening the file <_exe_name>.sou first, eg tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */
	offset_table = NULL;

#ifdef USE_MAD
	sprintf(buf, "%s.so3", _scumm->_exe_name);
	if (!file->open(buf, _scumm->getGameDataPath())) {
		file->open("monster.so3", _scumm->getGameDataPath());
	}
	if (file->isOpen())
		_vorbis_mode = false;
#endif

#ifdef USE_VORBIS
	if (!file->isOpen()) {
		sprintf(buf, "%s.sog", _scumm->_exe_name);
		if (!file->open(buf, _scumm->getGameDataPath()))
			file->open("monster.sog", _scumm->getGameDataPath());
		if (file->isOpen())
			_vorbis_mode = true;
	}
#endif

	if (file->isOpen()) {
		/* Now load the 'offset' index in memory to be able to find the MP3 data

		   The format of the .SO3 file is easy :
		   - number of bytes of the 'index' part
		   - N times the following fields (4 bytes each) :
		   + offset in the original sound file
		   + offset of the MP3 data in the .SO3 file WITHOUT taking into account
		   the index field and the 'size' field
		   + the number of 'tags'
		   + the size of the MP3 data
		   - and then N times :
		   + the tags
		   + the MP3 data
		 */
		int size, compressed_offset;
		MP3OffsetTable *cur;
		compressed_offset = file->readUint32BE();
		offset_table = (MP3OffsetTable *) malloc(compressed_offset);
		num_sound_effects = compressed_offset / 16;

		size = compressed_offset;
		cur = offset_table;
		while (size > 0) {
			cur[0].org_offset = file->readUint32BE();
			cur[0].new_offset = file->readUint32BE() + compressed_offset + 4; /* The + 4 is to take into accound the 'size' field */
			cur[0].num_tags = file->readUint32BE();
			cur[0].compressed_size = file->readUint32BE();
			size -= 4 * 4;
			cur++;
		}
		return file;
	}

	sprintf(buf, "%s.sou", _scumm->_exe_name);
	if (!file->open(buf, _scumm->getGameDataPath())) {
		file->open("monster.sou", _scumm->getGameDataPath());
	}
	return file;
}

void Sound::stopSfxSound() {
	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->stopAll();
	} else {
		_scumm->_mixer->stopAll();
	}
}


bool Sound::isSfxFinished() {
	return !_scumm->_mixer->hasActiveChannel();
}

uint32 Sound::decode12BitsSample(byte * src, byte ** dst, uint32 size, bool stereo = false) {
	uint32 s_size = (size / 3) * 4;
	uint32 loop_size = s_size / 4;
	if (stereo == true) {
		s_size *= 2;
	}
	byte *ptr = *dst = (byte*)malloc(s_size);

	uint32 tmp;
	while(loop_size--) {
		byte v1 = *src++;
		byte v2 = *src++;
		byte v3 = *src++;
		tmp = ((((v2 & 0x0f) << 8) | v1) << 4) - 0x8000;
		*ptr++ = (byte)((tmp >> 8) & 0xff);
		*ptr++ = (byte)(tmp & 0xff);
		if (stereo == true) {
			*ptr++ = (byte)((tmp >> 8) & 0xff);
			*ptr++ = (byte)(tmp & 0xff);
		}
		tmp = ((((v2 & 0xf0) << 4) | v3) << 4) - 0x8000;
		*ptr++ = (byte)((tmp >> 8) & 0xff);
		*ptr++ = (byte)(tmp & 0xff);
		if (stereo == true) {
			*ptr++ = (byte)((tmp >> 8) & 0xff);
			*ptr++ = (byte)(tmp & 0xff);
		}
	}
	return s_size;
}

static void music_handler (void * engine) {
	g_scumm->_sound->bundleMusicHandler(g_scumm);
}

#define OUTPUT_SIZE 66150 // ((22050 * 2 * 2) / 4) * 3

void Sound::playBundleMusic(char * song) {

	if (_scumm->_silentDigitalImuse == true) {
		return;
	}

	if (_nameBundleMusic == NULL) {
		// FIXME: we have MUSDISK1.BUN and MUSDISK2.BUN in COMI.
		if (_scumm->_gameId == GID_CMI) {
			printf("Opening bundle\n");
			if (_scumm->_bundle->openMusicFile("musdisk1.bun", _scumm->getGameDataPath()) == false)
				return;
		} else {
			if (_scumm->_bundle->openMusicFile("digmusic.bun", _scumm->getGameDataPath()) == false)
				return;
		}
		_musicBundleBufFinal = (byte*)malloc(OUTPUT_SIZE);
		_musicBundleBufOutput = (byte*)malloc(10 * 0x2000);
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
		_pauseBundleMusic = false;
		_musicBundleToBeRemoved = false;
		_musicBundleToBeChanged = false;
		_bundleMusicTrack = -1;
		_numberSamplesBundleMusic = _scumm->_bundle->getNumberOfMusicSamplesByName(song);
		_nameBundleMusic = song;
		_scumm->_timer->installProcedure(&music_handler, 1000);
		return;
	}
	if (strcmp(_nameBundleMusic, song) != 0) {
		_newNameBundleMusic = song;
		_musicBundleToBeRemoved = false;
		_musicBundleToBeChanged = true;
	}
}

void Sound::pauseBundleMusic(bool state) {
	_pauseBundleMusic = state;
}

void Sound::stopBundleMusic() {
	_musicBundleToBeRemoved = true;
}

void Sound::bundleMusicHandler(Scumm * scumm) {
	byte * ptr;
	int32 l, num = _numberSamplesBundleMusic, length, k;
	int32 rate = 22050, channels;
	int32 tag, size = -1, header_size = 0;
	
	if (_pauseBundleMusic)
		return;

	if (_musicBundleToBeRemoved == true) {
		_scumm->_timer->releaseProcedure(&music_handler);
		_nameBundleMusic = NULL;
		if (_bundleMusicTrack != -1) {
			_scumm->_mixer->stop(_bundleMusicTrack);
		}
		_bundleMusicTrack = -1;
		if (_musicBundleBufFinal) {
			free(_musicBundleBufFinal);
			_musicBundleBufFinal = NULL;
		}
		if (_musicBundleBufOutput) {
			free(_musicBundleBufOutput);
			_musicBundleBufOutput = NULL;
		}
		return;
	}

	if (_musicBundleToBeChanged == true) {
		_nameBundleMusic = _newNameBundleMusic;
		_numberSamplesBundleMusic = _scumm->_bundle->getNumberOfMusicSamplesByName(_nameBundleMusic);
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
		_musicBundleToBeChanged = false;
	}

	ptr = _musicBundleBufOutput;

	for (k = 0, l = _currentSampleBundleMusic; l < num; k++) {
		length = _scumm->_bundle->decompressMusicSampleByName(_nameBundleMusic, l, (_musicBundleBufOutput + ((k * 0x2000) + _offsetBufBundleMusic)), channels);
		_offsetSampleBundleMusic += length;

		if (l == 0) {
			tag = READ_BE_UINT32(ptr); ptr += 4;
			if (tag != MKID_BE('iMUS')) {
				warning("Decompression of bundle song failed");
				_musicBundleToBeRemoved = true;
				return;
			}

			ptr += 12;
			while(tag != MKID_BE('DATA')) {
				tag = READ_BE_UINT32(ptr);  ptr += 4;
				switch(tag) {
				case MKID_BE('FRMT'):
					size = READ_BE_UINT32(ptr); ptr += 24;
				break;
				case MKID_BE('TEXT'):
				case MKID_BE('REGN'):
				case MKID_BE('STOP'):
				case MKID_BE('JUMP'):
					size = READ_BE_UINT32(ptr); ptr += size + 4;
				break;
					case MKID_BE('DATA'):
					size = READ_BE_UINT32(ptr); ptr += 4;
				break;

				default:
					error("Unknown sound header %c%c%c%c", tag>>24, tag>>16, tag>>8, tag);
				}
			}
			if (size < 0) {
				warning("Decompression sound failed (no size field)");
				_musicBundleToBeRemoved = true;
				return;
			}
			header_size = (ptr - _musicBundleBufOutput);
		}
	
		l++;
		_currentSampleBundleMusic = l;

		if (_offsetSampleBundleMusic >= OUTPUT_SIZE + header_size) {
			memcpy(_musicBundleBufFinal, (_musicBundleBufOutput + header_size), OUTPUT_SIZE);
			_offsetBufBundleMusic = _offsetSampleBundleMusic - OUTPUT_SIZE - header_size;
			memcpy(_musicBundleBufOutput, (_musicBundleBufOutput + (OUTPUT_SIZE + header_size)), _offsetBufBundleMusic);
			_offsetSampleBundleMusic = _offsetBufBundleMusic;
			break;
		}
	}

	if (_currentSampleBundleMusic == num) {
		_currentSampleBundleMusic = 0;
		_offsetSampleBundleMusic = 0;
		_offsetBufBundleMusic = 0;
	}

	size = OUTPUT_SIZE;
	ptr = _musicBundleBufFinal;

	byte * buffer = NULL;
	uint32 final_size = decode12BitsSample(ptr, &buffer, size);

	if (_bundleMusicTrack == -1) {
		_bundleMusicTrack = _scumm->_mixer->playStream(NULL, _scumm->_mixer->_beginSlots - 1, buffer, final_size, rate,
															SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_16BITS | SoundMixer::FLAG_STEREO, -1, 2000000);
	} else {
		_scumm->_mixer->append(_bundleMusicTrack, buffer, final_size, rate,
														SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_16BITS | SoundMixer::FLAG_STEREO);
	}
}

int Sound::playBundleSound(char *sound) {
	byte * ptr;
	bool result;
	
	if (_scumm->_gameId == GID_CMI)
		// FIXME: HACK! There are actually two voice files in COMI... I dunno how to do this
		// right, though :-/
		result = _scumm->_bundle->openVoiceFile("voxdisk1.bun", _scumm->getGameDataPath());
	else if (_scumm->_gameId == GID_DIG)
		result = _scumm->_bundle->openVoiceFile("digvoice.bun", _scumm->getGameDataPath());
	else
		error("Don't know which bundle file to load");

	if (!result) {
		return -1;
	}

	int32 rate = 22050, channels;
	int32 tag, size = -1, bits = 0;

	if (_scumm->_gameId == GID_CMI) {
		char name[20];
		strcpy(name, sound);
		strcat(name, ".IMX");
		ptr = (byte *)malloc(1000000);
		if (_scumm->_bundle->decompressVoiceSampleByName(name, ptr, channels) == 0) {
			delete ptr;
			return -1;
		}
	} else {
		ptr = (byte *)malloc(1000000);
		if (_scumm->_bundle->decompressVoiceSampleByName(sound, ptr, channels) == 0) {
			delete ptr;
			return -1;
		}
	}

	tag = READ_BE_UINT32(ptr); ptr+=4;
	if (tag != MKID_BE('iMUS')) {
		warning("Decompression of bundle sound failed");
		free(ptr);
		return -1;
	}

	ptr += 12;
	while(tag != MKID_BE('DATA')) {
		tag = READ_BE_UINT32(ptr); ptr += 4;
		switch(tag) {
			case MKID_BE('FRMT'):
				ptr += 12;
				bits = READ_BE_UINT32(ptr); ptr += 4;
				rate = READ_BE_UINT32(ptr); ptr += 4;
				channels = READ_BE_UINT32(ptr); ptr += 4;
			break;
			case MKID_BE('TEXT'):
			case MKID_BE('REGN'):
			case MKID_BE('STOP'):
			case MKID_BE('JUMP'):
			case MKID_BE('SYNC'):
				size = READ_BE_UINT32(ptr); ptr += size + 4;
			break;

			case MKID_BE('DATA'):
				size = READ_BE_UINT32(ptr); ptr += 4;
			break;

			default:
			error("Unknown sound header %c%c%c%c", tag>>24, tag>>16, tag>>8, tag);
		}
	}

	if (size < 0) {
		warning("Decompression sound failed (no size field)");
		free(ptr);
		return -1;
	}
	
	byte * final = (byte *)malloc(size);
	memcpy(final, ptr, size);
	if (bits == 8) {
		return _scumm->_mixer->playRaw(NULL, final, size, rate, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
	} else if (bits == 16){
		return _scumm->_mixer->playRaw(NULL, final, size, rate, SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE);
	} else {
		warning("Sound::playBundleSound() to do more options to playRaw...");
		return -1;
	}
}

int Sound::playSfxSound(void *sound, uint32 size, uint rate, bool isUnsigned) {
	if (_soundsPaused)
		return -1;
	byte flags = SoundMixer::FLAG_AUTOFREE;
	if (isUnsigned)
		flags |= SoundMixer::FLAG_UNSIGNED;
	return _scumm->_mixer->playRaw(NULL, sound, size, rate, flags);
}

int Sound::playSfxSound_MP3(void *sound, uint32 size) {
#ifdef USE_MAD
	if (_soundsPaused)
		return -1;
	return _scumm->_mixer->playMP3(NULL, sound, size, SoundMixer::FLAG_AUTOFREE);
#endif
	return -1;
}

#ifdef USE_VORBIS
// Provide a virtual file to vorbisfile based on preloaded data
struct data_file_info {
	char *data;
	uint32 size;
	int curr_pos;
};

static size_t read_data(void *ptr, size_t size, size_t nmemb, void *datasource) {
	data_file_info *f = (data_file_info *) datasource;

	nmemb *= size;
	if (f->curr_pos < 0)
		return (size_t) -1;
	if (f->curr_pos > (int) f->size)
		nmemb = 0;
	else if (f->curr_pos + nmemb > f->size)
		nmemb = f->size - f->curr_pos;

	memcpy(ptr, f->data + f->curr_pos, nmemb);
	f->curr_pos += nmemb;
	return nmemb / size;
}

static int seek_data(void *datasource, ogg_int64_t offset, int whence) {
	data_file_info *f = (data_file_info *) datasource;

	switch (whence) {
	case SEEK_SET:
		f->curr_pos = offset;
		break;
	case SEEK_CUR:
		f->curr_pos += offset;
		break;
	case SEEK_END:
		f->curr_pos = f->size + offset;
		break;
	default:
		return -1;
	}
	return f->curr_pos;
}

static int close_data(void *datasource) {
	data_file_info *f = (data_file_info *) datasource;

	free(f->data);
	delete f;
	return 0;
}

static long tell_data(void *datasource) {
	data_file_info *f = (data_file_info *) datasource;

	return f->curr_pos;
}

static ov_callbacks data_wrap = {
	read_data, seek_data, close_data, tell_data
};
#endif

int Sound::playSfxSound_Vorbis(void *sound, uint32 size) {
#ifdef USE_VORBIS
	if (_soundsPaused)
		return -1;

	OggVorbis_File *ov_file = new OggVorbis_File;
	data_file_info *f = new data_file_info;
	f->data = (char *) sound;
	f->size = size;
	f->curr_pos = 0;

	if (ov_open_callbacks((void *) f, ov_file, NULL, 0, data_wrap) < 0) {
		warning("Invalid file format");
		delete ov_file;
		delete f;
		return -1;
	}

	return _scumm->_mixer->playVorbis(NULL, ov_file, 0, false);
#endif
	return -1;
}

// We use a real timer in an attempt to get better sync with CD tracks. This is
// necessary for games like Loom CD.

static void cd_timer_handler(void *ptr)
{
	Scumm *scumm = (Scumm *) ptr;

	// Maybe I could simply update _vars[VAR_MI1_TIMER] directly here, but
	// I don't feel comfortable just doing that from what might be a
	// separate thread. If someone tells me it's safe, I'll make the
	// change right away.
	
	// FIXME: Turn off the timer when it's no longer needed. In theory, it
	// should be possible to check with pollCD(), but since CD sound isn't
	// properly restarted when reloading a saved game, I don't dare to.

	scumm->_sound->_cd_timer_value += 6;
}

int Sound::readCDTimer()
{
	return _cd_timer_value;
}

void Sound::startCDTimer()
{
	int timer_interval;

	// The timer interval has been tuned for Loom CD and the Monkey 1
	// intro. I have to use 100 for Loom, or there will be a nasty stutter
	// when Chaos first appears, and I have to use 101 for Monkey 1 or the
	// intro music will be cut short.

	if (_scumm->_gameId == GID_LOOM256)
		timer_interval = 100;
	else 
		timer_interval = 101;

	_scumm->_timer->releaseProcedure(&cd_timer_handler);
	_cd_timer_value = _scumm->_vars[_scumm->VAR_MI1_TIMER];
	_scumm->_timer->installProcedure(&cd_timer_handler, timer_interval);
}

void Sound::stopCDTimer()
{
	_scumm->_timer->releaseProcedure(&cd_timer_handler);
}

void Sound::playCDTrack(int track, int num_loops, int start, int delay)
{
	if (playMP3CDTrack(track, num_loops, start, delay) == -1)
		_scumm->_system->play_cdrom(track, num_loops, start, delay);

	// Start the timer after starting the track. Starting an MP3 track is
	// almost instantaneous, but a CD player may take some time. Hopefully
	// play_cdrom() will block during that delay.

	startCDTimer();
}

void Sound::stopCD()
{
	stopCDTimer();
	if (stopMP3CD() == -1)
		_scumm->_system->stop_cdrom();
}

int Sound::pollCD()
{
	if (pollMP3CD())
		return 1;

	return _scumm->_system->poll_cdrom();
}

void Sound::updateCD()
{
	if (updateMP3CD() == -1)
		_scumm->_system->update_cdrom();
}

int Sound::getCachedTrack(int track) {
	int i;
#if defined(USE_MAD) || defined(USE_VORBIS)
	char track_name[1024];
	File *file = new File();
#endif
	int current_index;

	// See if we find the track in the cache
	for (i = 0; i < CACHE_TRACKS; i++)
		if (_cached_tracks[i] == track) {
			if (_track_info[i])
				return i;
			else
				return -1;
		}
	current_index = _current_cache++;
	_current_cache %= CACHE_TRACKS;

	// Not found, see if it exists

	// First, delete the previous track info object
	delete _track_info[current_index];
	_track_info[current_index] = NULL;

	_cached_tracks[current_index] = track;

#ifdef USE_MAD
	sprintf(track_name, "track%d.mp3", track);
	file->open(track_name, _scumm->getGameDataPath());

	if (file->isOpen()) {
		_track_info[current_index] = new MP3TrackInfo(file);
		if (_track_info[current_index]->error()) {
			delete _track_info[current_index];
			_track_info[current_index] = NULL;
			return -1;
		}
		return current_index;
	}
#endif

#ifdef USE_VORBIS
	sprintf(track_name, "track%d.ogg", track);
	file->open(track_name, _scumm->getGameDataPath());

	if (file->isOpen()) {
		_track_info[current_index] = new VorbisTrackInfo(file);
		if (_track_info[current_index]->error()) {
			delete _track_info[current_index];
			_track_info[current_index] = NULL;
			return -1;
		}
		return current_index;
	}
#endif

	debug(2, "Track %d not available in compressed format", track);
	return -1;
}

int Sound::playMP3CDTrack(int track, int num_loops, int start, int delay) {
	int index;
	_scumm->_vars[_scumm->VAR_MI1_TIMER] = 0;

	if (_soundsPaused)
		return 0;

	if ((num_loops == 0) && (start == 0)) {
		return 0;
	}

	index = getCachedTrack(track);
	if (index < 0)
		return -1;

	if (_dig_cd_playing)
		_scumm->_mixer->stop(_dig_cd_index);
	_dig_cd_index = _track_info[index]->play(_scumm->_mixer, start, delay);
	_dig_cd_playing = true;
	_dig_cd_track = track;
	_dig_cd_num_loops = num_loops;
	_dig_cd_start = start;
	_dig_cd_delay = delay;
	return 0;
}

int Sound::stopMP3CD() {
	if (_dig_cd_playing == true) {
		_scumm->_mixer->stop(_dig_cd_index);
		_dig_cd_playing = false;
		_dig_cd_track = 0;
		_dig_cd_num_loops = 0;
		_dig_cd_start = 0;
		_dig_cd_delay = 0;
		return 0;
	}
	return -1;
}

int Sound::pollMP3CD() {
	if (_dig_cd_playing == true)
		return 1;
	return 0;
}

int Sound::updateMP3CD() {
	if (_dig_cd_playing == false)
		return -1;

	if (_scumm->_mixer->_channels[_dig_cd_index] == NULL) {
		warning("Error in MP3 decoding");
		return -1;
	}

	if (_scumm->_mixer->_channels[_dig_cd_index]->soundFinished()) {
		if (_dig_cd_num_loops == -1 || --_dig_cd_num_loops > 0)
			playMP3CDTrack(_dig_cd_track, _dig_cd_num_loops, _dig_cd_start, _dig_cd_delay);
		else
			stopMP3CD();
	}

	return 0;
}

#ifdef USE_MAD
Sound::MP3TrackInfo::MP3TrackInfo(File *file) {
	struct mad_stream stream;
	struct mad_frame frame;
	unsigned char buffer[8192];
	unsigned int buflen = 0;
	int count = 0;

	// Check the format and bitrate
	mad_stream_init(&stream);
	mad_frame_init(&frame);

	while (1) {
		if (buflen < sizeof(buffer)) {
			int bytes;

			bytes = file->read(buffer + buflen, sizeof(buffer) - buflen);
			if (bytes <= 0) {
				if (bytes == -1) {
					warning("Invalid file format");
					goto error;
				}
				break;
			}

			buflen += bytes;
		}

		mad_stream_buffer(&stream, buffer, buflen);

		while (1) {
			if (mad_frame_decode(&frame, &stream) == -1) {
				if (!MAD_RECOVERABLE(stream.error))
					break;

				if (stream.error != MAD_ERROR_BADCRC)
					continue;
			}

			if (count++)
				break;
		}

		if (count || stream.error != MAD_ERROR_BUFLEN)
			break;

		memmove(buffer, stream.next_frame,
		        buflen = &buffer[buflen] - stream.next_frame);
	}

	if (count)
		memcpy(&_mad_header, &frame.header, sizeof(mad_header));
	else {
		warning("Invalid file format");
		goto error;
	}

	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	// Get file size
	_size = file->size();
	_file = file;
	_error_flag = false;
	return;

 error:
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	_error_flag = true;
	delete file;
}

int Sound::MP3TrackInfo::play(SoundMixer *mixer, int start, int delay) {
	unsigned int offset;
	mad_timer_t duration;

	// Calc offset. As all bitrates are in kilobit per seconds, the division by 200 is always exact
	offset = (start * (_mad_header.bitrate / (8 * 25))) / 3;

	// Calc delay
	if (!delay) {
		mad_timer_set(&duration, (_size * 8) / _mad_header.bitrate,
		              (_size * 8) % _mad_header.bitrate, _mad_header.bitrate);
	} else {
		mad_timer_set(&duration, delay / 75, delay % 75, 75);
	}	

	// Go
	_file->seek(offset, SEEK_SET);

	return mixer->playMP3CDTrack(NULL, _file, duration);
}

Sound::MP3TrackInfo::~MP3TrackInfo() {
	if (! _error_flag)
		_file->close();
}

#endif

#ifdef USE_VORBIS
// These are wrapper functions to allow using a File object to
// provide data to the OggVorbis_File object.

struct file_info {
	File *file;
	int start, curr_pos;
	size_t len;
};

static size_t read_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	file_info *f = (file_info *) datasource;
	int result;

	nmemb *= size;
	if (f->curr_pos > (int) f->len)
		nmemb = 0;
	else if (nmemb > f->len - f->curr_pos)
		nmemb = f->len - f->curr_pos;
	result = f->file->read(ptr, nmemb);
	if (result == -1) {
		f->curr_pos = f->file->pos() - f->start;
		return (size_t) -1;
	}
	else {
		f->curr_pos += result;
		return result / size;
	}
}

static int seek_wrap(void *datasource, ogg_int64_t offset, int whence) {
	file_info *f = (file_info *) datasource;

	if (whence == SEEK_SET)
		offset += f->start;
	else if (whence == SEEK_END) {
		offset += f->start + f->len;
		whence = SEEK_SET;
	}

	f->file->seek(offset, whence);
	f->curr_pos = f->file->pos() - f->start;
	return f->curr_pos;
}

static int close_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	f->file->close();
	delete f;
	return 0;
}

static long tell_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	return f->file->pos();
}

static ov_callbacks File_wrap = {
	read_wrap, seek_wrap, close_wrap, tell_wrap
};

Sound::VorbisTrackInfo::VorbisTrackInfo(File *file) {
	file_info *f = new file_info;

	f->file = file;
	f->start = 0;
	f->len = file->size();
	f->curr_pos = file->pos();

	if (ov_open_callbacks((void *) f, &_ov_file, NULL, 0, File_wrap) < 0) {
		warning("Invalid file format");
		_error_flag = true;
		delete f;
		delete file;
	}
	else {
		_error_flag = false;
		_file = file;

		// Check the file format
		if (ov_info(&_ov_file, -1)->rate != 22050)
			warning("Vorbis code currently only supports files encoded at 22050 Hz");
	}
}

int Sound::VorbisTrackInfo::play(SoundMixer *mixer, int start, int delay) {
	// fixme tremor handles delay differently
	ov_time_seek(&_ov_file, start / 75.0);
	return mixer->playVorbis(NULL, &_ov_file,
				 delay * ov_info(&_ov_file, -1)->rate / 75,
				 true);
}

Sound::VorbisTrackInfo::~VorbisTrackInfo() {
	if (! _error_flag) {
		ov_clear(&_ov_file);
		delete _file;
	}
}

#endif
