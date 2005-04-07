/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
#include "queen/sound.h"

#include "queen/input.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"

#include "sound/flac.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"

#define	SB_HEADER_SIZE	110
#define	STOP_MUSIC	-1

namespace Queen {

Sound::Sound(SoundMixer *mixer, QueenEngine *vm) : 
	_mixer(mixer), _vm(vm), _sfxToggle(true), _speechToggle(true), _musicToggle(true), _lastOverride(0) {
}

Sound::~Sound() {
}

Sound *Sound::giveSound(SoundMixer *mixer, QueenEngine *vm, uint8 compression) {
	if (!mixer->isReady())
		return new SilentSound(mixer, vm);
	
	switch (compression) {
	case COMPRESSION_NONE:
		return new SBSound(mixer, vm);
		break;
	case COMPRESSION_MP3:
#ifndef USE_MAD
		warning("Using MP3 compressed datafile, but MP3 support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new MP3Sound(mixer, vm);
#endif
		break;
	case COMPRESSION_OGG:
#ifndef USE_VORBIS
		warning("Using OGG compressed datafile, but OGG support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new OGGSound(mixer, vm);
#endif
		break;
	case COMPRESSION_FLAC:
#ifndef USE_FLAC
		warning("Using FLAC compressed datafile, but FLAC support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new FLACSound(mixer, vm);
#endif
		break;
	default:
		warning("Unknown compression type");
		return new SilentSound(mixer, vm);
	}
}

void Sound::waitFinished(bool isSpeech) {
	if (isSpeech)
		while (_mixer->isSoundHandleActive(_speechHandle))
			_vm->input()->delay(10);
	else
		while (_mixer->isSoundHandleActive(_sfxHandle))
			_vm->input()->delay(10);
}

void Sound::playSfx(uint16 sfx, bool isSpeech) {
	if (isSpeech && !speechOn()) return;
	else if (!sfxOn()) return;

	if (sfx != 0) {
		char name[13];
#ifndef __PALM_OS__
		strcpy(name, _sfxName[sfx - 1]);
#else
		strncpy(name, _sfxName + 10 * (sfx - 1), 10);	// saved as 8char + /0/0
#endif
		strcat(name, ".SB");
		waitFinished(isSpeech);
		if (_vm->resource()->fileExists(name)) {
			sfxPlay(name, isSpeech);
			_speechSfxExists = isSpeech;
		} else {
			_speechSfxExists = false;
		}
	}
}

void Sound::playSfx(const char *base, bool isSpeech) {
	if (isSpeech && !speechOn()) return;
	else if (!sfxOn()) return;
	
	char name[13];
	strcpy(name, base);
	// alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");
	waitFinished(isSpeech);
	if (_vm->resource()->fileExists(name)) {
		sfxPlay(name, isSpeech);
		_speechSfxExists = isSpeech;
	} else {
		_speechSfxExists = false;
	}
}

void Sound::playSong(int16 songNum) {
	if (songNum <= 0) {
		_vm->music()->stopSong();
		return;
	}
	
	int16 newTune;
	if (_vm->resource()->isDemo()) {
		if (songNum == 17) {
			_vm->music()->stopSong();
			return;
		}
		newTune = _songDemo[songNum - 1].tuneList[0] - 1;
	} else {
		newTune = _song[songNum - 1].tuneList[0] - 1;
	}

	if (_tune[newTune].sfx[0]) {
		if (sfxOn())
			playSfx(_tune[newTune].sfx[0], false);
		return;
	}

	if (!musicOn())
		return;

	int override = (_vm->resource()->isDemo()) ? _songDemo[songNum - 1].override : _song[songNum - 1].override;
	switch (override) {
	// Override all songs
	case  1:
		break;
	// Alter song settings (such as volume) and exit
	case  2:
		_vm->music()->toggleVChange();
	default:
		return;
		break;
	}
	
	_lastOverride = songNum;
	
	_vm->music()->queueTuneList(newTune);
	_vm->music()->playMusic();
}

void Sound::saveState(byte *&ptr) {
	WRITE_BE_UINT16(ptr, _lastOverride); ptr += 2;
}

void Sound::loadState(uint32 ver, byte *&ptr) {
	_lastOverride = (int16)READ_BE_INT16(ptr); ptr += 2;
}

void SBSound::playSound(byte *sound, uint32 size, bool isSpeech) {
	byte flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	_mixer->playRaw(isSpeech ? &_speechHandle : &_sfxHandle, sound, size, 11025, flags);
}

void SBSound::sfxPlay(const char *name, bool isSpeech) {
	uint32 size;
	uint8 *buf = _vm->resource()->loadFile(name, SB_HEADER_SIZE, &size, true);
	playSound(buf, size, isSpeech);
}

#ifdef USE_MAD
void MP3Sound::sfxPlay(const char *name, bool isSpeech) {
	uint32 size;
	File *f = _vm->resource()->giveCompressedSound(name, &size);
	_mixer->playInputStream(SoundMixer::kSFXSoundType, isSpeech ? &_speechHandle : &_sfxHandle, makeMP3Stream(f, size));
}
#endif

#ifdef USE_VORBIS
void OGGSound::sfxPlay(const char *name, bool isSpeech) {
	uint32 size;
	File *f = _vm->resource()->giveCompressedSound(name, &size);		
	_mixer->playInputStream(SoundMixer::kSFXSoundType, isSpeech ? &_speechHandle : &_sfxHandle, makeVorbisStream(f, size));
}
#endif

#ifdef USE_FLAC
void FLACSound::sfxPlay(const char *name, bool isSpeech) {
	uint32 size;
	File *f = _vm->resource()->giveCompressedSound(name, &size);		
	_mixer->playInputStream(SoundMixer::kSFXSoundType, isSpeech ? &_speechHandle : &_sfxHandle, makeFlacStream(f, size));
}
#endif

} //End of namespace Queen
