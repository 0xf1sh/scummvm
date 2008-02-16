/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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


#include "common/endian.h"

#include "gob/gob.h"
#include "gob/sound.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"

namespace Gob {

void SoundDesc::set(SoundType type, SoundSource src,
		byte *data, uint32 dSize) {

	free();

	_type = type;
	_source = src;
	_data = _dataPtr = data;
	_size = dSize;
}

void SoundDesc::load(SoundType type, SoundSource src,
		byte *data, uint32 dSize) {

	free();

	_source = src;
	switch (type) {
	case SOUND_ADL:
		loadADL(data, dSize);
		break;
	case SOUND_SND:
		loadSND(data, dSize);
		break;
	}
}

void SoundDesc::free() {
	if (_source != SOUND_TOT)
		delete[] _data;
	_data = _dataPtr = 0;
	_id = 0;
}

void SoundDesc::convToSigned() {
	if ((_type == SOUND_SND) && _data && _dataPtr)
		for (uint32 i = 0; i < _size; i++)
			_dataPtr[i] ^= 0x80;
}

void SoundDesc::loadSND(byte *data, uint32 dSize) {
	assert(dSize > 6);

	_type = SOUND_SND;
	_data = data;
	_dataPtr = data + 6;
	_frequency = MAX((int16) READ_BE_UINT16(data + 4), (int16) 4700);
	_flag = data[0] ? (data[0] & 0x7F) : 8;
	data[0] = 0;
	_size = MIN(READ_BE_UINT32(data), dSize - 6);
}

void SoundDesc::loadADL(byte *data, uint32 dSize) {
	_type = SOUND_ADL;
	_data = _dataPtr = data;
	_size = dSize;
}

Snd::Snd(GobEngine *vm) : _vm(vm) {
	_playingSound = 0;
	_curSoundDesc = 0;

	_rate = _vm->_mixer->getOutputRate();
	_end = true;
	_data = 0;
	_length = 0;
	_freq = 0;
	_repCount = 0;

	_offset = 0;
	_offsetFrac = 0;
	_offsetInc = 0;

	_cur = 0;
	_last = 0;

	_fade = false;
	_fadeVol = 65536;
	_fadeVolStep = 0;
	_fadeSamples = 0;
	_curFadeSamples = 0;

	_compositionSamples = 0;
	_compositionSampleCount = 0;
	_compositionPos = -1;

	_speakerStream = new Audio::PCSpeaker(_vm->_mixer->getOutputRate());

	_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_handle,
			this, -1, 255, 0, false, true);
	_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
			_speakerStream, -1, 50, 0, false, true);
}

Snd::~Snd() {
	// stop permanent streams manually:

	// First the speaker stream
	_vm->_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;

	// Next, this stream (class Snd is an AudioStream, too)
	_vm->_mixer->stopHandle(_handle);
}

void Snd::speakerOn(int16 frequency, int32 length) {
	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, frequency, length);
}

void Snd::speakerOff() {
	_speakerStream->stop();
}

void Snd::speakerOnUpdate(uint32 milis) {
	if (_speakerStream->isPlaying())
		_speakerStream->stop(milis);
}

void Snd::stopSound(int16 fadeLength, SoundDesc *sndDesc) {
	Common::StackLock slock(_mutex);

	if (sndDesc && (sndDesc != _curSoundDesc))
		return;

	if (fadeLength <= 0) {
		_data = 0;
		_end = true;
		_playingSound = 0;
		_curSoundDesc = 0;
		return;
	}

	_fade = true;
	_fadeVol = 65536;
	_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
	_fadeVolStep = MAX((int32) 1, (int32) (65536 / _fadeSamples));
	_curFadeSamples = 0;
}

void Snd::setRepeating(int32 repCount) {
	Common::StackLock slock(_mutex);

	_repCount = repCount;
}

void Snd::waitEndPlay(bool interruptible, bool stopComp) {
	if (stopComp)
		_compositionPos = -1;
	while (!_end && !_vm->_quitRequested) {
		if (interruptible && (_vm->_util->checkKey() == 0x11B)) {
			WRITE_VAR(57, -1);
			return;
		}
		_vm->_util->longDelay(200);
	}
	stopSound(0);
}

void Snd::stopComposition() {
	if (_compositionPos != -1) {
		stopSound(0);
		_compositionPos = -1;
	}
}

