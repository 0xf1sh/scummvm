/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#include "common/file.h"
#include "common/stdafx.h"
#include "common/util.h"
#include "common/util.h"

#include "gui/message.h"

#include "sword1/control.h"
#include "sword1/logic.h"
#include "sword1/mouse.h"
#include "sword1/music.h"
#include "sword1/objectman.h"
#include "sword1/resman.h"
#include "sword1/sound.h"
#include "sword1/sword1.h"
#include "sword1/sworddefs.h"
#include "sword1/swordres.h"

namespace Sword1 {

#define SAVEFILE_WRITE true
#define SAVEFILE_READ false

enum LangStrings {
	STR_PAUSED = 0,
	STR_INSERT_CD_A,
	STR_INSERT_CD_B,
	STR_INCORRECT_CD,
	STR_SAVE,
	STR_RESTORE,
	STR_RESTART,
	STR_START,
	STR_QUIT,
	STR_SPEED,
	STR_VOLUME,
	STR_TEXT,
	STR_DONE,
	STR_OK,
	STR_CANCEL,
	STR_MUSIC,
	STR_SPEECH,
	STR_FX,
	STR_THE_END,
	STR_DRIVE_FULL
};

enum ButtonIds {
	BUTTON_DONE = 1,
	BUTTON_MAIN_PANEL,
	BUTTON_SAVE_PANEL,
	BUTTON_RESTORE_PANEL,
	BUTTON_RESTART,
	BUTTON_QUIT,
	BUTTON_SPEED,
	BUTTON_VOLUME_PANEL,
	BUTTON_TEXT,
	BUTTON_CONFIRM,
//-
	BUTTON_SCROLL_UP_FAST,
	BUTTON_SCROLL_UP_SLOW,
	BUTTON_SCROLL_DOWN_SLOW,
	BUTTON_SCROLL_DOWN_FAST,
	BUTTON_SAVE_SELECT1,
	BUTTON_SAVE_SELECT2,
	BUTTON_SAVE_SELECT3,
	BUTTON_SAVE_SELECT4,
	BUTTON_SAVE_SELECT5,
	BUTTON_SAVE_SELECT6,
	BUTTON_SAVE_SELECT7,
	BUTTON_SAVE_SELECT8,
	BUTTON_SAVE_RESTORE_OKAY,
	BUTTON_SAVE_CANCEL,
//-
	CONFIRM_OKAY,
	CONFIRM_CANCEL
};

enum TextModes {
	TEXT_LEFT_ALIGN = 0,
	TEXT_CENTER,
	TEXT_RIGHT_ALIGN,
	TEXT_RED_FONT = 128
};

ControlButton::ControlButton(uint16 x, uint16 y, uint32 resId, uint8 id, ResMan *pResMan, uint8 *screenBuf, OSystem *system) {
	_x = x;
	_y = y;
	_id = id;
	_resId = resId;
	_resMan = pResMan;
	_frameIdx = 0;
	_resMan->resOpen(_resId);
	FrameHeader *tmp = _resMan->fetchFrame(_resMan->fetchRes(_resId), 0);
	_width = FROM_LE_16(tmp->width);
	_height = FROM_LE_16(tmp->height);
	if ((x == 0) && (y == 0)) { // center the frame (used for panels);
		_x = (640 - _width) / 2;
		_y = (480 - _height) / 2;
	}
	_dstBuf = screenBuf + _y * SCREEN_WIDTH + _x;
	_system = system;
}

ControlButton::~ControlButton(void) {
	_resMan->resClose(_resId);
}

bool ControlButton::isSaveslot(void) {
	return ((_resId >= SR_SLAB1) && (_resId <= SR_SLAB4));
}

void ControlButton::draw(void) {
	FrameHeader *fHead = _resMan->fetchFrame(_resMan->fetchRes(_resId), _frameIdx);
	uint8 *src = (uint8*)fHead + sizeof(FrameHeader);
	uint8 *dst = _dstBuf;
	for (uint16 cnt = 0; cnt < FROM_LE_16(fHead->height); cnt++) {
		for (uint16 cntx = 0; cntx < FROM_LE_16(fHead->width); cntx++)
			if (src[cntx])
				dst[cntx] = src[cntx];
		dst += SCREEN_WIDTH;
		src += FROM_LE_16(fHead->width);
	}
	_system->copyRectToScreen(_dstBuf, SCREEN_WIDTH, _x, _y, _width, _height);
}

bool ControlButton::wasClicked(uint16 mouseX, uint16 mouseY) {
	if ((_x <= mouseX) && (_y <= mouseY) && (_x + _width >= mouseX) && (_y + _height >= mouseY))
		return true;
	else
		return false;
}

void ControlButton::setSelected(uint8 selected) {
	_frameIdx = selected;
	draw();
}

Control::Control(ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, Mouse *pMouse, Sound *pSound, Music *pMusic, const char *savePath) {
	_resMan = pResMan;
	_objMan = pObjMan;
	_system = system;
	_mouse = pMouse;
	_music = pMusic;
	_sound = pSound;
	strcpy(_savePath, savePath);
	_lStrings = _languageStrings + MIN(SwordEngine::_systemVars.language, (uint8)BS1_SPANISH) * 20;
}

void Control::askForCd(void) {
	_screenBuf = (uint8*)malloc(640 * 480);
	uint32 fontId = SR_FONT;
	if (SwordEngine::_systemVars.language == BS1_CZECH)
		fontId = CZECH_SR_FONT;
	_font = (uint8*)_resMan->openFetchRes(fontId);
	uint8 *pal = (uint8*)_resMan->openFetchRes(SR_PALETTE);
	uint8 *palOut = (uint8*)malloc(256 * 4);
	for (uint16 cnt = 1; cnt < 256; cnt++) {
		palOut[cnt * 4 + 0] = pal[cnt * 3 + 0] << 2;
		palOut[cnt * 4 + 1] = pal[cnt * 3 + 1] << 2;
		palOut[cnt * 4 + 2] = pal[cnt * 3 + 2] << 2;
	}
	palOut[0] = palOut[1] = palOut[2] = palOut[3] = 0;
	_resMan->resClose(SR_PALETTE);
	_system->setPalette(palOut, 0, 256);
	free(palOut);

	File test;
	char fName[10];
	uint8 textA[50];
	sprintf(fName, "cd%d.id", SwordEngine::_systemVars.currentCD);
	sprintf((char*)textA, "%s%d", _lStrings[STR_INSERT_CD_A], SwordEngine::_systemVars.currentCD);
	bool notAccepted = true;
	bool refreshText = true;
	do {
		if (refreshText) {
			memset(_screenBuf, 0, 640 * 480);
			renderText(textA, 320, 220, TEXT_CENTER);
			renderText(_lStrings[STR_INSERT_CD_B], 320, 240, TEXT_CENTER);
			_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
			_system->updateScreen();
		}
		delay(300);
		if (_keyPressed) {
			test.open(fName);
			if (!test.isOpen()) {
				memset(_screenBuf, 0, 640 * 480);
				renderText(_lStrings[STR_INCORRECT_CD], 320, 230, TEXT_CENTER);
				_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
				_system->updateScreen();
				delay(2000);
				refreshText = true;
			} else {
				test.close();
				notAccepted = false;
			}
		}
	} while (notAccepted);

	_resMan->resClose(fontId);
	free(_screenBuf);
}

uint8 Control::runPanel(void) {
	_mouseDown = false;
	_restoreBuf = NULL;
	_keyPressed = _numButtons = 0;
	_screenBuf = (uint8*)malloc(640 * 480);
	uint32 fontId = SR_FONT, redFontId = SR_REDFONT;
	if (SwordEngine::_systemVars.language == BS1_CZECH) {
		fontId = CZECH_SR_FONT;
		redFontId = CZECH_SR_REDFONT;
	}
	_font = (uint8*)_resMan->openFetchRes(fontId);
	_redFont = (uint8*)_resMan->openFetchRes(redFontId);

	uint8 *pal = (uint8*)_resMan->openFetchRes(SR_PALETTE);
	uint8 *palOut = (uint8*)malloc(256 * 4);
	for (uint16 cnt = 1; cnt < 256; cnt++) {
		palOut[cnt * 4 + 0] = pal[cnt * 3 + 0] << 2;
		palOut[cnt * 4 + 1] = pal[cnt * 3 + 1] << 2;
		palOut[cnt * 4 + 2] = pal[cnt * 3 + 2] << 2;
	}
	palOut[0] = palOut[1] = palOut[2] = palOut[3] = 0;
	_resMan->resClose(SR_PALETTE);
	_system->setPalette(palOut, 0, 256);
	free(palOut);
	uint8 mode = 0, newMode = BUTTON_MAIN_PANEL;
	bool fullRefresh = false;
	_mouse->controlPanel(true);
	uint8 retVal = CONTROL_NOTHING_DONE;
	_music->startMusic(61, 1);

	do {
		if (newMode) {
			mode = newMode;
			fullRefresh = true;
			destroyButtons();
			memset(_screenBuf, 0, 640 * 480);
		}
		switch (mode) {
			case BUTTON_MAIN_PANEL:
				if (fullRefresh)
					setupMainPanel();
				break;
			case BUTTON_SAVE_PANEL:
				if (fullRefresh) {
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
					setupSaveRestorePanel(true);
				}
				if (_keyPressed)
					handleSaveKey(_keyPressed);
				break;
			case BUTTON_RESTORE_PANEL:
				if (fullRefresh)
					setupSaveRestorePanel(false);
				break;
			case BUTTON_VOLUME_PANEL:
				if (fullRefresh)
					setupVolumePanel();
				break;
			default:
				break;
		}
		if (fullRefresh) {
			fullRefresh = false;
			_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, 480);
		}
		_system->updateScreen();
		delay(1000 / 12);
		newMode = getClicks(mode, &retVal);
	} while ((newMode != 1) && (retVal == 0));
	destroyButtons();
	_resMan->resClose(fontId);
	_resMan->resClose(redFontId);
	memset(_screenBuf, 0, 640 * 480);
	_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
	free(_screenBuf);
	_mouse->controlPanel(false);
	_music->startMusic(Logic::_scriptVars[CURRENT_MUSIC], 1);
	return retVal;
}

