/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/error.h"
#include "common/system.h"
#include "common/file.h"

#include "engines/util.h"

#include "audio/mixer.h"
 
#include "zvision/zvision.h"
#include "zvision/console.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/cursor_manager.h"
#include "zvision/zfs_archive.h"
#include "zvision/detection.h"

#include "zvision/utility.h"

namespace ZVision {
 
ZVision::ZVision(OSystem *syst, const ZVisionGameDescription *gameDesc)
		: Engine(syst),
		  _gameDescription(gameDesc),
		  _workingWindow((WINDOW_WIDTH - WORKING_WINDOW_WIDTH) / 2, (WINDOW_HEIGHT - WORKING_WINDOW_HEIGHT) / 2, ((WINDOW_WIDTH - WORKING_WINDOW_WIDTH) / 2) + WORKING_WINDOW_WIDTH, ((WINDOW_HEIGHT - WORKING_WINDOW_HEIGHT) / 2) + WORKING_WINDOW_HEIGHT),
		  _pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0), /*RGB 555*/
		  _desiredFrameTime(33), /* ~30 fps */
		  _clock(_system) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().
 
	// Do not initialize graphics here
 
	// Here is the right place to set up the engine specific debug channels
	//DebugMan.addDebugChannel(kZVisionDebugExample, "example", "this is just an example for a engine specific debug channel");
	//DebugMan.addDebugChannel(kZVisionDebugExample2, "example2", "also an example");
 
	// Register random source
	_rnd = new Common::RandomSource("zvision");

	// Create managers
	_scriptManager = new ScriptManager(this);
	_renderManager = new RenderManager(_system, _workingWindow);
	_cursorManager = new CursorManager(this, &_pixelFormat);

	debug("ZVision::ZVision");
}

ZVision::~ZVision() {
	debug("ZVision::~ZVision");
 
	// Dispose of resources
	delete _console;
	delete _cursorManager;
	delete _renderManager;
	delete _scriptManager;
	delete _rnd;
 
	// Remove all of our debug levels
	DebugMan.clearAllDebugChannels();
}

void ZVision::initialize() {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	// TODO: There are 10 file clashes when we flatten the directories. From a quick look, the files are exactly the same, so it shouldn't matter. But I'm noting it here just in-case it does become a problem.
	SearchMan.addSubDirectoryMatching(gameDataDir, "data1", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data2", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data3", 0, 4, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zassets1", 0, 2, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zassets2", 0, 2, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "znemmx", 0, 1, true);
	SearchMan.addSubDirectoryMatching(gameDataDir, "zgi", 0, 4, true);

	// Find zfs archive files
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.zfs");

	// Register the file entries within the zfs archives with the SearchMan
	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::String name = (*iter)->getName();
		Common::SeekableReadStream *stream = (*iter)->createReadStream();
		ZfsArchive *archive = new ZfsArchive(name, stream);

		delete stream;

		SearchMan.add(name, archive);
	}

	initGraphics(WINDOW_WIDTH, WINDOW_HEIGHT, true, &_pixelFormat);

	_scriptManager->initialize();
	// Has to be done after graphics has been initialized
	_cursorManager->initialize();

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
}

Common::Error ZVision::run() {
	initialize();

	// Main loop
	while (!shouldQuit()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();
		
		processEvents();

		_scriptManager->update(_clock.getDeltaTime());

		// Update the screen
		_system->updateScreen();

		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
	}

	return Common::kNoError;
}

void ZVision::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);

	if (pause) {
		_clock.stop();
	} else {
		_clock.start();
	}
}

ScriptManager *ZVision::getScriptManager() const {
	return _scriptManager;
}

RenderManager *ZVision::getRenderManager() const {
	return _renderManager;
}

Common::RandomSource *ZVision::getRandomSource() const {
	return _rnd;
}

ZVisionGameId ZVision::getGameId() const {
	return _gameDescription->gameId;
}

void ZVision::cycleThroughCursors() {
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.zcr");

	Common::ArchiveMemberList::iterator iter = list.begin();
	ZorkCursor cursor;
	bool cursorChanged = false;

	_system->showMouse(true);

	bool done = false;
	while (!done && !shouldQuit()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();

		while (_eventMan->pollEvent(_event)) {
			if (_event.type == Common::EVENT_KEYDOWN) {
				switch (_event.kbd.keycode) {
				case Common::KEYCODE_LEFT:
					--iter;
					cursorChanged = true;
					break;
				case Common::KEYCODE_RIGHT:
					++iter;
					cursorChanged = true;
					break;
				case Common::KEYCODE_RETURN:
					debug("%s", (*iter)->getName().c_str());
					break;
				case Common::KEYCODE_ESCAPE:
					done = true;
					break;
				default:
					break;
				}
			}
		}

		if (cursorChanged) {
			cursor = ZorkCursor((*iter)->getName());

			_system->setMouseCursor(cursor.getSurface(), cursor.getWidth(), cursor.getHeight(), cursor.getHotspotX(), cursor.getHotspotY(), cursor.getHeight(), true, &_pixelFormat);
			cursorChanged = false;
		}

		_system->updateScreen();

		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
	}
}

} // End of namespace ZVision
