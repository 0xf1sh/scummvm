/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#include "backends/fs/fs.h"
#include "common/util.h"

void FSList::sort() {
	// Simple selection sort
	for (int i = 0; i < _size-1; i++) {
		int min = i;
		for (int j = i+1; j < _size; j++)
			if (_data[j] < _data[min])
				min = j;
		if (min != i)
			SWAP(_data[min], _data[i]);
	}
}


FilesystemNode AbstractFilesystemNode::wrap(AbstractFilesystemNode *node) {
	FilesystemNode wrapper;
	wrapper._realNode = node;
	return wrapper;
}


FilesystemNode::FilesystemNode() {
	_realNode = getRoot();
	_refCount = new int(1);
}

FilesystemNode::FilesystemNode(const FilesystemNode &node)
	: AbstractFilesystemNode() {
	_realNode = node._realNode;
	_refCount = node._refCount;
	++(*_refCount);
}

FilesystemNode::FilesystemNode(const String &p) {
	_realNode = getNodeForPath(p);
	_refCount = new int(1);
}

FilesystemNode::~FilesystemNode() {
	decRefCount();
}

void FilesystemNode::decRefCount() {
	--(*_refCount);
	if (*_refCount <= 0) {
		delete _refCount;
		delete _realNode;
	}
}

FilesystemNode &FilesystemNode::operator  =(const FilesystemNode &node) {
	++(*node._refCount);

	decRefCount();

	_realNode = node._realNode;
	_refCount = node._refCount;

	return *this;
}

FilesystemNode FilesystemNode::getParent() const {
	AbstractFilesystemNode *node = _realNode->parent();
	if (node == 0) {
		return *this;
	} else {
		return AbstractFilesystemNode::wrap(node);
	}
}
