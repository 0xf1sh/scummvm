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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "mads/sound.h"
#include "mads/sound_nebular.h"

namespace MADS {

namespace Nebular {

AdlibChannel::AdlibChannel() {
	_activeCount = 0;
	_field1 = 0;
	_field2 = 0;
	_field3 = 0;
	_field4 = 0;
	_sampleIndex = 0;
	_volume = 0;
	_field7 = 0;
	_field8 = 0;
	_field9 = 0;
	_fieldA = 0;
	_fieldB = 0;
	_fieldC = 0;
	_fieldD = 0;
	_fieldE = 0;
	_ptr1 = nullptr;
	_pSrc = nullptr;
	_ptr3 = nullptr;
	_ptr4 = nullptr;
	_field17 = 0;
	_field19 = 0;
	_soundData = nullptr;
	_field1D = 0;
	_field1E = 0;
	_field1F = 0;
}

void AdlibChannel::reset() {
	_activeCount = 0;
	_field1 = 0;
	_field2 = 0;
	_field3 = 0;
}

void AdlibChannel::enable(int flag) {
	if (_activeCount) {
		_fieldE = flag;

		// WORKAROUND: Original set _soundData pointer to flag. Since this seems
		// just intended to invalidate any prior pointer, I've replaced it with
		// a simple null pointer
		_soundData = nullptr; 
	}
}

void AdlibChannel::setPtr2(byte *pData) {
	_pSrc = pData;
	_field2 = 0xFF;
	_fieldA = 1;
	_field9 = 1;
}

void AdlibChannel::load(byte *pData) {
	_ptr1 = _pSrc = _ptr3 = pData;
	_ptr4 = _soundData = pData;
	_fieldA = 0xFF;
	_activeCount = 1;
	_fieldD = 64;
	_field1 = 0;
	_field1F = 0;
	_field2 = _field3 = 0;
	_volume = _field7 = 0;
	_field1D = _field1E = 0;
	_fieldE = 0;
	_field9 = 0;
	_fieldB = 0;
	_field17 = 0;
	_field19 = 0;
}

void AdlibChannel::check(byte *nullPtr) {
	if (_activeCount && _fieldE) {
		if (!_field1E) {
			_pSrc = nullPtr;
			_fieldE = 0;
		} else {
			_field2 = 0xFF;
			_fieldA = 4;
			if (!_field9)
				_field9 = 1;
		}
	}
}

/*-----------------------------------------------------------------------*/

AdlibSample::AdlibSample(Common::SeekableReadStream &s) {
	_attackRate = s.readByte();
	_decayRate = s.readByte();
	_sustainLevel = s.readByte();
	_releaseRate = s.readByte();
	_egTyp = s.readByte() != 0;
	_ksr = s.readByte() != 0;
	_totalLevel = s.readByte();
	_scalingLevel = s.readByte();
	_waveformSelect = s.readByte();
	_freqMultiple = s.readByte();
	_feedback = s.readByte();
	_ampMod = s.readByte() != 0;
	_vib = s.readByte();
	_alg = s.readByte();
	_fieldE = s.readByte();
	s.skip(1);
	_freqMask = s.readUint16LE();
	_freqBase = s.readUint16LE();
	_field14 = s.readUint16LE();
}

/*-----------------------------------------------------------------------*/

ASound::ASound(Audio::Mixer *mixer, const Common::String &filename, int dataOffset) {
	// Open up the appropriate sound file
	if (!_soundFile.open(filename))
		error("Could not open file - %s", filename.c_str());

	// Initialise fields
	_activeChannelPtr = nullptr;
	_samplePtr = nullptr;
	_frameCounter = 0;
	_isDisabled = false;
	_v1 = 0;
	_v2 = 0;
	_activeChannelNumber = 0;
	_freqMask1 = _freqMask2 = 0;
	_freqBase1 = _freqBase2 = 0;
	_channelNum1 = _channelNum2 = 0;
	_v7 = 0;
	_v8 = 0;
	_v9 = 0;
	_v10 = 0;
	_pollResult = 0;
	_resultFlag = 0;
	_nullData[0] = _nullData[1] = 0;
	Common::fill(&_ports[0], &_ports[256], 0);
	_stateFlag = false;
	_activeChannelReg = 0;
	_v11 = 0;
	_randomSeed = 1234;
	_amDep = _vibDep = _splitPoint = true;

	_samplesTillCallback = 0;
	_samplesTillCallbackRemainder = 0;
	_samplesPerCallback = getRate() / CALLBACKS_PER_SECOND;
	_samplesPerCallbackRemainder = getRate() % CALLBACKS_PER_SECOND;

	// Store passed parameters, and setup OPL
	_dataOffset = dataOffset;
	_mixer = mixer;
	_opl = OPL::Config::create();
	assert(_opl);

	_opl->init(getRate());
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, 
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	// Initialise the Adlib
	adlibInit();

	// Reset the adlib
	command0();
}

ASound::~ASound() {
	Common::List<CachedDataEntry>::iterator i;
	for (i = _dataCache.begin(); i != _dataCache.end(); ++i)
		delete[] (*i)._data;

	_mixer->stopHandle(_soundHandle);
	delete _opl;
}

void ASound::adlibInit() {
	write(4, 0x60);
	write(4, 0x80);
	write(2, 0xff);
	write(4, 0x21);
	write(4, 0x60);
	write(4, 0x80);
}

int ASound::stop() { 
	command0();
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

int ASound::poll() {
	// Update any playing sounds
	update();

	// Return result
	int result = _pollResult;
	_pollResult = 0;
	return result;
}

void ASound::noise() {
	int randomVal = getRandomNumber();

	if (_v1) {
		setFrequency(_channelNum1, (randomVal ^ 0xFFFF) & _freqMask1 + _freqBase1);
	}

	if (_v2) {
		setFrequency(_channelNum2, randomVal & _freqMask2 + _freqBase2);		
	}
}

void ASound::write(int reg, int val) {
	_queue.push(RegisterValue(reg, val));
}

int ASound::write2(int state, int reg, int val) {
	// TODO: Original has a state parameter, not used when in Adlib mode?
	_ports[reg] = val;
	write(reg, val);
	return state;
}

void ASound::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void ASound::channelOn(int reg, int volume) {
	write2(8, reg, (_ports[reg] & 0xC0) | (volume & 0x3F));
}

void ASound::channelOff(int reg) {
	write2(8, reg, _ports[reg] | 0x3F);
}

void ASound::resultCheck() {
	if (_resultFlag != 1) {
		_resultFlag = 1;
		_pollResult = 1;
	}
}

byte *ASound::loadData(int offset, int size) {
	// First scan for an existing copy	
	Common::List<CachedDataEntry>::iterator i;
	for (i = _dataCache.begin(); i != _dataCache.end(); ++i) {
		CachedDataEntry &e = *i;
		if (e._offset == offset)
			return e._data;
	}

	// No existing entry found, so load up data and store as a new entry
	CachedDataEntry rec;
	rec._offset = offset;
	rec._data = new byte[size];
	_soundFile.seek(_dataOffset + offset);
	_soundFile.read(rec._data, size);
	_dataCache.push_back(rec);

	// Return the data
	return rec._data;
}

void ASound::playSound(int offset, int size) {
	// Load the specified data block
	playSound(loadData(offset, size));
}

void ASound::playSound(byte *pData) {
	// Scan for a high level free channel
	for (int i = 5; i < ADLIB_CHANNEL_COUNT; ++i) {
		if (!_channels[i]._activeCount) {
			_channels[i].load(pData);
			return;
		}
	}

	// None found, do a secondary scan for an interruptable channel
	for (int i = ADLIB_CHANNEL_COUNT - 1; i >= ADLIB_CHANNEL_MIDWAY; --i) {
		if (_channels[i]._fieldE == 0xFF) {
			_channels[i].load(pData);
			return;
		}
	}
}

bool ASound::isSoundActive(byte *pData) {
	for (int i = 0; i < ADLIB_CHANNEL_MIDWAY; ++i) {
		if (_channels[i]._activeCount && _channels[i]._soundData == pData)
			return true;
	}

	return false;
}

void ASound::setFrequency(int channel, int freq) {
	write2(8, 0xA0 + channel, freq & 0xFF);
	write2(8, 0xB0 + channel, (freq >> 8) | 0x20);
}

int ASound::getRandomNumber() {
	int v = 0x9248 + (int)_randomSeed;
	_randomSeed = ((v >> 3) | (v << 13)) & 0xFFFF;
	return _randomSeed;
}

void ASound::update() {
	getRandomNumber();
	if (_isDisabled)
		return;

	++_frameCounter;
	pollChannels();
	checkChannels();

	if (_v1 == _v2) {
		if (_resultFlag != -1) {
			_resultFlag = -1;
			_pollResult = -1;
		}
	} else {
		if (_v1) {
			_freqBase1 += _v7;
			if (!--_v1) {
				if (!_v2 || _channelNum1 != _channelNum2) {
					write2(8, 0xA0 + _channelNum1, 0);
					write2(8, 0xB0 + _channelNum1, 0);
				}
			}
		}

		if (_v2) {
			_freqBase2 += _v8;
			if (!--_v2) {
				if (!_v1 || _channelNum2 != _channelNum1) {
					write2(8, 0xA0 + _channelNum2, 0);
					write2(8, 0xB0 + _channelNum2, 0);
				}
			}
		}
	}
}

void ASound::pollChannels() {
	_activeChannelNumber = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i) {
		_activeChannelPtr = &_channels[i];
		pollActiveChannel();
	}
}

void ASound::checkChannels() {
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].check(_nullData);
}

void ASound::pollActiveChannel() {
	AdlibChannel *chan = _activeChannelPtr; 
	bool updateFlag = true;

	if (chan->_activeCount) {
		if (chan->_field8 > 0 && --chan->_field8 == 0)
			updateOctave();

		if (--_activeChannelPtr->_activeCount <= 0) {
			for (;;) {
				byte *pSrc = chan->_pSrc;
				if (!(*pSrc & 0x80) || (*pSrc <= 0xF0)) {
					if (updateFlag)
						updateActiveChannel();

					chan->_field4 = *pSrc++;
					chan->_activeCount = *pSrc++;
					chan->_pSrc += 2;

					if (!chan->_field4 || !chan->_activeCount) {
						updateOctave();
					} else {
						chan->_field8 = chan->_activeCount - chan->_field7;
						updateChannelState();
					}

					// Break out of processing loop
					break;
				} else {
					updateFlag = false;

					switch ((~*pSrc) & 0xF) {
					case 0:
						if (!chan->_field17) {
							if (*++pSrc == 0) {
								chan->_pSrc += 2;
								chan->_ptr3 = chan->_pSrc;
								chan->_field17 = 0;
							} else {
								chan->_field17 = *pSrc;
								chan->_pSrc = chan->_ptr3;
							}
						} else if (--chan->_field17) {
							chan->_pSrc = chan->_ptr3;
						} else {
							chan->_pSrc += 2;
							chan->_ptr3 = chan->_pSrc;
						}
						break;

					case 1:
						if (!chan->_field19) {
							if (*++pSrc == 0) {
								chan->_pSrc += 2;
								chan->_ptr4 = chan->_pSrc;
								chan->_ptr3 = chan->_pSrc;
								chan->_field17 = 0;
								chan->_field19 = 0;
							} else {
								chan->_field19 = *pSrc;
								chan->_pSrc = chan->_ptr4;
								chan->_ptr3 = chan->_ptr4;
							}
						} else if (--chan->_field19) {
							chan->_ptr4 = chan->_pSrc;
							chan->_ptr3 = chan->_pSrc;
						} else {
							chan->_pSrc += 2;
							chan->_ptr4 = chan->_pSrc;
							chan->_ptr3 = chan->_pSrc;
						}
						break;

					case 2:
						// Loop sound data
						chan->_field1 = 0;
						chan->_field2 = chan->_field3 = 0;
						chan->_volume = chan->_field7 = 0;
						chan->_field1D = chan->_field1E = 0;
						chan->_field8 = 0;
						chan->_field9 = 0;
						chan->_fieldB = 0;
						chan->_field17 = 0;
						chan->_field19 = 0;
						chan->_fieldD = 0x40;
						chan->_ptr1 = chan->_soundData;
						chan->_pSrc = chan->_soundData;
						chan->_ptr3 = chan->_soundData;
						chan->_ptr4 = chan->_soundData;

						chan->_pSrc += 2;
						break;

					case 3:
						chan->_sampleIndex = *++pSrc;
						chan->_pSrc += 2;
						loadSample(chan->_sampleIndex);
						break;

					case 4:
						chan->_field7 = *++pSrc;
						chan->_pSrc += 2;
						break;

					case 5:
						chan->_field1 = *++pSrc;
						chan->_pSrc += 2;
						break;

					case 6:
						++pSrc;
						if (chan->_fieldE) {
							chan->_pSrc += 2;
						} else {
							chan->_volume = *pSrc >> 1;
							updateFlag = true;
							chan->_pSrc += 2;
						}
						break;

					case 7:
						++pSrc;
						if (!chan->_fieldE) {
							chan->_fieldA = *pSrc;
							chan->_field2 = *++pSrc;
							chan->_field9 = 1;
						}

						chan->_pSrc += 3;
						break;

					case 8:
						chan->_field1D = *++pSrc;
						chan->_pSrc += 2;
						break;

					case 9: {
						int v1 = *++pSrc;
						++pSrc;
						int v2 = (v1 - 1) & getRandomNumber();
						int v3 = pSrc[v2];
						int v4 = pSrc[v1];

						pSrc[v4 + v1 + 1] = v3;
						chan->_pSrc += v1 + 3;
						break;
					}

					case 10:
						++pSrc;
						if (chan->_fieldE) {
							chan->_pSrc += 2;
						} else {
							chan->_field1E = *pSrc >> 1;
							updateFlag = true;
							chan->_pSrc += 2;
						}
						break;

					case 11:
						chan->_fieldD = *++pSrc;
						updateFlag = true;
						chan->_pSrc += 2;
						break;

					case 12:
						chan->_fieldC = *++pSrc;
						chan->_field3 = *++pSrc;
						chan->_fieldB = 1;
						chan->_pSrc += 2;
						break;

					case 13:
						++pSrc;
						chan->_pSrc += 2;
						break;

					case 14:
						chan->_field1F = *++pSrc;
						chan->_pSrc += 2;
						break;
					
					default:
						break;
					}
				}
			}
		}

		if (chan->_field1)
			updateFNumber();

		updateFlag = false;
		if (chan->_field9 || chan->_fieldB) {
			if (!--chan->_field9) {
				chan->_field9 = chan->_fieldA;
				if (chan->_field2) {
					int8 newVal = (int8)chan->_field2 + (int8)chan->_field1E;
					if (newVal < 0) {
						chan->_field9 = 0;
						newVal = 0;
					} else if (newVal > 63) {
						chan->_field9 = 0;
						newVal = 63;
					}

					chan->_field1E = newVal;
					updateFlag = true;
				}
			}

			if (!--chan->_fieldB) {
				chan->_fieldB = chan->_fieldC;
				if (chan->_field3) {
					chan->_fieldD = chan->_field3;
					updateFlag = true;
				}
			}

			if (updateFlag)
				updateActiveChannel();
		}
	}

	++_activeChannelNumber;
}

void ASound::updateOctave() {
 	int reg = 0xB0 + _activeChannelNumber;
	write2(8, reg, _ports[reg] & 0xDF);
}

static int _vList1[] = {
	0x200, 0x21E, 0x23F, 0x261, 0x285, 0x2AB, 
	0x2D4, 0x2FF, 0x32D, 0x35D, 0x390, 0x3C7
};

void ASound::updateChannelState() {
	updateActiveChannel();

	if (_channelData[_activeChannelNumber]._field0) {
		if (_channelNum1 == _activeChannelNumber)
			_stateFlag = 0;
		if (_channelNum2 == _activeChannelNumber)
			_stateFlag = 1;

		if (!_stateFlag) {
			_stateFlag = 1;
			if (_v1)
				write2(8, 0xB0 + _channelNum1, _ports[0xB0 + _channelNum1] & 0xDF);

			_channelNum1 = _activeChannelNumber;
			_v1 = _channelData[_channelNum1]._field0;
			_freqMask1 = _channelData[_channelNum1]._freqMask;
			_freqBase1 = _channelData[_channelNum1]._freqBase;
			_v7 = _channelData[_channelNum1]._field6;
		} else {
			_stateFlag = 0;
			if (_v2)
				write2(8, 0xB0 + _channelNum2, _ports[0xB0 + _channelNum2] & 0xDF);

			_channelNum2 = _activeChannelNumber;
			_v2 = _channelData[_channelNum2]._field0;
			_freqMask2 = _channelData[_channelNum2]._freqMask;
			_freqBase2 = _channelData[_channelNum2]._freqBase;
			_v8 = _channelData[_channelNum2]._field6;
		}

		resultCheck();
	} else {
		int reg = 0xA0 + _activeChannelNumber;
		int vTimes = (_activeChannelPtr->_field4 + _activeChannelPtr->_field1F) / 12;
		int vOffset = (_activeChannelPtr->_field4 + _activeChannelPtr->_field1F) % 12;
		int val = _vList1[vOffset] + _activeChannelPtr->_field1D;
		write2(8, reg, val & 0xFF);

		reg += 0x10;
		write2(8, reg, (_ports[reg] & 0x20) | (vTimes << 2) | (val >> 8));

		write2(8, reg, _ports[reg] | 0x20);
	}
}

static const int outputIndexes[] = {
	3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 13, 16, 14, 17
};
static const int outputChannels[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21, 0
};
static const int volumeList[] = {
	0x3F, 0x3F, 0x36, 0x31, 0x2D, 0x2A, 0x28, 0x26, 0x24, 0x22, 0x21, 0x20, 0x1F, 0x1E, 0x1D, 0x1C,
	0x1B, 0x1A, 0x19, 0x19, 0x18, 0x17, 0x17, 0x16, 0x16, 0x15, 0x15, 0x14, 0x14, 0x13, 0x12, 0x12,
	0x11, 0x11, 0x10, 0x10, 0x0F, 0x0F, 0x0E, 0x0E, 0x0D, 0x0D, 0x0C, 0x0C, 0x0B, 0x0B, 0x0A, 0x0A,
	0x0A, 0x09, 0x09, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void ASound::updateActiveChannel() {
	int reg = 0x40 + outputChannels[outputIndexes[_activeChannelNumber * 2]];
	int portVal = _ports[reg] & 0xFFC0;
	int newVolume = CLIP(_activeChannelPtr->_volume + _activeChannelPtr->_field1E, 0, 63);
	
	// Note: Original had a whole block not seeming to be used, since the initialisation
	// sets a variable to 5660h, and doesn't change it, so the branch is never taken
	int val = CLIP(newVolume - volumeList[_activeChannelPtr->_fieldD], 0, 63);
	val = (63 - val) | portVal;

	int val2 = CLIP(newVolume - volumeList[-(_activeChannelPtr->_fieldD - 127)], 0, 63);
	val2 = (63 - val2) | portVal;
	write2(0, reg, val);
	write2(2, reg, val2);
}

void ASound::loadSample(int sampleIndex) {
	_activeChannelReg = 0xB0 + _activeChannelNumber;
	write2(8, _activeChannelReg, _ports[_activeChannelReg] & 0xDF);

	_activeChannelReg = _activeChannelNumber;
	_samplePtr = &_samples[sampleIndex * 2];
	_v11 = outputChannels[outputIndexes[_activeChannelReg * 2 - 1]];
	processSample();

	AdlibChannelData &cd = _channelData[_activeChannelNumber];
	cd._field6 = _samplePtr->_field14;
	cd._freqBase = _samplePtr->_freqBase;
	cd._freqMask = _samplePtr->_freqMask;
	cd._field0 = _samplePtr->_fieldE;

	_samplePtr = &_samples[sampleIndex * 2 + 1];
	_v11 = outputChannels[outputIndexes[_activeChannelReg * 2]];
	processSample();
}

void ASound::processSample() {
	// Write out vib flags and split point
	write2(8, 0x40 + _v11, 0x3F);
	int depthRhythm = _ports[0xBD] & 0x3F | (_amDep ? 0x80 : 0) |
		(_vibDep ? 0x40 : 0);
	write2(8, 0xBD, depthRhythm);
	write2(8, 8, _splitPoint ? 0x40 : 0);

	// Write out feedback & Alg
	int val = (_samplePtr->_feedback << 1) | (1 - _samplePtr->_alg);
	write2(8, 0xC0 + _activeChannelReg, val);

	// Write out attack/decay rate
	val = (_samplePtr->_attackRate << 4) | (_samplePtr->_decayRate & 0xF);
	write2(8, 0x60 + _v11, val);

	// Write out sustain level/release rate
	val = (_samplePtr->_sustainLevel << 4) | (_samplePtr->_releaseRate & 0xF);
	write2(8, 0x80 + _v11, val);

	// Write out misc flags
	val = (_samplePtr->_ampMod ? 0x80 : 0) | (_samplePtr->_vib ? 0x40 : 0)
		| (_samplePtr->_egTyp ? 0x20 : 0) | (_samplePtr->_ksr ? 0x10 : 0)
		| (_samplePtr->_freqMultiple & 0xF);
	write2(8, 0x20 + _v11, val);

	// Write out waveform select
	write2(8, 0xE0 + _v11, _samplePtr->_waveformSelect & 3);
	
	// Write out total level & scaling level
	val = -(_samplePtr->_totalLevel & 0x3F - 0x3F) | (_samplePtr->_scalingLevel << 6);
	write2(8, 0x40 + _v11, val);
}

void ASound::updateFNumber() {
	int loReg = 0xA0 + _activeChannelNumber;
	int hiReg = 0xB0 + _activeChannelNumber;
	int val1 = (_ports[hiReg] & 0x1F) << 8;
	val1 += _ports[loReg] + _activeChannelPtr->_field1;
	write2(8, loReg, val1);

	int val2 = (_ports[hiReg] & 0x20) | (val1 >> 8);
	write2(8, hiReg, val2);
}

int ASound::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_driverMutex);

