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


#ifndef ACTOR_H
#define ACTOR_H

#include "common/scummsys.h"

#include "scumm.h"

enum MoveFlags {
	MF_NEW_LEG = 1,
	MF_IN_LEG = 2,
	MF_TURN = 4,
	MF_LAST_LEG = 8,
	MF_FROZEN = 0x80
};

struct ActorWalkData {
	int16 destx, desty;						// Final destination
	byte destbox;
	int16 destdir;
	byte curbox;
	int16 x, y;										// Current position
	int16 newx, newy;							// Next position on our way to the destination
	int32 deltaXFactor, deltaYFactor;
	uint16 xfrac, yfrac;
	int point3x, point3y;
};

struct CostumeData {
	byte active[16];
	uint16 animCounter1;
	byte animCounter2;
	uint16 stopped;
	uint16 curpos[16];
	uint16 start[16];
	uint16 end[16];
	uint16 frame[16];

	void reset() {
		stopped = 0;
		for (int i = 0; i < 16; i++) {
			active[i] = 0;
			curpos[i] = start[i] = end[i] = frame[i] = 0xFFFF;
		}
	}
};

class Actor {

public:
	static byte kInvalidBox;
	
	static void initActorClass(Scumm *scumm);

public:
	int x, y, top, bottom;
	int elevation;
	uint width;
	byte number;
	uint16 facing;
	uint16 costume;
	byte room;
	byte talkColor;
	int talkFrequency;
	byte scalex, scaley;
	byte charset;
	int16 newDirection;
	byte moving;
	bool ignoreBoxes;
	byte forceClip;
	byte initFrame, walkFrame, standFrame, talkFrame1, talkFrame2;
	bool needRedraw, needBgReset, costumeNeedsInit, visible;
	byte shadow_mode;
	bool flip;
	uint speedx, speedy;
	byte frame;
	byte walkbox;
	byte animProgress, animSpeed;
	int16 talkPosX, talkPosY;
	uint16 talk_script, walk_script;
	bool ignoreTurns;	// TODO - we do not honor this flag at all currently!
	int8 layer;
	ActorWalkData walkdata;
	int16 animVariable[16];
	uint16 sound[8];
	CostumeData cost;
	byte palette[256];
protected:
	static Scumm *_vm;

public:

	// Constructor, sets all data to 0
	Actor() {
		assert(_vm != 0);
		top = bottom = 0;
		number = 0;
		needRedraw = needBgReset = costumeNeedsInit = visible = false;
		flip = false;
		speedx = speedy = 0;
		frame = 0;
		walkbox = 0;
		animProgress = 0;
		memset(animVariable, 0, sizeof(animVariable));
		memset(palette, 0, sizeof(palette));

		walk_script = 0;

		initActor(1);
	}
	
//protected:
	void hideActor();
	void showActor();

	void initActor(int mode);
	void putActor(int x, int y, byte room);
	void setActorWalkSpeed(uint newSpeedX, uint newSpeedY);
protected:
	int calcMovementFactor(int newx, int newy);
	int actorWalkStep();
	int remapDirection(int dir, bool is_walking);
	void setupActorScale();

	void setBox(int box);
	int updateActorDirection(bool is_walking);

public:
	void adjustActorPos();
	AdjustBoxResult adjustXYToBeInBox(int dstX, int dstY);

	void setDirection(int direction);
	void faceToObject(int obj);
	void turnToDirection(int newdir);
	void walkActor();
	void drawActorCostume();
	void animateCostume();
	void setActorCostume(int c);
	
	void animateLimb(int limb, int f);
	
	byte *getActorName();
	void startWalkActor(int x, int y, int dir);
	void stopActorMoving();
	void startWalkAnim(int cmd, int angle);
	void startAnimActor(int frame);

	void remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold);
	void walkActorOld();

	void animateActor(int anim);

	bool isInCurrentRoom() {
		return room == _vm->_currentRoom;
	}
	
	int getActorXYPos(int &x, int &y);

	int getRoom() {
		return room;
	}

	int getAnimVar(byte var) {
		return animVariable[var];
	}
	void setAnimVar(byte var, int value) {
		animVariable[var] = value;
	}
	
	void classChanged(int cls, bool value);
	
protected:
	bool isInClass(int cls);
	
	bool isPlayer();
};

#endif
