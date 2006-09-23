/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_H
#define CINE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/util.h"

#include "engines/engine.h"

#include "cine/texte.h"
#include "cine/rel.h"
#include "cine/script.h"
#include "cine/part.h"
#include "cine/prc.h"
#include "cine/resource.h"
#include "cine/msg.h"
#include "cine/bg.h"
#include "cine/pal.h"
#include "cine/gfx.h"
#include "cine/anim.h"

//#define DUMP_SCRIPTS

namespace Cine {

enum CineGameId {
	GID_FW = 1,
	GID_OS
};

class CineEngine : public Engine {
	int _gameId;

protected:
	int init();
	int go();
	void shutdown();

public:
	CineEngine(OSystem *syst);
	virtual ~CineEngine();
	int getGameId() {
		return _gameId;
	}
};

#define BOOT_PRC_NAME "AUTO00.PRC"

enum {
	VAR_MOUSE_X_MODE = 253,
	VAR_MOUSE_X_POS = 249,
	VAR_MOUSE_Y_MODE = 251,
	VAR_MOUSE_Y_POS = 250
};

enum {
	MOUSE_CURSOR_NORMAL = 0,
	MOUSE_CURSOR_DISK,
	MOUSE_CURSOR_CROSS
};

enum {
	kCineDebugScript = 1 << 0
};

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};


extern Common::SaveFileManager *g_saveFileMan; // TEMP

} // End of namespace Cine

#endif
