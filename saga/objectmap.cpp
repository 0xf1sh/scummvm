/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Object map / Object click-area module 

// Polygon Hit Test code ( HitTestPoly() ) adapted from code (C) Eric Haines
// appearing in Graphics Gems IV, "Point in Polygon Strategies."
// p. 24-46, code: p. 34-45
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/console.h"
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/objectmap.h"
#include "saga/stream.h"

namespace Saga {

HitZone::HitZone(MemoryReadStreamEndian *readStream, int index): _index(index) {
	int i, j;
	HitZone::ClickArea *clickArea;
	Point *point;

	_flags = readStream->readByte();
	_clickAreasCount = readStream->readByte();
	_rightButtonVerb = readStream->readByte();
	readStream->readByte(); // pad
	_nameIndex = readStream->readUint16();
	_scriptNumber = readStream->readUint16();

	_clickAreas = (HitZone::ClickArea *)malloc(_clickAreasCount * sizeof(*_clickAreas));

	if (_clickAreas == NULL) {
		memoryError("HitZone::HitZone");
	}

	for (i = 0; i < _clickAreasCount; i++) {
		clickArea = &_clickAreas[i];
		clickArea->pointsCount = readStream->readUint16();
		
		assert(clickArea->pointsCount);

		clickArea->points = (Point *)malloc(clickArea->pointsCount * sizeof(*(clickArea->points)));
		if (clickArea->points == NULL) {
			memoryError("HitZone::HitZone");
		}

		for (j = 0; j < clickArea->pointsCount; j++) {
			point = &clickArea->points[j];
			point->x = readStream->readSint16();
			point->y = readStream->readSint16();
		}
	}
}

HitZone::~HitZone() {
	for (int i = 0; i < _clickAreasCount; i++) {
		free(_clickAreas[i].points);
	}
	free(_clickAreas);
}

bool HitZone::getSpecialPoint(Point &specialPoint) const {
	int i, pointsCount;
	HitZone::ClickArea *clickArea;
	Point *points;

	for (i = 0; i < _clickAreasCount; i++) {
		clickArea = &_clickAreas[i];
		pointsCount = clickArea->pointsCount;
		points = clickArea->points;
		if (pointsCount == 1) {
			specialPoint = points[0];
			return true;
		}
	}
	return false;
}
bool HitZone::hitTest(const Point &testPoint) {
	int i, pointsCount;
	HitZone::ClickArea *clickArea;
	Point *points;

	if (_flags & kHitZoneEnabled) {
		for (i = 0; i < _clickAreasCount; i++) {
			clickArea = &_clickAreas[i];
			pointsCount = clickArea->pointsCount;
			points = clickArea->points;

			if (pointsCount == 2) {
				// Hit-test a box region
				if ((testPoint.x >= points[0].x) && 
					(testPoint.x <= points[1].x) &&
					(testPoint.y >= points[0].y) &&
					(testPoint.y <= points[1].y)) {
						return true;
					}
			} else {
				if (pointsCount > 2) {
					// Hit-test a polygon
					if (hitTestPoly(points, pointsCount, testPoint)) {
						return true;
					}				
				}
			}
		}
	}
	return false;
}

void HitZone::draw(SURFACE *ds, int color) {
	int i, pointsCount;
	HitZone::ClickArea *clickArea;
	Point *points;
	for (i = 0; i < _clickAreasCount; i++) {
		clickArea = &_clickAreas[i];
		pointsCount = clickArea->pointsCount;
		points = clickArea->points;
		if (pointsCount == 2) {
			// 2 points represent a box
			drawFrame(ds, &points[0], &points[1], color);
		} else {
			if (pointsCount > 2) {
				// Otherwise draw a polyline
				drawPolyLine(ds, points, pointsCount, color);
			}
		}
	}
}


// Loads an object map resource ( objects ( clickareas ( points ) ) ) 
void ObjectMap::load(const byte *resourcePointer, size_t resourceLength) {
	int i;

	if (resourceLength < 4) {
		error("ObjectMap::load wrong resourceLength");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);

	_hitZoneListCount = readS.readSint16();
	if (_hitZoneListCount < 0) {
		error("ObjectMap::load _hitZoneListCount < 0");
	}

	if (_hitZoneList)
		error("ObjectMap::load _hitZoneList != NULL");

	_hitZoneList = (HitZone **) malloc(_hitZoneListCount * sizeof(HitZone *));
	if (_hitZoneList == NULL) {
		memoryError("ObjectMap::load");
	}

	for (i = 0; i < _hitZoneListCount; i++) {
		_hitZoneList[i] = new HitZone(&readS, i);
	}
}

void ObjectMap::freeMem() {
	int i;

	if (_hitZoneList) {
		for (i = 0; i < _hitZoneListCount; i++) {
			delete _hitZoneList[i];
		}

		free(_hitZoneList);
		_hitZoneList = NULL;
	}
}



void ObjectMap::draw(SURFACE *ds, const Point& testPoint, int color, int color2) {
	int i;
	int hitZoneIndex;
	char txtBuf[32];

	hitZoneIndex = hitTest(testPoint);

	for (i = 0; i < _hitZoneListCount; i++) {		
		_hitZoneList[i]->draw(ds, (hitZoneIndex == i) ? color2 : color);
	}

	if (hitZoneIndex != -1) {		
		snprintf(txtBuf, sizeof(txtBuf), "hitZone %d", hitZoneIndex);
		_vm->_font->draw(SMALL_FONT_ID, ds, txtBuf, 0, 2, 2,
			kITEColorBrightWhite, kITEColorBlack, FONT_OUTLINE);

	}
}

int ObjectMap::hitTest(const Point& testPoint) {
	int i;

	// Loop through all scene objects
	for (i = 0; i < _hitZoneListCount; i++) {
		if (_hitZoneList[i]->hitTest(testPoint)) {
			return i;
		}
	}

	return -1;
}

void ObjectMap::cmdInfo(void) {
	_vm->_console->DebugPrintf("%d zone(s) loaded.\n\n", _hitZoneListCount);
}

} // End of namespace Saga
