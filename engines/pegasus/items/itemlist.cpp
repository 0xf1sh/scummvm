/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/error.h"
#include "common/stream.h"

#include "engines/pegasus/items/item.h"
#include "engines/pegasus/items/itemlist.h"

namespace Pegasus {

// TODO: Don't use global construction!
ItemList g_allItems;

ItemList::ItemList() {
}

ItemList::~ItemList() {
}

Common::Error ItemList::writeToStream(Common::WriteStream *stream) {
	stream->writeUint32BE(size());

	for (ItemIterator it = begin(); it != end(); it++) {
		stream->writeUint16BE((*it)->getObjectID());
		(*it)->writeToStream(stream);
	}

	if (stream->err())
		return Common::kWritingFailed;
	
	return Common::kNoError;
}

Common::Error ItemList::readFromStream(Common::ReadStream *stream) {
	uint32 itemCount = stream->readUint32BE();

	for (uint32 i = 0; i < itemCount; i++) {
		tItemID itemID = stream->readUint16BE();
		g_allItems.findItemByID(itemID)->readFromStream(stream);
	}

	if (stream->err())
		return Common::kReadingFailed;
	
	return Common::kNoError;
}

Item *ItemList::findItemByID(const tItemID id) {
	for (ItemIterator it = begin(); it != end(); it++)
		if ((*it)->getObjectID() == id)
			return *it;

	return 0;
}

} // End of namespace Pegasus
