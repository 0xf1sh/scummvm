/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

namespace Scumm {

BundleDirCache::BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		_budleDirCache[fileId].bundleTable = NULL;
		_budleDirCache[fileId].fileName[0] = 0;
		_budleDirCache[fileId].numFiles = 0;
	}
}

BundleDirCache::~BundleDirCache() {
	for (int fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		if (_budleDirCache[fileId].bundleTable != NULL)
			free (_budleDirCache[fileId].bundleTable);
	}
}

BundleDirCache::AudioTable *BundleDirCache::getTable(const char *filename) {
	int slot = matchFile(filename);
	assert(slot != -1);
	return _budleDirCache[slot].bundleTable;
}

int32 BundleDirCache::getNumFiles(const char *filename) {
	int slot = matchFile(filename);
	assert(slot != -1);
	return _budleDirCache[slot].numFiles;
}

int BundleDirCache::matchFile(const char *filename) {
	int32 tag, offset;
	bool found = false;
	int freeSlot = -1;
	int fileId;

	for (fileId = 0; fileId < ARRAYSIZE(_budleDirCache); fileId++) {
		if ((_budleDirCache[fileId].bundleTable == NULL) && (freeSlot == -1)) {
			freeSlot = fileId;
		}
		if (scumm_stricmp(filename, _budleDirCache[fileId].fileName) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		File file;

		if (file.open(filename) == false) {
			warning("BundleDirCache::matchFile() Can't open bundle file: %s", filename);
			return false;
		}

		if (freeSlot == -1)
			error("BundleDirCache::matchFileFile() Can't find free slot for file bundle dir cache");

		tag = file.readUint32BE();
		offset = file.readUint32BE();
		
		strcpy(_budleDirCache[freeSlot].fileName, filename);
		_budleDirCache[freeSlot].numFiles = file.readUint32BE();
		_budleDirCache[freeSlot].bundleTable = (AudioTable *) malloc(_budleDirCache[freeSlot].numFiles * sizeof(AudioTable));

		file.seek(offset, SEEK_SET);

		for (int32 i = 0; i < _budleDirCache[freeSlot].numFiles; i++) {
			char name[13], c;
			int32 z = 0;
			int32 z2;

			for (z2 = 0; z2 < 8; z2++)
				if ((c = file.readByte()) != 0)
					name[z++] = c;
			name[z++] = '.';
			for (z2 = 0; z2 < 4; z2++)
				if ((c = file.readByte()) != 0)
					name[z++] = c;

			name[z] = '\0';
			strcpy(_budleDirCache[freeSlot].bundleTable[i].filename, name);
			_budleDirCache[freeSlot].bundleTable[i].offset = file.readUint32BE();
			file.seek(4, SEEK_CUR);
		}
		return freeSlot;
	} else {
		return fileId;
	}
}

BundleMgr::BundleMgr(BundleDirCache *cache) {
	_cache = cache;
	_bundleTable = NULL;
	_compTable = NULL;
	_numFiles = 0;
	_numCompItems = 0;
	_curSample = -1;
	_fileBundleId = -1;
	_compInput = NULL;
}

BundleMgr::~BundleMgr() {
	closeFile();
}

bool BundleMgr::openFile(const char *filename) {
	if (_file.isOpen())
		return true;

	if (_file.open(filename) == false) {
		warning("BundleMgr::openFile() Can't open bundle file: %s", filename);
		return false;
	}

	_numFiles = _cache->getNumFiles(filename);
	assert(_numFiles);
	_bundleTable = _cache->getTable(filename);
	assert(_bundleTable);
	_compTableLoaded = false;
	_outputSize = 0;
	_lastBlock = -1;

	return true;
}

void BundleMgr::closeFile() {
	if (_file.isOpen()) {
		_file.close();
		_bundleTable = NULL;
		_numFiles = 0;
		_numCompItems = 0;
		_compTableLoaded = false;
		_lastBlock = -1;
		_outputSize = 0;
		_curSample = -1;
		free(_compTable);
		_compTable = NULL;
		if (_compInput) {
			free(_compInput);
			_compInput = NULL;
		}
	}
}

int32 BundleMgr::decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside) {
	return decompressSampleByIndex(_curSample, offset, size, comp_final, header_size, header_outside);
}

