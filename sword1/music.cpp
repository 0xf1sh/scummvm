/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "music.h"
#include "sound/mixer.h"
#include "common/util.h"
#include "common/file.h"

namespace Sword1 {

// This means fading takes 3 seconds.
#define FADE_LENGTH 3

// These functions are only called from Music, so I'm just going to
// assume that if locking is needed it has already been taken care of.

void MusicHandle::fadeDown() {
	if (streaming()) {
		if (_fading < 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = FADE_LENGTH * getRate();
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

void MusicHandle::fadeUp() {
	if (streaming()) {
		if (_fading > 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = -1;
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

bool MusicHandle::endOfData() const {
	return !streaming();
}

int MusicHandle::readBuffer(int16 *buffer, const int numSamples) {
	int samples;
	for (samples = 0; samples < numSamples && !endOfData(); samples++) {
		int16 sample = _file.readUint16LE();
		if (_file.ioFailed()) {
			if (!_looping) {
				stop();
				sample = 0;
			} else {
				_file.clearIOFailed();
				_file.seek(WAVEHEADERSIZE);
				sample = _file.readUint16LE();
			}
		}
		if (_fading > 0) {
			if (--_fading == 0) {
				_looping = false;
				_file.close();
			}
			sample = (sample * _fading) / _fadeSamples;
		} else if (_fading < 0) {
			_fading--;
			sample = -(sample * _fading) / _fadeSamples;
			if (_fading <= -_fadeSamples)
				_fading = 0;
		}
		*buffer++ = sample;
	}
	return samples;
}

bool MusicHandle::play(const char *filename, bool loop) {
	uint8 wavHeader[WAVEHEADERSIZE];
	stop();
	if (!_file.open(filename)) {
		warning("Music file %s could not be opened", filename);
		return false;
	}
	_file.read(wavHeader, WAVEHEADERSIZE);
	_stereo = (READ_LE_UINT16(wavHeader + 0x16) == 2);
	_rate = READ_LE_UINT16(wavHeader + 0x18);
	_looping = loop;
	fadeUp();
	return true;
}

void MusicHandle::stop() {
	if (_file.isOpen())
		_file.close();
	_fading = 0;
	_looping = false;
}

Music::Music(OSystem *system, SoundMixer *pMixer) {
	_system = system;
	_mixer = pMixer;
	_mixer->setupPremix(passMixerFunc, this);
	_mutex = _system->createMutex();
	_converter[0] = NULL;
	_converter[1] = NULL;
	_volumeL = _volumeR = 192;
}

Music::~Music() {
	_mixer->setupPremix(0, 0);
	delete _converter[0];
	delete _converter[1];
	if (_mutex)
		_system->deleteMutex(_mutex);
}

void Music::passMixerFunc(void *param, int16 *buf, uint len) {
	((Music*)param)->mixer(buf, len);
}

void Music::mixer(int16 *buf, uint32 len) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming() && _converter[i])
			_converter[i]->flow(_handles[i], buf, len, _volumeL, _volumeR);
}

void Music::setVolume(uint8 volL, uint8 volR) {
	_volumeL = (st_volume_t)volL;
	_volumeR = (st_volume_t)volR;
}

void Music::giveVolume(uint8 *volL, uint8 *volR) {
	*volL = (uint8)_volumeL;
	*volR = (uint8)_volumeR;
}

void Music::startMusic(int32 tuneId, int32 loopFlag) {
	Common::StackLock lock(_mutex);
	if (strlen(_tuneList[tuneId]) > 0) {
		int newStream = 0;
		if (_handles[0].streaming() && _handles[1].streaming()) {
			int streamToStop;
			// Both streams playing - one must be forced to stop.
			if (!_handles[0].fading() && !_handles[1].fading()) {
				// None of them are fading. Shouldn't happen,
				// so it doesn't matter which one we pick.
				streamToStop = 0;
			} else if (_handles[0].fading() && !_handles[1].fading()) {
				// Stream 0 is fading, so pick that one.
				streamToStop = 0;
			} else if (!_handles[0].fading() && _handles[1].fading()) {
				// Stream 1 is fading, so pick that one.
				streamToStop = 1;
			} else {
				// Both streams are fading. Pick the one that
				// is closest to silent.
				if (ABS(_handles[0].fading()) < ABS(_handles[1].fading()))
					streamToStop = 0;
				else
					streamToStop = 1;
			}
			_handles[streamToStop].stop();
		}
		if (_handles[0].streaming()) {
			_handles[0].fadeDown();
			newStream = 1;
		} else if (_handles[1].streaming()) {
			_handles[1].fadeDown();
			newStream = 0;
		}
		char fName[20];
		sprintf(fName, "%s.wav", _tuneList[tuneId]);
		if (_handles[newStream].play(fName, loopFlag != 0)) {
			delete _converter[newStream];
			_converter[newStream] = makeRateConverter(_handles[newStream].getRate(), _mixer->getOutputRate(), _handles[newStream].isStereo(), false);
		}
	} else {
		if (_handles[0].streaming())
			_handles[0].fadeDown();
		if (_handles[1].streaming())
			_handles[1].fadeDown();
	}
}

void Music::fadeDown() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming())
			_handles[i].fadeDown();
}

} // End of namespace Sword1
