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
 */

#ifndef HOPKINS_LINES_H
#define HOPKINS_LINES_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

struct LigneZoneItem {
	int count;
	int field2;
	int16 *zoneData;
};

struct LigneItem {
	int field0;
	int field2;
	int field4;
	int field6;
	int field8;
	int16 *lineData;
};

struct SmoothItem {
	int field0;
	int field2;
};

class LinesManager {
private:
	HopkinsEngine *_vm;
public:
	LigneZoneItem _zoneLine[401];
	LigneItem Ligne[400];
	SmoothItem SMOOTH[4000];
	int next_ligne;
	int TOTAL_LIGNES;
	int NV_LIGNEDEP;
	int NV_LIGNEOFS;
	int NV_POSI;
	int NVPX;
	int NVPY;
	int SMOOTH_SENS;
	int SMOOTH_X, SMOOTH_Y;
public:
	LinesManager();
	void setParent(HopkinsEngine *vm);

	void CLEAR_ZONE();
	int ZONE_OBJET(int posX, int posY);
	int OPTI_ZONE(int posX, int minZoneNum, bool lastRow);
	void removeZoneLine(int idx);
	void addZoneLine(int idx, int a2, int a3, int a4, int a5, int bobZoneIdx);
	void RESET_OBSTACLE();
	void RETIRE_LIGNE(int idx);
	void AJOUTE_LIGNE(int idx, int a2, int a3, int a4, int a5, int a6, int a7);
	bool colision2_ligne(int a1, int a2, int *a3, int *a4, int a5, int a6);
	void INIPARCOURS();
	int CONTOURNE1(int a1, int a2, int a3, int a4, int a5, int16 *a6, int a7, int a8, int a9);
	int CONTOURNE(int a1, int a2, int a3, int a4, int a5, int16 *a6, int a7);
	int MIRACLE(int a1, int a2, int a3, int a4, int a5);
	int GENIAL(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int16 *a8, int a9);
	int16 *PARCOURS2(int srcX, int srcY, int destX, int destY);
	int PARC_PERS(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
	int VERIF_SMOOTH(int a1, int a2, int a3, int a4);
	int SMOOTH_MOVE(int a3, int a4, int a5, int a6);
	int PLAN_TEST(int a1, int a2, int a3, int a4, int a5, int a6);
	int TEST_LIGNE(int a1, int a2, int *a3, int *a4, int *a5);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
