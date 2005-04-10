/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/resource.h"
#include "sound/mididrv.h"

namespace Scumm {

void ScummEngine_v2::readClassicIndexFile() {
	int i;

	if (_gameId == GID_MANIAC) {
		if (_features & GF_NES)
			_numGlobalObjects = 775;
		else
			_numGlobalObjects = 800;
		_numRooms = 55;

		if (_features & GF_NES)
			// costumes 25-36 are special. see v1MMNEScostTables[] in costume.cpp
			// costumes 37-76 are room graphics resources
			// costume 77 is a character set translation table
			// costume 78 is a preposition list
			// costume 79 is unused but allocated, so the total is a nice even number :)
			_numCostumes = 80;
		else
			_numCostumes = 35;

		_numScripts = 200;
		_numSounds = 100;
	} else if (_gameId == GID_ZAK) {
		_numGlobalObjects = 775;
		_numRooms = 61;
		_numCostumes = 37;
		_numScripts = 155;
		_numSounds = 120;
	}

	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	_fileHandle->readUint16LE(); /* version magic number */
	for (i = 0; i != _numGlobalObjects; i++) {
		byte tmp = _fileHandle->readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}

	for (i = 0; i < _numRooms; i++) {
		res.roomno[rtRoom][i] = i;
	}
	_fileHandle->seek(_numRooms, SEEK_CUR);
	for (i = 0; i < _numRooms; i++) {
		res.roomoffs[rtRoom][i] = _fileHandle->readUint16LE();
		if (res.roomoffs[rtRoom][i] == 0xFFFF)
			res.roomoffs[rtRoom][i] = 0xFFFFFFFF;
	}

	for (i = 0; i < _numCostumes; i++) {
		res.roomno[rtCostume][i] = _fileHandle->readByte();
	}
	for (i = 0; i < _numCostumes; i++) {
		res.roomoffs[rtCostume][i] = _fileHandle->readUint16LE();
		if (res.roomoffs[rtCostume][i] == 0xFFFF)
			res.roomoffs[rtCostume][i] = 0xFFFFFFFF;
	}

	for (i = 0; i < _numScripts; i++) {
		res.roomno[rtScript][i] = _fileHandle->readByte();
	}
	for (i = 0; i < _numScripts; i++) {
		res.roomoffs[rtScript][i] = _fileHandle->readUint16LE();
		if (res.roomoffs[rtScript][i] == 0xFFFF)
			res.roomoffs[rtScript][i] = 0xFFFFFFFF;
	}

	for (i = 0; i < _numSounds; i++) {
		res.roomno[rtSound][i] = _fileHandle->readByte();
	}
	for (i = 0; i < _numSounds; i++) {
		res.roomoffs[rtSound][i] = _fileHandle->readUint16LE();
		if (res.roomoffs[rtSound][i] == 0xFFFF)
			res.roomoffs[rtSound][i] = 0xFFFFFFFF;
	}
}

void ScummEngine_v2::readEnhancedIndexFile() {

	_numGlobalObjects = _fileHandle->readUint16LE();
	_fileHandle->seek(_numGlobalObjects, SEEK_CUR);
	_numRooms = _fileHandle->readByte();
	_fileHandle->seek(_numRooms * 3, SEEK_CUR);
	_numCostumes = _fileHandle->readByte();
	_fileHandle->seek(_numCostumes * 3, SEEK_CUR);
	_numScripts = _fileHandle->readByte();
	_fileHandle->seek(_numScripts * 3, SEEK_CUR);
	_numSounds = _fileHandle->readByte();

	_fileHandle->clearIOFailed();
	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	_fileHandle->readUint16LE(); /* version magic number */
	readGlobalObjects();
	readResTypeList(rtRoom, MKID('ROOM'), "room");
	readResTypeList(rtCostume, MKID('COST'), "costume");
	readResTypeList(rtScript, MKID('SCRP'), "script");
	readResTypeList(rtSound, MKID('SOUN'), "sound");
}

void ScummEngine_v2::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);

	for (i = 0; i != num; i++) {
		byte tmp = _fileHandle->readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}
}

void ScummEngine_v2::readIndexFile() {
	int magic = 0;
	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	magic = _fileHandle->readUint16LE();

	switch (magic) {
		case 0x0100:
			printf("Enhanced V2 game detected\n");
			readEnhancedIndexFile();			
			break;
		case 0x0A31:
			printf("Classic V1 game detected\n");
			_version = 1;
			readClassicIndexFile();
			break;
		case 0x4643:
			if (!(_features & GF_NES))
				error("Use maniac target");
			printf("NES V1 game detected\n");
			_version = 1;
			readClassicIndexFile();
			break;
		default:
			error("Unknown magic id (0x%X) - this version is unsupported", magic);
			break;
	}

	closeRoom();
}

void ScummEngine_v2::loadCharset(int num) {
	// Stub, V2 font resources are hardcoded into the engine.
}

} // End of namespace Scumm
