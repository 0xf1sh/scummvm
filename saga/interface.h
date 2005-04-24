/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// Game interface module private header file

#ifndef SAGA_INTERFACE_H__
#define SAGA_INTERFACE_H__

#include "saga/sprite.h"
#include "saga/script.h"

namespace Saga {

enum InterfaceUpdateFlags {
	UPDATE_MOUSEMOVE = 1,
	UPDATE_LEFTBUTTONCLICK = 2,
	UPDATE_RIGHTBUTTONCLICK = 4,
	UPDATE_MOUSECLICK = UPDATE_LEFTBUTTONCLICK | UPDATE_RIGHTBUTTONCLICK
};

#define ITE_INVENTORY_SIZE 24

#define VERB_STRLIMIT 32

#define STATUS_TEXT_LEN 128

// Converse-specific stuff
#define CONVERSE_MAX_TEXTS      64
#define CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define CONVERSE_TEXT_HEIGHT	10
#define CONVERSE_TEXT_LINES     4
#define CONVERSE_MAX_WORK_STRING      128

enum PanelModes {
	kPanelNull,
	kPanelMain,
	kPanelOption,
	kPanelTextBox,
	kPanelQuit,
	kPanelError,
	kPanelLoad,
	kPanelConverse,
	kPanelProtect,
	kPanelPlacard,
	kPanelMap,
	kPanelInventory,
	kPanelFade
};

struct InterfacePanel {
	int x;
	int y;
	byte *image;
	size_t imageLength;
	int imageWidth;
	int imageHeight;
	
	PanelButton *currentButton;
	int buttonsCount;
	PanelButton *buttons;
	SpriteList sprites;
	
	PanelButton *getButton(int index) {
		if ((index >= 0) && (index < buttonsCount)) {
			return &buttons[index];
		}
		return NULL;
	}

	void calcPanelButtonRect(const PanelButton* panelButton, Rect &rect) {
		rect.left = x + panelButton->xOffset;
		rect.right = rect.left + panelButton->width;
		rect.top = y + panelButton->yOffset;
		rect.bottom = rect.top + panelButton->height;
	}

	PanelButton *hitTest(const Point& mousePoint, int buttonType) {
		PanelButton *panelButton;
		Rect rect;
		int i;
		for (i = 0; i < buttonsCount; i++) {
			panelButton = &buttons[i];
			if (panelButton != NULL) {
				if ((panelButton->type & buttonType) > 0) {
					calcPanelButtonRect(panelButton, rect);
					if (rect.contains(mousePoint)) {
						return panelButton;
					}
				}
			}
		}
		return NULL;
	}

};



struct Converse {
	char *text;
	int stringNum;
	int textNum;
	int replyId;
	int replyFlags;
	int replyBit;
};

enum ITEColors {
	kITEColorBrightWhite = 0x01,
	kITEColorWhite = 0x02,
	kITEColorLightGrey = 0x04,
	kITEColorGrey = 0x0a,
	kITEColorDarkGrey = 0x0b,
	kITEColorGreen = 0xba,
	kITEColorBlack = 0x0f,
	kITEColorRed = 0x65,
	kITEColorBlue = 0x93
};


class Interface {
public:

	Interface(SagaEngine *vm);
	~Interface(void);

	int activate();
	int deactivate();
	bool isActive() { return _active; }
	int setMode(int mode, bool force = false);
	int getMode(void) const { return _panelMode; }
	void rememberMode();
	void restoreMode();
	bool isInMainMode() { return _inMainMode; }
	void setStatusText(const char *text, int statusColor = -1);
	int loadScenePortraits(int resourceId);
	int setLeftPortrait(int portrait);
	int setRightPortrait(int portrait);
	int draw();
	int update(const Point& mousePoint, int updateFlag);
	void drawStatusBar();
	void setVerbState(int verb, int state);

	bool processKeyCode(int keyCode);
	
private:
	void drawInventory(SURFACE *backBuffer);
	void updateInventory(int pos);
	void inventoryChangePos(int chg);
	void inventorySetPos(int key);

public:
	void refreshInventory() {
		updateInventory(_inventoryCount);
		draw();
	}
	void addToInventory(int objectId, int pos = -1);
	void removeFromInventory(int objectId);
	void clearInventory();
	int inventoryItemPosition(int objectId);
	int getInventoryContentByPanelButton(PanelButton * panelButton) {
		int cell = _inventoryStart + panelButton->id;
		if (cell >= _inventoryCount) {
			return 0;
		}
		return _inventory[cell];
	}
	
	PanelButton *inventoryHitTest(const Point& mousePoint) {
		return _mainPanel.hitTest(mousePoint, kPanelButtonInventory);
	}
private:
	PanelButton *verbHitTest(const Point& mousePoint);
	void handleCommandUpdate(const Point& mousePoint);
	void handleCommandClick(const Point& mousePoint);
	PanelButton *converseHitTest(const Point& mousePoint) {
		return _conversePanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleConverseUpdate(const Point& mousePoint);
	void handleConverseClick(const Point& mousePoint);
	
	void lockMode() { _lockedMode = _panelMode; }
	void unlockMode() { _panelMode = _lockedMode; }

	void drawPanelButtonText(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton, int textColor, int textShadowColor);
	void drawPanelButtonArrow(SURFACE *ds, InterfacePanel *panel, PanelButton *panelButton);
	void drawVerbPanel(SURFACE *backBuffer, PanelButton* panelButton);

public:
	void converseInit(void);
	void converseClear(void);
	bool converseAddText(const char *text, int replyId, byte replyFlags, int replyBit);
	void converseDisplayText();
	void converseSetTextLines(int row);
	void converseChangePos(int chg);
	void converseSetPos(int key);

private:
	void converseDisplayTextLines(SURFACE *ds);
	PanelButton *getPanelButtonByVerbType(int verb) {
		if ((verb < 0) || (verb >= kVerbTypesMax)) {
			error("Interface::getPanelButtonByVerbType wrong verb");
		}
		return _verbTypeToPanelButton[verb];
	}
private:
	SagaEngine *_vm;

	bool _initialized;
	RSCFILE_CONTEXT *_interfaceContext;
	InterfacePanel _mainPanel;
	PanelButton *_inventoryUpButton;
	PanelButton *_inventoryDownButton;
	InterfacePanel _conversePanel;
	PanelButton *_converseUpButton;
	PanelButton *_converseDownButton;
	SpriteList _defPortraits;
	SpriteList _scenePortraits;
	PanelButton *_verbTypeToPanelButton[kVerbTypesMax];

	bool _active;
	int _panelMode;
	int _savedMode;
	int _lockedMode;
	bool _inMainMode;
	char _statusText[STATUS_TEXT_LEN];
	int _statusOnceColor;
	int _leftPortrait;
	int _rightPortrait;
	
	Point _lastMousePoint;

	uint16 *_inventory;
	int _inventorySize;
	int _inventoryStart;
	int _inventoryEnd;
	int _inventoryPos;
	int _inventoryBox;
	int _inventoryCount;

	char _converseWorkString[CONVERSE_MAX_WORK_STRING];
	Converse _converseText[CONVERSE_MAX_TEXTS];
	int _converseTextCount;
	int _converseStrCount;
	int _converseStartPos;
	int _converseEndPos;
	int _conversePos;
};

} // End of namespace Saga

#endif				/* INTERFACE_H__ */