	int32 samplesLeft = numSamples;
	memset(buffer, 0, sizeof(int16) * numSamples);
	while (samplesLeft) {
		if (!_samplesTillCallback) {
			poll();
			flush();

			_samplesTillCallback = _samplesPerCallback;
			_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
			if (_samplesTillCallbackRemainder >= CALLBACKS_PER_SECOND) {
				_samplesTillCallback++;
				_samplesTillCallbackRemainder -= CALLBACKS_PER_SECOND;
			}
		}

		int32 render = MIN<int>(samplesLeft, _samplesTillCallback);
		samplesLeft -= render;
		_samplesTillCallback -= render;

		_opl->readBuffer(buffer, render);
		buffer += render;
	}
	return numSamples;
}

int ASound::command0() {
	bool isDisabled = _isDisabled;
	_isDisabled = true;

	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].reset();

	_v1 = 0;
	_v2 = 0;
	_freqMask1 = _freqMask2 = 0;
	_freqBase1 = _freqBase2 = 0;
	_v7 = 0;
	_v8 = 0;	

	// Reset Adlib port registers
	for (int reg = 0x4F; reg >= 0x40; --reg)
		write2(8, reg, 0x3F);
	for (int reg = 0xFF; reg >= 0x60; --reg)
		write2(8, reg, 0);
	for (int reg = 0x3F; reg > 0; --reg)
		write2(8, reg, 0);
	write2(8, 1, 0x20);

	_isDisabled = isDisabled;
	return 0;
}

