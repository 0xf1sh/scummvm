/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/boxes.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/resource.h"
#include "scumm/saveload.h"
#include "scumm/sound.h"
#include "scumm/usage_bits.h"
#include "scumm/wiz_he.h"

namespace Scumm {

byte Actor::kInvalidBox = 0;
ScummEngine *Actor::_vm = 0;

void Actor::initActorClass(ScummEngine *scumm) {
	_vm = scumm;
	if (_vm->_features & GF_SMALL_HEADER) {
		kInvalidBox = 255;
	}
}

Actor::Actor() {
	assert(_vm != 0);
	_number = 0;

	initActor(-1);
}

void Actor::initActor(int mode) {
	if (mode == -1) {
		_offsX = _offsY = 0;
		_top = _bottom = 0;
		_needRedraw = _needBgReset = _costumeNeedsInit = _visible = false;
		_flip = false;
		_speedx = 8;
		_speedy = 2;
		_frame = 0;
		_walkbox = 0;
		_animProgress = 0;
		_heSkipLimbs = false;
		_drawToBackBuf = false;
		memset(_animVariable, 0, sizeof(_animVariable));
		memset(_palette, 0, sizeof(_palette));
		memset(_sound, 0, sizeof(_sound));
		memset(&_cost, 0, sizeof(CostumeData));
		memset(&_walkdata, 0, sizeof(ActorWalkData));
		_walkdata.point3.x = 32000;
		_walkScript = 0;
		memset(_heTalkQueue, 0, sizeof(_heTalkQueue));
		
		mode = 1;
	}

	if (mode == 1) {
		_costume = 0;
		_room = 0;
		_pos.x = 0;
		_pos.y = 0;
		_facing = 180;
		_heCondMask = 1;
		_heNoTalkAnimation = 0;
		if (_vm->_version >= 7)
			_visible = false;
		_heSkipLimbs = false;
	} else if (mode == 2) {
		_facing = 180;
		_heCondMask = 1;
		_heSkipLimbs = false;
	}
	_elevation = 0;
	_width = 24;
	_talkColor = 15;
	_talkPosX = 0;
	_talkPosY = -80;
	_boxscale = _scaley = _scalex = 0xFF;
	_charset = 0;
	memset(_sound, 0, sizeof(_sound));
	_targetFacing = _facing;

	stopActorMoving();

	_shadowMode = 0;
	_layer = 0;

	setActorWalkSpeed(8, 2);
	_animSpeed = 0;
	if (_vm->_version >= 6)
		_animProgress = 0;

	_ignoreBoxes = false;
	_forceClip = (_vm->_version >= 7) ? 100 : 0;
	_ignoreTurns = false;

	if (_vm->_features & GF_HUMONGOUS)
		_flip = 0;

	_talkFrequency = 256;
	_talkPan = 64;
	_talkVolume = 127;

	if (_vm->_version <= 2) {
		_initFrame = 2;
		_walkFrame = 0;
		_standFrame = 1;
		_talkStartFrame = 5;
		_talkStopFrame = 4;
	} else {
		_initFrame = 1;
		_walkFrame = 2;
		_standFrame = 3;
		_talkStartFrame = 4;
		_talkStopFrame = 5;
	}

	_heTalking = false;
	_walkScript = 0;
	_talkScript = 0;

	_clipOverride = _vm->_actorClipOverride;

	_auxBlock.visible = false;
	_hePaletteNum = 0;

	_vm->_classData[_number] = (_vm->_version >= 7) ? _vm->_classData[0] : 0;
}

void Actor::stopActorMoving() {
	if (_walkScript)
		_vm->stopScript(_walkScript);
	_moving = 0;
}

void Actor::setActorWalkSpeed(uint newSpeedX, uint newSpeedY) {
	if (newSpeedX == _speedx && newSpeedY == _speedy)
		return;

	_speedx = newSpeedX;
	_speedy = newSpeedY;

	if (_moving) {
		calcMovementFactor(_walkdata.next);
	}
}

int ScummEngine::getAngleFromPos(int x, int y) const {
	if (_gameId == GID_DIG || _gameId == GID_CMI) {
		double temp = atan2((double)x, (double)-y);
		return normalizeAngle((int)(temp * 180 / 3.1415926535));
	} else {
		if (abs(y) * 2 < abs(x)) {
			if (x > 0)
				return 90;
			return 270;
		} else {
			if (y > 0)
				return 180;
			return 0;
		}
	}
}

int Actor::calcMovementFactor(const Common::Point& next) {
	Common::Point _actorPos(_pos);
	int diffX, diffY;
	int32 deltaXFactor, deltaYFactor;

	if (_actorPos == next)
		return 0;

	diffX = next.x - _actorPos.x;
	diffY = next.y - _actorPos.y;
	deltaYFactor = _speedy << 16;

	if (diffY < 0)
		deltaYFactor = -deltaYFactor;

	deltaXFactor = deltaYFactor * diffX;
	if (diffY != 0) {
		deltaXFactor /= diffY;
	} else {
		deltaYFactor = 0;
	}

	if ((uint) abs((int)(deltaXFactor >> 16)) > _speedx) {
		deltaXFactor = _speedx << 16;
		if (diffX < 0)
			deltaXFactor = -deltaXFactor;

		deltaYFactor = deltaXFactor * diffY;
		if (diffX != 0) {
			deltaYFactor /= diffX;
		} else {
			deltaXFactor = 0;
		}
	}

	_walkdata.cur = _actorPos;
	_walkdata.next = next;
	_walkdata.deltaXFactor = deltaXFactor;
	_walkdata.deltaYFactor = deltaYFactor;
	_walkdata.xfrac = 0;
	_walkdata.yfrac = 0;

	_targetFacing = _vm->getAngleFromPos(deltaXFactor, deltaYFactor);

	return actorWalkStep();
}

int Actor::remapDirection(int dir, bool is_walking) {
	int specdir;
	byte flags;
	bool flipX;
	bool flipY;

	// FIXME - It seems that at least in The Dig the original code does
	// check _ignoreBoxes here. However, it breaks some animations in Loom,
	// causing Bobbin to face towards the camera instead of away from it
	// in some places: After the tree has been destroyed by lightning, and
	// when entering the dark tunnels beyond the dragon's lair at the very
	// least. Possibly other places as well.
	//
	// The Dig also checks if the actor is in the current room, but that's
	// not necessary here because we never call the function unless the
	// actor is in the current room anyway.
	
	if (!_ignoreBoxes || (_vm->_gameId == GID_LOOM || _vm->_gameId == GID_LOOM256)) {
		specdir = _vm->_extraBoxFlags[_walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				specdir = specdir & 0x3FFF;
				if (specdir - 90 < dir && dir < specdir + 90)
					dir = specdir;
				else
					dir = specdir + 180;
			}
		}

		flags = _vm->getBoxFlags(_walkbox);

		flipX = (_walkdata.deltaXFactor > 0);
		flipY = (_walkdata.deltaYFactor > 0);

		// Check for X-Flip
		if ((flags & kBoxXFlip) || isInClass(kObjectClassXFlip)) {
			dir = 360 - dir;
			flipX = !flipX;
		}
		// Check for Y-Flip
		if ((flags & kBoxYFlip) || isInClass(kObjectClassYFlip)) {
			dir = 180 - dir;
			flipY = !flipY;
		}

		switch (flags & 7) {
		case 1:
			if (_vm->_version >= 7) {
				if (dir < 180)
					return 90;
				else
					return 270;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipX ? 90 : 270;
				else	                               // Actor is standing/turning
					return (dir == 90) ? 90 : 270;
			}
		case 2:
			if (_vm->_version >= 7) {
				if (dir > 90 && dir < 270)
					return 180;
				else
					return 0;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipY ? 180 : 0;
				else	                               // Actor is standing/turning
					return (dir == 0) ? 0 : 180;
			}
		case 3:
			return 270;
		case 4:
			return 90;
		case 5:
			return 0;
		case 6:
			return 180;
		}
	}
	// OR 1024 in to signal direction interpolation should be done
	return normalizeAngle(dir) | 1024;
}

