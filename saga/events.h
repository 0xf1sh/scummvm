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

// Event management module header file

#ifndef SAGA_EVENT_H
#define SAGA_EVENT_H

#include "saga/list.h"

namespace Saga {

enum EVENT_TYPES {
	ONESHOT_EVENT,
	CONTINUOUS_EVENT,
	INTERVAL_EVENT,
	IMMEDIATE_EVENT
};

enum EVENT_FLAGS {
	SIGNALED = 0x8000,
	NODESTROY = 0x4000
};

enum EVENT_CODES {
	BG_EVENT = 1,
	ANIM_EVENT,
	MUSIC_EVENT,
	VOICE_EVENT,
	SOUND_EVENT,
	SCENE_EVENT,
	TEXT_EVENT,
	PAL_EVENT,
	PALANIM_EVENT,
	TRANSITION_EVENT,
	INTERFACE_EVENT,
	ACTOR_EVENT,
	SCRIPT_EVENT,
	CURSOR_EVENT
};

enum EVENT_OPS {
	// INSTANTANEOUS events
	// BG events
	EVENT_DISPLAY = 1,
	// ANIM events
	// EVENT_PLAY = 1, // reused
	EVENT_FRAME = 2,
	EVENT_SETFLAG = 3,
	EVENT_CLEARFLAG = 4,
	// MUISC & SOUND events
	EVENT_PLAY = 1,
	EVENT_STOP = 2,
	// SCENE events
	EVENT_END = 2,
	// TEXT events
	EVENT_HIDE = 2,
	EVENT_REMOVE = 3,
	// PALANIM events
	EVENT_CYCLESTART = 1,
	EVENT_CYCLESTEP = 2,
	// INTERFACE events
	EVENT_ACTIVATE = 1,
	EVENT_DEACTIVATE,
	// ACTOR events
	EVENT_MOVE = 1,
	// SCRIPT events
	EVENT_BLOCKING = 1,
	EVENT_NONBLOCKING = 2,
	// CURSOR events
	EVENT_SHOW = 1,
	// EVENT_HIDE = 2, // reused

	// CONTINUOUS events
	// PALETTE events
	EVENT_PALTOBLACK = 1,
	EVENT_BLACKTOPAL = 2,
	// TRANSITION events
	EVENT_DISSOLVE = 1,
	EVENT_DISSOLVE_BGMASK = 2
};

enum EVENT_PARAMS {
	NO_SET_PALETTE,
	SET_PALETTE
};

struct EVENT {
	unsigned int type;
	unsigned int code; // Event operation category & flags 
	int op;            // Event operation
	long param;        // Optional event parameter
	long param2;
	long param3;
	long param4;
	long param5;
	void *data;        // Optional event data
	long time;         // Elapsed time until event
	long duration;     // Duration of event
	long d_reserved;

	EVENT *chain;    // Event chain (For consecutive events)
	EVENT() { memset(this, 0, sizeof(*this)); }
};

typedef SortedList<EVENT> EventList;

#define EVENT_WARNINGCOUNT 1000
#define EVENT_MASK 0x00FF

enum EVENT_STATUSCODE {
	EVENT_INVALIDCODE = 0,
	EVENT_DELETE,
	EVENT_CONTINUE,
	EVENT_BREAK
};

class Events {
 public:
	Events(SagaEngine *vm);
	~Events(void);
	int handleEvents(long msec);
	int clearList();
	int freeList();
	EVENT *queue(EVENT *event);
	EVENT *chain(EVENT *eead_event, EVENT *add_event);

 private:
	int handleContinuous(EVENT * event);
	int handleOneShot(EVENT * event);
	int handleInterval(EVENT * event);
	int handleImmediate(EVENT *event);
	int processEventTime(long msec);
	int initializeEvent(EVENT * event);

 private:
	SagaEngine *_vm;
	bool _initialized;

	EventList _eventList;
};

} // End of namespace Saga

#endif