int ASound::command1() {
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int ASound::command2() {
	for (int i = 0; i < ADLIB_CHANNEL_MIDWAY; ++i)
		_channels[i].setPtr2(_nullData);
	return 0;
}

int ASound::command3() {
	for (int i = 0; i < ADLIB_CHANNEL_MIDWAY; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int ASound::command4() {
	for (int i = ADLIB_CHANNEL_MIDWAY; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].setPtr2(_nullData);
	return 0;
}

int ASound::command5() {
	for (int i = 5; i < ADLIB_CHANNEL_COUNT; ++i)
		_channels[i].enable(0xFF);
	return 0;
}

int ASound::command6() {
	_v9 = _v1;
	_v1 = 0;
	_v10 = _v2;
	_v2 = 0;

	channelOff(0x43);
	channelOff(0x44);
	channelOff(0x45);
	channelOff(0x4B);
	channelOff(0x4C);
	channelOff(0x4D);
	channelOff(0x53);
	channelOff(0x54);
	channelOff(0x55);

	return 0;
}

int ASound::command7() {
	channelOn(0x43, _channels[0]._volume);
	channelOn(0x44, _channels[1]._volume);
	channelOn(0x45, _channels[2]._volume);
	channelOn(0x4B, _channels[3]._volume);
	channelOn(0x4C, _channels[4]._volume);
	channelOn(0x4D, _channels[5]._volume);

	_v1 = _v9;
	_v2 = _v10;

	if (_v9 != _v10)
		resultCheck();

	_isDisabled = 0;
	return _v10;
}

int ASound::command8() {
	int result = 0;
	for (int i = 0; i < ADLIB_CHANNEL_COUNT; ++i)
		result |= _channels[i]._activeCount;

	return result;
}

/*-----------------------------------------------------------------------*/

const ASound1::CommandPtr ASound1::_commandList[42] = {
	&ASound1::command0, &ASound1::command1, &ASound1::command2, &ASound1::command3,
	&ASound1::command4, &ASound1::command5, &ASound1::command6, &ASound1::command7, 
	&ASound1::command8, &ASound1::command9, &ASound1::command10, &ASound1::command11,
	&ASound1::command12, &ASound1::command13, &ASound1::command14, &ASound1::command15,
	&ASound1::command16, &ASound1::command17, &ASound1::command18, &ASound1::command19, 
	&ASound1::command20, &ASound1::command21, &ASound1::command22, &ASound1::command23, 
	&ASound1::command24, &ASound1::command25, &ASound1::command26, &ASound1::command27,
	&ASound1::command28, &ASound1::command29, &ASound1::command30, &ASound1::command31, 
	&ASound1::command32, &ASound1::command33, &ASound1::command34, &ASound1::command35,
	&ASound1::command36, &ASound1::command37, &ASound1::command38, &ASound1::command39, 
	&ASound1::command40, &ASound1::command41
};

ASound1::ASound1(Audio::Mixer *mixer): ASound(mixer, "asound.001", 0x1520) {
	_cmd23Toggle = false;
	
	// Load sound samples
	_soundFile.seek(_dataOffset + 0x12C);
	for (int i = 0; i < 98; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

int ASound1::command(int commandId) {
	if (commandId > 41)
		return 0;

	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound1::command9() {
	playSound(0xC68, 12);
	return 0;
}

int ASound1::command10() {
	byte *pData1 = loadData(0x130E, 48);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x133E, 392));
		_channels[2].load(loadData(0x14C6, 46));
		_channels[3].load(loadData(0x14F4, 48));
	}

	return 0;
}

int ASound1::command11() {
	command111213();
	_channels[0]._field1E = 0;
	_channels[1]._field1E = 0;
	return 0;
}

int ASound1::command12() {
	command111213();
	_channels[0]._field1E = 40;
	_channels[1]._field1E = 0;
	return 0;
}

int ASound1::command13() {
	command111213();
	_channels[0]._field1E = 40;
	_channels[1]._field1E = 50;
	return 0;
}

int ASound1::command14() {
	playSound(0x1216, 248);
	return 0;
}

int ASound1::command15() {
	byte *pData1 = loadData(0x1524, 152);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[4].load(pData1);
		_channels[5].load(loadData(0x15BC, 94));
		_channels[6].load(loadData(0x161A, 94));
		_channels[7].load(loadData(0x1678, 42));
		_channels[8].load(loadData(0x16A2, 42));
	}

	return 0;
}

int ASound1::command16() {
	playSound(0xC74, 14);
	return 0;
}

int ASound1::command17() {
	playSound(0xE9A, 10);
	return 0;
}

int ASound1::command18() {
	command1();
	playSound(0xCA6, 20);
	return 0;
}

int ASound1::command19() {
	command1();
	playSound(0xCBA, 74);
	return 0;
}

int ASound1::command20() {
	byte *pData = loadData(0xD18, 28);
	if (!isSoundActive(pData))
		playSound(pData);
	return 0;
}

int ASound1::command21() {
	playSound(0xD04, 20);
	return 0;
}

int ASound1::command22() {
	byte *pData = loadData(0xD34, 10);
	pData[6] = (getRandomNumber() & 7) + 85;

	if (!isSoundActive(pData))
		playSound(pData);

	return 0;
}

int ASound1::command23() {
	_cmd23Toggle = !_cmd23Toggle;
	playSound(_cmd23Toggle ? 0xD3E : 0xD46, 8);
	return 0;
}

int ASound1::command24() {
	playSound(0xD4E, 18);
	playSound(0xD60, 20);
	playSound(0xD74, 14);
	return 0;
}

int ASound1::command25() {
	byte *pData = loadData(0xD82, 16);
	if (!isSoundActive(pData))
		playSound(pData);

	return 0;
}

int ASound1::command26() {
	error("TODO: command26");
	return 0;
}

int ASound1::command27() {
	error("TODO: ASound::command27");
	return 0;
}

int ASound1::command28() {
	playSound(0xD92, 28);
	return 0;
}

int ASound1::command29() {
	error("TODO: ASound::command29");
	return 0;
}

int ASound1::command30() {
	error("TODO: ASound::command30");
	return 0;
}

int ASound1::command31() {
	byte *pData = loadData(0xDAE, 14);
	if (!isSoundActive(pData))
		playSound(pData);

	return 0;
}

int ASound1::command32() {
	error("TODO: ASound::command32");
	return 0;
}

int ASound1::command33() {
	playSound(0xDBC, 10);
	playSound(0xDC6, 10);
	return 0;
}

int ASound1::command34() {
	int v = getRandomNumber() & 0x20;
	if (!v) 
		v = 0x60;

	byte *pData = loadData(0xDD0, 22);
	pData[8] = pData[15] = v;
	playSound(pData);
	return 0;
}

int ASound1::command35() {
	playSound(0xDE6, 16);
	return 0;
}

int ASound1::command36() {
	playSound(0xE10, 10);
	command34();

	return 0;
}

int ASound1::command37() {
	playSound(0xE1A, 14);
	return 0;
}

int ASound1::command38() {
	playSound(0xE28, 114);
	return 0;
}

int ASound1::command39() {
	byte *pData1 = loadData(0x16CC, 82);
	if (!isSoundActive(pData1)) {
		_channels[5].load(pData1);
		_channels[6].load(loadData(0x171E, 30));
		_channels[7].load(loadData(0x173C, 40));
		_channels[8].load(loadData(0x1764, 64));
	}
	return 0;
}

int ASound1::command40() {
	playSound(0xDF6, 26);
	return 0;
}

int ASound1::command41() {
	playSound(0xC32, 34);
	playSound(0xC54, 20);
	return 0;
}

void ASound1::command111213() {
	byte *pData1 = loadData(0xEF6, 408);
	if (!isSoundActive(pData1)) {
		command1();
		_channels[0].load(pData1);
		_channels[1].load(loadData(0x108E, 266));
		_channels[2].load(loadData(0x1198, 66));
		_channels[2].load(loadData(0x11DA, 60));
	}
}

void ASound1::command2627293032() {
	// TODO: This method takes a parameter off the stack for several levels up.
	// i.e. something the caller's caller pushed onto the stack. Need to figure
	// out a better way to pass parameters down if this is actually in use.
}

} // End of namespace Nebular

} // End of namespace MADS