int Actor::updateActorDirection(bool is_walking) {
	int from;
	int dirType;
	int dir;
	bool shouldInterpolate;

	if ((_vm->_version == 6) && _ignoreTurns)
		return _facing;

	dirType = (_vm->_version >= 7) ? _vm->akos_hasManyDirections(_costume) : false;

	from = toSimpleDir(dirType, _facing);
	dir = remapDirection(_targetFacing, is_walking);

	if (_vm->_version >= 7)
		// Direction interpolation interfers with walk scripts in Dig; they perform
		// (much better) interpolation themselves.
		shouldInterpolate = false;	
	else
		shouldInterpolate = (dir & 1024) ? true : false;
	dir &= 1023;

	if (shouldInterpolate) {
		int to = toSimpleDir(dirType, dir);
		int num = dirType ? 8 : 4;

		// Turn left or right, depending on which is shorter.
		int diff = to - from;
		if (abs(diff) > (num >> 1))
			diff = -diff;

		if (diff > 0) {
			to = from + 1;
		} else if (diff < 0){
			to = from - 1;
		}

		dir = fromSimpleDir(dirType, (to + num) % num);
	}

	return dir;
}

void Actor::setBox(int box) {
	_walkbox = box;
	setupActorScale();
}

int Actor::actorWalkStep() {
	int tmpX, tmpY;
	Common::Point _actorPos;
	int distX, distY;
	int nextFacing;

	_needRedraw = true;

	nextFacing = updateActorDirection(true);
	if (!(_moving & MF_IN_LEG) || _facing != nextFacing) {
		if (_walkFrame != _frame || _facing != nextFacing) {
			startWalkAnim(1, nextFacing);
		}
		_moving |= MF_IN_LEG;
	}

	_actorPos = _pos;

	if (_walkbox != _walkdata.curbox && _vm->checkXYInBoxBounds(_walkdata.curbox, _actorPos.x, _actorPos.y)) {
		setBox(_walkdata.curbox);
	}

	distX = abs(_walkdata.next.x - _walkdata.cur.x);
	distY = abs(_walkdata.next.y - _walkdata.cur.y);

	if (abs(_actorPos.x - _walkdata.cur.x) >= distX && abs(_actorPos.y - _walkdata.cur.y) >= distY) {
		_moving &= ~MF_IN_LEG;
		return 0;
	}

	tmpX = (_actorPos.x << 16) + _walkdata.xfrac + (_walkdata.deltaXFactor >> 8) * _scalex;
	_walkdata.xfrac = (uint16)tmpX;
	_actorPos.x = (tmpX >> 16);

	tmpY = (_actorPos.y << 16) + _walkdata.yfrac + (_walkdata.deltaYFactor >> 8) * _scaley;
	_walkdata.yfrac = (uint16)tmpY;
	_actorPos.y = (tmpY >> 16);

	if (abs(_actorPos.x - _walkdata.cur.x) > distX) {
		_actorPos.x = _walkdata.next.x;
	}

	if (abs(_actorPos.y - _walkdata.cur.y) > distY) {
		_actorPos.y = _walkdata.next.y;
	}

	_pos = _actorPos;
	return 1;
}


void Actor::setupActorScale() {

	if (_vm->_features & GF_NO_SCALING) {
		_scalex = 0xFF;
		_scaley = 0xFF;
		return;
	}

	if (_ignoreBoxes)
		return;

	// For some boxes, we ignore the scaling and use whatever values the
	// scripts set. This is used e.g. in the Mystery Vortex in Sam&Max.
	// Older games used the flag 0x20 differently, though.
	if (_vm->_gameId == GID_SAMNMAX && (_vm->getBoxFlags(_walkbox) & kBoxIgnoreScale))
		return;

	_boxscale = _vm->getBoxScale(_walkbox);

	uint16 scale = _vm->getScale(_walkbox, _pos.x, _pos.y);
	assert(scale <= 0xFF);

	_scalex = _scaley = (byte)scale;
}

void Actor::startAnimActor(int f) {
	if (_vm->_version >= 7 && !((_vm->_gameId == GID_FT) && (_vm->_features & GF_DEMO) && (_vm->_features & GF_PC))) {
		switch (f) {
		case 1001:
			f = _initFrame;
			break;
		case 1002:
			f = _walkFrame;
			break;
		case 1003:
			f = _standFrame;
			break;
		case 1004:
			f = _talkStartFrame;
			break;
		case 1005:
			f = _talkStopFrame;
			break;
		}

		if (_costume != 0) {
			_animProgress = 0;
			_needRedraw = true;
			if (f == _initFrame)
				_cost.reset();
			_vm->costumeDecodeData(this, f, (uint) - 1);
			_frame = f;
		}
	} else {
		switch (f) {
		case 0x38:
			f = _initFrame;
			break;
		case 0x39:
			f = _walkFrame;
			break;
		case 0x3A:
			f = _standFrame;
			break;
		case 0x3B:
			f = _talkStartFrame;
			break;
		case 0x3C:
			f = _talkStopFrame;
			break;
		}

		assert(f != 0x3E);

		if (isInCurrentRoom() && _costume != 0) {
			_animProgress = 0;
			_cost.animCounter = 0;
			_needRedraw = true;
			// V1 - V2 games don't seem to need a _cost.reset() at this point.
			// Causes Zak to lose his body in several scenes, see bug #771508
			if (_vm->_version >= 3 && f == _initFrame) {
				_cost.reset();
				_auxBlock.visible = false;
			}
			_vm->costumeDecodeData(this, f, (uint) - 1);
			_frame = f;
		}
	}
}

void Actor::animateActor(int anim) {
	int cmd, dir;

	if (_vm->_version >= 7 && !((_vm->_gameId == GID_FT) && (_vm->_features & GF_DEMO) && (_vm->_features & GF_PC))) {

		if (anim == 0xFF)
			anim = 2000;

		cmd = anim / 1000;
		dir = anim % 1000;

	} else {

		cmd = anim / 4;
		dir = oldDirToNewDir(anim % 4);

		// Convert into old cmd code
		cmd = 0x3F - cmd + 2;

	}

	switch (cmd) {
	case 2:				// stop walking
		startAnimActor(_standFrame);
		stopActorMoving();
		break;
	case 3:				// change direction immediatly
		_moving &= ~MF_TURN;
		setDirection(dir);
		break;
	case 4:				// turn to new direction
		turnToDirection(dir);
		break;
	default:
		if (_vm->_version <= 2)
			startAnimActor(anim / 4);
		else
			startAnimActor(anim);
	}
}

void Actor::setDirection(int direction) {
	uint aMask;
	int i;
	uint16 vald;

	// Do nothing if actor is already facing in the given direction
	if (_facing == direction)
		return;

	// Normalize the angle
	_facing = normalizeAngle(direction);

	// If there is no costume set for this actor, we are finished
	if (_costume == 0)
		return;

	// Update the costume for the new direction (and mark the actor for redraw)
	aMask = 0x8000;
	for (i = 0; i < 16; i++, aMask >>= 1) {
		vald = _cost.frame[i];
		if (vald == 0xFFFF)
			continue;
		_vm->costumeDecodeData(this, vald, (_vm->_version <= 2) ? 0xFFFF : aMask);
	}

	_needRedraw = true;
}