uint8 Control::getClicks(uint8 mode, uint8 *retVal) {
	uint8 checkButtons = _numButtons;
	if (mode == BUTTON_VOLUME_PANEL) {
		handleVolumeClicks();
		checkButtons = 1;
	}

	if (!_mouseState)
		return 0;
	if (_mouseState & BS1L_BUTTON_DOWN)
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->wasClicked(_mouseX, _mouseY)) {
				_selectedButton = cnt;
				_buttons[cnt]->setSelected(1);
				if (_buttons[cnt]->isSaveslot())
					showSavegameNames();
			}
	if (_mouseState & BS1L_BUTTON_UP) {
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->wasClicked(_mouseX, _mouseY))
				if (_selectedButton == cnt) {
					// saveslots stay selected after clicking
					if (!_buttons[cnt]->isSaveslot())
						_buttons[cnt]->setSelected(0);
					_selectedButton = 255;
					return handleButtonClick(_buttons[cnt]->_id, mode, retVal);
				}
		if (_selectedButton < checkButtons) {
			_buttons[_selectedButton]->setSelected(0);
			if (_buttons[_selectedButton]->isSaveslot())
				showSavegameNames();
		}
		_selectedButton = 255;
	}
	return 0;
}

uint8 Control::handleButtonClick(uint8 id, uint8 mode, uint8 *retVal) {
	switch(mode) {
		case BUTTON_MAIN_PANEL:
			if (id == BUTTON_RESTART) {
				if (SwordEngine::_systemVars.deathScreenFlag) // if player is dead or has just started, don't ask for confirmation
					*retVal |= CONTROL_RESTART_GAME;
				else if (getConfirm(_lStrings[STR_RESTART]))
					*retVal |= CONTROL_RESTART_GAME;
				else
					return mode;
			} else if ((id == BUTTON_RESTORE_PANEL) || (id == BUTTON_SAVE_PANEL) ||
				(id == BUTTON_DONE) || (id == BUTTON_VOLUME_PANEL))
				return id;
			else if (id == BUTTON_TEXT) {
				SwordEngine::_systemVars.showText ^= 1;
				_buttons[5]->setSelected(SwordEngine::_systemVars.showText);
			} else if (id == BUTTON_QUIT) {
				if (getConfirm(_lStrings[STR_QUIT]))
					_system->quit();
				else
					return mode;
			}
			break;
		case BUTTON_SAVE_PANEL:
		case BUTTON_RESTORE_PANEL:
			if ((id >= BUTTON_SCROLL_UP_FAST) && (id <= BUTTON_SCROLL_DOWN_FAST))
				saveNameScroll(id, mode == BUTTON_SAVE_PANEL);
			else if ((id >= BUTTON_SAVE_SELECT1) && (id <= BUTTON_SAVE_SELECT8))
				saveNameSelect(id, mode == BUTTON_SAVE_PANEL);
			else if (id == BUTTON_SAVE_RESTORE_OKAY) {
				if (mode == BUTTON_SAVE_PANEL) {
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
					if (saveToFile()) // don't go back to main panel if save fails.
						return BUTTON_MAIN_PANEL;
				} else {
					if (restoreFromFile()) { // don't go back to main panel if restore fails.
						*retVal |= CONTROL_GAME_RESTORED;
						return BUTTON_MAIN_PANEL;
					}
				}
			} else if (id == BUTTON_SAVE_CANCEL) {
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
				return BUTTON_MAIN_PANEL; // mode down to main panel
			}
			break;
		case BUTTON_VOLUME_PANEL:
			return id;
   	}
	return 0;
}

