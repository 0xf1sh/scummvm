/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "scumm.h"
#include "actor.h"
#include "bomp.h"
#include "object.h"
#include "resource.h"
#include "usage_bits.h"

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct BompHeader {			/* Bomp header */
	union {
		struct {
			uint16 unk;
			uint16 width, height;
		} GCC_PACK old;

		struct {
			uint32 width, height;
		} GCC_PACK v8;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif


bool Scumm::getClass(int obj, int cls)
{
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getClass");
	cls &= 0x7F;
	checkRange(32, 1, cls, "Class %d out of range in getClass");

	if (_features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
			case kObjectClassUntouchable:
				cls = 24;
				break;
			case kObjectClassPlayer:
				cls = 23;
				break;
			case kObjectClassXFlip:
				cls = 19;
				break;
			case kObjectClassYFlip:
				cls = 18;
				break;
		}
	}

	return (_classData[obj] & (1 << (cls - 1))) != 0;
}

void Scumm::putClass(int obj, int cls, bool set)
{
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putClass");
	cls &= 0x7F;
	checkRange(32, 1, cls, "Class %d out of range in putClass");

	if (_features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
			case kObjectClassUntouchable:
				cls = 24;
				break;
			case kObjectClassPlayer:
				cls = 23;
				break;
			case kObjectClassXFlip:
				cls = 19;
				break;
			case kObjectClassYFlip:
				cls = 18;
				break;
		}
	}

	// FIXME: It isn't enough for the Indy3 intro to make the
	// little trains ignore boxes, they have to always clip as
	// well. Is this yet another walkbox 0 error?
	if (_gameId == GID_INDY3_256 && cls == kObjectClassIgnoreBoxes && _currentRoom == 76)
		putClass(obj, kObjectClassAlwaysClip, set);

	if (set)
		_classData[obj] |= (1 << (cls - 1));
	else
		_classData[obj] &= ~(1 << (cls - 1));

	if (1 <= obj && obj < _numActors) {
		_actors[obj].classChanged(cls, set);
	}
}

int Scumm::getOwner(int obj) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getOwner");
	return _objectOwnerTable[obj];
}

void Scumm::putOwner(int obj, int owner) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putOwner");
	checkRange(0xFF, 0, owner, "Owner %d out of range in putOwner");
	_objectOwnerTable[obj] = owner;
}

int Scumm::getState(int obj) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getState");
	return _objectStateTable[obj];
}

void Scumm::putState(int obj, int state) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putState");
	checkRange(0xFF, 0, state, "State %d out of range in putState");
	_objectStateTable[obj] = state;
}

int Scumm::getObjectRoom(int obj) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getObjectRoom");
	return _objectRoomTable[obj];
}

int Scumm::getObjectIndex(int object) {
	int i;

	if (object < 1)
		return -1;

	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object)
			return i;
	}
	return -1;
}

int Scumm::whereIsObject(int object) {
	int i;

	if (object >= _numGlobalObjects)
		return WIO_NOT_FOUND;

	if (object < 1)
		return WIO_NOT_FOUND;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM) {
		for (i = 0; i < _maxInventoryItems; i++)
			if (_inventory[i] == object)
				return WIO_INVENTORY;
		return WIO_NOT_FOUND;
	}

	for (i = (_numLocalObjects-1); i > 0; i--)
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index)
				return WIO_FLOBJECT;
			return WIO_ROOM;
		}

	return WIO_NOT_FOUND;
}

int Scumm::getObjectOrActorXY(int object, int &x, int &y) {
	if (object < _numActors) {
		Actor *act = derefActorSafe(object, "getObjectOrActorXY");
		if (!act) 
			return 0; 
		else
			return act->getActorXYPos(x, y);
	}

	switch (whereIsObject(object)) {
	case WIO_NOT_FOUND:
		return -1;
	case WIO_INVENTORY:
		if (_objectOwnerTable[object] < _numActors)
			return derefActorSafe(_objectOwnerTable[object], "getObjectOrActorXY(2)")->getActorXYPos(x, y);
		else
			return 0xFF;
	}
	getObjectXYPos(object, x, y);
	return 0;
}

/**
 * Return the position of an object.
 * Returns X, Y and direction in angles
 */
void Scumm::getObjectXYPos(int object, int &x, int &y, int &dir) {
	ObjectData *od = &_objs[getObjectIndex(object)];
	int state;
	const byte *ptr;
	const ImageHeader *imhd;

	if (_features & GF_AFTER_V6) {
		state = getState(object) - 1;
		if (state < 0)
			state = 0;

		if (od->fl_object_index) {
			ptr = getResourceAddress(rtFlObject, od->fl_object_index);
			ptr = findResource(MKID('OBIM'), ptr);
		} else {
			ptr = getResourceAddress(rtRoom, _roomResource);
			ptr += od->OBIMoffset;
		}
		if (!ptr) {
			// FIXME: We used to assert here, but it seems that in the nexus
			// in The Dig, this can happen, at least with old savegames, and
			// it's safe to continue...
			warning("getObjectXYPos: Can't find object %d", object);
			return;
		}
		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), ptr);
		if (_features & GF_AFTER_V8) {
			x = od->x_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].x);
			y = od->y_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].y);
		} else if (_features & GF_AFTER_V7) {
			x = od->x_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].x);
			y = od->y_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].y);
		} else {
			x = od->x_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].x);
			y = od->y_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].y);
		}
	} else {
		x = od->walk_x;
		y = od->walk_y;
	}
	if (_features & GF_AFTER_V8)
		dir = fromSimpleDir(1, od->actordir);
	else
		dir = oldDirToNewDir(od->actordir & 3);
}

int Scumm::getObjActToObjActDist(int a, int b) {
	int x, y, x2, y2;
	Actor *acta = NULL;
	Actor *actb = NULL;

	if (a < _numActors)
		acta = derefActorSafe(a, "getObjActToObjActDist");

	if (b < _numActors)
		actb = derefActorSafe(b, "getObjActToObjActDist(2)");

	if (acta && actb && acta->getRoom() == actb->getRoom() && acta->getRoom() && !acta->isInCurrentRoom())
		return 0;

	if (getObjectOrActorXY(a, x, y) == -1)
		return 0xFF;

	if (getObjectOrActorXY(b, x2, y2) == -1)
		return 0xFF;

	if (acta) {
		AdjustBoxResult r = acta->adjustXYToBeInBox(x2, y2);
		x2 = r.x;
		y2 = r.y;
	}

	y = abs(y - y2);
	x = abs(x - x2);

	if (y > x)
		x = y;
	return x;
}

