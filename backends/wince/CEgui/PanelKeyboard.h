/* ScummVM - Scumm Interpreter
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

#ifndef CEGUI_PANELKEYBOARD
#define CEGUI_PANELKEYBOARD

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "Toolbar.h"
#include "EventsBuffer.h"

using CEKEYS::Key;
using CEKEYS::EventsBuffer;

namespace CEGUI {

	class PanelKeyboard : public Toolbar {
	public:
		PanelKeyboard(WORD reference);
		virtual ~PanelKeyboard(); 
		virtual bool action(int x, int y, bool pushed);
	private:
		Key _key;
	};
}

#endif