void Snd::nextCompositionPos() {
	int8 slot;

	while ((++_compositionPos < 50) &&
			((slot = _composition[_compositionPos]) != -1)) {
		if ((slot >= 0) && (slot < _compositionSampleCount)) {
			SoundDesc &sample = _compositionSamples[slot];
			if (!sample.empty() && (sample.getType() == SOUND_SND)) {
				setSample(sample, 1, 0, 0);
				return;
			}
		}
		if (_compositionPos == 49)
			_compositionPos = -1;
	}
	_compositionPos = -1;
}

void Snd::playComposition(int16 *composition, int16 freqVal,
		SoundDesc *sndDescs, int8 sndCount) {
	int i;

	waitEndPlay();
	stopComposition();

	_compositionSamples = sndDescs ? sndDescs : _vm->_game->_soundSamples;
	_compositionSampleCount = sndCount;

	i = -1;
	do {
		i++;
		_composition[i] = composition[i];
	} while ((i < 50) && (composition[i] != -1));

	_compositionPos = -1;
	nextCompositionPos();
}

void Snd::setSample(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {

	if (frequency <= 0)
		frequency = sndDesc._frequency;

	_curSoundDesc = &sndDesc;
	sndDesc._repCount = repCount - 1;
	sndDesc._frequency = frequency;

	_data = (int8 *) sndDesc.getData();
	_length = sndDesc.size();
	_freq = frequency;

	_repCount = repCount;
	_end = false;
	_playingSound = 1;

	_offset = 0;
	_offsetFrac = 0;
	_offsetInc = (_freq << FRAC_BITS) / _rate;

	_last = _cur;
	_cur = _data[0];

	_curFadeSamples = 0;
	if (fadeLength == 0) {
		_fade = false;
		_fadeVol = 65536;
		_fadeSamples = 0;
		_fadeVolStep = 0;
	} else {
		_fade = true;
		_fadeVol = 0;
		_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
		_fadeVolStep = - MAX((int32) 1, (int32) (65536 / _fadeSamples));
	}
}

bool Snd::loadSample(SoundDesc &sndDesc, const char *fileName) {
	byte *data;
	uint32 size;

	data = (byte *) _vm->_dataIO->getData(fileName);
	if (!data)
		return false;

	size = _vm->_dataIO->getDataSize(fileName);
	sndDesc.load(SOUND_SND, SOUND_FILE, data, size);

	return true;
}

void Snd::freeSample(SoundDesc &sndDesc) {
	stopSound(0, &sndDesc);
	sndDesc.free();
}

void Snd::playSample(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {
	Common::StackLock slock(_mutex);

	if (!_end)
		return;

	setSample(sndDesc, repCount, frequency, fadeLength);
}

void Snd::checkEndSample() {
	if (_compositionPos != -1)
		nextCompositionPos();
	else if ((_repCount == -1) || (--_repCount > 0)) {
		_offset = 0;
		_offsetFrac = 0;
		_end = false;
		_playingSound = 1;
	} else {
		_end = true;
		_playingSound = 0;
	}
}

int Snd::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);

	for (int i = 0; i < numSamples; i++) {
		if (!_data)
			return i;
		if (_end || (_offset >= _length))
			checkEndSample();
		if (_end)
			return i;

		// Linear interpolation. See sound/rate.cpp

		int16 val = (_last + (((_cur - _last) * _offsetFrac +
					FRAC_HALF) >> FRAC_BITS)) << 8;
		*buffer++ = (val * _fadeVol) >> 16;

		_offsetFrac += _offsetInc;

		// Was there an integral change?
		if (fracToInt(_offsetFrac) > 0) {
			_last = _cur;
			_cur = _data[_offset];
			_offset += fracToInt(_offsetFrac);
			_offsetFrac &= FRAC_LO_MASK;
		}

		if (_fade) {

			if (++_curFadeSamples >= _fadeSamples) {
				if (_fadeVolStep > 0) {
					_data = 0;
					_end = true;
					_playingSound = 0;
					_compositionPos = -1;
					_curSoundDesc = 0;
				} else {
					_fadeVol = 65536;
					_fade = false;
				}
			} else
				_fadeVol -= _fadeVolStep;

			if (_fadeVol < 0)
				_fadeVol = 0;

		}
	}
	return numSamples;
}

} // End of namespace Gob