int Scumm::findObject(int x, int y) {
	int i, b;
	byte a;
	const int mask = (_features & GF_AFTER_V2) ? 0x8 : 0xF;

	for (i = 1; i < _numLocalObjects; i++) {
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;
		if (_features & GF_AFTER_V2 && _objs[i].state & 0x2)
			continue;
		b = i;
		do {
			a = _objs[b].parentstate;
			b = _objs[b].parent;
			if (b == 0) {
				if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
				    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					return _objs[i].obj_nr;
				break;
			}
		} while ((_objs[b].state & mask) == a);
	}
	return 0;
}

void Scumm::drawRoomObject(int i, int arg) {
	ObjectData *od;
	byte a;
	const int mask = (_features & GF_AFTER_V2) ? 0x8 : 0xF;

	od = &_objs[i];
	if ((i < 1) || (od->obj_nr < 1) || !od->state)
		return;

	do {
		a = od->parentstate;
		if (!od->parent) {
			drawObject(i, arg);
			break;
		}
		od = &_objs[od->parent];
	} while ((od->state & mask) == a);
}

void Scumm::drawRoomObjects(int arg) {
	int i;
	const int mask = (_features & GF_AFTER_V2) ? 0x8 : 0xF;

	if (_features & GF_DRAWOBJ_OTHER_ORDER) {
		for (i = 1; i < _numLocalObjects; i++)
			if (_objs[i].obj_nr > 0)
				drawRoomObject(i, arg);
	} else {
		for (i = (_numLocalObjects-1); i > 0; i--)
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask)) {
				drawRoomObject(i, arg);
			}
	}
}

static const uint32 IMxx_tags[] = {
	MKID('IM00'),
	MKID('IM01'),
	MKID('IM02'),
	MKID('IM03'),
	MKID('IM04'),
	MKID('IM05'),
	MKID('IM06'),
	MKID('IM07'),
	MKID('IM08'),
	MKID('IM09'),
	MKID('IM0A'),
	MKID('IM0B'),
	MKID('IM0C'),
	MKID('IM0D'),
	MKID('IM0E'),
	MKID('IM0F')
};

void Scumm::drawObject(int obj, int arg) {
	ObjectData *od;
	int xpos, ypos, height, width;
	const byte *ptr;
	int x, a, numstrip;
	int tmp;

	if (_BgNeedsRedraw)
		arg = 0;

	od = &_objs[obj];

	if (od->obj_nr == 0)
		return;

	checkRange(_numGlobalObjects - 1, 0, od->obj_nr, "Object %d out of range in drawObject");

	xpos = od->x_pos >> 3;
	ypos = od->y_pos;

	width = od->width >> 3;
	height = od->height &= 0xFFFFFFF8;	// Mask out last 3 bits

	if (width == 0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;

	if (od->fl_object_index) {
		ptr = getResourceAddress(rtFlObject, od->fl_object_index);
		ptr = findResource(MKID('OBIM'), ptr);
	} else {
		ptr = getResourceAddress(rtRoom, _roomResource);
		ptr = ptr + od->OBIMoffset;
	}

	if (_features & GF_OLD_BUNDLE)
		ptr += 0;
	else if (_features & GF_SMALL_HEADER)
		ptr += 8;
	else if (_features & GF_AFTER_V8) {
		ptr = findResource(MKID('IMAG'), ptr);
		if (!ptr)
			return;

		ptr = findResource(MKID('WRAP'), ptr);
		assert(ptr);
		ptr = findResource(MKID('OFFS'), ptr);
		assert(ptr);
		// Get the address of the specified SMAP (corresponding to IMxx)
		ptr += READ_LE_UINT32(ptr + 4 + 4*getState(od->obj_nr));
	} else
		ptr = findResource(IMxx_tags[getState(od->obj_nr)], ptr);
	if (!ptr)
		return;

	x = 0xFFFF;

	for (a = numstrip = 0; a < width; a++) {
		tmp = xpos + a;
		if (arg == 1 && _screenStartStrip != tmp)
			continue;
		if (arg == 2 && _screenEndStrip != tmp)
			continue;
		if (tmp < _screenStartStrip || tmp > _screenEndStrip)
			continue;
		setGfxUsageBit(tmp, USAGE_BIT_DIRTY);
		if (tmp < x)
			x = tmp;
		numstrip++;
	}

	if (numstrip != 0) {
		byte flags = Gdi::dbAllowMaskOr;
		// Sam & Max needs this to fix object-layering problems with
		// the inventory and conversation icons.
		if ((_features & GF_AFTER_V7 || _gameId == GID_SAMNMAX) && getClass(od->obj_nr, kObjectClassIgnoreBoxes))
			flags |= Gdi::dbDrawMaskOnAll;
		gdi.drawBitmap(ptr, &virtscr[0], x, ypos, width << 3, height, x - xpos, numstrip, flags);
	}
}

void Scumm::clearRoomObjects() {
	int i;

	if (_features & GF_SMALL_HEADER) {
		for (i = 0; i < _numLocalObjects; i++) {
			_objs[i].obj_nr = 0;
		}
	} else {
		// FIXME: Locking/FlObjects stuff?
		for (i = 0; i < _numLocalObjects; i++) {
			if (_objs[i].obj_nr < 1)	// Optimise codepath
				continue;

			// Nuke all non-flObjects (flObjects are nuked in script.cpp)
			if (_objs[i].fl_object_index == 0) {
				_objs[i].obj_nr = 0;
			} else {
				// Nuke all unlocked flObjects
				if (!(res.flags[rtFlObject][_objs[i].fl_object_index] & RF_LOCK)) {
					nukeResource(rtFlObject, _objs[i].fl_object_index);
					_objs[i].obj_nr = 0;
					_objs[i].fl_object_index = 0;
				} 
			}
		}
	}
}

void Scumm::loadRoomObjects() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room, *searchptr, *rootptr;
	const ImageHeader *imhd;
	const RoomHeader *roomhdr;
	const CodeHeader *cdhd;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);
	roomhdr = (const RoomHeader *)findResourceData(MKID('RMHD'), room);

	if (_features & GF_AFTER_V8)
		_numObjectsInRoom = (byte)READ_LE_UINT32(&(roomhdr->v8.numObjects));
	else if (_features & GF_AFTER_V7)
		_numObjectsInRoom = READ_LE_UINT16(&(roomhdr->v7.numObjects));
	else
		_numObjectsInRoom = READ_LE_UINT16(&(roomhdr->old.numObjects));

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_features & GF_AFTER_V8)
		searchptr = rootptr = getResourceAddress(rtRoomScripts, _roomResource);
	else
		searchptr = rootptr = room;
	assert(searchptr);

	// Load in new room objects
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = findResource(MKID('OBCD'), searchptr);
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - rootptr;
		cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), ptr);

		if (_features & GF_AFTER_V7)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));
		else if (_features & GF_AFTER_V6)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));
		else
			od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			ptr = findResource(MKID('VERB'), ptr, 0);
			dumpResource(buf, od->obj_nr, ptr);
		}

		searchptr = NULL;
	}

	searchptr = room;
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = findResource(MKID('OBIM'), searchptr);
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), ptr);
		if (_features & GF_AFTER_V8)
			// In V8, IMHD has no obj_id, but rather a name string. We map the name
			// back to an object id using a table derived from the DOBJ resource.
			obim_id = _objectIDMap[imhd->v8.name];
		else if (_features & GF_AFTER_V7)
			obim_id = READ_LE_UINT16(&imhd->v7.obj_id);
		else
			obim_id = READ_LE_UINT16(&imhd->old.obj_id);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
		searchptr = NULL;
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index)
			setupRoomObject(&_objs[i], room);
	}

	CHECK_HEAP
}

