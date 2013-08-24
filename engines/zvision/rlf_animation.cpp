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

#include "common/scummsys.h"

#include "common/str.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/endian.h"

#include "zvision/rlf_animation.h"


namespace ZVision {

RlfAnimation::RlfAnimation(const Common::String &fileName, bool stream) 
		: _stream(stream),
		  _lastFrameRead(0),
		  _frameCount(0),
		  _width(0),
		  _height(0),
		  _frameTime(0),
		  _frames(0),
		  _currentFrameBuffer(0),
		  _currentFrame(-1),
		  _frameBufferByteSize(0) {
	if (!_file.open(fileName)) {
		warning("RLF animation file %s could not be opened", fileName.c_str());
		return;
	}

	if (!readHeader()) {
		warning("%s is not a RLF animation file. Wrong magic number", fileName.c_str());
		return;
	}

	_currentFrameBuffer = new uint16[_width * _height];
	_frameBufferByteSize = _width * _height * sizeof(uint16);

	if (!stream) {
		_frames = new Frame[_frameCount];

		// Read in each frame
		for (uint i = 0; i < _frameCount; i++) {
			_frames[i] = readNextFrame();
		}
	}
};

RlfAnimation::~RlfAnimation() {
	if (_frames != 0) {
		delete[] _frames;
	}
	if (_currentFrameBuffer != 0) {
		delete[] _currentFrameBuffer;
	}
}

bool RlfAnimation::readHeader() {
	if (_file.readUint32BE() != MKTAG('F', 'E', 'L', 'R')) {
		return false;
	}

	// Read the header
	_file.readUint32LE();                // Size1
	_file.readUint32LE();                // Unknown1
	_file.readUint32LE();                // Unknown2
	_frameCount = _file.readUint32LE();  // Frame count

	// Since we don't need any of the data, we can just seek right to the
	// entries we need rather than read in all the individual entries.
	_file.seek(136, SEEK_CUR);

	//// Read CIN header
	//_file.readUint32BE();          // Magic number FNIC
	//_file.readUint32LE();          // Size2
	//_file.readUint32LE();          // Unknown3
	//_file.readUint32LE();          // Unknown4
	//_file.readUint32LE();          // Unknown5
	//_file.seek(0x18, SEEK_CUR);    // VRLE
	//_file.readUint32LE();          // LRVD
	//_file.readUint32LE();          // Unknown6
	//_file.seek(0x18, SEEK_CUR);    // HRLE
	//_file.readUint32LE();          // ELHD
	//_file.readUint32LE();          // Unknown7
	//_file.seek(0x18, SEEK_CUR);    // HKEY
	//_file.readUint32LE();          // ELRH

	//// Read MIN info header
	//_file.readUint32BE();          // Magic number FNIM
	//_file.readUint32LE();          // Size3
	//_file.readUint32LE();          // OEDV
	//_file.readUint32LE();          // Unknown8
	//_file.readUint32LE();          // Unknown9
	//_file.readUint32LE();          // Unknown10
	_width = _file.readUint32LE();   // Width
	_height = _file.readUint32LE();  // Height

	// Read time header
	_file.readUint32BE();                    // Magic number EMIT
	_file.readUint32LE();                    // Size4
	_file.readUint32LE();                    // Unknown11
	_frameTime = _file.readUint32LE() / 10;  // Frame time in microseconds

	return true;
}

RlfAnimation::Frame RlfAnimation::readNextFrame() {
	RlfAnimation::Frame frame;

	_file.readUint32BE();                        // Magic number MARF
	uint32 size = _file.readUint32LE();          // Size
	_file.readUint32LE();                        // Unknown1
	_file.readUint32LE();                        // Unknown2
	uint32 type = _file.readUint32BE();          // Either ELHD or ELRH
	uint32 headerSize = _file.readUint32LE();    // Offset from the beginning of this frame to the frame data. Should always be 28
	_file.readUint32LE();                        // Unknown3

	frame.encodedSize = size - headerSize;
	frame.encodedData = new int8[frame.encodedSize];
	_file.read(frame.encodedData, frame.encodedSize);

	if (type == MKTAG('E', 'L', 'H', 'D')) {
		frame.type = Masked;
	} else if (type == MKTAG('E', 'L', 'R', 'H')) {
		frame.type = Simple;
		_completeFrames.push_back(_lastFrameRead);
	} else {
		warning("Frame %u doesn't have type that can be decoded", _lastFrameRead);
	}

	_lastFrameRead++;
	return frame;
}

const uint16 *RlfAnimation::getFrameData(uint frameNumber) {
	assert(!_stream);
	assert(frameNumber < _frameCount && frameNumber >= 0);

	if (frameNumber == _currentFrame) {
		return _currentFrameBuffer;
	}

	uint closestFrame = _currentFrame;
	uint distance = ABS(_currentFrame - frameNumber);
	for (Common::List<uint>::const_iterator iter = _completeFrames.begin(); iter != _completeFrames.end(); iter++) {
		uint newDistance = ABS((*iter) - frameNumber);
		if (closestFrame == -1 || newDistance < distance) {
			closestFrame = (*iter);
			distance = newDistance;
		}
	}

	bool forwards = frameNumber > closestFrame;
	if (forwards) {
		for (uint i = closestFrame; i <= frameNumber; i++) {
			applyFrameToCurrent(i);
		}
	} else {
		for (uint i = closestFrame; i >= frameNumber; i--) {
			applyFrameToCurrent(i);
		}
	}

	_currentFrame = frameNumber;
	return _currentFrameBuffer;
}

const uint16 *RlfAnimation::getNextFrame() {
	assert(_currentFrame + 1 < _frameCount);

	if (_stream) {
		applyFrameToCurrent(readNextFrame());
	} else {
		applyFrameToCurrent(_currentFrame + 1);
	}

	_currentFrame++;
	return _currentFrameBuffer;
}

const uint16 *RlfAnimation::getPreviousFrame() {
	assert(!_stream);
	assert(_currentFrame - 1 >= 0);

	applyFrameToCurrent(_currentFrame - 1);
	_currentFrame--;
	return _currentFrameBuffer;
}

void RlfAnimation::applyFrameToCurrent(uint frameNumber) {
	if (_frames[frameNumber].type == Masked) {
		decodeMaskedRunLengthEncoding(_frames[frameNumber].encodedData, (int8 *)_currentFrameBuffer, _frames[frameNumber].encodedSize, _frameBufferByteSize);
	} else if (_frames[frameNumber].type == Simple) {
		decodeSimpleRunLengthEncoding(_frames[frameNumber].encodedData, (int8 *)_currentFrameBuffer, _frames[frameNumber].encodedSize, _frameBufferByteSize);
	}
}

void RlfAnimation::applyFrameToCurrent(const RlfAnimation::Frame &frame) {
	if (frame.type == Masked) {
		decodeMaskedRunLengthEncoding(frame.encodedData, (int8 *)_currentFrameBuffer, frame.encodedSize, _frameBufferByteSize);
	} else if (frame.type == Simple) {
		decodeSimpleRunLengthEncoding(frame.encodedData, (int8 *)_currentFrameBuffer, frame.encodedSize, _frameBufferByteSize);
	}
}

void RlfAnimation::decodeMaskedRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const {
	uint32 sourceOffset = 0;
	uint32 destOffset = 0;

	while (sourceOffset < sourceSize) {
		int8 numberOfSamples = source[sourceOffset];
		sourceOffset++;

		// If numberOfSamples is negative, the next abs(numberOfSamples) samples should
		// be copied directly from source to dest
		if (numberOfSamples < 0) {
			numberOfSamples = ABS(numberOfSamples);

			while (numberOfSamples > 0) {
				if (sourceOffset + 1 >= sourceSize) {
					return;
				} else if (destOffset + 1 >= destSize) {
					warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, READ_LE_UINT16(source + sourceOffset));

				sourceOffset += 2;
				destOffset += 2;
				numberOfSamples--;
			}

		// If numberOfSamples is >= 0, move destOffset forward ((numberOfSamples * 2) + 2)
		// This function assumes the dest buffer has been memset with 0's.
		} else {
			if (sourceOffset + 1 >= sourceSize) {
				return;
			} else if (destOffset + 1 >= destSize) {
				warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
				return;
			}

			destOffset += (numberOfSamples * 2) + 2;
		}
	}
}

void RlfAnimation::decodeSimpleRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const {
	uint32 sourceOffset = 0;
	uint32 destOffset = 0;

	while (sourceOffset < sourceSize) {
		int8 numberOfSamples = source[sourceOffset];
		sourceOffset++;

		// If numberOfSamples is negative, the next abs(numberOfSamples) samples should
		// be copied directly from source to dest
		if (numberOfSamples < 0) {
			numberOfSamples = ABS(numberOfSamples);

			while (numberOfSamples > 0) {
				if (sourceOffset + 1 >= sourceSize) {
					return;
				} else if (destOffset + 1 >= destSize) {
					warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, READ_LE_UINT16(source + sourceOffset));

				sourceOffset += 2;
				destOffset += 2;
				numberOfSamples--;
			}

		// If numberOfSamples is >= 0, copy one sample from source to the 
		// next (numberOfSamples + 2) dest spots
		} else {
			if (sourceOffset + 1 >= sourceSize) {
				return;
			}

			uint16 sampleColor = READ_LE_UINT16(source + sourceOffset);
			sourceOffset += 2;

			numberOfSamples += 2;
			while (numberOfSamples > 0) {
				if (destOffset + 1 >= destSize) {
					warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, sampleColor);
				destOffset += 2;
				numberOfSamples--;
			}
		}
	}
}

} // End of namespace ZVision
