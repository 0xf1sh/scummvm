/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#ifndef AUTOROUTE_H
#define AUTOROUTE_H

#include "stdafx.h"
#include "common/scummsys.h"

namespace Sky {

struct Compact;
class Grid;
class SkyCompact;

class AutoRoute {
public:
	AutoRoute(Grid *pGrid, SkyCompact *compact);
	~AutoRoute(void);
	uint16 autoRoute(Compact *cpt);
private:
	uint16 checkBlock(uint16 *blockPos);
	void clipCoordX(uint16 x, uint8 &blkX, int16 &initX);
	void clipCoordY(uint16 y, uint8 &blkY, int16 &initY);
	void initWalkGrid(uint8 screen, uint8 width);
	bool calcWalkGrid(uint8 startX, uint8 startY, uint8 destX, uint8 destY);
	uint16 *makeRouteData(uint8 startX, uint8 startY, uint8 destX, uint8 destY);
	uint16 *checkInitMove(uint16 *data, int16 initStaX);
	Grid *_grid;
	SkyCompact *_skyCompact;
	uint16 *_routeGrid;
	uint16 *_routeBuf;
	static const int16 _routeDirections[4];
	static const uint16 _logicCommands[4];
};

} // End of namespace Sky

#endif // AUTOROUTE_H