void Scumm::loadRoomObjectsOldBundle() {
	int i;
	ObjectData *od;
	const byte *room, *ptr;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);

	_numObjectsInRoom = room[20];

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_features & GF_AFTER_V2)
		ptr = room + 28;
	else
		ptr = room + 29;

	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		od->OBIMoffset = READ_LE_UINT16(ptr);
		od->OBCDoffset = READ_LE_UINT16(ptr + 2 * _numObjectsInRoom);
		setupRoomObject(od, room);

		ptr += 2;

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, room + od->OBCDoffset);
		}
	}

	CHECK_HEAP
}

void Scumm::loadRoomObjectsSmall() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room, *searchptr;
	const RoomHeader *roomhdr;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);
	roomhdr = (const RoomHeader *)findResourceData(MKID('RMHD'), room);

	_numObjectsInRoom = READ_LE_UINT16(&(roomhdr->old.numObjects));

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	searchptr = room;
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = findResourceSmall(MKID('OBCD'), searchptr);
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - room;
		od->obj_nr = READ_LE_UINT16(ptr + 6);
		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		}

		searchptr = NULL;
	}

	searchptr = room;
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = findResourceSmall(MKID('OBIM'), searchptr);
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		obim_id = READ_LE_UINT16(ptr + 6);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
		searchptr = NULL;
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index) {
			setupRoomObject(&_objs[i], room);
		}
	}

	CHECK_HEAP
}

void Scumm::setupRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	const CodeHeader *cdhd = NULL;
	const ImageHeader *imhd = NULL;

	assert(room);

	if (_features & GF_SMALL_HEADER) {

		const byte *ptr = room + od->OBCDoffset;

		if (_features & GF_OLD_BUNDLE)
			ptr -= 2;

		od->obj_nr = READ_LE_UINT16(ptr + 6);

		od->x_pos = *(ptr + 9) << 3;
		od->y_pos = ((*(ptr + 10)) & 0x7F) << 3;

		od->parentstate = (*(ptr + 10) & 0x80) ? 1 : 0;
		if (_features & GF_AFTER_V2)
			od->parentstate <<= 3;

		od->width = *(ptr + 11) << 3;

		od->parent = *(ptr + 12);

		if (_features & GF_AFTER_V2) {
			od->walk_x = *(ptr + 13) * 8;
			od->walk_y = (*(ptr + 14) & 0x1f) * 8;
			od->actordir = (*(ptr + 15)) & 7;
			od->height = *(ptr + 15) & 0xf8;
		} else {
			od->walk_x = READ_LE_UINT16(ptr + 13);
			od->walk_y = READ_LE_UINT16(ptr + 15);
			od->actordir = (*(ptr + 17)) & 7;
			od->height = *(ptr + 17) & 0xf8;
		}

		return;
	}

	if (searchptr == NULL) {
		if (_features & GF_AFTER_V8)
			searchptr = getResourceAddress(rtRoomScripts, _roomResource);
		else
			searchptr = room;
	}
		
	cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), searchptr + od->OBCDoffset);
	if (cdhd == NULL)
		error("Room %d missing CDHD blocks(s)", _roomResource);

	if (_features & GF_AFTER_V8) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), room + od->OBIMoffset);
		od->x_pos = (int)READ_LE_UINT32(&imhd->v8.x_pos);
		od->y_pos = (int)READ_LE_UINT32(&imhd->v8.y_pos);
		od->width = (uint)READ_LE_UINT32(&imhd->v8.width);
		od->height = (uint)READ_LE_UINT32(&imhd->v8.height);
		// HACK: This is done sinec an angle doesn't fit into a byte (360 > 256)
		od->actordir = toSimpleDir(1, READ_LE_UINT32(&imhd->v8.actordir));

	} else if (_features & GF_AFTER_V7) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), room + od->OBIMoffset);
		od->x_pos = READ_LE_UINT16(&imhd->v7.x_pos);
		od->y_pos = READ_LE_UINT16(&imhd->v7.y_pos);
		od->width = READ_LE_UINT16(&imhd->v7.width);
		od->height = READ_LE_UINT16(&imhd->v7.height);
		od->actordir = READ_LE_UINT16(&imhd->v7.actordir);

	} else if (_features & GF_AFTER_V6) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));

		od->width = READ_LE_UINT16(&cdhd->v6.w);
		od->height = READ_LE_UINT16(&cdhd->v6.h);
		od->x_pos = ((int16)READ_LE_UINT16(&cdhd->v6.x));
		od->y_pos = ((int16)READ_LE_UINT16(&cdhd->v6.y));
		if (cdhd->v6.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v6.flags & 0xF);
		}
		od->parent = cdhd->v6.parent;
		od->actordir = cdhd->v6.actordir;
	} else {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		od->width = cdhd->v5.w << 3;
		od->height = cdhd->v5.h << 3;
		od->x_pos = cdhd->v5.x << 3;
		od->y_pos = cdhd->v5.y << 3;
		if (cdhd->v5.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v5.flags & 0xF);
		}
		od->parent = cdhd->v5.parent;
		od->walk_x = READ_LE_UINT16(&cdhd->v5.walk_x);
		od->walk_y = READ_LE_UINT16(&cdhd->v5.walk_y);
		od->actordir = cdhd->v5.actordir;
	}

	od->fl_object_index = 0;
}