void Actor::putActor(int dstX, int dstY, byte newRoom) {
	if (_visible && _vm->_currentRoom != newRoom && _vm->getTalkingActor() == _number) {
		_vm->stopTalk();
	}

	// WORKAROUND: The green transparency of the tank in the Hall of Oddities is
	// is positioned one pixel too far to the left. This appears to be a
	// bug in the original game as well.
	if (_vm->_gameId == GID_SAMNMAX && newRoom == 16 && _number == 5 && dstX == 235 && dstY == 236)
		dstX++;

	_pos.x = dstX;
	_pos.y = dstY;
	_room = newRoom;
	_needRedraw = true;

	if (_vm->VAR(_vm->VAR_EGO) == _number) {
		_vm->_egoPositioned = true;
	}

	if (_visible) {
		if (isInCurrentRoom()) {
			if (_moving) {
				stopActorMoving();
				startAnimActor(_standFrame);
			}
			adjustActorPos();
		} else {
			if (_vm->_heversion >= 71)
				_vm->queueAuxBlock(this);
			hideActor();
		}
	} else {
		if (isInCurrentRoom())
			showActor();
	}
}

int Actor::getActorXYPos(int &xPos, int &yPos) const {
	if (!isInCurrentRoom())
		return -1;

	xPos = _pos.x;
	yPos = _pos.y;
	return 0;
}

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY) {
	const uint thresholdTable[] = { 30, 80, 0 };
	AdjustBoxResult abr;
	int16 tmpX, tmpY;
	int tmpDist, bestDist, threshold, numBoxes;
	byte flags, bestBox;
	int box;
	const int firstValidBox = (_vm->_features & GF_SMALL_HEADER) ? 0 : 1;

	abr.x = dstX;
	abr.y = dstY;
	abr.box = kInvalidBox;

	if (_ignoreBoxes)
		return abr;

	for (int tIdx = 0; tIdx < ARRAYSIZE(thresholdTable); tIdx++) {
		threshold = thresholdTable[tIdx];

		numBoxes = _vm->getNumBoxes() - 1;
		if (numBoxes < firstValidBox)
			return abr;

		bestDist = (_vm->_version >= 7) ? 0x7FFFFFFF : 0xFFFF;
		if (_vm->_version <= 2)
			bestDist *= 8*2;	// Adjust for the fact that we multiply x by 8 and y by 2
		bestBox = kInvalidBox;

		// We iterate (backwards) over all boxes, searching the one closest
		// to the desired coordinates.
		for (box = numBoxes; box >= firstValidBox; box--) {
			flags = _vm->getBoxFlags(box);

			// Skip over invisible boxes
			if (flags & kBoxInvisible && !(flags & kBoxPlayerOnly && !isPlayer()))
				continue;
			
			// For increased performance, we perform a quick test if
			// the coordinates can even be within a distance of 'threshold'
			// pixels of the box.
			if (threshold > 0 && _vm->inBoxQuickReject(box, dstX, dstY, threshold))
				continue;

			// Check if the point is contained in the box. If it is,
			// we don't have to search anymore.
			if (_vm->checkXYInBoxBounds(box, dstX, dstY)) {
				abr.x = dstX;
				abr.y = dstY;
				abr.box = box;
				return abr;
			}

			// Find the point in the box which is closest to our point.
			tmpDist = _vm->getClosestPtOnBox(box, dstX, dstY, tmpX, tmpY);

			// Check if the box is closer than the previous boxes.
			if (tmpDist < bestDist) {
				abr.x = tmpX;
				abr.y = tmpY;
	
				if (tmpDist == 0) {
					abr.box = box;
					return abr;
				}
				bestDist = tmpDist;
				bestBox = box;
			}
		}

		// If the closest ('best') box we found is within the threshold, or if
		// we are on the last run (i.e. threshold == 0), return that box.
		if (threshold == 0 || threshold * threshold >= bestDist) {
			abr.box = bestBox;
			return abr;
		}
	}

	return abr;
}

void Actor::adjustActorPos() {
	AdjustBoxResult abr;

	abr = adjustXYToBeInBox(_pos.x, _pos.y);

	_pos.x = abr.x;
	_pos.y = abr.y;
	_walkdata.destbox = abr.box;

	setBox(abr.box);

	_walkdata.dest.x = -1;

	stopActorMoving();
	_cost.soundCounter = 0;

	if (_walkbox != kInvalidBox) {
		byte flags = _vm->getBoxFlags(_walkbox);
		if (flags & 7) {
			turnToDirection(_facing);
		}
	}
}

void Actor::faceToObject(int obj) {
	int x2, y2, dir;
	
	if (!isInCurrentRoom())
		return;

	if (_vm->getObjectOrActorXY(obj, x2, y2) == -1)
		return;

	dir = (x2 > _pos.x) ? 90 : 270;
	turnToDirection(dir);
}

void Actor::turnToDirection(int newdir) {
	if (newdir == -1 || _ignoreTurns)
		return;

	_moving &= ~MF_TURN;

	if (newdir != _facing) {
		if (_vm->_version <= 3)
			_moving = MF_TURN;
		else
			_moving |= MF_TURN;
		_targetFacing = newdir;
	}
}

void Actor::hideActor() {
	if (!_visible)
		return;

	if (_moving) {
		stopActorMoving();
		startAnimActor(_standFrame);
	}
	_visible = false;
	_cost.soundCounter = 0;
	_needRedraw = false;
	_needBgReset = true;
	_auxBlock.visible = false;
}

void Actor::showActor() {
	if (_vm->_currentRoom == 0 || _visible)
		return;

	adjustActorPos();

	_vm->ensureResourceLoaded(rtCostume, _costume);

	if (_costumeNeedsInit) {
		startAnimActor(_initFrame);
		if (_vm->_version <= 2) {
			startAnimActor(_standFrame);
			startAnimActor(_talkStopFrame);
		}
		_costumeNeedsInit = false;
	}

	// FIXME: Evil hack to work around bug #770717
	if (!_moving && _vm->_version <= 2)
		startAnimActor(_standFrame);

	stopActorMoving();
	_visible = true;
	_needRedraw = true;
}

// V1 Maniac doesn't have a ScummVar for VAR_TALK_ACTOR, and just uses
// an internal variable. Emulate this to prevent overwriting script vars...
int ScummEngine::getTalkingActor() {
	if (_gameId == GID_MANIAC && _version == 1)
		return _V1TalkingActor;
	else
		return VAR(VAR_TALK_ACTOR);
}

void ScummEngine::setTalkingActor(int value) {
	if (_gameId == GID_MANIAC && _version == 1)
		_V1TalkingActor = value;
	else
		VAR(VAR_TALK_ACTOR) = value;
}

void ScummEngine::putActors() {
	Actor *a;
	int i;

	for (i = 1; i < _numActors; i++) {
		a = &_actors[i];
		if (a && a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->_room);
	}
}

static const int v1MMActorTalkColor[25] = {
	1, 7, 2, 14, 8, 1, 3, 7, 7, 12, 1, 13, 1, 4, 5, 5, 4, 3, 1, 5, 1, 1, 1, 7, 7
};

void ScummEngine::setupV1ActorTalkColor() {
	int i;

	for (i = 1; i < _numActors; i++)
		_actors[i]._talkColor = v1MMActorTalkColor[i];
}

void ScummEngine::showActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			_actors[i].showActor();
	}
}

void ScummEngine::walkActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			if (_version <= 3)
				_actors[i].walkActorOld();
			else
				_actors[i].walkActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void ScummEngine::playActorSounds() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i]._cost.soundCounter && _actors[i].isInCurrentRoom() && _actors[i]._sound) {
			_currentScript = 0xFF;
			_sound->addSoundToQueue(_actors[i]._sound[0]);
			for (i = 1; i < _numActors; i++) {
				_actors[i]._cost.soundCounter = 0;
			}
			return;
		}
	}
}

Actor *ScummEngine::derefActor(int id, const char *errmsg) const {
	if (id == 0)
		debugC(DEBUG_ACTORS, "derefActor(0, \"%s\") in script %d, opcode 0x%x", 
			errmsg, vm.slot[_curExecScript].number, _opcode);

	if (id < 0 || id >= _numActors || _actors[id]._number != id) {
		if (errmsg)
			error("Invalid actor %d in %s", id, errmsg);
		else
			error("Invalid actor %d", id);
	}
	return &_actors[id];
}

