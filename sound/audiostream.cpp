/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"


// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))


namespace Audio {

struct StreamFileFormat {
	/** Decodername */
	const char* decoderName;
	const char* fileExtension;
	/**
	 * Pointer to a function which tries to open a file of type StreamFormat.
	 * Return NULL in case of an error (invalid/nonexisting file).
	 */
	AudioStream* (*openStreamFile)(Common::SeekableReadStream *stream, bool disposeAfterUse,
					uint32 startTime, uint32 duration, uint numLoops);
};

static const StreamFileFormat STREAM_FILEFORMATS[] = {
	/* decoderName,		fileExt, openStreamFuntion */
#ifdef USE_FLAC
	{ "Flac",			"flac", makeFlacStream },
	{ "Flac",			"fla",  makeFlacStream },
#endif
#ifdef USE_VORBIS
	{ "Ogg Vorbis",		"ogg",  makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "MPEG Layer 3",	"mp3",  makeMP3Stream },
#endif

	{ NULL, NULL, NULL } // Terminator
};

AudioStream* AudioStream::openStreamFile(const char *filename) {
	char buffer[1024];
	const uint len = strlen(filename);
	assert(len+6 < sizeof(buffer)); // we need a bigger buffer if wrong

	memcpy(buffer, filename, len);
	buffer[len] = '.';
	char *ext = &buffer[len+1];

	AudioStream* stream = NULL;
	Common::File *fileHandle = new Common::File();

	for (int i = 0; i < ARRAYSIZE(STREAM_FILEFORMATS)-1 && stream == NULL; ++i) {
		strcpy(ext, STREAM_FILEFORMATS[i].fileExtension);
		fileHandle->open(buffer);
		if (fileHandle->isOpen()) {
			stream = STREAM_FILEFORMATS[i].openStreamFile(fileHandle, true, 0, 0, 1);
			fileHandle = 0;
			break;
		}
	}

	delete fileHandle;

	if (stream == NULL) {
		debug(1, "AudioStream: Could not open compressed AudioFile %s", filename);
	}

	return stream;
}

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -


/**
 * A simple raw audio stream, purely memory based. It operates on a single
 * block of data, which is passed to it upon creation.
 * Optionally supports looping the sound.
 *
 * Design note: This code tries to be as optimized as possible (without
 * resorting to assembly, that is). To this end, it is written as a template
 * class. This way the compiler can actually create optimized code for each
 * special code. This results in a total of 12 versions of the code being
 * generated.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class LinearMemoryStream : public AudioStream {
protected:
	const byte *_ptr;
	const byte *_end;
	const byte *_loopPtr;
	const byte *_loopEnd;
	const int _rate;
	const byte *_origPtr;

	inline bool eosIntern() const	{ return _ptr >= _end; };
public:
	LinearMemoryStream(int rate, const byte *ptr, uint len, uint loopOffset, uint loopLen, bool autoFreeMemory)
		: _ptr(ptr), _end(ptr+len), _loopPtr(0), _loopEnd(0), _rate(rate) {

		// Verify the buffer sizes are sane
		if (is16Bit && stereo)
			assert((len & 3) == 0 && (loopLen & 3) == 0);
		else if (is16Bit || stereo)
			assert((len & 1) == 0 && (loopLen & 1) == 0);

		if (loopLen) {
			_loopPtr = _ptr + loopOffset;
			_loopEnd = _loopPtr + loopLen;
		}
		if (stereo)	// Stereo requires even sized data
			assert(len % 2 == 0);

		_origPtr = autoFreeMemory ? ptr : 0;
	}
	~LinearMemoryStream() {
		free(const_cast<byte *>(_origPtr));
	}
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const		{ return stereo; }
	bool endOfData() const		{ return eosIntern(); }

	int getRate() const			{ return _rate; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int LinearMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_end - _ptr) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _ptr, isLE);
			_ptr += (is16Bit ? 2 : 1);
			samples++;
		}
		// Loop, if looping was specified
		if (_loopPtr && eosIntern()) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
	}
	return samples;
}


#pragma mark -
#pragma mark --- Input stream factory ---
#pragma mark -

/* In the following, we use preprocessor / macro tricks to simplify the code
 * which instantiates the input streams. We used to use template functions for
 * this, but MSVC6 / EVC 3-4 (used for WinCE builds) are extremely buggy when it
 * comes to this feature of C++... so as a compromise we use macros to cut down
 * on the (source) code duplication a bit.
 * So while normally macro tricks are said to make maintenance harder, in this
 * particular case it should actually help it :-)
 */

#define MAKE_LINEAR(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, true>(rate, ptr, len, loopOffset, loopLen, autoFree); \
			else  \
				return new LinearMemoryStream<STEREO, true, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree); \
		} else \
			return new LinearMemoryStream<STEREO, false, UNSIGNED, false>(rate, ptr, len, loopOffset, loopLen, autoFree)

