/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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

#ifndef GOB_GOB_H
#define GOB_GOB_H

#include "common/stdafx.h"
#include "common/system.h"
#include "sound/mixer.h"
#include "common/config-manager.h"

#include "base/engine.h"

#define	VAR_OFFSET(offs)		(*(uint32 *)(inter_variables + (offs)))
#define	VAR(var)			VAR_OFFSET((var) << 2)
#define VAR_ADDRESS(var)		(&VAR(var))

#define	WRITE_VAR_OFFSET(offs, val)	(VAR_OFFSET(offs) = (val))
#define WRITE_VAR(var, val)		WRITE_VAR_OFFSET((var) << 2, (val))

namespace Gob {

class GobEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);

protected:
	int go();
	int init(GameDetector &detector);

public:
	GobEngine(GameDetector * detector, OSystem * syst);
	virtual ~GobEngine();

	void shutdown();

	Common::RandomSource _rnd;
};

extern GobEngine *_vm;
 
} // End of namespace Gob
#endif