void Control::deselectSaveslots(void) {
	for (uint8 cnt = 0; cnt < 8; cnt++)
		_buttons[cnt]->setSelected(0);
}

void Control::setupMainPanel(void) {
	uint32 panelId;
	
	if (SwordEngine::_systemVars.deathScreenFlag == 1)
		panelId = SR_DEATHPANEL;
	else
		panelId = SR_PANEL_ENGLISH + MIN(SwordEngine::_systemVars.language, (uint8)BS1_SPANISH);
	
	ControlButton *panel = new ControlButton( 0, 0, panelId, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;

	if (SwordEngine::_systemVars.deathScreenFlag)
		createButtons(_deathButtons, 3);		
	else {
		createButtons(_panelButtons, 7);
		_buttons[5]->setSelected(SwordEngine::_systemVars.showText);
	}

	if (SwordEngine::_systemVars.deathScreenFlag == 2) // end of game
		renderText(_lStrings[STR_THE_END], 480, 188 + 40, TEXT_RIGHT_ALIGN);

	if (SwordEngine::_systemVars.deathScreenFlag == 0) { // normal panel
		renderText(_lStrings[STR_SAVE], 180, 188 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_DONE], 460, 332 + 40, TEXT_RIGHT_ALIGN);
		renderText(_lStrings[STR_RESTORE], 180, 224 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_RESTART], 180, 260 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_QUIT], 180, 296 + 40, TEXT_LEFT_ALIGN);

		renderText(_lStrings[STR_VOLUME], 460, 188 + 40, TEXT_RIGHT_ALIGN);
		renderText(_lStrings[STR_TEXT], 460, 224 + 40, TEXT_RIGHT_ALIGN);
	} else {
		renderText(_lStrings[STR_RESTORE], 285, 224 + 40, TEXT_LEFT_ALIGN);
		if (SwordEngine::_systemVars.deathScreenFlag == 3) // just started game
			renderText(_lStrings[STR_START], 285, 260 + 40, TEXT_LEFT_ALIGN);
		else
			renderText(_lStrings[STR_RESTART], 285, 260 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_QUIT], 285, 296 + 40, TEXT_LEFT_ALIGN);
	}
}

