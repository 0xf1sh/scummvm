/* Copyright (C) 1994-2004 Revolution Software Ltd
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

// The new memory manager, now only used by the resource manager. The original
// one would allocated a 12 MB memory pool at startup, which may have been
// appropriate for the original Playstation version but didn't work very well
// with our PocketPC version.
//
// There is one thing that prevents us from replacing the whole memory manager
// with the standard memory allocation functions: Broken Sword II absolutely,
// positively needs to be able to encode pointers as 32-bit integers. The
// original engine did this simply by casting between pointers and integers,
// but as far as I know that's not a very portable thing to do.
//
// If it had only used pointers as opcode parameters it would have been
// possible, albeit messy, to extend the stack data type. However, there is
// code in walker.cpp that obviously violates that assumption, and there are
// probably other cases as well.
//
// Instead, we take advantage of the fact that the original memory manager
// could only handle up to 999 blocks of memory. That means we can encode a
// pointer as a 10-bit id and a 22-bit offset into the block. Judging by early
// testing, both should be plenty.
//
// The number zero is used to represent the NULL pointer.

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/console.h"

namespace Sword2 {

#define MAX_BLOCKS 999

#define Debug_Printf _vm->_debugger->DebugPrintf

MemoryManager::MemoryManager(Sword2Engine *vm) : _vm(vm) {
	// The id stack contains all the possible ids for the memory blocks.
	// We use this to ensure that no two blocks ever have the same id.

	// The memory blocks are stored in an array, indexed on the block's
	// id. This means that given a block id we can find the pointer with a
	// simple array lookup.

	// The memory block index is an array of pointers to the memory block
	// array, sorted on the memory block's pointer. This means that given
	// a pointer into a memory block we can find its id with binary
	// searching.
	//
	// A balanced tree might have been more efficient - the index has to
	// be re-sorted every time a block is allocated or freed - but such
	// beasts are tricky to implement. Anyway, it wouldn't have made
	// encoding or decoding pointers any faster, and these are by far the
	// most common operations.

	_idStack = (int16 *) malloc(MAX_BLOCKS * sizeof(int16));
	_memBlocks = (MemBlock *) malloc(MAX_BLOCKS * sizeof(MemBlock));
	_memBlockIndex = (MemBlock **) malloc(MAX_BLOCKS * sizeof(MemBlock *));

	_totAlloc = 0;
	_numBlocks = 0;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		_idStack[i] = MAX_BLOCKS - i - 1;
		_memBlocks[i].ptr = NULL;
		_memBlockIndex[i] = NULL;
	}

	_idStackPtr = MAX_BLOCKS;
}

MemoryManager::~MemoryManager() {
	for (int i = 0; i < MAX_BLOCKS; i++)
		free(_memBlocks[i].ptr);
	free(_memBlocks);
	free(_memBlockIndex);
	free(_idStack);
}

int32 MemoryManager::encodePtr(byte *ptr) {
	if (ptr == NULL)
		return 0;

	int idx = findPointerInIndex(ptr);

	assert(idx != -1);

	uint32 id = _memBlockIndex[idx]->id;
	uint32 offset = ptr - _memBlocks[id].ptr;

	assert(id < 0x03ff);
	assert(offset <= 0x003fffff);
	assert(offset < _memBlocks[id].size);

	return ((id + 1) << 22) | (ptr - _memBlocks[id].ptr);
}

byte *MemoryManager::decodePtr(int32 n) {
	if (n == 0)
		return NULL;

	uint32 id = ((n & 0xffc00000) >> 22) - 1;
	uint32 offset = n & 0x003fffff;

	assert(_memBlocks[id].ptr);
	assert(offset < _memBlocks[id].size);

	return _memBlocks[id].ptr + offset;
}

int16 MemoryManager::findExactPointerInIndex(byte *ptr) {
	int left = 0;
	int right = _numBlocks - 1;

	while (right >= left) {
		int n = (left + right) / 2;

		if (_memBlockIndex[n]->ptr == ptr)
			return n;

		if (_memBlockIndex[n]->ptr > ptr)
			right = n - 1;
		else
			left = n + 1;
	}

	return -1;
}

int16 MemoryManager::findPointerInIndex(byte *ptr) {
	int left = 0;
	int right = _numBlocks - 1;

	while (right >= left) {
		int n = (left + right) / 2;

		if (_memBlockIndex[n]->ptr <= ptr && _memBlockIndex[n]->ptr + _memBlockIndex[n]->size > ptr)
			return n;

		if (_memBlockIndex[n]->ptr > ptr)
			right = n - 1;
		else
			left = n + 1;
	}

	return -1;
}

int16 MemoryManager::findInsertionPointInIndex(byte *ptr) {
	if (_numBlocks == 0)
		return 0;

	int left = 0;
	int right = _numBlocks - 1;
	int n = 0;

	while (right >= left) {
		n = (left + right) / 2;

		if (_memBlockIndex[n]->ptr == ptr)
			return -1;

		if (_memBlockIndex[n]->ptr > ptr)
			right = n - 1;
		else
			left = n + 1;
	}

	if (_memBlockIndex[n]->ptr < ptr)
		n++;

	return n;
}

byte *MemoryManager::memAlloc(uint32 size, int16 uid) {
	assert(_idStackPtr > 0);

	// Get the new block's id from the stack.
	int16 id = _idStack[--_idStackPtr];

	// Allocate the new memory block
	byte *ptr = (byte *) malloc(size);

	assert(ptr);

	_memBlocks[id].id = id;
	_memBlocks[id].uid = uid;
	_memBlocks[id].ptr = ptr;
	_memBlocks[id].size = size;

	// Update the memory block index.
	int16 idx = findInsertionPointInIndex(ptr);

	assert(idx != -1);

	for (int i = _numBlocks; i > idx; i--)
		_memBlockIndex[i] = _memBlockIndex[i - 1];

	_memBlockIndex[idx] = &_memBlocks[id];
	_numBlocks++;
	_totAlloc += size;

	return _memBlocks[id].ptr;
}

void MemoryManager::memFree(byte *ptr) {
	int16 idx = findExactPointerInIndex(ptr);

	if (idx == -1) {
		warning("Freeing non-allocated pointer %p", ptr);
		return;
	}

	// Put back the id on the stack
	_idStack[_idStackPtr++] = _memBlockIndex[idx]->id;

	// Release the memory block
	free(_memBlockIndex[idx]->ptr);
	_memBlockIndex[idx]->ptr = NULL;

	_totAlloc -= _memBlockIndex[idx]->size;
	
	// Remove the memory block from the index
	_numBlocks--;

	for (int i = idx; i < _numBlocks; i++)
		_memBlockIndex[i] = _memBlockIndex[i + 1];
}

static int compare_blocks(const void *p1, const void *p2) {
	const MemBlock *m1 = *(const MemBlock * const *) p1;
	const MemBlock *m2 = *(const MemBlock * const *) p2;

	if (m1->size < m2->size)
		return 1;
	if (m1->size > m2->size)
		return -1;
	return 0;
}

void MemoryManager::memDisplay() {
	MemBlock **blocks = (MemBlock **) malloc(_numBlocks * sizeof(MemBlock));
	int i, j;

	for (i = 0, j = 0; i < MAX_BLOCKS; i++) {
		if (_memBlocks[i].ptr)
			blocks[j++] = &_memBlocks[i];
	}

	qsort(blocks, _numBlocks, sizeof(MemBlock *), compare_blocks);

	Debug_Printf("     size id  res  type                 name\n");
	Debug_Printf("---------------------------------------------------------------------------\n");

	for (i = 0; i < _numBlocks; i++) {
		StandardHeader *head = (StandardHeader *) blocks[i]->ptr;
		const char *type;

		switch (head->fileType) {
		case ANIMATION_FILE:
			type = "ANIMATION_FILE";
			break;
		case SCREEN_FILE:
			type = "SCREEN_FILE";
			break;
		case GAME_OBJECT:
			type  = "GAME_OBJECT";
			break;
		case WALK_GRID_FILE:
			type = "WALK_GRID_FILE";
			break;
		case GLOBAL_VAR_FILE:
			type = "GLOBAL_VAR_FILE";
			break;
		case PARALLAX_FILE_null:
			type = "PARALLAX_FILE_null";
			break;
		case RUN_LIST:
			type = "RUN_LIST";
			break;
		case TEXT_FILE:
			type = "TEXT_FILE";
			break;
		case SCREEN_MANAGER:
			type = "SCREEN_MANAGER";
			break;
		case MOUSE_FILE:
			type = "MOUSE_FILE";
			break;
		case WAV_FILE:
			type = "WAV_FILE";
			break;
		case ICON_FILE:
			type = "ICON_FILE";
			break;
		case PALETTE_FILE:
			type = "PALETTE_FILE";
			break;
		default:
			type = "<unknown>";
			break;
		}

		Debug_Printf("%9ld %-3d %-4d %-20s %s\n", blocks[i]->size, blocks[i]->id, blocks[i]->uid, type, head->name);
	}
		
	free(blocks);

	Debug_Printf("---------------------------------------------------------------------------\n");
	Debug_Printf("%9ld\n", _totAlloc);
}

void MemoryManager::memStatusStr(char *buf) {
	if (_totAlloc < 1024)
		sprintf(buf, "%u bytes in %d memory blocks", _totAlloc, _numBlocks);
	else if (_totAlloc < 1024 * 1024)
		sprintf(buf, "%uK in %d memory blocks", _totAlloc / 1024, _numBlocks);
	else
		sprintf(buf, "%.02fM in %d memory blocks", _totAlloc / 1048576., _numBlocks);
}

} // End of namespace Sword2