void Scumm::fixObjectFlags() {
	int i;
	ObjectData *od = &_objs[1];
	for (i = 1; i < _numLocalObjects; i++, od++) {
		if (od->obj_nr > 0)
			od->state = _objectStateTable[od->obj_nr];
	}
}

void Scumm::processDrawQue() {
	int i, j;
	for (i = 0; i < _drawObjectQueNr; i++) {
		j = _drawObjectQue[i];
		if (j)
			drawObject(j, 0);
	}
	_drawObjectQueNr = 0;
}

void Scumm::clearOwnerOf(int obj) {
	int i, j;
	uint16 *a;

	stopObjectScript(obj);

	if (getOwner(obj) == OF_OWNER_ROOM) {
		i = 0;
		do {
			if (_objs[i].obj_nr == obj) {
				if (!_objs[i].fl_object_index)
					return;
				nukeResource(rtFlObject, _objs[i].fl_object_index);
				_objs[i].obj_nr = 0;
				_objs[i].fl_object_index = 0;
			}
		} while (++i < _numLocalObjects);
		return;
	}

	for (i = 1; i < _maxInventoryItems; i++) {
		if (_inventory[i] == obj) {
			j = whereIsObject(obj);
			if (j == WIO_INVENTORY) {
				nukeResource(rtInventory, i);
				_inventory[i] = 0;
			}
			a = &_inventory[1];
			for (i = 1; i < _maxInventoryItems - 1; i++, a++) {
				if (!a[0] && a[1]) {
					a[0] = a[1];
					a[1] = 0;
					_baseInventoryItems[i] = _baseInventoryItems[i + 1];
					_baseInventoryItems[i + 1] = NULL;
				}
			}
			return;
		}
	}
}

void Scumm::removeObjectFromRoom(int obj) {
	int i, j, strip;

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr == (uint16)obj) {
			if (_objs[i].width != 0) {
				for (j = 0; j < _objs[i].width >> 3; j++) {
					strip = (_objs[i].x_pos >> 3) + j;

					// Clip value
					if (strip < _screenStartStrip)
						continue;
					if (strip > _screenEndStrip)
						break;

					setGfxUsageBit(strip, USAGE_BIT_DIRTY);
				}
			}
			_BgNeedsRedraw = true;
			return;
		}
	}
}

void Scumm::addObjectToDrawQue(int object) {
	_drawObjectQue[_drawObjectQueNr++] = object;
	if ((unsigned int)_drawObjectQueNr > ARRAYSIZE(_drawObjectQue))
		error("Draw Object Que overflow");
}

void Scumm::clearDrawObjectQueue() {
	_drawObjectQueNr = 0;
}

byte *Scumm::getObjOrActorName(int obj) {
	byte *objptr;
	int i;

	if (obj < _numActors)
		return derefActor(obj, "getObjOrActorName")->getActorName();

	if (_features & GF_AFTER_V6) {
		for (i = 0; i < _numNewNames; i++) {
			if (_newNames[i] == obj) {
				debug(5, "Found new name for object %d at _newNames[i]", obj, i);
				return getResourceAddress(rtObjectName, i);
				break;
			}
		}
	}

	objptr = getOBCDFromObject(obj);
	if (objptr == NULL)
		return NULL;

	if (_features & GF_SMALL_HEADER) {
		byte offset = 0;

		if (_features & GF_AFTER_V2)
			offset = *(objptr + 14);
		else if (_features & GF_OLD_BUNDLE)
			offset = *(objptr + 16);
		else
			offset = READ_LE_UINT16(objptr + 18);

		return (objptr + offset);
	}

#if 0
	return findResourceData(MKID('OBNA'), objptr);
#else
	// FIXME: we can't use findResourceData anymore, because it returns const
	// data, while this function *must* return a non-const pointer. That is so
	// because in o2_setObjectName / o5_setObjectName we directly modify this
	// data. Now, we could add a non-const version of findResourceData, too
	// (C++ makes that easy); but this here is really the *only* place in all
	// of ScummVM where it wold be needed! That seems kind of a waste...
	//
	// So for now, I duplicate some code from findResourceData / findResource
	// here. However, a much nicer solution might be (with stress on "might")
	// to use the same technique as in V6 games: that is, use a seperate
	// resource for changed names. That would be the cleanest solution, but
	// might proof to be infeasible, as it might lead to unforseen regressions.
	
	uint32 tag = MKID('OBNA');
	byte *searchin = objptr;
	uint32 curpos, totalsize, size;

	assert(searchin);

	searchin += 4;
	totalsize = READ_BE_UINT32_UNALIGNED(searchin);
	curpos = 8;
	searchin += 4;

	while (curpos < totalsize) {
		if (READ_UINT32_UNALIGNED(searchin) == tag)
			return searchin + _resourceHeaderSize;

		size = READ_BE_UINT32_UNALIGNED(searchin + 4);
		if ((int32)size <= 0) {
			error("(%c%c%c%c) Not found in %d... illegal block len %d",
						tag & 0xFF, (tag >> 8) & 0xFF, (tag >> 16) & 0xFF, (tag >> 24) & 0xFF, 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
#endif
}

uint32 Scumm::getOBCDOffs(int object) {
	int i;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM)
		return 0;
	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index != 0)
				return 8;
			return _objs[i].OBCDoffset;
		}
	}
	return 0;
}