void Control::setupSaveRestorePanel(bool saving) {
	FrameHeader *savePanel = _resMan->fetchFrame(_resMan->openFetchRes(SR_WINDOW), 0);
	uint16 panelX = (640 - FROM_LE_16(savePanel->width)) / 2;
	uint16 panelY = (480 - FROM_LE_16(savePanel->height)) / 2;
	ControlButton *panel = new ControlButton(panelX, panelY, SR_WINDOW, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;
	_resMan->resClose(SR_WINDOW);
	createButtons(_saveButtons, 14);
	renderText(_lStrings[STR_CANCEL], _saveButtons[13].x - 10, _saveButtons[13].y, TEXT_RIGHT_ALIGN);
	if (saving) {
		renderText(_lStrings[STR_SAVE], _saveButtons[12].x + 30, _saveButtons[13].y, TEXT_LEFT_ALIGN);
	} else {
		renderText(_lStrings[STR_RESTORE], _saveButtons[12].x + 30, _saveButtons[13].y, TEXT_LEFT_ALIGN);
	}
	readSavegameDescriptions();
	_selectedSavegame = 255;
	showSavegameNames();
}

void Control::setupVolumePanel(void) {
	ControlButton *panel = new ControlButton( 0, 0, SR_VOLUME, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;

	renderText(_lStrings[STR_MUSIC], 149, 39 + 40, TEXT_LEFT_ALIGN);
	renderText(_lStrings[STR_SPEECH], 320, 39 + 40, TEXT_CENTER);
	renderText(_lStrings[STR_FX], 438, 39 + 40, TEXT_LEFT_ALIGN);

	createButtons(_volumeButtons, 4);
	renderText(_lStrings[STR_DONE], _volumeButtons[0].x - 10, _volumeButtons[0].y, TEXT_RIGHT_ALIGN);

	uint8 volL, volR;
	_music->giveVolume(&volL, &volR);
	renderVolumeBar(1, volL, volR);
	_sound->giveSpeechVol(&volL, &volR);
	renderVolumeBar(2, volL, volR);
	_sound->giveSfxVol(&volL, &volR);
	renderVolumeBar(3, volL, volR);
}

void Control::handleVolumeClicks(void) {
	if (_mouseDown) {
		uint8 clickedId = 0;
		for (uint8 cnt = 1; cnt < 4; cnt++)
			if (_buttons[cnt]->wasClicked(_mouseX, _mouseY))
				clickedId = cnt;
		if (clickedId) { // these are circle shaped, so check again if it was clicked.
			uint8 clickDest = 0;
			int16 mouseDiffX = _mouseX - (_volumeButtons[clickedId].x + 48);
			int16 mouseDiffY = _mouseY - (_volumeButtons[clickedId].y + 48);
			int16 mouseOffs = (int16)sqrt((double)(mouseDiffX * mouseDiffX + mouseDiffY * mouseDiffY));
			// check if the player really hit the button (but not the center).
			if ((mouseOffs <= 42) && (mouseOffs >= 8)) {
				if (mouseDiffX > 8) { // right part
					if (mouseDiffY < -8) // upper right
						clickDest = 2;
					else if (ABS(mouseDiffY) <= 8) // right
						clickDest = 3;
					else				 // lower right
						clickDest = 4;
				} else if (mouseDiffX < -8) { // left part
					if (mouseDiffY < -8) // upper left
						clickDest = 8;
					else if (ABS(mouseDiffY) <= 8) // left
						clickDest = 7;
					else				 // lower left
						clickDest = 6;
				} else { // middle
					if (mouseDiffY < -8)
						clickDest = 1; // upper
					else if (mouseDiffY > 8)
						clickDest = 5; // lower
				}
			}
			_buttons[clickedId]->setSelected(clickDest);
			changeVolume(clickedId, clickDest);
		}
	} else if (_mouseState & BS1L_BUTTON_UP) {
		_buttons[1]->setSelected(0);
		_buttons[2]->setSelected(0);
		_buttons[3]->setSelected(0);
	}
}

void Control::changeVolume(uint8 id, uint8 action) {
	// ids: 1 = music, 2 = speech, 3 = sfx
	uint8 volL = 0, volR = 0;
	if (id == 1)
		_music->giveVolume(&volL, &volR);	
	else if (id == 2)
		_sound->giveSpeechVol(&volL, &volR);
	else if (id == 3)
		_sound->giveSfxVol(&volL, &volR);
	
	int8 direction = 0;
	if ((action >= 4) && (action <= 6)) // lower part of the button => decrease volume
		direction = -1;
	else if ((action == 8) || (action == 1) || (action == 2)) // upper part => increase volume
		direction = 1;
	else if ((action == 3) || (action == 7)) // middle part => pan volume
		direction = 1;
	int8 factorL = 8, factorR = 8;
	if ((action >= 6) && (action <= 8)) { // left part => left pan
		factorL = 8;
		factorR = (action == 7) ? -8 : 0;
	} else if ((action >= 2) && (action <= 4)) { // right part
		factorR = 8;
		factorL = (action == 3) ? -8 : 0;
	}
	int16 resVolL = volL + direction * factorL;
	int16 resVolR = volR + direction * factorR;

	volL = (uint8)MAX((int16)0, MIN(resVolL, (int16)255));
	volR = (uint8)MAX((int16)0, MIN(resVolR, (int16)255));	

	if (id == 1)
		_music->setVolume(volL, volR);
	else if (id == 2)
		_sound->setSpeechVol(volL, volR);
	else if (id == 3)
		_sound->setSfxVol(volL, volR);

	renderVolumeBar(id, volL, volR);
}

bool Control::getConfirm(const uint8 *title) {
	ControlButton *panel = new ControlButton( 0, 0, SR_CONFIRM, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;
	renderText(title, 320, 160, TEXT_CENTER);
	ControlButton *buttons[2];
	buttons[0] = new ControlButton( 260, 192 + 40, SR_BUTTON, 0, _resMan, _screenBuf, _system);
	renderText(_lStrings[STR_OK], 640 - 260, 192 + 40, TEXT_RIGHT_ALIGN);
	buttons[1] = new ControlButton( 260, 256 + 40, SR_BUTTON, 0, _resMan, _screenBuf, _system);
	renderText(_lStrings[STR_CANCEL], 640 - 260, 256 + 40, TEXT_RIGHT_ALIGN);
	uint8 retVal = 0;
	uint8 clickVal = 0;
	do {
		buttons[0]->draw();
		buttons[1]->draw();
		_system->updateScreen();
		delay(1000 / 12);
		if (_mouseState & BS1L_BUTTON_DOWN) {
			if (buttons[0]->wasClicked(_mouseX, _mouseY))
				clickVal = 1;
			else if (buttons[1]->wasClicked(_mouseX, _mouseY))
				clickVal = 2;
			else
				clickVal = 0;
			if (clickVal)
				buttons[clickVal - 1]->setSelected(1);
		}
		if ((_mouseState & BS1L_BUTTON_UP) && (clickVal)) {
			if (buttons[clickVal - 1]->wasClicked(_mouseX, _mouseY))
				retVal = clickVal;
			else
				buttons[clickVal - 1]->setSelected(0);
			clickVal = 0;
		}
	} while (!retVal);
	delete buttons[0];
	delete buttons[1];
	return retVal == 1;
}

bool Control::keyAccepted(uint8 key) {
	// this routine needs changes for Czech keys... No idea how to do that, though.
	static const char allowedSpecials[] = "�������������,.:-()?! \"\'";
	if (((key >= 'A') && (key <= 'Z')) ||
		((key >= 'a') && (key <= 'z')) ||
		((key >= '0') && (key <= '9')) ||
		strchr(allowedSpecials, key))
		return true;
	else
		return false;
}

void Control::handleSaveKey(uint8 key) {
	if (_selectedSavegame < 255) {
		uint8 len = strlen((char*)_saveNames[_selectedSavegame]);
		if ((key == 8) && len)  // backspace
			_saveNames[_selectedSavegame][len - 1] = '\0';
		else if (keyAccepted(key) && (len < 31)) {
			_saveNames[_selectedSavegame][len] = key;
			_saveNames[_selectedSavegame][len + 1] = '\0';
		}
		showSavegameNames();
	}
}

bool Control::saveToFile(void) {
	if ((_selectedSavegame == 255) || !strlen((char*)_saveNames[_selectedSavegame]))
		return false; // no saveslot selected or no name entered
	saveGameToFile(_selectedSavegame);
	writeSavegameDescriptions();
	return true;
}

bool Control::restoreFromFile(void) {
	if (_selectedSavegame < 255) {
		return restoreGameFromFile(_selectedSavegame);
	} else
		return false;
}

void Control::readSavegameDescriptions(void) {
	SaveFileManager *mgr = _system->get_savefile_manager();
	SaveFile *inf;
	inf = mgr->open_savefile("SAVEGAME.INF", _savePath, SAVEFILE_READ);
	_saveScrollPos = _saveFiles = 0;
	_selectedSavegame = 255;
	if (inf && inf->isOpen()) {
		uint8 curFileNum = 0;
		uint8 ch;
		do {
			uint8 pos = 0;
			do {
				ch = inf->readByte();
				if ((ch == 10) || (ch == 255))
					_saveNames[curFileNum][pos] = '\0';
				else
					_saveNames[curFileNum][pos] = ch;
				pos++;
			} while ((ch != 10) && (ch != 255));
			curFileNum++;
		} while (ch != 255);
		_saveFiles = curFileNum;
		for (uint8 cnt = _saveFiles; cnt < 64; cnt++)
			_saveNames[cnt][0] = '\0';
	} else
		for (uint8 cnt = 0; cnt < 64; cnt++)
			_saveNames[cnt][0] = '\0';
	delete inf;
	delete mgr;
}

int Control::displayMessage(const char *altButton, const char *message, ...) {
#ifdef __PALM_OS__
	char buf[256]; // 1024 is too big overflow the stack
#else
	char buf[1024];
#endif
	va_list va;

	va_start(va, message);
	vsprintf(buf, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	return dialog.runModal();
}

void Control::writeSavegameDescriptions(void) {
	SaveFileManager *mgr = _system->get_savefile_manager();
	SaveFile *outf;
	outf = mgr->open_savefile("SAVEGAME.INF", _savePath, SAVEFILE_WRITE);
	
	if (!outf) {
		// Display an error message, and do nothing
		displayMessage(0, "Unable to write to path '%s'", _savePath);
		return;
	}
	
	// if the player accidently clicked the last slot and then deselected it again,
	// we'd still have _saveFiles == 64, so get rid of the empty end.
	while (strlen((char*)_saveNames[_saveFiles - 1]) == 0)
		_saveFiles--;
	for (uint8 cnt = 0; cnt < _saveFiles; cnt++) {
		outf->write(_saveNames[cnt], strlen((char*)_saveNames[cnt]));
		if (cnt < _saveFiles - 1)
			outf->writeByte(10);
		else
			outf->writeByte(255);
	}
	delete outf;
	delete mgr;
}

bool Control::savegamesExist(void) {
	bool retVal = false;
	SaveFileManager *mgr = _system->get_savefile_manager();
	SaveFile *inf;
	inf = mgr->open_savefile("SAVEGAME.INF", _savePath, SAVEFILE_READ);
	if (inf && inf->isOpen())
		retVal = true;
	delete inf;
	delete mgr;
	return retVal;
}

void Control::showSavegameNames(void) {
	for (uint8 cnt = 0; cnt < 8; cnt++) {
		_buttons[cnt]->draw();
		uint8 textMode = TEXT_LEFT_ALIGN;
		uint16 ycoord = _saveButtons[cnt].y + 2;
		if (cnt + _saveScrollPos == _selectedSavegame) {
			textMode |= TEXT_RED_FONT;
			ycoord += 2;
		}
		renderText(_saveNames[cnt + _saveScrollPos], _saveButtons[cnt].x + 6, ycoord, textMode);
	}
}

void Control::saveNameSelect(uint8 id, bool saving) {
	deselectSaveslots();
	_buttons[id - BUTTON_SAVE_SELECT1]->setSelected(1);
	uint8 num = (id - BUTTON_SAVE_SELECT1) + _saveScrollPos;
	if (saving && (_selectedSavegame != 255)) // the player may have entered something, clear it again
		strcpy((char*)_saveNames[_selectedSavegame], (char*)_oldName);
	if (num < _saveFiles) {
		_selectedSavegame = num;
		strcpy((char*)_oldName, (char*)_saveNames[num]); // save for later
	} else {
		if (!saving)
			_buttons[id - BUTTON_SAVE_SELECT1]->setSelected(0); // no save in slot, deselect it
		else {
			if (_saveFiles <= num)
				_saveFiles = num + 1;
			_selectedSavegame = num;
			_oldName[0] = '\0';
		}
	}
	showSavegameNames();
}

void Control::saveNameScroll(uint8 scroll, bool saving) {
	uint16 maxScroll;
	if (saving)
		maxScroll = 64;
	else
		maxScroll = _saveFiles; // for loading, we can only scroll as far as there are savegames
	if (scroll == BUTTON_SCROLL_UP_FAST) {
		if (_saveScrollPos >= 8)
			_saveScrollPos -= 8;
		else
			_saveScrollPos = 0;
	} else if (scroll == BUTTON_SCROLL_UP_SLOW) {
		if (_saveScrollPos >= 1)
			_saveScrollPos--;
	} else if (scroll == BUTTON_SCROLL_DOWN_SLOW) {
		if (_saveScrollPos < maxScroll - 8)
			_saveScrollPos++;
	} else if (scroll == BUTTON_SCROLL_DOWN_FAST) {
		if (_saveScrollPos < maxScroll - 16)
			_saveScrollPos += 8;
		else
			_saveScrollPos = maxScroll - 8;
	}
	_selectedSavegame = 255; // deselect savegame
	deselectSaveslots();
	showSavegameNames();
}

void Control::createButtons(const ButtonInfo *buttons, uint8 num) {
	for (uint8 cnt = 0; cnt < num; cnt++) {
		_buttons[cnt] = new ControlButton(buttons[cnt].x, buttons[cnt].y, buttons[cnt].resId, buttons[cnt].id, _resMan, _screenBuf, _system);
		_buttons[cnt]->draw();
	}
	_numButtons = num;
}

void Control::destroyButtons(void) {
	for (uint8 cnt = 0; cnt < _numButtons; cnt++)
		delete _buttons[cnt];
	_numButtons = 0;
}

uint16 Control::getTextWidth(const uint8 *str) {
	uint16 width = 0;
	while (*str) {
		width += FROM_LE_16(_resMan->fetchFrame(_font, *str - 32)->width) - 3;
		str++;
	}
	return width;
}

void Control::renderText(const uint8 *str, uint16 x, uint16 y, uint8 mode) {
	uint8 *font = _font;
	if (mode & TEXT_RED_FONT)
		font = _redFont;
	mode &= ~TEXT_RED_FONT;
	
	if (mode == TEXT_RIGHT_ALIGN) // negative x coordinate means right-aligned.
		x -= getTextWidth(str);
	else if (mode == TEXT_CENTER)
		x -= getTextWidth(str) / 2;
	
	uint16 destX = x;
	while (*str) {
		uint8 *dst = _screenBuf + y * SCREEN_WIDTH + destX;

		FrameHeader *chSpr = _resMan->fetchFrame(font, *str - 32);
		uint8 *sprData = (uint8*)chSpr + sizeof(FrameHeader);
		for (uint16 cnty = 0; cnty < FROM_LE_16(chSpr->height); cnty++) {
			for (uint16 cntx = 0; cntx < FROM_LE_16(chSpr->width); cntx++) {
				if (sprData[cntx])
					dst[cntx] = sprData[cntx];
			}
			sprData += FROM_LE_16(chSpr->width);
			dst += SCREEN_WIDTH;
		}
		destX += FROM_LE_16(chSpr->width) - 3;
		str++;
	}
	_system->copyRectToScreen(_screenBuf + y * SCREEN_WIDTH + x, SCREEN_WIDTH, x, y, (destX - x) + 3, 28);
}

void Control::renderVolumeBar(uint8 id, uint8 volL, uint8 volR) {
	uint16 destX = _volumeButtons[id].x + 20;
	uint16 destY = _volumeButtons[id].y + 116;
	
	for (uint8 chCnt = 0; chCnt < 2; chCnt++) {
		uint8 vol = (chCnt == 0) ? volL : volR;
		FrameHeader *frHead = _resMan->fetchFrame(_resMan->openFetchRes(SR_VLIGHT), (vol + 15) >> 4);
		uint8 *destMem = _screenBuf + destY * SCREEN_WIDTH + destX;
		uint8 *srcMem = (uint8*)frHead + sizeof(FrameHeader);
		for (uint16 cnty = 0; cnty < FROM_LE_16(frHead->height); cnty++) {
			memcpy(destMem, srcMem, FROM_LE_16(frHead->width));
			srcMem += FROM_LE_16(frHead->width);
			destMem += SCREEN_WIDTH;
		}
		_system->copyRectToScreen(_screenBuf + destY * SCREEN_WIDTH + destX, SCREEN_WIDTH, destX, destY, FROM_LE_16(frHead->width), FROM_LE_16(frHead->height));
		_resMan->resClose(SR_VLIGHT);
		destX += 32;
	}
}

void Control::saveGameToFile(uint8 slot) {
	char fName[15];
	uint16 cnt;
	sprintf(fName, "SAVEGAME.%03d", slot);
	uint16 liveBuf[TOTAL_SECTIONS];
	SaveFileManager *mgr = _system->get_savefile_manager();
	SaveFile *outf;
	outf = mgr->open_savefile(fName, _savePath, SAVEFILE_WRITE);
	if (!outf || !outf->isOpen()) {
		// Display an error message, and do nothing
		displayMessage(0, "Unable to create file '%s' in directory '%s'", fName, _savePath);
		return;
	}

	_objMan->saveLiveList(liveBuf);
	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		outf->writeUint16LE(liveBuf[cnt]);

	Object *cpt = _objMan->fetchObject(PLAYER);
	Logic::_scriptVars[CHANGE_DIR] = cpt->o_dir;
	Logic::_scriptVars[CHANGE_X] = cpt->o_xcoord;
	Logic::_scriptVars[CHANGE_Y] = cpt->o_ycoord;
	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[CHANGE_PLACE] = cpt->o_place;

	for (cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++)
		outf->writeUint32LE(Logic::_scriptVars[cnt]);

	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	uint32 *playerRaw = (uint32*)cpt;
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++)
		outf->writeUint32LE(playerRaw[cnt2]);
	delete outf;
	delete mgr;
}

bool Control::restoreGameFromFile(uint8 slot) {
	char fName[15];
	uint16 cnt;
	sprintf(fName, "SAVEGAME.%03d", slot);
	SaveFileManager *mgr = _system->get_savefile_manager();
	SaveFile *inf;
	inf = mgr->open_savefile(fName, _savePath, SAVEFILE_READ);
	if (!inf || !inf->isOpen()) {
		// Display an error message, and do nothing
		displayMessage(0, "Can't open file '%s' in directory '%s'", fName, _savePath);
		delete mgr;
		return false;
	}

	_restoreBuf = (uint8*)malloc(
		TOTAL_SECTIONS * 2 + 
		NUM_SCRIPT_VARS * 4 +
		(sizeof(Object) - 12000));

	uint16 *liveBuf = (uint16*)_restoreBuf;
	uint32 *scriptBuf = (uint32*)(_restoreBuf + 2 * TOTAL_SECTIONS);
	uint32 *playerBuf = (uint32*)(_restoreBuf + 2 * TOTAL_SECTIONS + 4 * NUM_SCRIPT_VARS);

	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		liveBuf[cnt] = inf->readUint16LE();
	
	for (cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++)
		scriptBuf[cnt] = inf->readUint32LE();

	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++)
		playerBuf[cnt2] = inf->readUint32LE();

	delete inf;
	delete mgr;
	return true;
}

void Control::doRestore(void) {
	uint8 *bufPos = _restoreBuf;
	_objMan->loadLiveList((uint16*)bufPos);
	bufPos += TOTAL_SECTIONS * 2;
	for (uint16 cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++) {
		Logic::_scriptVars[cnt] = *(uint32*)bufPos;
		bufPos += 4;
	}
	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	uint32 *playerRaw = (uint32*)_objMan->fetchObject(PLAYER);
	Object *cpt = _objMan->fetchObject(PLAYER);
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++) {
		*playerRaw = *(uint32*)bufPos;
		playerRaw++;
		bufPos += 4;
	}
	free(_restoreBuf);
	Logic::_scriptVars[CHANGE_DIR] = cpt->o_dir;
	Logic::_scriptVars[CHANGE_X] = cpt->o_xcoord;
	Logic::_scriptVars[CHANGE_Y] = cpt->o_ycoord;
	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[CHANGE_PLACE] = cpt->o_place;
	SwordEngine::_systemVars.justRestoredGame = 1;
}

void Control::delay(uint32 msecs) {
	OSystem::Event event;

	uint32 endTime = _system->get_msecs() + msecs;
	_keyPressed = 0;	//reset
	_mouseState = 0;

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
			case OSystem::EVENT_KEYDOWN:

				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_keyPressed = 8;
				else
					_keyPressed = (byte)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_mouseDown = true;
				_mouseState |= BS1L_BUTTON_DOWN;
#ifdef _WIN32_WCE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_LBUTTONUP:
				_mouseDown = false;
				_mouseState |= BS1L_BUTTON_UP;
				break;
			case OSystem::EVENT_QUIT:
				_system->quit();
				break;
			default:
				break;
			}
		}
		_system->delay_msecs(10);
	} while (_system->get_msecs() < endTime);
}

