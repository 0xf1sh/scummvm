/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef ENGINE_H
#define ENGINE_H

#include "scummsys.h"
#include "system.h"

#define SCUMMVM_VERSION "0.3.2cvs"
#define SCUMMVM_CVS "2003-02-26"

class SoundMixer;
class GameDetector;
struct VersionSettings;

/* FIXME - BIG HACK for MidiEmu */
extern OSystem *g_system;
extern SoundMixer *g_mixer;

class Engine {
public:
	OSystem *_system;
	SoundMixer *_mixer;
	Timer * _timer;

protected:
	char *_gameDataPath;

public:
	Engine(GameDetector *detector, OSystem *syst);
	virtual ~Engine();

	// Invoke the main engine loop using this method
	virtual void go() = 0;

	// Get the save game dir path
	const char *getSavePath() const;

	virtual const char *getGameDataPath() const { return _gameDataPath; }

	// Create a new engine object based on the detector - either 
	// a Scumm or a SimonState object currently.
	static Engine *createFromDetector(GameDetector *detector, OSystem *syst);
	
	// Specific for each engine preparare of erroe string
	virtual void errorString(const char *buf_input, char *buf_output) = 0;
};

extern Engine *g_engine;

#if defined(__GNUC__)
void CDECL error(const char *s, ...) NORETURN;
#else
void CDECL NORETURN error(const char *s, ...);
#endif

void CDECL warning(const char *s, ...);

void CDECL debug(int level, const char *s, ...);
void checkHeap();

// Factory functions => no need to include the specific classes
// in this header. This serves two purposes:
// 1) Clean seperation from the game modules (scumm, simon) and the generic code
// 2) Faster (compiler doesn't have to parse lengthy header files)
extern Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst);
extern Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst);
extern Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst);

extern const VersionSettings *Engine_SIMON_targetList();
extern const VersionSettings *Engine_SCUMM_targetList();
extern const VersionSettings *Engine_SKY_targetList();

#endif