Actor *ScummEngine::derefActorSafe(int id, const char *errmsg) const {
	if (id == 0)
		debugC(DEBUG_ACTORS, "derefActorSafe(0, \"%s\") in script %d, opcode 0x%x", 
			errmsg, vm.slot[_curExecScript].number, _opcode);

	if (id < 0 || id >= _numActors || _actors[id]._number != id) {
		debugC(DEBUG_ACTORS, "Invalid actor %d in %s (script %d, opcode 0x%x)",
			 id, errmsg, vm.slot[_curExecScript].number, _opcode);
		return NULL;
	}
	return &_actors[id];
}

static int compareDrawOrder(const void* a, const void* b)
{
	const Actor* actor1 = *(const Actor *const*)a;
	const Actor* actor2 = *(const Actor *const*)b;
	int diff;

	// The actor in the higher layer is ordered lower
	diff = actor1->_layer - actor2->_layer;
	if (diff < 0)
		return +1;
	if (diff > 0)
		return -1;

	// The actor with higher y value is ordered higher
	diff = actor1->_pos.y - actor2->_pos.y;
	if (diff < 0)
		return -1;
	if (diff > 0)
		return +1;

	// FIXME: This hack works around bug #775097. It's probably wrong, though :-/
	// Would be interesting if somebody could check the disassembly (see also the
	// comment on the above mentioned tracker item).
	if (g_scumm->_gameId == GID_TENTACLE) {
		diff = actor1->_forceClip - actor2->_forceClip;
		if (diff < 0)
			return -1;
		if (diff > 0)
			return +1;
	}

	// The qsort() function is not guaranteed to be stable (i.e. it may
	// re-order "equal" elements in an array it sorts). Hence we use the
	// actor number as tie-breaker. This is needed for the Sam & Max intro,
	// and possibly other cases as well. See bug #758167.

	return actor1->_number - actor2->_number;
}

void ScummEngine::processActors() {
	if (_skipProcessActors)
		return;

	int numactors = 0;

	// TODO : put this actors as a member array. It never has to grow or shrink
	// since _numActors is constant within a game.
	Actor** actors = new Actor * [_numActors];
	
	// Make a list of all actors in this room
	for (int i = 1; i < _numActors; i++) {
		if (_version == 8 && _actors[i]._layer < 0)
			continue;
		if (_actors[i].isInCurrentRoom() && _actors[i]._costume)
			actors[numactors++] = &_actors[i];
	}
	if (!numactors) {
		delete [] actors;
		return;
	}

	// Sort actors by position before we draw them (to ensure that actors in
	// front are drawn after those "behind" them).
	qsort(actors, numactors, sizeof (Actor*), compareDrawOrder);

	Actor** end = actors + numactors;

	// Finally draw the now sorted actors
	for (Actor** ac = actors; ac != end; ++ac) {
		Actor* a = *ac;
		CHECK_HEAP
		a->drawActorCostume();
		CHECK_HEAP
		a->animateCostume();
	}
	
	delete [] actors;

	if (_features & GF_NEW_COSTUMES)
		akos_processQueue();
}

// Used in Scumm v8, to allow the verb coin to be drawn over the inventory
// chest. I'm assuming that draw order won't matter here.
void ScummEngine::processUpperActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom() && _actors[i]._costume && _actors[i]._layer < 0) {
			CHECK_HEAP
			_actors[i].drawActorCostume();
			CHECK_HEAP
			_actors[i].animateCostume();
		}
	}
}

void Actor::drawActorCostume(bool hitTestMode) {
	if (!hitTestMode) {
		if (!_needRedraw)
			return;
	
		_needRedraw = false;
	}

	setupActorScale();

	BaseCostumeRenderer* bcr = _vm->_costumeRenderer;

	bcr->_actorID = _number;

	bcr->_actorX = _pos.x + _offsX - _vm->virtscr[0].xstart;
	bcr->_actorY = _pos.y + _offsY - _elevation;

	if ((_vm->_version <= 2) && !(_vm->_features & GF_NES)) {
		// HACK: We have to adjust the x position by one strip (8 pixels) in
		// V2 games. However, it is not quite clear to me why. And to fully
		// match the original, it seems we have to offset by 2 strips if the
		// actor is facing left (270 degree).
		// V1 games are once again slightly different, here we only have
		// to adjust the 270 degree case...
		if (_facing == 270)
			bcr->_actorX += 16;
		else if (_vm->_version == 2)
			bcr->_actorX += 8;
	}

	bcr->_clipOverride = _clipOverride;

	if (_vm->_version == 4 && _boxscale & 0x8000) {
		bcr->_scaleX = bcr->_scaleY = _vm->getScaleFromSlot((_boxscale & 0x7fff) + 1, _pos.x, _pos.y);
	} else {
		bcr->_scaleX = _scalex;
		bcr->_scaleY = _scaley;
	}

	bcr->_shadow_mode = _shadowMode;
	if ((_vm->_features & GF_SMALL_HEADER) || _vm->_heversion >= 71)
		bcr->_shadow_table = NULL;
	else if (_vm->_heversion == 70)
		bcr->_shadow_table = _vm->_HEV7ActorPalette;
	else
		bcr->_shadow_table = _vm->_shadowPalette;

	bcr->setCostume(_costume);
	bcr->setPalette(_palette);
	bcr->setFacing(this);

	if (_vm->_version >= 7) {

		bcr->_zbuf = _forceClip;
		if (bcr->_zbuf == 100) {
			bcr->_zbuf = _vm->getMaskFromBox(_walkbox);
			if (bcr->_zbuf > _vm->gdi._numZBuffer-1)
				bcr->_zbuf = _vm->gdi._numZBuffer-1;
		}

	} else {
		if (_forceClip)
			bcr->_zbuf = _forceClip;
		else if (isInClass(kObjectClassNeverClip))
			bcr->_zbuf = 0;
		else {
			bcr->_zbuf = _vm->getMaskFromBox(_walkbox);
			if (bcr->_zbuf > _vm->gdi._numZBuffer-1)
				bcr->_zbuf = _vm->gdi._numZBuffer-1;
		}

	}

	bcr->_draw_top = 0x7fffffff;
	bcr->_draw_bottom = 0;

	bcr->_skipLimbs = (_heSkipLimbs != 0);
	bcr->_paletteNum = _hePaletteNum;
	
	if (_vm->_heversion >= 80 && _heNoTalkAnimation == 0) {
		_heCondMask &= 0xFFFFFC00;
		_heCondMask |= 1;
		if (_vm->getTalkingActor() == _number) {
			// Checks if talk sound is active?
			// Otherwise just do rand animation
			int rnd = _vm->_rnd.getRandomNumberRng(1, 10);
			setTalkCondition(rnd);
		} 
	}
	_heNoTalkAnimation = 0;

	// If the actor is partially hidden, redraw it next frame.
	// Only done for pre-AKOS, though.
	if (bcr->drawCostume(_vm->virtscr[0], _vm->gdi._numStrips, this, _drawToBackBuf) & 1) {
		_needRedraw = (_vm->_version <= 6);
	}

	if (!hitTestMode) {
		// Record the vertical extent of the drawn actor
		_top = bcr->_draw_top;
		_bottom = bcr->_draw_bottom;
	}
}

bool Actor::actorHitTest(int x, int y) {
	AkosRenderer *ar = (AkosRenderer *)_vm->_costumeRenderer;

	ar->_actorHitX = x;
	ar->_actorHitY = y;
	ar->_actorHitMode = true;
	ar->_actorHitResult = false;

	drawActorCostume(true);

	ar->_actorHitMode = false;
	
	return ar->_actorHitResult;
}

void Actor::animateCostume() {
	if (_costume == 0)
		return;

	_animProgress++;
	if (_animProgress >= _animSpeed) {
		_animProgress = 0;

		if (_vm->_features & GF_NEW_COSTUMES) {
			byte *akos = _vm->getResourceAddress(rtCostume, _costume);
			assert(akos);
			if (_vm->akos_increaseAnims(akos, this)) {
				_needRedraw = true;
			}
		} else {
			LoadedCostume lc(_vm);
			lc.loadCostume(_costume);
			if (lc.increaseAnims(this)) {
				_needRedraw = true;
			}
		}
	}
}