const ButtonInfo Control::_deathButtons[3] = {
	{250, 224 + 40, SR_BUTTON, BUTTON_RESTORE_PANEL },
	{250, 260 + 40, SR_BUTTON, BUTTON_RESTART },
	{250, 296 + 40, SR_BUTTON, BUTTON_QUIT }
};

const ButtonInfo Control::_panelButtons[7] = {
	{145, 188 + 40, SR_BUTTON, BUTTON_SAVE_PANEL },
	{145, 224 + 40, SR_BUTTON, BUTTON_RESTORE_PANEL },
	{145, 260 + 40, SR_BUTTON, BUTTON_RESTART },
	{145, 296 + 40, SR_BUTTON, BUTTON_QUIT },
	{475, 188 + 40, SR_BUTTON, BUTTON_VOLUME_PANEL },
	{475, 224 + 40, SR_TEXT_BUTTON, BUTTON_TEXT },
	{475, 332 + 40, SR_BUTTON, BUTTON_DONE }
};

const ButtonInfo Control::_saveButtons[16] = { 
	{114,  32 + 40, SR_SLAB1, BUTTON_SAVE_SELECT1 },
	{114,  68 + 40, SR_SLAB2, BUTTON_SAVE_SELECT2 },
	{114, 104 + 40, SR_SLAB3, BUTTON_SAVE_SELECT3 },
	{114, 140 + 40, SR_SLAB4, BUTTON_SAVE_SELECT4 },
	{114, 176 + 40, SR_SLAB1, BUTTON_SAVE_SELECT5 },
	{114, 212 + 40, SR_SLAB2, BUTTON_SAVE_SELECT6 },
	{114, 248 + 40, SR_SLAB3, BUTTON_SAVE_SELECT7 },
	{114, 284 + 40, SR_SLAB4, BUTTON_SAVE_SELECT8 },

	{516,  25 + 40, SR_BUTUF, BUTTON_SCROLL_UP_FAST },
	{516,  45 + 40, SR_BUTUS, BUTTON_SCROLL_UP_SLOW },
	{516, 289 + 40, SR_BUTDS, BUTTON_SCROLL_DOWN_SLOW },
	{516, 310 + 40, SR_BUTDF, BUTTON_SCROLL_DOWN_FAST },

	{125, 338 + 40, SR_BUTTON, BUTTON_SAVE_RESTORE_OKAY},
	{462, 338 + 40, SR_BUTTON, BUTTON_SAVE_CANCEL}
};

