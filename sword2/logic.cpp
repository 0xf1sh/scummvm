/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/console.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"

#define LEVEL (_curObjectHub->logic_level)

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

Logic::Logic(Sword2Engine *vm) :
	_vm(vm), _kills(0), _smackerLeadIn(0), _smackerLeadOut(0),
	_sequenceTextLines(0), _speechTime(0), _animId(0), _speechAnimType(0),
	_leftClickDelay(0), _rightClickDelay(0), _defaultResponseId(0),
	_officialTextNumber(0),	_speechTextBlocNo(0), _choosing(false) {
	_scriptVars = NULL;
	memset(_subjectList, 0, sizeof(_subjectList));
	memset(_eventList, 0, sizeof(_eventList));
	memset(_syncList, 0, sizeof(_syncList));
	_router = new Router(_vm);
}

Logic::~Logic() {
	delete _router;
}

/**
 * Do one cycle of the current session.
 */

int Logic::processSession(void) {
	// might change during the session, so take a copy here
	uint32 run_list = _currentRunList;

	_pc = 0;	// first object in list

	// by minusing the pc we can cause an immediate cessation of logic
	// processing on the current list

	while (_pc != 0xffffffff) {
		uint32 ret, script;
		char *raw_script_ad;

		StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(run_list);
		assert(head->fileType == RUN_LIST);

		uint32 *game_object_list = (uint32 *) (head + 1);

		// read the next id
		uint id = game_object_list[_pc++];
		_scriptVars[ID] = id;

		_vm->_resman->closeResource(run_list);

		if (!_scriptVars[ID]) {
			// End of list - end the session naturally
			return 0;
		}

		head = (StandardHeader *) _vm->_resman->openResource(_scriptVars[ID]);
		assert(head->fileType == GAME_OBJECT);

		_curObjectHub = (ObjectHub *) (head + 1);

		debug(5, "Level %d id(%d) pc(%d)",
			LEVEL,
			_curObjectHub->script_id[LEVEL],
			_curObjectHub->script_pc[LEVEL]);

		// Do the logic for this object. We keep going until a function
		// says to stop - remember, system operations are run via
		// function calls to drivers now.

		do {
			// There is a distinction between running one of our
			// own scripts and that of another object.

			script = _curObjectHub->script_id[LEVEL];

			if (script / SIZE == _scriptVars[ID]) {
				// It's our own script

				debug(5, "Run script %d pc=%d",
					script / SIZE,
					_curObjectHub->script_pc[LEVEL]);

				// This is the script data. Script and data
				// object are the same.

				raw_script_ad = (char *) head;

				ret = runScript(raw_script_ad, raw_script_ad, &_curObjectHub->script_pc[LEVEL]);
			} else {
				// We're running the script of another game
				// object - get our data object address.

				StandardHeader *far_head = (StandardHeader *) _vm->_resman->openResource(script / SIZE);
				assert(far_head->fileType == GAME_OBJECT || far_head->fileType == SCREEN_MANAGER);

				raw_script_ad = (char *) far_head;
				char *raw_data_ad = (char *) head;

				ret = runScript(raw_script_ad, raw_data_ad, &_curObjectHub->script_pc[LEVEL]);

				_vm->_resman->closeResource(script / SIZE);

				// reset to us for service script
				raw_script_ad = raw_data_ad;
			}

			if (ret == 1) {
				// The script finished - drop down a level
				if (LEVEL)
					LEVEL--;
				else {
					// Hmmm, level 0 terminated :-| Let's
					// be different this time and simply
					// let it restart next go :-)

					// Note that this really does happen a
					// lot, so don't make it a warning.

					debug(5, "object %d script 0 terminated!", id);

					// reset to rerun, drop out for a cycle
					_curObjectHub->script_pc[LEVEL] = _curObjectHub->script_id[LEVEL] & 0xffff;
					ret = 0;
				}
			} else if (ret > 2) {
				error("processSession: illegal script return type %d", ret);
			}

			// if ret == 2 then we simply go around again - a new
			// script or subroutine will kick in and run

			// keep processing scripts until 0 for quit is returned
		} while (ret);

		// Any post logic system requests to go here

		// Clear any syncs that were waiting for this character - it
		// has used them or now looses them

		clearSyncs(_scriptVars[ID]);

		if (_pc != 0xffffffff) {
			// The session is still valid so run the graphics/mouse
			// service script
			uint32 null_pc = 0;
			runScript(raw_script_ad, raw_script_ad, &null_pc);
		}

		// and that's it so close the object resource

		_vm->_resman->closeResource(_scriptVars[ID]);
	}

	// Leaving a room so remove all ids that must reboot correctly. Then
	// restart the loop.

	for (uint32 i = 0; i < _kills; i++)
		_vm->_resman->remove(_objectKillList[i]);

	resetKillList();
	return 1;
}

/**
 * Bring an immediate halt to the session and cause a new one to start without
 * a screen update.
 */

void Logic::expressChangeSession(uint32 sesh_id) {
	// Set new session and force the old one to quit.
	_currentRunList = sesh_id;
	_pc = 0xffffffff;

	// Reset now in case we double-clicked an exit prior to changing screen
	_scriptVars[EXIT_FADING] = 0;

	// We're trashing the list - presumably to change room. In theory,
	// sync waiting in the list could be left behind and never removed -
	// so we trash the lot
	memset(_syncList, 0, sizeof(_syncList));

	// Various clean-ups
	_router->clearWalkGridList();
	_vm->_sound->clearFxQueue();
	_router->freeAllRouteMem();
}

/**
 * @return The private _currentRunList variable.
 */

uint32 Logic::getRunList(void) {
	return _currentRunList;
}

/**
 * Move the current object up a level. Called by fnGosub command. Remember:
 * only the logic object has access to _curObjectHub.
 */

void Logic::logicUp(uint32 new_script) {
	debug(5, "new pc = %d", new_script & 0xffff);

	// going up a level - and we'll keep going this cycle
	LEVEL++;

	assert(LEVEL < 3);	// Can be 0, 1, 2
	logicReplace(new_script);
}

/**
 * Force the level to one.
 */

void Logic::logicOne(uint32 new_script) {
	LEVEL = 1;
	logicReplace(new_script);
}

/**
 * Change current logic. Script must quit with a TERMINATE directive, which
 * does not write to &pc
 */

void Logic::logicReplace(uint32 new_script) {
	_curObjectHub->script_id[LEVEL] = new_script;
	_curObjectHub->script_pc[LEVEL] = new_script & 0xffff;
}

void Logic::examineRunList(void) {
	uint32 *game_object_list;
	StandardHeader *file_header;

	if (_currentRunList) {
		// open and lock in place
		game_object_list = (uint32 *) (_vm->_resman->openResource(_currentRunList) + sizeof(StandardHeader));

		Debug_Printf("Runlist number %d\n", _currentRunList);

		for (int i = 0; game_object_list[i]; i++) {
			file_header = (StandardHeader *) _vm->_resman->openResource(game_object_list[i]);
			Debug_Printf("%d %s\n", game_object_list[i], file_header->name);
			_vm->_resman->closeResource(game_object_list[i]);
		}

		_vm->_resman->closeResource(_currentRunList);
	} else
		Debug_Printf("No run list set\n");
}

void Logic::resetKillList(void) {
	_kills = 0;
}

} // End of namespace Sword2