void Actor::animateLimb(int limb, int f) {
	// This methods is very similiar to animateCostume(). 
	// However, instead of animating *all* the limbs, it only animates
	// the specified limb to be at the frame specified by "f". 

	if (!f)
		return;

	_animProgress++;
	if (_animProgress >= _animSpeed) {
		_animProgress = 0;

		if (_costume == 0)
			return;

		const byte *aksq, *akfo;
		uint size;
		byte *akos = _vm->getResourceAddress(rtCostume, _costume);
		assert(akos);

		aksq = _vm->findResourceData(MKID('AKSQ'), akos);
		akfo = _vm->findResourceData(MKID('AKFO'), akos);
	
		size = _vm->getResourceDataSize(akfo) / 2;
	
		while (f--) {
			if (_cost.active[limb] != 0)
				_vm->akos_increaseAnim(this, limb, aksq, (const uint16 *)akfo, size);
		}

//		_needRedraw = true;
//		_needBgReset = true;
	}
}

void ScummEngine::setActorRedrawFlags() {
	int i, j;

	if (_fullRedraw) {
		for (j = 1; j < _numActors; j++) {
			_actors[j]._needRedraw = true;
		}
	} else {
		for (i = 0; i < gdi._numStrips; i++) {
			int strip = _screenStartStrip + i;
			if (testGfxAnyUsageBits(strip)) {
				for (j = 1; j < _numActors; j++) {
					if (testGfxUsageBit(strip, j) && testGfxOtherUsageBits(strip, j)) {
						_actors[j]._needRedraw = true;
					}
				}
			}
		}
	}
}

void ScummEngine::resetActorBgs() {
	int i, j;

	for (i = 0; i < gdi._numStrips; i++) {
		int strip = _screenStartStrip + i;
		clearGfxUsageBit(strip, USAGE_BIT_DIRTY);
		clearGfxUsageBit(strip, USAGE_BIT_RESTORED);
		for (j = 1; j < _numActors; j++) {
			if (testGfxUsageBit(strip, j) &&
				((_actors[j]._top != 0x7fffffff && _actors[j]._needRedraw) || _actors[j]._needBgReset)) {
				clearGfxUsageBit(strip, j);
				if ((_actors[j]._bottom - _actors[j]._top) >= 0)
					gdi.resetBackground(_actors[j]._top, _actors[j]._bottom, i);
			}
		}
	}

	for (i = 1; i < _numActors; i++) {
		_actors[i]._needBgReset = false;
	}
}

int ScummEngine::getActorFromPos(int x, int y) {
	int i;

	if (!testGfxAnyUsageBits(x / 8))
		return 0;
	for (i = 1; i < _numActors; i++) {
		if (testGfxUsageBit(x / 8, i) && !getClass(i, kObjectClassUntouchable)
			&& y >= _actors[i]._top && y <= _actors[i]._bottom) {
			if (_version > 2 || i != VAR(VAR_EGO))
				return i;
		}
	}
	return 0;
}

void ScummEngine::actorTalk(const byte *msg) {
	Actor *a;

	_lastStringTag[0] = 0;
	addMessageToStack(msg, _charsetBuffer, sizeof(_charsetBuffer));
	
	// Play associated speech, if any
	playSpeech((byte *)_lastStringTag);

	// FIXME: Workaround for bugs #770039 and #770049 
	if (_gameId == GID_LOOM || _gameId == GID_LOOM256) {
		if (!*_charsetBuffer)
			return;
	}

	if (_actorToPrintStrFor == 0xFF) {
		if ((_version <= 7 && !_keepText) || (_version == 8 && VAR(VAR_HAVE_MSG))) {
			stopTalk();
		}
		setTalkingActor(0xFF);
	} else {
		int oldact;
		
		// FIXME: Workaround for bug #770724
		if (_gameId == GID_LOOM && _roomResource == 23 &&
			vm.slot[_currentScript].number == 232 && _actorToPrintStrFor == 0) {
			_actorToPrintStrFor = 2;	// Could be anything from 2 to 5. Maybe compare to original?
		}
		
		a = derefActor(_actorToPrintStrFor, "actorTalk");
		if (!a->isInCurrentRoom() && (_version <= 6)) {
			oldact = 0xFF;
		} else {
			if ((_version <= 7 && !_keepText) || (_version == 8 && VAR(VAR_HAVE_MSG)))
				stopTalk();
			setTalkingActor(a->_number);
			a->_heTalking = true;
			if (!_string[0].no_talk_anim) {
				a->runActorTalkScript(a->_talkStartFrame);
				_useTalkAnims = true;
			}
			oldact = getTalkingActor();
		}
		if (oldact >= 0x80)
			return;
	}

	if (_heversion >= 72 || getTalkingActor() > 0x7F) {
		_charsetColor = (byte)_string[0].color;
	} else {
		a = derefActor(getTalkingActor(), "actorTalk(2)");
		_charsetColor = a->_talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	if (_version <= 7)
		VAR(VAR_HAVE_MSG) = 0xFF;
	if (VAR_CHARCOUNT != 0xFF)
		VAR(VAR_CHARCOUNT) = 0;
	CHARSET_1();
}

void Actor::runActorTalkScript(int f) {
	if (_vm->_version == 8 && _vm->VAR(_vm->VAR_HAVE_MSG) == 2) 
		return;

	if (_talkScript) {
		int script = _talkScript;
		int args[16];
		memset(args, 0, sizeof(args));
		args[1] = f;
		args[0] = _number;

		_vm->runScript(script, 1, 0, args);
	} else {
		if (_frame != f)
			startAnimActor(f);
	}
}

void ScummEngine::stopTalk() {
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = getTalkingActor();
	if (act && act < 0x80) {
		Actor *a = derefActor(act, "stopTalk");
		if ((_version >= 7 && !_string[0].no_talk_anim) ||
			(_version <= 6 && a->isInCurrentRoom() && _useTalkAnims)) {
			a->runActorTalkScript(a->_talkStopFrame);
			_useTalkAnims = false;
		}
		if (_version <= 7 && !(_features & GF_HUMONGOUS))
			setTalkingActor(0xFF);
		a->_heTalking = false;
	}
	if (_version == 8 || _features & GF_HUMONGOUS)
		setTalkingActor(0);
	if (_version == 8)
		VAR(VAR_HAVE_MSG) = 0;
	_keepText = false;
	_charset->restoreCharsetBg();
}

void Actor::setActorCostume(int c) {
	int i;

	if ((_vm->_features & GF_HUMONGOUS) && (c == -1  || c == -2)) {
		_heSkipLimbs = (c == -1);
		_needRedraw = true;
		return;
	}

	// Based on disassembly. It seems that high byte is not used at all, though
	// it is attached to all horizontally flipped object, like left eye.
	if (_vm->_heversion == 60)
		c &= 0xff;

	_costumeNeedsInit = true;
	
	if (_vm->_features & GF_NEW_COSTUMES) {
		_cost.reset();
		_auxBlock.visible = false;
		memset(_animVariable, 0, sizeof(_animVariable));
		_costume = c;
		
		if (_vm->_heversion >= 71)
			_vm->queueAuxBlock(this);
		
		if (_visible) {
			if (_costume) {
				_vm->ensureResourceLoaded(rtCostume, _costume);
			}
			startAnimActor(_initFrame);
		}
	} else {
		if (_visible) {
			hideActor();
			_cost.reset();
			_costume = c;
			showActor();
		} else {
			_costume = c;
			_cost.reset();
		}
	}


	// V1 zak uses palette[] as a dynamic costume color array.
	if (_vm->_version == 1)
		return;

	if (_vm->_features & GF_NEW_COSTUMES) {
		for (i = 0; i < 256; i++)
			_palette[i] = 0xFF;
	} else if (_vm->_features & GF_OLD_BUNDLE) {
		for (i = 0; i < 16; i++)
			_palette[i] = i;

		// Make stuff more visible on CGA. Based on disassembly
		if (_vm->_renderMode == Common::kRenderCGA && _vm->_version > 2) {
			_palette[6] = 5;
			_palette[7] = 15;
		}
	} else {
		for (i = 0; i < 32; i++)
			_palette[i] = 0xFF;
	}
}

void Actor::startWalkActor(int destX, int destY, int dir) {
	AdjustBoxResult abr;

	if (_vm->_version <= 3) {
		abr.x = destX;
		abr.y = destY;
	} else {
		abr = adjustXYToBeInBox(destX, destY);
	}

	if (!isInCurrentRoom()) {
		_pos.x = abr.x;
		_pos.y = abr.y;
		if (!(_vm->_version == 6 && _ignoreTurns) && dir != -1)
			setDirection(dir);
		return;
	}

	if (_ignoreBoxes) {
		abr.box = kInvalidBox;
		_walkbox = kInvalidBox;
	} else {
		if (_vm->checkXYInBoxBounds(_walkdata.destbox, abr.x, abr.y)) {
			abr.box = _walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(abr.x, abr.y);
		}
		if (_moving && _walkdata.destdir == dir && _walkdata.dest.x == abr.x && _walkdata.dest.y == abr.y)
			return;
	}

	if (_pos.x == abr.x && _pos.y == abr.y) {
		turnToDirection(dir);
		return;
	}

	_walkdata.dest.x = abr.x;
	_walkdata.dest.y = abr.y;
	_walkdata.destbox = abr.box;
	_walkdata.destdir = dir;
	_moving = (_moving & MF_IN_LEG) | MF_NEW_LEG;
	_walkdata.point3.x = 32000;

	_walkdata.curbox = _walkbox;
}

void Actor::startWalkAnim(int cmd, int angle) {
	if (angle == -1)
		angle = _facing;

	/* Note: walk scripts aren't required to make the Dig
	 * work as usual
	 */
	if (_walkScript) {
		int args[16];
		memset(args, 0, sizeof(args));
		args[0] = _number;
		args[1] = cmd;
		args[2] = angle;
		_vm->runScript(_walkScript, 1, 0, args);
	} else {
		switch (cmd) {
		case 1:										/* start walk */
			setDirection(angle);
			startAnimActor(_walkFrame);
			break;
		case 2:										/* change dir only */
			setDirection(angle);
			break;
		case 3:										/* stop walk */
			turnToDirection(angle);
			startAnimActor(_standFrame);
			break;
		}
	}
}

void Actor::walkActor() {
	int new_dir, next_box;
	Common::Point foundPath;

	if (_vm->_version >= 7) {
		if (_moving & MF_FROZEN) {
			if (_moving & MF_TURN) {
				new_dir = updateActorDirection(false);
				if (_facing != new_dir)
					setDirection(new_dir);
				else
					_moving &= ~MF_TURN;
			}
			return;
		}
	}

	if (!_moving)
		return;

	if (!(_moving & MF_NEW_LEG)) {
		if (_moving & MF_IN_LEG && actorWalkStep())
			return;

		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			setBox(_walkdata.destbox);
			startWalkAnim(3, _walkdata.destdir);
			return;
		}

		if (_moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (_facing != new_dir)
				setDirection(new_dir);
			else
				_moving = 0;
			return;
		}

		setBox(_walkdata.curbox);
		_moving &= MF_IN_LEG;
	}

	_moving &= ~MF_NEW_LEG;
	do {

		if (_walkbox == kInvalidBox) {
			setBox(_walkdata.destbox);
			_walkdata.curbox = _walkdata.destbox;
			break;
		}

		if (_walkbox == _walkdata.destbox)
			break;

		next_box = _vm->getPathToDestBox(_walkbox, _walkdata.destbox);
		if (next_box < 0) {
			_walkdata.destbox = _walkbox;
			_moving |= MF_LAST_LEG;
			return;
		}

		_walkdata.curbox = next_box;
		
		if (findPathTowards(_walkbox, next_box, _walkdata.destbox, foundPath))
			break;

		if (calcMovementFactor(foundPath))
			return;

		setBox(_walkdata.curbox);
	} while (1);

	_moving |= MF_LAST_LEG;
	calcMovementFactor(_walkdata.dest);
}

