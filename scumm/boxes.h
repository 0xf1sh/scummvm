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

#ifndef BOXES_H
#define BOXES_H

#include "common/rect.h"

#define SIZEOF_BOX_V2 8
#define SIZEOF_BOX_V3 18
#define SIZEOF_BOX 20
#define SIZEOF_BOX_V8 52

typedef enum {
	kBoxXFlip		= 0x08,
	kBoxYFlip		= 0x10,
	kBoxPlayerOnly	= 0x20,
	kBoxLocked		= 0x40,
	kBoxInvisible	= 0x80
} BoxFlags;

struct AdjustBoxResult {	/* Result type of AdjustBox functions */
	int16 x, y;
	uint16 dist;
};

struct BoxCoords {			/* Box coordinates */
	ScummVM::Point ul;
	ScummVM::Point ur;
	ScummVM::Point ll;
	ScummVM::Point lr;
};

struct Box;
struct PathNode;
struct PathVertex;

#endif