int32 BundleMgr::decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside) {
	int32 i, tag, num, final_size, output_size;
	int skip, first_block, last_block;
	
	if (index != -1)
		_curSample = index;

	if (_file.isOpen() == false) {
		warning("BundleMgr::decompressSampleByIndex() File is not open!");
		return 0;
	}

	if (!_compTableLoaded) {
		_file.seek(_bundleTable[index].offset, SEEK_SET);
		tag = _file.readUint32BE();
		_numCompItems = num = _file.readUint32BE();
		_file.seek(8, SEEK_CUR);

		if (tag != MKID_BE('COMP')) {
			warning("BundleMgr::decompressSampleByIndex() Compressed sound %d invalid (%s)", index, tag2str(tag));
			return 0;
		}

		_compTable = (CompTable *)malloc(sizeof(CompTable) * num);
		int32 maxSize = 0;
		for (i = 0; i < num; i++) {
			_compTable[i].offset = _file.readUint32BE();
			_compTable[i].size = _file.readUint32BE();
			_compTable[i].codec = _file.readUint32BE();
			_file.seek(4, SEEK_CUR);
			if (_compTable[i].size > maxSize)
				maxSize = _compTable[i].size;
		}
		// CMI hack: one more byte at the end of input buffer
		_compInput = (byte *)malloc(maxSize + 1);
		_compTableLoaded = true;
	}

	first_block = (offset + header_size) / 0x2000;
	last_block = (offset + size + header_size - 1) / 0x2000;

	// case when (offset + size + header_size - 1) is more one byte after sound resource
	if ((last_block >= _numCompItems) && (_numCompItems > 0))
		last_block = _numCompItems - 1;

	int32 blocks_final_size = 0x2000 * (1 + last_block - first_block);
	*comp_final = (byte *)malloc(blocks_final_size);
	final_size = 0;

	skip = offset - (first_block * 0x2000) + header_size;

	for (i = first_block; i <= last_block; i++) {
		if (_lastBlock != i) {
			// CMI hack: one more zero byte at the end of input buffer
			_compInput[_compTable[i].size] = 0;
			_file.seek(_bundleTable[index].offset + _compTable[i].offset, SEEK_SET);
			_file.read(_compInput, _compTable[i].size);
			_outputSize = BundleCodecs::decompressCodec(_compTable[i].codec, _compInput, _compOutput, _compTable[i].size);
			_lastBlock = i;
		}

		output_size = _outputSize;

		if (header_outside) {
			output_size -= skip;
		} else {
			if ((header_size != 0) && (skip >= header_size))
				output_size -= skip;
		}

		if (output_size > size)
			output_size = size;

		assert(final_size + output_size <= blocks_final_size);

		memcpy(*comp_final + final_size, _compOutput + skip, output_size);
		final_size += output_size;

		size -= output_size;
		assert(size >= 0);
		if (size == 0)
			break;

		skip = 0;
	}

	return final_size;
}

int32 BundleMgr::decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final, bool header_outside) {
	int32 final_size = 0, i;

	if (!_file.isOpen()) {
		warning("BundleMgr::decompressSampleByName() File is not open!");
		return 0;
	}

	for (i = 0; i < _numFiles; i++) {
		if (!scumm_stricmp(name, _bundleTable[i].filename)) {
			final_size = decompressSampleByIndex(i, offset, size, comp_final, 0, header_outside);
			return final_size;
		}
	}
	debug(2, "BundleMgr::decompressSampleByName() Failed finding voice %s", name);
	return final_size;
}

} // End of namespace Scumm