byte *Scumm::getOBCDFromObject(int obj) {
	int i;

	if (_objectOwnerTable[obj] != OF_OWNER_ROOM) {
		for (i = 0; i < _maxInventoryItems; i++) {
			if (_inventory[i] == obj)
				return getResourceAddress(rtInventory, i);
		}
	} else {
		for (i = (_numLocalObjects-1); i > 0; --i) {
			if (_objs[i].obj_nr == obj) {
				if (_objs[i].fl_object_index)
					return getResourceAddress(rtFlObject, _objs[i].fl_object_index) + 8;
				if (_features & GF_AFTER_V8)
					return getResourceAddress(rtRoomScripts, _roomResource) + _objs[i].OBCDoffset;
				else
					return getResourceAddress(rtRoom, _roomResource) + _objs[i].OBCDoffset;
			}
		}
	}
	return 0;
}

void Scumm::addObjectToInventory(uint obj, uint room) {
	int i, slot;
	uint32 size;
	const byte *ptr;
	byte *dst;
	FindObjectInRoom foir;

	debug(1, "Adding object %d from room %d into inventory", obj, room);

	CHECK_HEAP
	if (whereIsObject(obj) == WIO_FLOBJECT) {
		i = getObjectIndex(obj);
		ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index) + 8;
		size = READ_BE_UINT32_UNALIGNED(ptr + 4);
	} else {
		findObjectInRoom(&foir, foCodeHeader, obj, room);
		if (_features & GF_OLD_BUNDLE)
			size = READ_LE_UINT16(foir.obcd);
		else if (_features & GF_SMALL_HEADER)
			size = READ_LE_UINT32(foir.obcd);
		else
			size = READ_BE_UINT32_UNALIGNED(foir.obcd + 4);
		ptr = foir.obcd;
	}

	slot = getInventorySlot();
	_inventory[slot] = obj;
	dst = createResource(rtInventory, slot, size);
	assert(dst);
	memcpy(dst, ptr, size);

	CHECK_HEAP
}

void Scumm::findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint id, uint room) {

	const CodeHeader *cdhd;
	int i, numobj;
	const byte *roomptr, *obcdptr, *obimptr, *searchptr;
	const ImageHeader *imhd;
	int id2;
	int id3;

	if (findWhat & foCheckAlreadyLoaded && getObjectIndex(id) != -1) {
		if (_features & GF_OLD_BUNDLE) {
			// I am not sure if this is even needed for old games...
			// but using RES_SIZE definitely won't work with OLD_BUNDLE, since it
			// assumes the size is 32 bit but in old games it's 16 bit
			error("findObjectInRoom foCheckAlreadyLoaded NYI for GF_OLD_BUNDLE (id = %d, room = %d)", id, room);
		}
		fo->obcd = obcdptr = getOBCDFromObject(id);
		assert((const byte *)obcdptr > (byte *)256);
		fo->obim = obimptr = obcdptr + RES_SIZE(obcdptr);
		fo->cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), obcdptr);
		fo->imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), obimptr);
		return;
	}

	fo->roomptr = roomptr = getResourceAddress(rtRoom, room);
	if (!roomptr)
		error("findObjectInRoom: failed getting roomptr to %d", room);

	if (_features & GF_OLD_BUNDLE) {
		numobj = roomptr[20];
	} else {
		const RoomHeader *roomhdr = (const RoomHeader *)findResourceData(MKID('RMHD'), roomptr);
	
		if (_features & GF_AFTER_V8)
			numobj = READ_LE_UINT32(&(roomhdr->v8.numObjects));
		else if (_features & GF_AFTER_V7)
			numobj = READ_LE_UINT16(&(roomhdr->v7.numObjects));
		else
			numobj = READ_LE_UINT16(&(roomhdr->old.numObjects));
	}
	
	if (numobj == 0)
		error("findObjectInRoom: No object found in room %d", room);
	if (numobj > _numLocalObjects)
		error("findObjectInRoom: More (%d) than %d objects in room %d", numobj, _numLocalObjects, room);

	if (_features & GF_OLD_BUNDLE) {
		if (_features & GF_AFTER_V2)
			searchptr = roomptr + 28;
		else
			searchptr = roomptr + 29;

		for (i = 0; i < numobj; i++) {
			obimptr = roomptr + READ_LE_UINT16(searchptr);
			obcdptr = roomptr + READ_LE_UINT16(searchptr + 2 * _numObjectsInRoom);
			id2 = READ_LE_UINT16(obcdptr + 4);

			if (id2 == (uint16)id) {
				if (findWhat & foCodeHeader) {
					fo->obcd = obcdptr;
					fo->cdhd = (const CodeHeader *)(obcdptr + 10);	// TODO - FIXME
				}
				if (findWhat & foImageHeader) {
					fo->obim = obimptr;
					fo->imhd = NULL;
				}
				break;
			}
			searchptr += 2;
		}
		return;
	}

	if (findWhat & foCodeHeader) {
		if (_features & GF_AFTER_V8)
			searchptr = getResourceAddress(rtRoomScripts, room);
		else
			searchptr = roomptr;
		assert(searchptr);
		for (i = 0;;) {
			if (_features & GF_SMALL_HEADER)
				obcdptr = findResourceSmall(MKID('OBCD'), searchptr);
			else
				obcdptr = findResource(MKID('OBCD'), searchptr);
			if (obcdptr == NULL)
				error("findObjectInRoom: Not enough code blocks in room %d", room);
			cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), obcdptr);

			if (_features & GF_SMALL_HEADER)
				id2 = READ_LE_UINT16(obcdptr + 6);
			else if (_features & GF_AFTER_V7)
				id2 = READ_LE_UINT16(&(cdhd->v7.obj_id));
			else if (_features & GF_AFTER_V6)
				id2 = READ_LE_UINT16(&(cdhd->v6.obj_id));
			else
				id2 = READ_LE_UINT16(&(cdhd->v5.obj_id));

			if (id2 == (uint16)id) {
				fo->obcd = obcdptr;
				fo->cdhd = cdhd;
				break;
			}
			if (++i == numobj)
				error("findObjectInRoom: Object %d not found in room %d", id, room);
			searchptr = NULL;
		}
	}

	roomptr = fo->roomptr;
	if (findWhat & foImageHeader) {
		searchptr = roomptr;
		assert(searchptr);
		for (i = 0;;) {
			if (_features & GF_SMALL_HEADER)
				obimptr = findResourceSmall(MKID('OBIM'), searchptr);
			else
				obimptr = findResource(MKID('OBIM'), searchptr);
			if (obimptr == NULL)
				error("findObjectInRoom: Not enough image blocks in room %d", room);
			imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), obimptr);
			if (_features & GF_SMALL_HEADER)
				id3 = READ_LE_UINT16(obimptr + 6);
			else if (_features & GF_AFTER_V8)
				// In V8, IMHD has no obj_id, but rather a name string. We map the name
				// back to an object id using a table derived from the DOBJ resource.
				id3 = _objectIDMap[imhd->v8.name];
			else if (_features & GF_AFTER_V7)
				id3 = READ_LE_UINT16(&imhd->v7.obj_id);
			else
				id3 = READ_LE_UINT16(&imhd->old.obj_id);

			if (id3 == (uint16)id) {
				fo->obim = obimptr;
				fo->imhd = imhd;
				break;
			}
			if (++i == numobj)
				error("findObjectInRoom: Object %d image not found in room %d", id, room);
			searchptr = NULL;
		}
	}
}

