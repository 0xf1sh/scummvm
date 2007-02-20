/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Common {

class String;

/**
 * Virtual base class for both ReadStream and WriteStream.
 */
class Stream {
public:
	virtual ~Stream() {}

	/**
	 * Returns true if any I/O failure occured.
	 * This flag is never cleared automatically. In order to clear it,
	 * client code has to call clearIOFailed() explicitly.
	 *
	 * @todo Instead of returning a plain bool, maybe we should define
	 *       a list of error codes which can be returned here.
	 */
	virtual bool ioFailed() const { return false; }

	/**
	 * Reset the I/O error status.
	 */
	virtual void clearIOFailed() {}
};

/**
 * Generic interface for a writable data stream.
 */
class WriteStream : virtual public Stream {
public:
	/**
	 * Write data into the stream. Subclasses must implement this
	 * method; all other write methods are implemented using it.
	 *
	 * @param dataPtr	pointer to the data to be written
	 * @param dataSize	number of bytes to be written
	 * @return the number of bytes which were actually written.
	 */
	virtual uint32 write(const void *dataPtr, uint32 dataSize) = 0;

	/**
	 * Commit any buffered data to the underlying channel or
	 * storage medium; unbuffered streams can use the default
	 * implementation.
	 */
	virtual void flush() {}

	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	void writeByte(byte value) {
		write(&value, 1);
	}

	void writeSByte(int8 value) {
		write(&value, 1);
	}

	void writeUint16LE(uint16 value) {
		writeByte((byte)(value & 0xff));
		writeByte((byte)(value >> 8));
	}

	void writeUint32LE(uint32 value) {
		writeUint16LE((uint16)(value & 0xffff));
		writeUint16LE((uint16)(value >> 16));
	}

	void writeUint16BE(uint16 value) {
		writeByte((byte)(value >> 8));
		writeByte((byte)(value & 0xff));
	}

	void writeUint32BE(uint32 value) {
		writeUint16BE((uint16)(value >> 16));
		writeUint16BE((uint16)(value & 0xffff));
	}

	void writeSint16LE(int16 value) {
		writeUint16LE((uint16)value);
	}

	void writeSint32LE(int32 value) {
		writeUint32LE((uint32)value);
	}

	void writeSint16BE(int16 value) {
		writeUint16BE((uint16)value);
	}

	void writeSint32BE(int32 value) {
		writeUint32BE((uint32)value);
	}

	void writeString(const String &str);
};


/**
 * Generic interface for a readable data stream.
 */
class ReadStream : virtual public Stream {
public:
	/**
	 * Returns true if the end of the stream has been reached.
	 */
	virtual bool eos() const = 0;

	/**
	 * Read data from the stream. Subclasses must implement this
	 * method; all other read methods are implemented using it.
	 *
	 * @param dataPtr	pointer to a buffer into which the data is read
	 * @param dataSize	number of bytes to be read
	 * @return the number of bytes which were actually read.
	 */
	virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;


	// The remaining methods all have default implementations; subclasses
	// need not (and should not) overload them.

	byte readByte() {
		byte b = 0;
		read(&b, 1);
		return b;
	}

	int8 readSByte() {
		int8 b = 0;
		read(&b, 1);
		return b;
	}

	uint16 readUint16LE() {
		uint16 a = readByte();
		uint16 b = readByte();
		return a | (b << 8);
	}

	uint32 readUint32LE() {
		uint32 a = readUint16LE();
		uint32 b = readUint16LE();
		return (b << 16) | a;
	}

	uint16 readUint16BE() {
		uint16 b = readByte();
		uint16 a = readByte();
		return a | (b << 8);
	}

	uint32 readUint32BE() {
		uint32 b = readUint16BE();
		uint32 a = readUint16BE();
		return (b << 16) | a;
	}

	int16 readSint16LE() {
		return (int16)readUint16LE();
	}

	int32 readSint32LE() {
		return (int32)readUint32LE();
	}

	int16 readSint16BE() {
		return (int16)readUint16BE();
	}

	int32 readSint32BE() {
		return (int32)readUint32BE();
	}
};


/**
 * Interface for a seekable & readable data stream.
 *
 * @todo We really need better error handling here!
 *       Like seek should somehow indicate whether it failed.
 */
class SeekableReadStream : virtual public ReadStream {
public:

	virtual uint32 pos() const = 0;
	virtual uint32 size() const = 0;

	virtual void seek(int32 offset, int whence = SEEK_SET) = 0;

	void skip(uint32 offset) { seek(offset, SEEK_CUR); }

	/**
	 * Read one line of text from a CR or CR/LF terminated plain text file.
	 * This method is a rough analog of the (f)gets function.
	 *
	 * @param buf	the buffer to store into
	 * @param bufSize	the size of the buffer
	 * @return a pointer to the read string, or NULL if an error occured
	 * @note The line terminator (CR or CR/LF) is stripped and not inserted
	 *       into the buffer.
	 */
	virtual char *readLine(char *buf, size_t bufSize);
};

/**
 * SubReadStream provides access to a ReadStream restricted to the range
 * [currentPosition, currentPosition+end).
 * Manipulating the parent stream directly /will/ mess up a substream.
 * Likewise, manipulating two substreams of a parent stream will cause them to
 * step on each others toes.
 */
class SubReadStream : virtual public ReadStream {
protected:
	ReadStream *_parentStream;
	uint32 _pos;
	uint32 _end;
	bool _disposeParentStream;
public:
	SubReadStream(ReadStream *parentStream, uint32 end, bool disposeParentStream = false)
		: _parentStream(parentStream),
		  _pos(0),
		  _end(end),
		  _disposeParentStream(disposeParentStream) {}
	~SubReadStream() {
		if (_disposeParentStream) delete _parentStream;
	}

	virtual bool eos() const { return _pos == _end; }
	virtual uint32 read(void *dataPtr, uint32 dataSize);
};

/*
 * SeekableSubReadStream provides access to a SeekableReadStream restricted to
 * the range [begin, end).
 * The same caveats apply to SeekableSubReadStream as do to SeekableReadStream.
 */
class SeekableSubReadStream : public SubReadStream, public SeekableReadStream {
protected:
	SeekableReadStream *_parentStream;
	uint32 _begin;
public:
	SeekableSubReadStream(SeekableReadStream *parentStream, uint32 begin, uint32 end, bool disposeParentStream = false);

	virtual uint32 pos() const { return _pos - _begin; }
	virtual uint32 size() const { return _end - _begin; }

	virtual void seek(int32 offset, int whence = SEEK_SET);
};

/**
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : public SeekableReadStream {
private:
	const byte *_ptr;
	const byte * const _ptrOrig;
	const uint32 _bufSize;
	uint32 _pos;
	byte _encbyte;
	bool _disposeMemory;

public:
	MemoryReadStream(const byte *buf, uint32 len, bool disposeMemory = false) :
		_ptr(buf),
		_ptrOrig(buf),
		_bufSize(len),
		_pos(0),
		_encbyte(0),
		_disposeMemory(disposeMemory) {}

	~MemoryReadStream() {
		if (_disposeMemory)
			free((void *)_ptrOrig);
	}

	void setEnc(byte value) { _encbyte = value; }

	uint32 read(void *dataPtr, uint32 dataSize) {
		// Read at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos)
			dataSize = _bufSize - _pos;
		memcpy(dataPtr, _ptr, dataSize);

		if (_encbyte) {
			byte *p = (byte *)dataPtr;
			byte *end = p + dataSize;
			while (p < end)
				*p++ ^= _encbyte;
		}

		_ptr += dataSize;
		_pos += dataSize;

		return dataSize;
	}

	bool eos() const { return _pos == _bufSize; }
	uint32 pos() const { return _pos; }
	uint32 size() const { return _bufSize; }

	void seek(int32 offs, int whence = SEEK_SET);
};

/**
 * Simple memory based 'stream', which implements the WriteStream interface for
 * a plain memory block.
 */
class MemoryWriteStream : public WriteStream {
private:
	byte *_ptr;
	const byte * const _ptrOrig;
	const uint32 _bufSize;
	uint32 _pos;
public:
	MemoryWriteStream(byte *buf, uint32 len) : _ptr(buf), _ptrOrig(buf), _bufSize(len), _pos(0) {}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		// Write at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos)
			dataSize = _bufSize - _pos;
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		return dataSize;
	}

	bool eos() const { return _pos == _bufSize; }
	uint32 pos() const { return _pos; }
	uint32 size() const { return _bufSize; }
};

}	// End of namespace Common

#endif