const ButtonInfo Control::_volumeButtons[4] = {
	{ 478, 338 + 40, SR_BUTTON, BUTTON_MAIN_PANEL },
	{ 138, 135, SR_VKNOB, 0 },
	{ 273, 135, SR_VKNOB, 0 },
	{ 404, 135, SR_VKNOB, 0 },
};

const uint8 Control::_languageStrings[8 * 20][43] = {
	// BS1_ENGLISH:
	"PAUSED",
	"PLEASE INSERT CD-",
	"THEN PRESS A KEY",
	"INCORRECT CD",
	"Save",
	"Restore",
	"Restart",
	"Start",
	"Quit",
	"Speed",
	"Volume",
	"Text",
	"Done",
	"OK",
	"Cancel",
	"Music",
	"Speech",
	"Fx",
	"The End",
	"DRIVE FULL!",
// BS1_FRENCH:
	"PAUSE",
	"INS\xC9REZ LE CD-",
	"ET APPUYES SUR UNE TOUCHE",
	"CD INCORRECT",
	"Sauvegarder",
	"Recharger",
	"Recommencer",
	"Commencer",
	"Quitter",
	"Vitesse",
	"Volume",
	"Texte",
	"Termin\xE9",
	"OK",
	"Annuler",
	"Musique",
	"Voix",
	"Fx",
	"Fin",
	"DISQUE PLEIN!",
//BS1_GERMAN:
	"PAUSE",
	"BITTE LEGEN SIE CD-",
	"EIN UND DR\xDC CKEN SIE EINE BELIEBIGE TASTE",
	"FALSCHE CD",
	"Speichern",
	"Laden",
	"Neues Spiel",
	"Start",
	"Beenden",
	"Geschwindigkeit",
	"Lautst\xE4rke",
	"Text",
	"Fertig",
	"OK",
	"Abbrechen",
	"Musik",
	"Sprache",
	"Fx",
	"Ende",
	"DRIVE FULL!",
//BS1_ITALIAN:
	"PAUSA",
	"INSERITE IL CD-",
	"E PREMETE UN TASTO",
	"CD ERRATO",
	"Salva",
	"Ripristina",
	"Ricomincia",
	"Inizio",
	"Esci",
	"Velocit\xE0",
	"Volume",
	"Testo",
	"Fatto",
	"OK",
	"Annula",
	"Musica",
	"Parlato",
	"Fx",
	"Fine",
	"DISCO PIENO!",
//BS1_SPANISH:
	"PAUSA",
	"POR FAVOR INTRODUCE EL CD-",
	"Y PULSA UNA TECLA",
	"CD INCORRECTO",
	"Guardar",
	"Recuperar",
	"Reiniciar",
	"Empezar",
	"Abandonar",
	"Velocidad",
	"Volumen",
	"Texto",
	"Hecho",
	"OK",
	"Cancelar",
	"M\xFAsica",
	"Di\xE1logo",
	"Fx",
	"Fin",
	"DISCO LLENO",
// BS1_CZECH:
	"\xAC\x41S SE ZASTAVIL",
	"VLO\xA6TE DO MECHANIKY CD DISK",
	"PAK STISKN\xB7TE LIBOVOLNOU KL\xB5VESU",
	"TO NEBUDE TO SPR\xB5VN\x90 CD",
	"Ulo\xA7it pozici",
	"Nahr\xA0t pozici",
	"Za\x9F\xA1t znovu",
	"Start",
	"Ukon\x9Fit hru",
	"Rychlost",
	"Hlasitost",
	"Titulky",
	"Souhlas\xA1m",
	"Ano",
	"Ne",
	"Hudba",
	"Mluven, slovo",
	"Zvuky",
	"Konec",
	"Disk pln\xEC",
//BS1_PORTUGESE:
	"PAUSA",
	"FAVOR INSERIR CD",
	"E DIGITAR UMA TECLA",
	"CD INCORRETO",
	"Salvar",
	"Restaurar",
	"Reiniciar",
	"Iniciar",
	"Sair",
	"Velocidade",
	"Volume",
	"Texto",
	"Feito",
	"OK",
	"Cancelar",
	"M\xFAsica",
	"Voz",
	"Efeitos",
	"Fim",
	"UNIDADE CHEIA!",
};

} // End of namespace Sword1