int Scumm::getInventorySlot() {
	int i;
	for (i = 1; i <= _maxInventoryItems; i++) {
		if (_inventory[i] == 0)
			return i;
	}
	error("Inventory full, %d max items", _maxInventoryItems);
	return -1;
}

void Scumm::SamInventoryHack(int obj) {
	// FIXME: Sam and Max hack
	int base = 6;

	while (base < 80) {
		int value = readArray(178, 0, base);
		if (value == obj)
			return;
		if (value == 0) {
			_scummVars[179]++;
			writeArray(178, 0, base, obj);
			return;
		}
		base++;
	}
}

void Scumm::setOwnerOf(int obj, int owner) {
	ScriptSlot *ss;
	if (owner == 0) {
		clearOwnerOf(obj);
		ss = &vm.slot[_currentScript];
		if (ss->where == WIO_INVENTORY && _inventory[ss->number] == obj) {
			putOwner(obj, 0);
			runHook(0);
			stopObjectCode();
			return;
		}
	}
	if ((owner == 2) && (_gameId == GID_SAMNMAX))
		SamInventoryHack(obj);

	putOwner(obj, owner);
	runHook(0);
}

int Scumm::getObjX(int obj) {
	if (obj < _numActors) {
		if (obj < 1)
			return 0;									/* fix for indy4's map */
		return derefActor(obj, "getObjX")->x;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return x;
	}
}

int Scumm::getObjY(int obj) {
	if (obj < _numActors) {
		if (obj < 1)
			return 0;									/* fix for indy4's map */
		return derefActor(obj, "getObjY")->y;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return y;
	}
}

int Scumm::getObjOldDir(int obj) {
	return newDirToOldDir(getObjNewDir(obj));
}

int Scumm::getObjNewDir(int obj) {
	int dir;
	if (obj < _numActors) {
		dir = derefActor(obj, "getObjNewDir")->facing;
	} else {
		int x, y;
		getObjectXYPos(obj, x, y, dir);
	}
	return dir;
}

int Scumm::findInventory(int owner, int idx) {
	int count = 1, i, obj;
	for (i = 0; i != _maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner && count++ == idx)
			return obj;
	}
	return 0;
}

int Scumm::getInventoryCount(int owner) {
	int i, obj;
	int count = 0;
	for (i = 0; i != _maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner)
			count++;
	}
	return count;
}

void Scumm::setObjectState(int obj, int state, int x, int y) {
	int i;

	i = getObjectIndex(obj);
	if (i == -1) {
		warning("setObjectState: no such object %d", obj);
		return;
	}

	if (x != -1) {
		_objs[i].x_pos = x << 3;
		_objs[i].y_pos = y << 3;
	}

	addObjectToDrawQue(i);
	putState(obj, state);
}

static int getDist(int x, int y, int x2, int y2) {
	int a = abs(y - y2);
	int b = abs(x - x2);
	if (a > b)
		return a;
	return b;
}

int Scumm::getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f) {
	int i, j;
	int x, y;
	int x2, y2;

	j = i = 0xFF;

	if (is_obj_1) {
		if (getObjectOrActorXY(b, x, y) == -1)
			return -1;
		if (b < _numActors)
			i = derefActor(b, "unkObjProc1")->scalex;
	} else {
		x = b;
		y = c;
	}

	if (is_obj_2) {
		if (getObjectOrActorXY(e, x2, y2) == -1)
			return -1;
		if (e < _numActors)
			j = derefActor(e, "unkObjProc1(2)")->scalex;
	} else {
		x2 = e;
		y2 = f;
	}

	return getDist(x, y, x2, y2) * 0xFF / ((i + j) >> 1);
}

