/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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

 // Console module header file

#ifndef SAGA_CONSOLE_H_
#define SAGA_CONSOLE_H_

#include "common/debugger.h"

namespace Saga {

class Console : public Common::Debugger<Console> {
public:
	Console(SagaEngine *vm);
	~Console(void);

protected:
	virtual void preEnter();
	virtual void postEnter();

private:
	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);

	bool Cmd_ActorMove(int argc, const char **argv);
	bool Cmd_ActorMoveRel(int argc, const char **argv);
	bool Cmd_ActorSetO(int argc, const char **argv);
	bool Cmd_ActorSetAct(int argc, const char **argv);

	bool Cmd_AnimInfo(int argc, const char **argv);

	bool Cmd_SceneChange(int argc, const char **argv);
	bool Cmd_SceneInfo(int argc, const char **argv);
	bool Cmd_ActionInfo(int argc, const char **argv);
	bool Cmd_ObjectInfo(int argc, const char **argv);

	bool Cmd_ScriptInfo(int argc, const char **argv);
	bool Cmd_ScriptExec(int argc, const char **argv);
	bool Cmd_ScriptToggleStep(int argc, const char **argv);

private:
	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
