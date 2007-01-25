/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/map.h"

namespace Gob {

Goblin_v2::Goblin_v2(GobEngine *vm) : Goblin_v1(vm) {
	_gobsCount = -1;
	_rotStates[0][0] = 0; _rotStates[0][1] = 18; _rotStates[0][2] = 19; _rotStates[0][3] = 20;
	_rotStates[1][0] = 13; _rotStates[1][1] = 2; _rotStates[1][2] = 12; _rotStates[1][3] = 14;
	_rotStates[2][0] = 16; _rotStates[2][1] = 15; _rotStates[2][2] = 4; _rotStates[2][3] = 17;
	_rotStates[3][0] = 23; _rotStates[3][1] = 21; _rotStates[3][2] = 22; _rotStates[3][3] = 6;
}

void Goblin_v2::freeObjects(void) {
	int i;

	if (_gobsCount < 0)
		return;

	for (i = 0; i < _gobsCount; i++) {
		delete[] _vm->_mult->_objects[i].goblinStates[0];
		delete[] _vm->_mult->_objects[i].goblinStates;
	}
	for (i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		if ((_vm->_goblin->_soundSlots[i] & 0x8000) == 0)
			_vm->_game->freeSoundSlot(_vm->_goblin->_soundSlots[i]);
//	delete[] off_2F2AB;
	_gobsCount = -1;
}

void Goblin_v2::placeObject(Gob_Object *objDesc, char animated,
		int16 index, int16 x, int16 y, int16 state) {
	Mult::Mult_Object *obj;
	Mult::Mult_AnimData *objAnim;
	int16 layer;
	int16 animation;

	obj = &_vm->_mult->_objects[index];
	objAnim = obj->pAnimData;

	obj->goblinX = x;
	obj->goblinY = y;
	objAnim->order = y;

	if (state == -1) {
		objAnim->frame = 0;
		objAnim->isPaused = 0;
		objAnim->isStatic = 0;
		objAnim->newCycle = 0;
		_vm->_scenery->updateAnim(objAnim->layer, 0, objAnim->animation, 0,
				*obj->pPosX, *obj->pPosY, 0);
		if (!_vm->_map->_bigTiles)
			*obj->pPosY = (y + 1) * _vm->_map->_tilesHeight
				- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		else
			*obj->pPosY = ((y + 1) / 2) * _vm->_map->_tilesHeight
				- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		*obj->pPosX = x * _vm->_map->_tilesWidth;
	} else {
		if (obj->goblinStates[state] != 0) {
			layer = obj->goblinStates[state][0].layer;
			animation = obj->goblinStates[state][0].animation;
			objAnim->state = state;
			objAnim->layer = layer;
			objAnim->animation = animation;
			objAnim->frame = 0;
			objAnim->isPaused = 0;
			objAnim->isStatic = 0;
			objAnim->newCycle = _vm->_scenery->_animations[animation].layers[layer].framesCount;
			_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
			if (!_vm->_map->_bigTiles)
				*obj->pPosY = (y + 1) * _vm->_map->_tilesHeight
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			else
				*obj->pPosY = ((y + 1) / 2) * _vm->_map->_tilesHeight
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			*obj->pPosX = x * _vm->_map->_tilesWidth;
			initiateMove(obj);
		} else
			initiateMove(obj);
	}
}

void Goblin_v2::initiateMove(Mult::Mult_Object *obj) {
	obj->destX = obj->gobDestX;
	obj->destY = obj->gobDestY;
	_vm->_map->findNearestToDest(obj);
	_vm->_map->findNearestToGob(obj);
	_vm->_map->optimizePoints(obj, obj->goblinX, obj->goblinY);
	obj->pAnimData->pathExistence = _vm->_map->checkDirectPath(obj,
			obj->goblinX, obj->goblinY, obj->gobDestX, obj->gobDestY);
	if (obj->pAnimData->pathExistence == 3) {
		obj->destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
		obj->destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
	}
}

void Goblin_v2::movePathFind(Mult::Mult_Object *obj, Gob_Object *gobDesc, int16 nextAct) {
	Mult::Mult_AnimData *animData;
	int16 framesCount;
	int16 gobX;
	int16 gobY;
	int16 gobDestX;
	int16 gobDestY;
	int16 destX;
	int16 destY;
	int16 dir;

	dir = 0;
	animData = obj->pAnimData;
	framesCount =
		_vm->_scenery->_animations[(int)animData->animation].layers[animData->layer].framesCount;
	animData->newCycle = framesCount;
	gobX = obj->goblinX;
	gobY = obj->goblinY;
	animData->order = gobY;
	gobDestX = obj->gobDestX;
	gobDestY = obj->gobDestY;
	animData->field_13 = gobDestX;
	animData->field_14 = gobDestY;
	destX = obj->destX;
	destY = obj->destY;

	if (animData->pathExistence == 1) {
		dir = _vm->_map->getDirection(gobX, gobY, destX, destY);
		if (dir == 0)
			animData->pathExistence = 0;
		if ((gobX == destX) && (gobY == destY))
			animData->pathExistence = 4;
	} else if (animData->pathExistence == 3) {
		if ((gobX == gobDestX) && (gobY == gobDestY)) {
			animData->pathExistence = 4;
			destX = gobDestX;
			destY = gobDestY;
		} else {
			if (_vm->_map->checkDirectPath(obj, gobX, gobY, gobDestX, gobDestY) != 1) {
				if ((gobX == destX) && (gobY == destY)) {
					if (obj->nearestWayPoint > obj->nearestDest) {
						_vm->_map->optimizePoints(obj, gobX, gobY);
						destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
						destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
						if (_vm->_map->checkDirectPath(obj, gobX, gobY, destX, destY) == 3) {
							WRITE_VAR(56, 1);
							animData->pathExistence = 0;
						}
						if (obj->nearestWayPoint > obj->nearestDest)
							obj->nearestWayPoint--;
					} else if (obj->nearestWayPoint < obj->nearestDest) { // loc_10E96
						_vm->_map->optimizePoints(obj, gobX, gobY);
						destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
						destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
						if (_vm->_map->checkDirectPath(obj, gobX, gobY, destX, destY) == 3) {
							WRITE_VAR(56, 1);
							animData->pathExistence = 0;
						}
						if (obj->nearestWayPoint < obj->nearestDest)
							obj->nearestWayPoint++;
					} else {
						if ((_vm->_map->checkDirectPath(obj, gobX, gobY, gobDestX, gobDestY) == 3) &&
								(_vm->_map->getPass(gobDestX, gobDestY) != 0)) {
							destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
							destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
							WRITE_VAR(56, 1);
						} else {
							animData->pathExistence = 1;
							destX = gobDestX;
							destY = gobDestY;
						}
					}
				}
			} else {
				destX = gobDestX;
				destY = gobDestY;
			}
			dir = _vm->_map->getDirection(gobX, gobY, destX, destY);
		}
	}

	obj->goblinX = gobX;
	obj->goblinY = gobY;
	obj->gobDestX = gobDestX;
	obj->gobDestY = gobDestY;
	obj->destX = destX;
	obj->destY = destY;

	switch (dir) {
	case Map::kDirNW:
		animData->nextState = 1;
		if (_vm->_map->_screenWidth == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 40;
			if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10)
				animData->nextState = 1;
		}
		break;

	case Map::kDirN:
		animData->nextState = animData->curLookDir == 2 ? 2 : rotateState(animData->curLookDir, 2);
		if (_vm->_map->_screenWidth == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) {
				if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10) {
					if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) == 10)
						animData->nextState = 42;
					else
						animData->nextState = 2;
				} else
					animData->nextState = 40;
			} else if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20)
				animData->nextState = 38;
			else if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19)
				animData->nextState = 26;
		}
		break;

	case Map::kDirNE:
		animData->nextState =	3;
		if (_vm->_map->_screenWidth == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 42;
			if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) != 10)
				animData->nextState = 3;
		}
		break;

	case Map::kDirW:
		animData->nextState = rotateState(animData->curLookDir, 0);
		break;

	case Map::kDirE:
		animData->nextState = rotateState(animData->curLookDir, 4);
		break;

	case Map::kDirSW:
		animData->nextState = 7;
		if (_vm->_map->_screenWidth == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 41;
			if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY) != 10)
				animData->nextState = 7;
		}
		break;

	case Map::kDirS:
		animData->nextState = animData->curLookDir == 6 ? 6 : rotateState(animData->curLookDir, 6);
		if (_vm->_map->_screenWidth == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20)
				animData->nextState = 39;
			else if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19)
				animData->nextState = 27;
		}
		break;

	case Map::kDirSE:
		animData->nextState = 5;
		if (_vm->_map->_screenWidth == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 43;
			if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY) != 10)
				animData->nextState = 5;
		}
		break;

	default:
		if (animData->curLookDir == 0)
			animData->nextState = 8;
		else if (animData->curLookDir == 2)
			animData->nextState = 29;
		else if (animData->curLookDir == 4)
			animData->nextState = 9;
		else if (animData->curLookDir == 6)
			animData->nextState = 28;
		break;
	}
}