void Scumm::setCursorImg(uint img, uint room, uint imgindex) {
	int w, h;
	const byte *dataptr, *bomp;
	uint32 size;
	FindObjectInRoom foir;

	if (room == (uint) - 1)
		room = getObjectRoom(img);

	findObjectInRoom(&foir, foCodeHeader | foImageHeader | foCheckAlreadyLoaded, img, room);

	if (_features & GF_AFTER_V8) {
		setCursorHotspot(READ_LE_UINT32(&foir.imhd->v8.hotspot[0].x),
		                  READ_LE_UINT32(&foir.imhd->v8.hotspot[0].y));
		w = READ_LE_UINT32(&foir.imhd->v8.width) >> 3;
		h = READ_LE_UINT32(&foir.imhd->v8.height) >> 3;
	} else if (_features & GF_AFTER_V7) {
		setCursorHotspot(READ_LE_UINT16(&foir.imhd->v7.hotspot[0].x),
		                  READ_LE_UINT16(&foir.imhd->v7.hotspot[0].y));
		w = READ_LE_UINT16(&foir.imhd->v7.width) >> 3;
		h = READ_LE_UINT16(&foir.imhd->v7.height) >> 3;
	} else {
		setCursorHotspot(READ_LE_UINT16(&foir.imhd->old.hotspot[0].x),
		                  READ_LE_UINT16(&foir.imhd->old.hotspot[0].y));
		w = READ_LE_UINT16(&foir.cdhd->v6.w) >> 3;
		h = READ_LE_UINT16(&foir.cdhd->v6.h) >> 3;
	}

	// TODO - for V8 don't use IMxx_tags. Rather, we do something similiar to the V8
	// code in drawBlastObject. It would be *much* nicer if we could aggregate this
	// common code into some helper functions, instead of having long convuluted
	// cases scattered all over the place.
	if (_features & GF_AFTER_V8) {
		dataptr = findResource(MKID('IMAG'), foir.obim);
		assert(dataptr);
		dataptr = findResource(MKID('WRAP'), dataptr);
		assert(dataptr);
		dataptr = findResource(MKID('OFFS'), dataptr);
		assert(dataptr);
		dataptr += READ_LE_UINT32(dataptr + 4 + 4*imgindex);
		// TODO - distinguish between SMAP and BOMP here?

		// HACK - adjust dataptr here until bomp code gets adjusted for V8
		bomp = dataptr;
	} else {
		dataptr = findResource(IMxx_tags[imgindex], foir.obim);
		if (dataptr == NULL)
			error("setCursorImg: No such image");
	
		size = READ_BE_UINT32_UNALIGNED(dataptr + 4);
		if (size > sizeof(_grabbedCursor))
			error("setCursorImg: Cursor image too large");
		
		bomp = findResource(MKID('BOMP'), dataptr);
	}

	if (bomp != NULL)
		useBompCursor(bomp, w, h);
	else
		useIm01Cursor(dataptr, w, h);

}

void Scumm::nukeFlObjects(int min, int max) {
	ObjectData *od;
	int i;

	warning("nukeFlObjects(%d,%d)", min, max);

	for (i = (_numLocalObjects-1), od = _objs; --i >= 0; od++)
		if (od->fl_object_index && od->obj_nr >= min && od->obj_nr <= max) {
			nukeResource(rtFlObject, od->fl_object_index);
			od->obj_nr = 0;
			od->fl_object_index = 0;
		}
}

void Scumm::enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
                          int objectHeight, int scaleX, int scaleY, int image, int mode) {
	BlastObject *eo;
	ObjectData *od;

	if (_blastObjectQueuePos >= (int)ARRAYSIZE(_blastObjectQueue)) {
		warning("enqueueObject: overflow");
		return;
	}
	
	int idx = getObjectIndex(objectNumber);

	eo = &_blastObjectQueue[_blastObjectQueuePos++];
	eo->number = objectNumber;
	eo->posX = objectX + (camera._cur.x & 7);
	eo->posY = objectY + (camera._cur.y - (_screenHeight / 2));
	if (objectWidth == 0) {
		od = &_objs[idx];
		eo->width = od->width;
	} else {
		eo->width = objectWidth;
	}
	if (objectHeight == 0) {
		od = &_objs[idx];
		eo->height = od->height;
	} else {
		eo->height = objectHeight;
	}

	eo->scaleX = scaleX;
	eo->scaleY = scaleY;
	eo->image = image;

	eo->mode = mode;
}

void Scumm::drawBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		drawBlastObject(eo);
	}
}

void Scumm::drawBlastObject(BlastObject *eo) {
	VirtScreen *vs;
	const byte *bomp, *ptr;
	int idx, objnum;
	BompDrawData bdd;

	vs = &virtscr[0];

	checkRange(_numGlobalObjects - 1, 30, eo->number, "Illegal Blast object %d");

	objnum = getObjectIndex(eo->number);
	if (objnum == -1)
		error("drawBlastObject: getObjectIndex on BlastObject %d failed", eo->number);

	idx = _objs[objnum].fl_object_index;
	if (idx) {
		ptr = getResourceAddress(rtFlObject, idx);
		ptr = findResource(MKID('OBIM'), ptr);
	} else {
		idx = objnum;
		ptr = getResourceAddress(rtRoom, _roomResource) + _objs[objnum].OBIMoffset;
	}
	if (!ptr)
		error("BlastObject object %d (%d) image not found", eo->number, idx);

	if (_features & GF_AFTER_V8) {
		// The OBIM contains an IMAG, which in turn contains a WRAP, which contains
		// an OFFS chunk and multiple BOMP chunks. To find the right BOMP, we can
		// either use the offsets in the OFFS chunk, or iterate over all BOMPs we find.
		// Here we use the first method.
		ptr = findResource(MKID('IMAG'), ptr);
		assert(ptr);
		ptr = findResource(MKID('WRAP'), ptr);
		assert(ptr);
		ptr = findResource(MKID('OFFS'), ptr);
		assert(ptr);
		// Get the address of the specified BOMP (we really should verify it's a BOMP and not a SMAP
		bomp = ptr + READ_LE_UINT32(ptr + 4 + 4*eo->image) + 8;
	} else {
		const byte *img = findResource(IMxx_tags[eo->image], ptr);
		if (!img)
			img = findResource(IMxx_tags[1], ptr);	// Backward compatibility with samnmax blast objects

		if (!img)
			error("blast-object %d invalid image %d (1-x)", eo->number, eo->image);
	
		bomp = findResourceData(MKID('BOMP'), img);
	}

	if (!bomp)
		error("object %d is not a blast object", eo->number);

	//hexdump(bomp,32);

	if (_features & GF_AFTER_V8) {
		bdd.srcwidth = READ_LE_UINT32(&((const BompHeader *)bomp)->v8.width);
		bdd.srcheight = READ_LE_UINT32(&((const BompHeader *)bomp)->v8.height);
	} else {
		bdd.srcwidth = READ_LE_UINT16(&((const BompHeader *)bomp)->old.width);
		bdd.srcheight = READ_LE_UINT16(&((const BompHeader *)bomp)->old.height);
	}
	
	bdd.out = vs->screenPtr + vs->xstart;
	bdd.outwidth = vs->width;
	bdd.outheight = vs->height;
	if (_features & GF_AFTER_V8) {
		bdd.dataptr = bomp + 8;	// Why this? See also useBompCursor
	} else {
		bdd.dataptr = bomp + 10;	// Why this? See also useBompCursor
	}
	bdd.x = eo->posX;
	bdd.y = eo->posY;
	bdd.scale_x = (byte)eo->scaleX;
	bdd.scale_y = (byte)eo->scaleY;


	if ((bdd.scale_x != 255) || (bdd.scale_y != 255)) {
		byte bomp_scaling_x[64], bomp_scaling_y[64];
		bdd.scalingXPtr = bomp_scaling_x;
		bdd.scalingYPtr = bomp_scaling_y;
		bdd.scaleRight = setupBompScale(bomp_scaling_x, bdd.srcwidth, bdd.scale_x);
		bdd.scaleBottom = setupBompScale(bomp_scaling_y, bdd.srcheight, bdd.scale_y);
		bdd.shadowMode = 0;
		drawBomp(bdd, 1, 3);
	} else {
		bdd.shadowMode = eo->mode;
		drawBomp(bdd, 1, 0);
	}

	updateDirtyRect(vs->number, bdd.x, bdd.x + bdd.srcwidth, bdd.y, bdd.y + bdd.srcheight, 0);
}