/*
void Actor::walkActorV12() {
	Common::Point foundPath, tmp;
	int new_dir, next_box;

	if (_moving & MF_TURN) {
		new_dir = updateActorDirection(false);
		if (_facing != new_dir)
			setDirection(new_dir);
		else
			_moving = 0;
		return;
	}
	
	if (!_moving)
		return;
	
	if (_moving & MF_IN_LEG) {
		actorWalkStep();
	} else {
		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			startWalkAnim(3, _walkdata.destdir);
		} else {
			setBox(_walkdata.curbox);
			if (_walkbox == _walkdata.destbox) {
				foundPath = _walkdata.dest;
				_moving |= MF_LAST_LEG;
			} else {
				next_box = _vm->getPathToDestBox(_walkbox, _walkdata.destbox);
				if (next_box < 0) {
					_moving |= MF_LAST_LEG;
					return;
				}
		
				// Can't walk through locked boxes
				int flags = _vm->getBoxFlags(next_box);
				if (flags & kBoxLocked && !(flags & kBoxPlayerOnly && !isPlayer())) {
					_moving |= MF_LAST_LEG;
				}

				_walkdata.curbox = next_box;

				_vm->getClosestPtOnBox(_walkdata.curbox, x, y, tmp.x, tmp.y);
				_vm->getClosestPtOnBox(_walkbox, tmp.x, tmp.y, foundPath.x, foundPath.y);
			}
			calcMovementFactor(foundPath);
		}
	}
}
*/

void Actor::walkActorOld() {
	Common::Point p2, p3;	// Gate locations
	int new_dir, next_box, loopCtr = 0;

	if (!_moving)
		return;
	
	if (!(_moving & MF_NEW_LEG)) {
		if (_moving & MF_IN_LEG && actorWalkStep())
			return;

		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			startWalkAnim(3, _walkdata.destdir);
			return;
		}

		if (_moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (_facing != new_dir)
				setDirection(new_dir);
			else
				_moving = 0;
			return;
		}

		if (_walkdata.point3.x != 32000) {
			if (calcMovementFactor(_walkdata.point3)) {
				_walkdata.point3.x = 32000;
				return;
			}
			_walkdata.point3.x = 32000;
		}

		setBox(_walkdata.curbox);
		_moving &= MF_IN_LEG;
	}

	_moving &= ~MF_NEW_LEG;
	do {
		loopCtr++;

		if (_walkbox == kInvalidBox) {
			setBox(_walkdata.destbox);
			_walkdata.curbox = _walkdata.destbox;
			break;
		}

		if (_walkbox == _walkdata.destbox)
			break;

		next_box = _vm->getPathToDestBox(_walkbox, _walkdata.destbox);

		// WORKAROUND: To fully fix bug #774783, we add a special case 
		// here, resulting in a different next_box value for Hitler.
		if ((_vm->_gameId == GID_INDY3) && _vm->_roomResource == 46 && _walkbox == 1 && _walkdata.destbox == 0 && _number == 9)
			next_box = 1;

		if (next_box < 0) {
			_moving |= MF_LAST_LEG;
			return;
		}
		
		// Can't walk through locked boxes
		int flags = _vm->getBoxFlags(next_box);
		if (flags & kBoxLocked && !(flags & kBoxPlayerOnly && !isPlayer())) {
			_moving |= MF_LAST_LEG;
// FIXME: Work in progress
//			_walkdata.destdir = _facing;
			return;
		}

		_walkdata.curbox = next_box;

		if (_vm->_version <= 2) {
			_vm->getClosestPtOnBox(_walkdata.curbox, _pos.x, _pos.y, p2.x, p2.y);
			_vm->getClosestPtOnBox(_walkbox, p2.x, p2.y, p3.x, p3.y);
// FIXME: Work in progress
//			calcMovementFactor(p3);
//			return;
		} else {
			findPathTowardsOld(_walkbox, next_box, _walkdata.destbox, p2, p3);
			if (p2.x == 32000 && p3.x == 32000) {
				break;
			}
	
			if (p2.x != 32000) {
				if (calcMovementFactor(p2)) {
					_walkdata.point3 = p3;
					return;
				}
			}
		}
		if (calcMovementFactor(p3))
			return;

		setBox(_walkdata.destbox);

		// FIXME: Ender added this recursion counter as a hack around
		//        a infinite loop in Maniac V1 - see bug #862245
		if (loopCtr > 100) {
			_moving |= MF_LAST_LEG;
			return;
		}
	} while (1);

	_moving |= MF_LAST_LEG;
	calcMovementFactor(_walkdata.dest);
}