void Goblin_v2::moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
		int16 nextAct, int16 framesCount) {
	Mult::Mult_AnimData *animData;
	int16 gobX;
	int16 gobY;
	int16 animation;
	int16 state;
	int16 layer;

	movePathFind(obj, 0, 0);
	playSounds(obj);

	animData = obj->pAnimData;

	framesCount =
		_vm->_scenery->_animations[(int)animData->animation].layers[animData->layer].framesCount;

	if (animData->isPaused == 0)
		animData->frame++;

	switch (animData->field_16) {
	case 0:
	case 1:
		animData->isPaused = 0;
		break;

	case 4:
		if (animData->frame == 0)
			animData->isPaused = 1;
		break;

	case 6:
		if (animData->frame >= framesCount)
			animData->isPaused = 1;
		break;
	}

	switch (animData->state) {
	case 0:
	case 1:
	case 7:
	case 13:
	case 16:
	case 23:
		// loc_11452
		animData->curLookDir = 0;
		break;

	case 2:
	case 15:
	case 18:
	case 21:
		animData->curLookDir = 2;
		break;

	case 3:
	case 4:
	case 5:
	case 12:
	case 19:
	case 22:
		animData->curLookDir = 4;
		break;

	case 6:
	case 14:
	case 17:
	case 20:
		animData->curLookDir = 6;
		break;

	case 8:
	case 9:
	case 28:
	case 29:
		if (animData->pathExistence == 4)
			animData->pathExistence = 5;
		break;
	}

	if ((animData->field_F != -1) && (animData->frame == framesCount) &&
			(animData->field_F != animData->state)) { // loc_114B6
		animData->nextState = animData->field_F;
		animData->field_F = -1;
		animData->state = animData->nextState;
		*obj->pPosX +=
			_vm->_scenery->_animations[animData->animation].layers[animData->layer].animDeltaX;
		*obj->pPosY +=
			_vm->_scenery->_animations[animData->animation].layers[animData->layer].animDeltaY;
		animation = obj->goblinStates[animData->nextState][0].animation;
		layer = obj->goblinStates[animData->nextState][0].layer;
		animData->layer = layer;
		animData->animation = animation;
		animData->frame = 0;
	} else {
		if (((animData->state >= 0) && (animData->state < 8)) ||
				(animData->state == 38) || (animData->state == 39)) { // loc_115C4
			state = animData->nextState;
			if (animData->frame == ((framesCount + 1) / 2)) {
				gobX = obj->goblinX;
				gobY = obj->goblinY;
				switch (state) {
				case 0:
					obj->goblinX--;
					break;

				case 1:
					obj->goblinX--;
					obj->goblinY--;
					break;

				case 2:
				case 38:
					obj->goblinY--;
					break;

				case 3:
					obj->goblinX++;
					obj->goblinY--;
					break;

				case 4:
					obj->goblinX++;
					break;

				case 5:
					obj->goblinX++;
					obj->goblinY++;
					break;

				case 6:
				case 39:
					obj->goblinY++;
					break;

				case 7:
					obj->goblinX--;
					obj->goblinY++;
					break;
				}
				if (animData->state != state) {
					animation = obj->goblinStates[state][0].animation;
					layer = obj->goblinStates[state][0].layer;
					animData->layer = layer;
					animData->animation = animation;
					animData->frame = 0;
					animData->state = state;
					_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
					if (_vm->_map->_bigTiles)
						*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
							(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) - (gobY + 1) / 2;
					else
						*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
							(_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
					*obj->pPosX = gobX * _vm->_map->_tilesWidth;
				}
			}
		}

		if (animData->frame >= framesCount) {
			state = animData->nextState;
			animation = obj->goblinStates[state][0].animation;
			layer = obj->goblinStates[state][0].layer;
			animData->layer = layer;
			animData->animation = animation;
			animData->frame = 0;
			animData->state = state;
			gobX = obj->goblinX;
			gobY = obj->goblinY;
			switch (state) {
			case 0:
				obj->goblinX--;
				break;

			case 1:
				obj->goblinX--;
				obj->goblinY--;
				break;

			case 2:
			case 38:
				obj->goblinY--;
				break;

			case 3:
				obj->goblinX++;
				obj->goblinY--;
				break;

			case 4:
				obj->goblinX++;
				break;

			case 5:
				obj->goblinX++;
				obj->goblinY++;
				break;

			case 6:
			case 39:
				obj->goblinY++;
				break;

			case 7:
				obj->goblinX--;
				obj->goblinY++;
				break;
			}
			_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
			if (_vm->_map->_bigTiles)
				*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
					(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) - (gobY + 1) / 2;
			else
				*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
					(_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			*obj->pPosX = gobX * _vm->_map->_tilesWidth;
		}
	}
}

void Goblin_v2::handleGoblins(void) {
	Mult::Mult_Object *obj0;
	Mult::Mult_Object *obj1;
	Mult::Mult_AnimData *anim0;
	Mult::Mult_AnimData *anim1;
	int16 pass;
	int16 gob1X;
	int16 gob2X;
	int16 gob1Y;
	int16 gob2Y;
	int16 var_A;
	int16 var_C;
	int16 di;
	int16 si;

	obj0 = &_vm->_mult->_objects[0];
	obj1 = &_vm->_mult->_objects[1];
	anim0 = obj0->pAnimData;
	anim1 = obj1->pAnimData;

	si = anim0->state;
	di = anim1->state;

	if (anim0->isBusy == 0) {
		if ((_word_2F9BC == 0) && (anim0->isStatic == 0)) {
			if ((VAR(_dword_2F9B6) == 0) && (si == 28)) {
				si = _vm->_util->getRandom(3) + 24;
				sub_195C7(0, si);
				WRITE_VAR(_dword_2F9B6, 100);
			} else
				WRITE_VAR(_dword_2F9B6, VAR(_dword_2F9B6) - 1);
		}
		if ((si == 8) || (si == 9) || (si == 29))
			anim0->curLookDir = 6;
	}
	if (anim1->isBusy == 0) {
		if ((_word_2F9BA == 0) && (anim1->isStatic == 0)) {
			if ((VAR(_dword_2F9B2) == 0) && (di == 28)) {
				di = _vm->_util->getRandom(3) + 24;
				sub_195C7(1, di);
				WRITE_VAR(_dword_2F9B2, 100);
			} else
				WRITE_VAR(_dword_2F9B2, VAR(_dword_2F9B2) - 1);
		}
		if ((di == 8) || (di == 9) || (di == 29))
			anim1->curLookDir = 6;
	}

	if ((anim0->isBusy == 1) && (anim0->isStatic == 0) &&
			((anim0->state == 28) || (anim0->state == 29)))
		anim0->curLookDir = 0;
	if ((anim1->isBusy == 1) && (anim1->isStatic == 0) &&
			((anim1->state == 28) || (anim1->state == 29)))
		anim1->curLookDir = 0;

	if (VAR(18) != ((uint32) -1)) {
		if (anim0->layer == 44)
			anim0->curLookDir = 4;
		else if (anim0->layer == 45)
			anim0->curLookDir = 0;
		if (anim0->isBusy == 0)
			anim0->curLookDir = 6;
	}
	if (VAR(19) != ((uint32) -1)) {
		if (anim1->layer == 48)
			anim1->curLookDir = 4;
		else if (anim1->layer == 49)
			anim1->curLookDir = 0;
		if (anim1->isBusy == 0)
			anim1->curLookDir = 6;
	}

	if ((anim0->layer == 45) && (anim0->curLookDir == 4) && (anim0->pathExistence == 5) &&
			(VAR(18) == ((uint32) -1)) && (_word_2F9C0 == 0)) {
		sub_195C7(0, 19);
	}
	if ((anim0->layer == 44) && (anim0->curLookDir == 0) && (anim0->pathExistence == 5) &&
			(VAR(18) == ((uint32) -1)) && (_word_2F9C0 == 0)) {
		sub_195C7(0, 16);
	}
	if ((anim1->layer == 49) && (anim1->curLookDir == 4) && (anim1->pathExistence == 5) &&
			(VAR(19) == ((uint32) -1)) && (_word_2F9BE == 0)) {
		sub_195C7(1, 19);
	}
	if ((anim1->layer == 48) && (anim1->curLookDir == 0) && (anim1->pathExistence == 5) &&
			(VAR(19) == ((uint32) -1)) && (_word_2F9BE == 0)) {
		sub_195C7(1, 16);
	}

	gob1X = obj0->goblinX;
	gob2X = obj1->goblinX;
	gob1Y = obj0->goblinY;
	gob2Y = obj1->goblinY;
	di = anim0->field_13;
	si = anim0->field_14;
	var_A = anim1->field_13;
	var_C = anim1->field_14;

	pass = _vm->_map->getPass(gob1X, gob1Y);
	if ((pass > 17) && (pass < 21))
		sub_19AB7(anim0);
	pass = _vm->_map->getPass(gob2X, gob2Y);
	if ((pass > 17) && (pass < 21))
		sub_19B45(anim1);

	if ((di < 0) || (di > 39) || (si < 0) || (si > 39))
		return;

	if (gob1Y > si) {
		if (_vm->_map->getPass(di, si) > 17) {
			do {
				si--;
			} while (_vm->_map->getPass(di, si) > 17);
			si++;
			if (_vm->_map->getPass(di - 1, si) == 0) {
				if (_vm->_map->getPass(di + 1, si) != 0)
					di++;
			} else
				di--;
			sub_197A6(di, si, 0);
		}
	} else {
		if (_vm->_map->getPass(di, si) > 17) {
			do {
				si++;
			} while (_vm->_map->getPass(di, si) > 17);
			si--;
			if (_vm->_map->getPass(di - 1, si) == 0) {
				if (_vm->_map->getPass(di + 1, si) != 0)
					di++;
			} else
				di--;
			sub_197A6(di, si, 0);
		}
	}
	if (gob2Y > var_C) {
		if (_vm->_map->getPass(var_A, var_C) > 17) {
			do {
				var_C--;
			} while (_vm->_map->getPass(var_A, var_C) > 17);
			var_C++;
			if (_vm->_map->getPass(var_A - 1, var_C) == 0) {
				if (_vm->_map->getPass(var_A + 1, var_C) != 0)
					var_A++;
			} else
				var_A--;
			sub_197A6(var_A, var_C, 1);
		}
	} else {
		if (_vm->_map->getPass(var_A, var_C) > 17) {
			do {
				var_C++;
			} while (_vm->_map->getPass(var_A, var_C) > 17);
			var_C--;
			if (_vm->_map->getPass(var_A - 1, var_C) == 0) {
				if (_vm->_map->getPass(var_A + 1, var_C) != 0)
					var_A++;
			} else
				var_A--;
			sub_197A6(var_A, var_C, 1);
		}
	}
}

} // End of namespace Gob