void Scumm::removeBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		removeBlastObject(eo);
	}

	clearEnqueue();
}

void Scumm::removeBlastObject(BlastObject *eo) {
	VirtScreen *vs = &virtscr[0];

	int top, bottom, left, right;
	int left_strip, right_strip;
	int i;

	top = eo->posY;
	bottom = eo->posY + eo->height;
	left = eo->posX;
	right = eo->posX + eo->width;

	if (bottom < 0 || right < 0 || top > vs->height || left > vs->width)
		return;

	if (top < 0)
		top = 0;
	if (bottom > vs->height)
		bottom = vs->height;
	if (left < 0)
		left = 0;
	if (right > vs->width)
		right = vs->width;

	left_strip = left >> 3;
	right_strip = (right - 1) >> 3;

	if (left_strip < 0)
		left_strip = 0;
	if (_features & GF_AFTER_V7) {
		if (right_strip > 409)
			right_strip = 409;
	} else {
		if (right_strip >= 200)
			right_strip = 200;
	}
	for (i = left_strip; i <= right_strip; i++)
		gdi.resetBackground(top, bottom, i);

	updateDirtyRect(0, left, right, top, bottom, USAGE_BIT_RESTORED);
}

int Scumm::findLocalObjectSlot() {
	int i;

	for (i = 1; i < _numLocalObjects; i++) {
		if (!_objs[i].obj_nr)
			return i;
	}

	return -1;
}

int Scumm::findFlObjectSlot() {
	int i;
	for (i = 1; i < _maxFLObject; i++) {
		if (_baseFLObject[i] == NULL)
			return i;
	}
	error("findFlObjectSlot: Out of FLObject slots");
	return -1;
}

void Scumm::loadFlObject(uint object, uint room) {
	FindObjectInRoom foir;
	int slot, objslot;
	ObjectData *od;
	byte *flob;
	uint32 obcd_size, obim_size, flob_size;
	bool isRoomLocked, isRoomScriptsLocked;

	// Don't load an already loaded object
	if (whereIsObject(object) != WIO_NOT_FOUND)
		return;

	// Locate the object in the room resource
	findObjectInRoom(&foir, foImageHeader | foCodeHeader, object, room);

	// Add an entry for the new floating object in the local object table
	if (!(objslot = findLocalObjectSlot()))
		error("loadFlObject: Local Object Table overflow");

	od = &_objs[objslot];

	// Dump object script
	if (_dumpScripts) {
		char buf[32];
		const byte *ptr = foir.obcd;
		sprintf(buf, "roomobj-%d-", room);
		ptr = findResource(MKID('VERB'), ptr, 0);
		dumpResource(buf, object, ptr);
	}

	// Setup sizes
	obcd_size = READ_BE_UINT32_UNALIGNED(foir.obcd + 4);
	od->OBCDoffset = 8;
	od->OBIMoffset = obcd_size + 8;
	obim_size = READ_BE_UINT32_UNALIGNED(foir.obim + 4);
	flob_size = obcd_size + obim_size + 8;

	// Lock room/roomScripts for the given room. They contains the OBCD/OBIM
	// data, and a call to createResource might expire them, hence we lock them.
	isRoomLocked = ((res.flags[rtRoom][room] & RF_LOCK) != 0);
	isRoomScriptsLocked = ((res.flags[rtRoomScripts][room] & RF_LOCK) != 0);
	if (!isRoomLocked)
		lock(rtRoom, room);
	if (_features & GF_AFTER_V8 && !isRoomScriptsLocked)
		lock(rtRoomScripts, room);

	// Allocate slot & memory for floating object
	slot = findFlObjectSlot();
	flob = createResource(rtFlObject, slot, flob_size);
	assert(flob);

	// Copy object code + object image to floating object
	((uint32 *)flob)[0] = MKID('FLOB');
	((uint32 *)flob)[1] = TO_BE_32(flob_size);

	memcpy(flob + 8, foir.obcd, obcd_size);
	memcpy(flob + 8 + obcd_size, foir.obim, obim_size);

	// Unlock room/roomScripts
	if (!isRoomLocked)
		unlock(rtRoom, room);
	if (_features & GF_AFTER_V8 && !isRoomScriptsLocked)
		unlock(rtRoomScripts, room);

	// Setup local object flags
	setupRoomObject(od, flob, flob);

	od->fl_object_index = slot;
}