byte *Actor::getActorName() {
	byte *ptr = _vm->getResourceAddress(rtActorName, _number);
	if (ptr == NULL) {
		warning("Failed to find name of actor %d", _number);
	}
	return ptr;
}

void Actor::remapActorPaletteColor(int color, int new_color) {
	const byte *akos, *akpl;
	int akpl_size, i;
	byte akpl_color;

	akos = _vm->getResourceAddress(rtCostume, _costume);
	if (!akos) {
		warning("Can't remap actor %d, costume %d not found", _number, _costume);
		return;
	}

	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	if (!akpl) {
		warning("Can't remap actor %d, costume %d doesn't contain an AKPL block", _number, _costume);
		return;
	}

	// Get the number palette entries
	akpl_size = _vm->getResourceDataSize(akpl);

	for (i = 0; i < akpl_size; i++) {
		akpl_color = *akpl++;
		if (akpl_color == color) {
			_palette[i] = new_color;
			return;
		}
	}
}

void Actor::remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold) {
	const byte *akos, *rgbs, *akpl;
	int akpl_size, i;
	int r, g, b;
	byte akpl_color;

	if (!isInCurrentRoom()) {
		debugC(DEBUG_ACTORS, "Remap actor %d not in current room", _number);
		return;
	} else if (_costume < 1 || _costume >= _vm->_numCostumes - 1) {
		debugC(DEBUG_ACTORS, "Remap actor %d invalid costume %d", _number, _costume);
		return;
	}

	akos = _vm->getResourceAddress(rtCostume, _costume);
	if (!akos) {
		warning("Can't remap actor %d, costume %d not found", _number, _costume);
		return;
	}

	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	if (!akpl) {
		warning("Can't remap actor %d, costume %d doesn't contain an AKPL block", _number, _costume);
		return;
	}

	// Get the number palette entries
	akpl_size = _vm->getResourceDataSize(akpl);

	rgbs = _vm->findResourceData(MKID('RGBS'), akos);

	if (!rgbs) {
		debugC(DEBUG_ACTORS, "Can't remap actor %d costume %d doesn't contain an RGB block", _number, _costume);
		return;
	}

	for (i = 0; i < akpl_size; i++) {
		r = *rgbs++;
		g = *rgbs++;
		b = *rgbs++;

		akpl_color = *akpl++;

		// allow remap of generic palette entry?
		if (!_shadowMode || akpl_color >= 16) {
			r = (r * r_fact) >> 8;
			g = (g * g_fact) >> 8;
			b = (b * b_fact) >> 8;
			_palette[i] = _vm->remapPaletteColor(r, g, b, threshold);
		}
	}
}

void Actor::classChanged(int cls, bool value) {
	if (cls == kObjectClassAlwaysClip)
		_forceClip = value;
	if (cls == kObjectClassIgnoreBoxes)
		_ignoreBoxes = value;
}

bool Actor::isInClass(int cls) {
	return _vm->getClass(_number, cls);
}

bool Actor::isPlayer() {
	if (_vm->_version <= 2)
		return _vm->VAR(42) <= _number && _number <= _vm->VAR(43);
	else
		return isInClass(kObjectClassPlayer);
}

void Actor::setUserCondition(int slot, int set) {
	debug(1, "Actor::setUserCondition(%d, %d)", slot, set);
	assert(slot >= 1 && slot <= 0x20);
	if (set == 0) {
		_heCondMask &= ~(1 << (slot + 0xF));
	} else {
		_heCondMask |= 1 << (slot + 0xF);
	}
	if (_heCondMask & 0x3FF) {
		_heCondMask &= ~1;
	} else {
		_heCondMask |= 1;
	}
}

bool Actor::isUserConditionSet(int slot) const {
	assert(slot >= 1 && slot <= 0x20);
	return (_heCondMask & (1 << (slot + 0xF))) != 0;
}

void Actor::setTalkCondition(int slot) {
	debug(1, "Actor::setTalkCondition(%d)", slot);
	assert(slot >= 1 && slot <= 0x10);
	_heCondMask = (_heCondMask & ~0x3FF) | 1;
	if (slot != 1) {
		_heCondMask |= 1 << (slot - 1);
		if (_heCondMask & 0x3FF) {
			_heCondMask &= ~1;
		} else {
			_heCondMask |= 1;
		}
	}	
}

bool Actor::isTalkConditionSet(int slot) const {	
	assert(slot >= 1 && slot <= 0x10);
	return (_heCondMask & (1 << (slot - 1))) != 0;
}

void ScummEngine::preProcessAuxQueue() {
	if (!_skipProcessActors) {
		for (int i = 0; i < _auxBlocksNum; ++i) {
			AuxBlock *ab = &_auxBlocks[i];
			if (ab->visible) {
				assert(ab->r.top <= ab->r.bottom);
				gdi.copyVirtScreenBuffers(ab->r);
			}
		}
	}
	_auxBlocksNum = 0;
}

void ScummEngine::postProcessAuxQueue() {
	if (!_skipProcessActors) {
		for (int i = 0; i < _auxEntriesNum; ++i) {
			AuxEntry *ae = &_auxEntries[i];
			if (ae->actorNum != -1) {
				Actor *a = derefActor(ae->actorNum, "postProcessAuxQueue");
				const uint8 *cost = getResourceAddress(rtCostume, a->_costume);
				int dy = a->_offsY + a->_pos.y - a->getElevation();
				int dx = a->_offsX + a->_pos.x;

				const uint8 *akax = findResource(MKID('AKAX'), cost);
				assert(akax);
				const uint8 *auxd = findPalInPals(akax, ae->subIndex) - _resourceHeaderSize;
				assert(auxd);
				const uint8 *frel = findResourceData(MKID('FREL'), auxd);
				if (frel) {
					warning("unhandled FREL block");
				}
				const uint8 *disp = findResourceData(MKID('DISP'), auxd);
				if (disp) {
					warning("unhandled DISP block");
				}
				const uint8 *axfd = findResourceData(MKID('AXFD'), auxd);
				assert(axfd);

				uint16 comp = READ_LE_UINT16(axfd);
				if (comp != 0) {
					int x = (int16)READ_LE_UINT16(axfd + 2) + dx;
					int y = (int16)READ_LE_UINT16(axfd + 4) + dy;
					int w = (int16)READ_LE_UINT16(axfd + 6);
					int h = (int16)READ_LE_UINT16(axfd + 8);
					VirtScreen *pvs = &virtscr[kMainVirtScreen];
					uint8 *dst1 = pvs->getPixels(0, pvs->topline);
					uint8 *dst2 = pvs->getBackPixels(0, pvs->topline);
					switch (comp) {
					case 1:
						Wiz::copyAuxImage(dst1, dst2, axfd + 10, pvs->w, pvs->h, x, y, w, h);
						break;
					default:
						warning("unimplemented compression type %d", comp);
						break;
					}
				}
				const uint8 *axur = findResourceData(MKID('AXUR'), auxd);
				if (axur) {
					uint16 n = READ_LE_UINT16(axur); axur += 2;
					while (n--) {
						int x1 = (int16)READ_LE_UINT16(axur + 0) + dx;
						int y1 = (int16)READ_LE_UINT16(axur + 2) + dy;
						int x2 = (int16)READ_LE_UINT16(axur + 4) + dx;
						int y2 = (int16)READ_LE_UINT16(axur + 6) + dy;					
						markRectAsDirty(kMainVirtScreen, x1, x2, y1, y2 + 1);
						axur += 8;
					}
				}
				const uint8 *axer = findResourceData(MKID('AXER'), auxd);
				if (axer) {
					a->_auxBlock.visible  = true;
					a->_auxBlock.r.left   = (int16)READ_LE_UINT16(axer + 0) + dx;
					a->_auxBlock.r.top    = (int16)READ_LE_UINT16(axer + 2) + dy;
					a->_auxBlock.r.right  = (int16)READ_LE_UINT16(axer + 4) + dx;
					a->_auxBlock.r.bottom = (int16)READ_LE_UINT16(axer + 6) + dy;
				}
			}
		}
	}
	_auxEntriesNum = 0;
}

