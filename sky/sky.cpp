/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "stdafx.h"
#include "sky/sky.h"
#include "sky/skydefs.h" //game specific defines
#include "sky/compact.h"
#include "sky/logic.h"
#include "sky/debug.h"
#include "sky/mouse.h"
#include "common/file.h"
#include "common/gameDetector.h"
#include <errno.h>
#include <time.h>

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

static const VersionSettings sky_settings[] = {
	/* Beneath a Steel Sky */
	{"sky", "Beneath a Steel Sky", GID_SKY_FIRST, 99, 99, 99, 0, "sky.dsk" },
	{NULL, NULL, 0, 0, 0, 0, 0, NULL}
};

const VersionSettings *Engine_SKY_targetList() {
	return sky_settings;
}

Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst) {
	return new SkyState(detector, syst);
}

void **SkyState::_itemList[300];

SkyState::SkyState(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {
	
	_game = detector->_gameId;

	if (!_mixer->bindToSystem(syst))
		warning("Sound initialisation failed.");

	_mixer->setVolume(detector->_sfx_volume); //unnecessary?
	
	_debugMode = detector->_debugMode;
	_debugLevel = detector->_debugLevel;
	_language = detector->_language;
	_detector = detector;

	_introTextSpace = 0;
	_introTextSave = 0;
}

void SkyState::showQuitMsg(void) {

	uint8 *textBuf1 = (uint8*)calloc(GAME_SCREEN_WIDTH * 14 + sizeof(struct dataFileHeader),1);
	uint8 *textBuf2 = (uint8*)calloc(GAME_SCREEN_WIDTH * 14 + sizeof(struct dataFileHeader),1);
	char *vText1, *vText2;
	uint8 *screenData = _skyScreen->giveCurrent();
	switch (_language) {
		case DE_DEU: vText1 = VIG_DE1; vText2 = VIG_DE2; break;
		case FR_FRA: vText1 = VIG_FR1; vText2 = VIG_FR2; break;
		case IT_ITA: vText1 = VIG_IT1; vText2 = VIG_IT2; break;
		case PT_BRA: vText1 = VIG_PT1; vText2 = VIG_PT2; break;
		default: vText1 = VIG_EN1; vText2 = VIG_EN2; break;
	}
	_skyText->displayText(vText1, textBuf1, true, 320, 255);
	_skyText->displayText(vText2, textBuf2, true, 320, 255);
	uint8 *curLine1 = textBuf1 + sizeof(struct dataFileHeader);
	uint8 *curLine2 = textBuf2 + sizeof(struct dataFileHeader);
	uint8 *targetLine = screenData + GAME_SCREEN_WIDTH * 80;
	for (uint8 cnty = 0; cnty < 14; cnty++) {
		for (uint16 cntx = 0; cntx < GAME_SCREEN_WIDTH; cntx++) {
			if (curLine1[cntx])
				targetLine[cntx] = curLine1[cntx];
			if (curLine2[cntx])
				(targetLine + 24 * GAME_SCREEN_WIDTH)[cntx] = curLine2[cntx];
		}
		curLine1 += GAME_SCREEN_WIDTH;
		curLine2 += GAME_SCREEN_WIDTH;
		targetLine += GAME_SCREEN_WIDTH;
	}
	_skyScreen->halvePalette();
	_skyScreen->showScreen(screenData);
	free(textBuf1); free(textBuf2);
}

SkyState::~SkyState() {

	delete _skyLogic;
	delete _skyGrid;
	delete _skySound;
	delete _skyMusic;
    showQuitMsg();	
	delete _skyText;
	delete _skyMouse;
	delete _skyScreen;
	delay(1500);
}

void SkyState::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SkyState::pollMouseXY() {

	_mouse_x = _sdl_mouse_x;
	_mouse_y = _sdl_mouse_y;
}

void SkyState::go() {

	if (!_dump_file)
		_dump_file = stdout;

	initialise();

	if (!isDemo(_gameVersion) || isCDVersion(_gameVersion))
		intro();

	loadBase0();

	while (1) {
		delay(100);
		_skyLogic->engine();
	}
}

void SkyState::initialise(void) {

	//initialise_memory();

	_skyDisk = new SkyDisk(_gameDataPath);
	_skySound = new SkySound(_mixer, _skyDisk);
	
	if (_detector->getMidiDriverType() == MD_ADLIB) {
		_skyMusic = new SkyAdlibMusic(_mixer, _skyDisk);
	} else {
		_skyMusic = new SkyGmMusic(_detector->createMidi(), _skyDisk);
	}

	_gameVersion = _skyDisk->determineGameVersion();
	_skyText = new SkyText(_skyDisk, _gameVersion, _language);
	_skyMouse = new SkyMouse(_skyDisk);
	_skyScreen = new SkyScreen(_system, _skyDisk);

	initVirgin();
	//initMouse();
	initItemList();
	//initScript();
	//initialiseRouter();
	loadFixedItems();
	_skyGrid = new SkyGrid(_skyDisk);
	_skyLogic = new SkyLogic(_skyScreen, _skyDisk, _skyGrid, _skyText, _skyMusic, _skyMouse, _skySound, _gameVersion);
	
	_timer = Engine::_timer; // initialize timer *after* _skyScreen has been initialized.
	_timer->installProcedure(&timerHandler, 1000000 / 50); //call 50 times per second
}

void SkyState::initItemList() {
	
	//See List.asm for (cryptic) item # descriptions

	for (int i = 0; i < 300; i++)
		_itemList[i] = (void **)NULL;

	//init the non-null items
	_itemList[119] = (void **)SkyCompact::data_0; // Compacts - Section 0
	_itemList[120] = (void **)SkyCompact::data_1; // Compacts - Section 1
	
	if (isDemo(_gameVersion)) {
		_itemList[121] = _itemList[122] = _itemList[123] = _itemList[124] = _itemList[125] = (void **)SkyCompact::data_0;
	} else {
		_itemList[121] = (void **)SkyCompact::data_2; // Compacts - Section 2
		_itemList[122] = (void **)SkyCompact::data_3; // Compacts - Section 3
		_itemList[123] = (void **)SkyCompact::data_4; // Compacts - Section 4
		_itemList[124] = (void **)SkyCompact::data_5; // Compacts - Section 5
		_itemList[125] = (void **)SkyCompact::data_6; // Compacts - Section 6
	}
}

void SkyState::loadBase0(void) {

	_skyLogic->fnEnterSection(0, 0, 0);
	_skyMusic->startMusic(2);
	_skyGrid->loadGrids();
}

void SkyState::loadFixedItems(void) {

	if (!isDemo(_gameVersion))
		_itemList[36] = (void **)_skyDisk->loadFile(26, NULL);

	_itemList[49] = (void **)_skyDisk->loadFile(49, NULL);
	_itemList[50] = (void **)_skyDisk->loadFile(50, NULL);
	_itemList[73] = (void **)_skyDisk->loadFile(73, NULL);
	_itemList[262] = (void **)_skyDisk->loadFile(262, NULL);

	if (isDemo(_gameVersion)) 
		return;
	
	_itemList[263] = (void **)_skyDisk->loadFile(263, NULL);
	_itemList[264] = (void **)_skyDisk->loadFile(264, NULL);
	_itemList[265] = (void **)_skyDisk->loadFile(265, NULL);
	_itemList[266] = (void **)_skyDisk->loadFile(266, NULL);
	_itemList[267] = (void **)_skyDisk->loadFile(267, NULL);
	_itemList[269] = (void **)_skyDisk->loadFile(269, NULL);
	_itemList[271] = (void **)_skyDisk->loadFile(271, NULL);
	_itemList[272] = (void **)_skyDisk->loadFile(272, NULL);
		
}

void **SkyState::fetchItem(uint32 num) {

	return _itemList[num];
}

void SkyState::timerHandler(void *ptr) {

	((SkyState*)ptr)->gotTimerTick();
}

void SkyState::gotTimerTick(void) {

	_skyScreen->handleTimer();
}

Compact *SkyState::fetchCompact(uint32 a) {
	SkyDebug::fetchCompact(a);
	uint32 sectionNum = (a & 0xf000) >> 12;
	uint32 compactNum = (a & 0x0fff);

	return (Compact *)(_itemList[119 + sectionNum][compactNum]);
}

void SkyState::delay(uint amount) { //copied and mutilated from Simon.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	_key_pressed = 0;	//reset
	
	_rnd.getRandomNumber(2);

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
					// Make sure backspace works right (this fixes a small issue on OS X)
					if (event.kbd.keycode == 8)
						_key_pressed = 8;
					else
						_key_pressed = (byte)event.kbd.ascii;
					break;

				case OSystem::EVENT_MOUSEMOVE:
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
					_mouse_pos_changed = true;
					break;

					case OSystem::EVENT_LBUTTONDOWN:
					_left_button_down++;
#ifdef _WIN32_WCE
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_RBUTTONDOWN:
					
					break;
			}
		}

		if (amount == 0)
			break;

		{
			uint this_delay = 20; // 1?
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

bool SkyState::isDemo(uint32 version) {
	switch (version) {
	case 267:
		return true;
	case 288:
		return false;
	case 303:
		return false;
	case 331:
		return false;
	case 365:
		return true;
	case 368:
		return false;
	case 372:
		return false;
	default:
		error("Unknown game version");
	}
}

bool SkyState::isCDVersion(uint32 version) {

	switch (version) {
	case 267:
		return false;
	case 288:
		return false;
	case 303:
		return false;
	case 331:
		return false;
	case 365:
		return true;
	case 368:
		return true;
	case 372:
		return true;
	default:
		error("Unknown game version");
	}
}

