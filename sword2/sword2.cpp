/* Copyright (C) 1994-2004 Revolution Software Ltd
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
#include "backends/fs/fs.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/d_sound.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

struct Sword2GameSettings {
	const char *name;
	const char *description;
	uint32 features;
	const char *detectname;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const Sword2GameSettings sword2_settings[] = {
	/* Broken Sword 2 */
	{"sword2", "Broken Sword II", GF_DEFAULT_TO_1X_SCALER, "players.clu" },
	{"sword2alt", "Broken Sword II (alt)", GF_DEFAULT_TO_1X_SCALER, "r2ctlns.ocx" },
	{"sword2demo", "Broken Sword II (Demo)", GF_DEFAULT_TO_1X_SCALER | Sword2::GF_DEMO, "players.clu" },
	{NULL, NULL, 0, NULL}
};

GameList Engine_SWORD2_gameList() {
	const Sword2GameSettings *g = sword2_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_SWORD2_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const Sword2GameSettings *g;
	
	// TODO: It would be nice if we had code here which distinguishes
	// between the 'sword2' and 'sword2demo' targets. The current code
	// can't do that since they use the same detectname.

	for (g = sword2_settings; g->name; ++g) {
		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(g->toGameSettings());
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst) {
	return new Sword2::Sword2Engine(detector, syst);
}

REGISTER_PLUGIN("Broken Sword II", Engine_SWORD2_gameList, Engine_SWORD2_create, Engine_SWORD2_detectGames)

namespace Sword2 {

Sword2Engine *g_sword2 = NULL;

Sword2Engine::Sword2Engine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	g_sword2 = this;
	_debugger = NULL;
	_sound = NULL;
	_graphics = NULL;
	_features = detector->_game.features;
	_targetName = strdup(detector->_targetName.c_str());
	_bootParam = ConfMan.getInt("boot_param");
	_saveSlot = ConfMan.getInt("save_slot");

	// Setup mixer
	if (!_mixer->isReady())
		warning("Sound initialization failed");

	// We have our own volume settings panel, so don't let ScummVM's mixer
	// soften the sound in any way.

	_mixer->setVolume(256);
	_mixer->setMusicVolume(256);

	_keyboardEvent.pending = false;
	_mouseEvent.pending = false;

	_mouseX = _mouseY = 0;

	// get some falling RAM and put it in your pocket, never let it slip
	// away

	_graphics = new Graphics(this, 640, 480);

	// Create the debugger as early as possible (but not before the
	// graphics object!) so that errors can be displayed in it. In
	// particular, we want errors about missing files to be clearly
	// visible to the user.

	_debugger = new Debugger(this);

	_memory = new MemoryManager(this);
	_resman = new ResourceManager(this);
	_logic = new Logic(this);
	_fontRenderer = new FontRenderer(this);
	_gui = new Gui(this);
	_sound = new Sound(this);

	_lastPaletteRes = 0;

	_largestLayerArea = 0;
	_largestSpriteArea = 0;

	strcpy(_largestLayerInfo,  "largest layer:  none registered");
	strcpy(_largestSpriteInfo, "largest sprite: none registered");

	_fps = 0;
	_cycleTime = 0;
	_frameCount = 0;

	_wantSfxDebug = false;

	// For the menus

	_totalTemp = 0;
	memset(_tempList, 0, sizeof(_tempList));

	_totalMasters = 0;
	memset(_masterMenuList, 0, sizeof(_masterMenuList));

	memset(&_thisScreen, 0, sizeof(_thisScreen));

	memset(_mouseList, 0, sizeof(_mouseList));

	_mouseTouching = 0;
	_oldMouseTouching = 0;
	_menuSelectedPos = 0;
	_examiningMenuIcon = false;
	_mousePointerRes = 0;
	_mouseMode = 0;
	_mouseStatus = false;
	_mouseModeLocked = false;
	_currentLuggageResource = 0;
	_oldButton = 0;
	_buttonClick = 0;
	_pointerTextBlocNo = 0;
	_playerActivityDelay = 0;
	_realLuggageItem = 0;

	// used to be a define, but now it's flexible
	_scrollFraction = 16;

	_gamePaused = false;
	_stepOneCycle = false;
	_graphicsLevelFudged = false;
}

Sword2Engine::~Sword2Engine() {
	free(_targetName);
	delete _debugger;
	delete _graphics;
	delete _sound;
	delete _gui;
	delete _fontRenderer;
	delete _logic;
	delete _resman;
	delete _memory;
}

void Sword2Engine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the
	// debugger. Otherwise exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		// (Print it again in case debugger segfaults)
		printf("%s\n", buf2);
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

int32 Sword2Engine::initialiseGame(void) {
	// During normal gameplay, we care neither about mouse button releases
	// nor the scroll wheel.
	setEventFilter(RD_LEFTBUTTONUP | RD_RIGHTBUTTONUP | RD_WHEELUP | RD_WHEELDOWN);

	// initialise global script variables
	// res 1 is the globals list
	Logic::_scriptVars = (uint32 *) (_resman->openResource(1) + sizeof(StandardHeader));

	// DON'T CLOSE VARIABLES RESOURCE - KEEP IT OPEN AT VERY START OF
	// MEMORY SO IT CAN'T MOVE!

	// DON'T CLOSE PLAYER OBJECT RESOURCE - KEEP IT OPEN IN MEMORY SO IT
	// CAN'T MOVE!

	_resman->openResource(8);

	// Set up font resource variables for this language version

	debug(5, "CALLING: initialiseFontResourceFlags");
	initialiseFontResourceFlags();

	// initialise the sound fx queue

	debug(5, "CALLING: Init_fx_queue");
	initFxQueue();

	// all demos (not just web)
	if (_features & GF_DEMO)
		Logic::_scriptVars[DEMO] = 1;
	else
		Logic::_scriptVars[DEMO] = 0;

	return 0;
}

void Sword2Engine::closeGame(void) {
	_quit = true;
}

bool Sword2Engine::checkForMouseEvents(void) {
	return _mouseEvent.pending;
}

MouseEvent *Sword2Engine::mouseEvent(void) {
	if (!_mouseEvent.pending)
		return NULL;

	_mouseEvent.pending = false;
	return &_mouseEvent;
}

KeyboardEvent *Sword2Engine::keyboardEvent(void) {
	if (!_keyboardEvent.pending)
		return NULL;

	_keyboardEvent.pending = false;
	return &_keyboardEvent;
}

uint32 Sword2Engine::setEventFilter(uint32 filter) {
	uint32 oldFilter = _eventFilter;

	_eventFilter = filter;
	return oldFilter;
}

/**
 * OSystem Event Handler. Full of cross platform goodness and 99% fat free!
 */

void Sword2Engine::parseEvents(void) {
	OSystem::Event event;
	
	while (_system->poll_event(&event)) {
		switch (event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			if (!(_eventFilter & RD_KEYDOWN)) {
				_keyboardEvent.pending = true;
				_keyboardEvent.ascii = event.kbd.ascii;
				_keyboardEvent.keycode = event.kbd.keycode;
				_keyboardEvent.modifiers = event.kbd.flags;
			}
			break;
		case OSystem::EVENT_MOUSEMOVE:
			if (!(_eventFilter & RD_KEYDOWN)) {
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y - RDMENU_MENUDEEP;
			}
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			if (!(_eventFilter & RD_LEFTBUTTONDOWN)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_LEFTBUTTONDOWN;
			}
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			if (!(_eventFilter & RD_RIGHTBUTTONDOWN)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_RIGHTBUTTONDOWN;
			}
			break;
		case OSystem::EVENT_LBUTTONUP:
			if (!(_eventFilter & RD_LEFTBUTTONUP)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_LEFTBUTTONUP;
			}
			break;
		case OSystem::EVENT_RBUTTONUP:
			if (!(_eventFilter & RD_RIGHTBUTTONUP)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_RIGHTBUTTONUP;
			}
			break;
		case OSystem::EVENT_WHEELUP:
			if (!(_eventFilter & RD_WHEELUP)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_WHEELUP;
			}
			break;
		case OSystem::EVENT_WHEELDOWN:
			if (!(_eventFilter & RD_WHEELDOWN)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_WHEELDOWN;
			}
			break;
		case OSystem::EVENT_QUIT:
			closeGame();
			break;
		default:
			break;
		}
	}
}

void Sword2Engine::gameCycle(void) {
	// do one game cycle

	// got a screen to run?
	if (_logic->getRunList()) {
		// run the logic session UNTIL a full loop has been performed
		do {
			// reset the graphic 'BuildUnit' list before a new
			// logic list (see fnRegisterFrame)
			resetRenderLists();

			// reset the mouse hot-spot list (see fnRegisterMouse
			// and fnRegisterFrame)
			resetMouseList();

			// keep going as long as new lists keep getting put in
			// - i.e. screen changes
		} while (_logic->processSession());
	} else {
		// start the console and print the start options perhaps?
		_debugger->attach("AWAITING START COMMAND: (Enter 's 1' then 'q' to start from beginning)");
	}

	// if this screen is wide, recompute the scroll offsets every cycle
	if (_thisScreen.scroll_flag)
		setScrolling();

	mouseEngine();
	processFxQueue();
}

void Sword2Engine::go() {
	_quit = false;

	debug(5, "CALLING: readOptionSettings");
	_gui->readOptionSettings();

	debug(5, "CALLING: initialiseGame");
	if (initialiseGame())
		return;

	if (_saveSlot != -1) {
		if (saveExists(_saveSlot))
			restoreGame(_saveSlot);
		else {
			setMouse(NORMAL_MOUSE_ID);
			if (!_gui->restoreControl())
				startGame();
		}
	} else if (!_bootParam && saveExists()) {
		int32 pars[2] = { 221, FX_LOOP };
		bool result;

		setMouse(NORMAL_MOUSE_ID);
		_logic->fnPlayMusic(pars);
		result = _gui->startControl();

		// If the game is started from the beginning, the cutscene
		// player will kill the music for us. Otherwise, the restore
		// will either have killed the music, or done a crossfade.

		if (_quit)
			return;

		if (result)
			startGame();
	} else
		startGame();

	debug(5, "CALLING: initialiseRenderCycle");
	_graphics->initialiseRenderCycle();

	_renderSkip = false;		// Toggled on 'S' key, to render only
					// 1 in 4 frames, to speed up game

	_gameCycle = 0;

	while (1) {
		if (_debugger->isAttached())
			_debugger->onFrame();

		// the screen is build. Mostly because of first scroll
		// cycle stuff

#ifdef _SWORD2_DEBUG
		// if we've just stepped forward one cycle while the
		// game was paused

		if (_stepOneCycle) {
			pauseGame();
			_stepOneCycle = false;
		}
#endif

		KeyboardEvent *ke = keyboardEvent();

		if (ke) {
			if ((ke->modifiers == OSystem::KBD_CTRL && ke->keycode == 'd') || ke->ascii == '#' || ke->ascii == '~') {
				_debugger->attach();
			} else if (ke->modifiers == 0 || ke->modifiers == OSystem::KBD_SHIFT) {
				switch (ke->keycode) {
				case 'p':
					if (_gamePaused)
						unpauseGame();
					else
						pauseGame();
					break;
				case 'c':
					if (!Logic::_scriptVars[DEMO] && !_logic->_choosing)
						_logic->fnPlayCredits(NULL);
					break;
#ifdef _SWORD2_DEBUG
				case ' ':
					if (_gamePaused) {
						_stepOneCycle = true;
						unpauseGame();
					}
					break;
				case 's':
					_renderSkip = !_renderSkip;
					break;
#endif
				default:
					break;
				}
			}
		}

		// skip GameCycle if we're paused
		if (!_gamePaused) {
			_gameCycle++;
			gameCycle();
		}

		// We can't use this as termination condition for the looop,
		// because we want the break to happen before updating the
		// screen again.

		if (_quit)
			break;

		// creates the debug text blocks
		_debugger->buildDebugText();

#ifdef _SWORD2_DEBUG
		// if not in console & '_renderSkip' is set, only render
		// display once every 4 game-cycles

		if (console_status || !_renderSkip || (_gameCycle % 4) == 0)
			buildDisplay();	// create and flip the screen
#else
		// create and flip the screen
		buildDisplay();
#endif
	}

	// Stop music instantly!
	killMusic();
}

void Sword2Engine::startGame(void) {
	// boot the game straight into a start script

	int screen_manager_id;

	debug(5, "startGame() STARTING:");

	// all demos not just web
	if (Logic::_scriptVars[DEMO])
		screen_manager_id = 19;		// DOCKS SECTION START
	else
		screen_manager_id = 949;	// INTRO & PARIS START

	// FIXME this could be validated against startup.inf for valid numbers
	// to stop people shooting themselves in the foot

	if (_bootParam != 0)
		screen_manager_id = _bootParam;
	
	char *raw_script;
	char *raw_data_ad;

	// the required start-scripts are both script #1 in the respective
	// ScreenManager objects

	uint32 null_pc = 1;

	// open george object, ready for start script to reference
	raw_data_ad = (char *) _resman->openResource(8);

	// open the ScreenManager object
	raw_script = (char *) _resman->openResource(screen_manager_id);

	// run the start script now (because no console)
	_logic->runScript(raw_script, raw_data_ad, &null_pc);

	// close the ScreenManager object
	_resman->closeResource(screen_manager_id);

	// close george
	_resman->closeResource(8);

	debug(5, "startGame() DONE.");
}

// FIXME: Move this to some better place?

void Sword2Engine::sleepUntil(uint32 time) {
	while (_system->get_msecs() < time) {
		// Make sure menu animations and fades don't suffer, but don't
		// redraw the entire scene.
		_graphics->processMenu();
		_graphics->updateDisplay(false);
		_system->delay_msecs(10);
	}
}

void Sword2Engine::pauseGame(void) {
	// don't allow Pause while screen fading or while black
	if (_graphics->getFadeStatus() != RDFADE_NONE)
		return;
	
	pauseAllSound();

	// make a normal mouse
	clearPointerText();

	// this is the only place allowed to do it this way
	_graphics->setLuggageAnim(NULL, 0);

	// blank cursor
	setMouse(0);

	// forces engine to choose a cursor
	_mouseTouching = 1;

	// if level at max, turn down because palette-matching won't work
	// when dimmed

	if (_gui->_currentGraphicsLevel == 3) {
		_gui->updateGraphicsLevel(2);
		_graphicsLevelFudged = true;
	}

	// don't dim it if we're single-stepping through frames
	// dim the palette during the pause

	if (!_stepOneCycle)
		_graphics->dimPalette();

	_gamePaused = true;
}

void Sword2Engine::unpauseGame(void) {
	if (Logic::_scriptVars[OBJECT_HELD] && _realLuggageItem)
		setLuggage(_realLuggageItem);

	unpauseAllSound();

	// put back game screen palette; see build_display.cpp
	setFullPalette(-1);

	// If graphics level at max, turn up again
	if (_graphicsLevelFudged) {
		_gui->updateGraphicsLevel(3);
		_graphicsLevelFudged = false;
	}

	_gamePaused = false;

	// if mouse is about or we're in a chooser menu
	if (!_mouseStatus || _logic->_choosing)
		setMouse(NORMAL_MOUSE_ID);
}

} // End of namespace Sword2