void ScummEngine::queueAuxBlock(Actor *a) {
	if (!a->_auxBlock.visible)
		return;

	assert(_auxBlocksNum < ARRAYSIZE(_auxBlocks));
	_auxBlocks[_auxBlocksNum] = a->_auxBlock;
	++_auxBlocksNum;
}

void ScummEngine::queueAuxEntry(int actorNum, int subIndex) {
	assert(_auxEntriesNum < ARRAYSIZE(_auxEntries));
	AuxEntry *ae = &_auxEntries[_auxEntriesNum];
	ae->actorNum = actorNum;
	ae->subIndex = subIndex;
	++_auxEntriesNum;
}


const SaveLoadEntry *Actor::getSaveLoadEntries() {
	static const SaveLoadEntry actorEntries[] = {
		MKLINE(Actor, _pos.x, sleInt16, VER(8)),
		MKLINE(Actor, _pos.y, sleInt16, VER(8)),
		MKLINE(Actor, _offsX, sleInt16, VER(32)),
		MKLINE(Actor, _offsY, sleInt16, VER(32)),
		MKLINE(Actor, _top, sleInt16, VER(8)),
		MKLINE(Actor, _bottom, sleInt16, VER(8)),
		MKLINE(Actor, _elevation, sleInt16, VER(8)),
		MKLINE(Actor, _width, sleUint16, VER(8)),
		MKLINE(Actor, _facing, sleUint16, VER(8)),
		MKLINE(Actor, _costume, sleUint16, VER(8)),
		MKLINE(Actor, _room, sleByte, VER(8)),
		MKLINE(Actor, _talkColor, sleByte, VER(8)),
		MKLINE(Actor, _talkFrequency, sleInt16, VER(16)),
		MKLINE(Actor, _talkPan, sleInt16, VER(24)),
		MKLINE(Actor, _talkVolume, sleInt16, VER(29)),
		MKLINE(Actor, _boxscale, sleUint16, VER(34)),
		MKLINE(Actor, _scalex, sleByte, VER(8)),
		MKLINE(Actor, _scaley, sleByte, VER(8)),
		MKLINE(Actor, _charset, sleByte, VER(8)),

		// Actor sound grew from 8 to 32 bytes
		MKARRAY_OLD(Actor, _sound[0], sleByte, 8, VER(8), VER(36)),
		MKARRAY(Actor, _sound[0], sleByte, 32, VER(37)),

		// Actor animVariable grew from 8 to 27
		MKARRAY_OLD(Actor, _animVariable[0], sleUint16, 8, VER(8), VER(40)),
		MKARRAY(Actor, _animVariable[0], sleUint16, 27, VER(41)),

		MKLINE(Actor, _targetFacing, sleUint16, VER(8)),
		MKLINE(Actor, _moving, sleByte, VER(8)),
		MKLINE(Actor, _ignoreBoxes, sleByte, VER(8)),
		MKLINE(Actor, _forceClip, sleByte, VER(8)),
		MKLINE(Actor, _initFrame, sleByte, VER(8)),
		MKLINE(Actor, _walkFrame, sleByte, VER(8)),
		MKLINE(Actor, _standFrame, sleByte, VER(8)),
		MKLINE(Actor, _talkStartFrame, sleByte, VER(8)),
		MKLINE(Actor, _talkStopFrame, sleByte, VER(8)),
		MKLINE(Actor, _speedx, sleUint16, VER(8)),
		MKLINE(Actor, _speedy, sleUint16, VER(8)),
		MKLINE(Actor, _cost.animCounter, sleUint16, VER(8)),
		MKLINE(Actor, _cost.soundCounter, sleByte, VER(8)),
		MKLINE(Actor, _drawToBackBuf, sleByte, VER(32)),
		MKLINE(Actor, _flip, sleByte, VER(32)),
		MKLINE(Actor, _heSkipLimbs, sleByte, VER(32)),

		// Actor palette grew from 64 to 256 bytes
		MKARRAY_OLD(Actor, _palette[0], sleByte, 64, VER(8), VER(9)),
		MKARRAY(Actor, _palette[0], sleByte, 256, VER(10)),
	
		MK_OBSOLETE(Actor, _mask, sleByte, VER(8), VER(9)),
		MKLINE(Actor, _shadowMode, sleByte, VER(8)),
		MKLINE(Actor, _visible, sleByte, VER(8)),
		MKLINE(Actor, _frame, sleByte, VER(8)),
		MKLINE(Actor, _animSpeed, sleByte, VER(8)),
		MKLINE(Actor, _animProgress, sleByte, VER(8)),
		MKLINE(Actor, _walkbox, sleByte, VER(8)),
		MKLINE(Actor, _needRedraw, sleByte, VER(8)),
		MKLINE(Actor, _needBgReset, sleByte, VER(8)),
		MKLINE(Actor, _costumeNeedsInit, sleByte, VER(8)),
		MKLINE(Actor, _heCondMask, sleUint32, VER(38)),

		MKLINE(Actor, _talkPosY, sleInt16, VER(8)),
		MKLINE(Actor, _talkPosX, sleInt16, VER(8)),
		MKLINE(Actor, _ignoreTurns, sleByte, VER(8)),
	
		MKLINE(Actor, _layer, sleByte, VER(8)),
	
		MKLINE(Actor, _talkScript, sleUint16, VER(8)),
		MKLINE(Actor, _walkScript, sleUint16, VER(8)),

		MKLINE(Actor, _walkdata.dest.x, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.dest.y, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.destbox, sleByte, VER(8)),
		MKLINE(Actor, _walkdata.destdir, sleUint16, VER(8)),
		MKLINE(Actor, _walkdata.curbox, sleByte, VER(8)),
		MKLINE(Actor, _walkdata.cur.x, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.cur.y, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.next.x, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.next.y, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.deltaXFactor, sleInt32, VER(8)),
		MKLINE(Actor, _walkdata.deltaYFactor, sleInt32, VER(8)),
		MKLINE(Actor, _walkdata.xfrac, sleUint16, VER(8)),
		MKLINE(Actor, _walkdata.yfrac, sleUint16, VER(8)),

		MKLINE(Actor, _walkdata.point3.x, sleUint16, VER(42)),
		MKLINE(Actor, _walkdata.point3.y, sleUint16, VER(42)),
	
		MKARRAY(Actor, _cost.active[0], sleByte, 16, VER(8)),
		MKLINE(Actor, _cost.stopped, sleUint16, VER(8)),
		MKARRAY(Actor, _cost.curpos[0], sleUint16, 16, VER(8)),
		MKARRAY(Actor, _cost.start[0], sleUint16, 16, VER(8)),
		MKARRAY(Actor, _cost.end[0], sleUint16, 16, VER(8)),
		MKARRAY(Actor, _cost.frame[0], sleUint16, 16, VER(8)),
		MKEND()
	};
	
	return actorEntries;
}

} // End of namespace Scumm
