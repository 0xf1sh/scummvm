/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef CRUISE_CTP_H
#define CRUISE_CTP_H

namespace Cruise {

struct ctpVar19SubStruct {
	uint16 boxIdx;		//0
	uint16 type;		//2
	uint16 minX;		//4
	uint16 maxX;		//6
	uint16 minY;		//8
	uint16 maxY;		//A
};

struct ctpVar19Struct {
	struct ctpVar19Struct *field_0;	//0
	ctpVar19SubStruct subStruct;
};

extern uint8 *workBuffer;
extern uint8 *polyStruct;
extern uint8 *adrStructPoly;
extern uint8 *polyStructNorm;
extern uint8 *polyStructExp;

extern uint8 *ctpVar17;
extern uint8 *polyStruct0;

int initCt(const char * ctpName);
int computeDistance(int varX, int varY, int paramX, int paramY);

int getNode(int nodeResult[2], int nodeId);
int setNodeColor(int nodeIdx, int nodeColor);
int setNodeState(int nodeIdx, int nodeState);

} // End of namespace Cruise

#endif
