/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#ifndef CEACTIONSPOCKET
#define CEACTIONSPOCKET

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"


#include "base/gameDetector.h"
#include "wince-sdl.h"
#include "Key.h"

#include "CEActions.h"

#define POCKET_ACTION_VERSION 3

enum pocketActionType {
        POCKET_ACTION_PAUSE = 0,
        POCKET_ACTION_SAVE,
        POCKET_ACTION_QUIT,
        POCKET_ACTION_SKIP,
        POCKET_ACTION_HIDE,
        POCKET_ACTION_KEYBOARD,
        POCKET_ACTION_SOUND,
        POCKET_ACTION_RIGHTCLICK,
        POCKET_ACTION_CURSOR,
        POCKET_ACTION_FREELOOK,
		POCKET_ACTION_ZOOM_UP,
		POCKET_ACTION_ZOOM_DOWN,

		POCKET_ACTION_LAST
};

class CEActionsPocket : public CEActions {
	public:
		// Actions
		bool perform(ActionType action, bool pushed = true);
		String actionName(ActionType action);
		int size();
		static void init(GameDetector &detector);
		void initInstance(OSystem_WINCE3 *mainSystem);

		// Action domain
		String domain();
		int version();

		// Utility
		bool needsRightClickMapping();
		bool needsHideToolbarMapping();
		bool needsZoomMapping();

		~CEActionsPocket();
	private:
		CEActionsPocket(GameDetector &detector);
		bool _right_click_needed;
		bool _hide_toolbar_needed;
		bool _zoom_needed;
	};	

#endif