AudioStream *makeLinearInputStream(int rate, byte flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen) {
	const bool isStereo   = (flags & Audio::Mixer::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::Mixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::Mixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::Mixer::FLAG_LITTLE_ENDIAN) != 0;
	const bool autoFree   = (flags & Audio::Mixer::FLAG_AUTOFREE) != 0;

	if (isStereo) {
		if (isUnsigned) {
			MAKE_LINEAR(true, true);
		} else {
			MAKE_LINEAR(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_LINEAR(false, true);
		} else {
			MAKE_LINEAR(false, false);
		}
	}
}


#pragma mark -
#pragma mark --- Appendable audio stream ---
#pragma mark -


/**
 * Wrapped memory stream.
 */
template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
class AppendableMemoryStream : public AppendableAudioStream {
protected:
	Common::Mutex _mutex;

	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;
	bool _finalized;
	const int _rate;

	inline bool eosIntern() const { return _end == _pos; };
public:
	AppendableMemoryStream(int rate, uint bufferSize);
	~AppendableMemoryStream();
	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const		{ return stereo; }
	bool endOfStream() const	{ return _finalized && eosIntern(); }
	bool endOfData() const		{ return eosIntern(); }

	int getRate() const			{ return _rate; }

	void append(const byte *data, uint32 len);
	void finish()				{ _finalized = true; }
};

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::AppendableMemoryStream(int rate, uint bufferSize)
 : _finalized(false), _rate(rate) {

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((bufferSize & 3) == 0);
	else if (is16Bit || stereo)
		assert((bufferSize & 1) == 0);

	_bufferStart = (byte *)malloc(bufferSize);
	assert(_bufferStart != NULL);

	_pos = _end = _bufferStart;
	_bufferEnd = _bufferStart + bufferSize;
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::~AppendableMemoryStream() {
	free(_bufferStart);
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
int AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		// Wrap around?
		if (_pos >= _bufferEnd)
			_pos = _pos - (_bufferEnd - _bufferStart);

		const byte *endMarker = (_pos > _end) ? _bufferEnd : _end;
		const int len = MIN(numSamples, samples + (int)(endMarker - _pos) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, _pos, isLE);
			_pos += (is16Bit ? 2 : 1);
			samples++;
		}
	}

	return samples;
}

template<bool stereo, bool is16Bit, bool isUnsigned, bool isLE>
void AppendableMemoryStream<stereo, is16Bit, isUnsigned, isLE>::append(const byte *data, uint32 len) {
	Common::StackLock lock(_mutex);

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((len & 3) == 0);
	else if (is16Bit || stereo)
		assert((len & 1) == 0);

	// Verify that the stream has not yet been finalized (by a call to finish())
	assert(!_finalized);

	if (_end + len > _bufferEnd) {
		// Wrap-around case
		uint32 size_to_end_of_buffer = _bufferEnd - _end;
		len -= size_to_end_of_buffer;
		if ((_end < _pos) || (_bufferStart + len >= _pos)) {
			debug(2, "AppendableMemoryStream: buffer overflow (A)");
			return;
		}
		memcpy(_end, data, size_to_end_of_buffer);
		memcpy(_bufferStart, data + size_to_end_of_buffer, len);
		_end = _bufferStart + len;
	} else {
		if ((_end < _pos) && (_end + len >= _pos)) {
			debug(2, "AppendableMemoryStream: buffer overflow (B)");
			return;
		}
		memcpy(_end, data, len);
		_end += len;
	}
}


#define MAKE_WRAPPED(STEREO, UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new AppendableMemoryStream<STEREO, true, UNSIGNED, true>(rate, len); \
			else  \
				return new AppendableMemoryStream<STEREO, true, UNSIGNED, false>(rate, len); \
		} else \
			return new AppendableMemoryStream<STEREO, false, UNSIGNED, false>(rate, len)

AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags, uint32 len) {
	const bool isStereo = (_flags & Audio::Mixer::FLAG_STEREO) != 0;
	const bool is16Bit = (_flags & Audio::Mixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & Audio::Mixer::FLAG_UNSIGNED) != 0;
	const bool isLE       = (_flags & Audio::Mixer::FLAG_LITTLE_ENDIAN) != 0;

	if (isStereo) {
		if (isUnsigned) {
			MAKE_WRAPPED(true, true);
		} else {
			MAKE_WRAPPED(true, false);
		}
	} else {
		if (isUnsigned) {
			MAKE_WRAPPED(false, true);
		} else {
			MAKE_WRAPPED(false, false);
		}
	}
}


} // End of namespace Audio
