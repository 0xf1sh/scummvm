/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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

#include "stdafx.h"
#include "common/util.h"
#include "gui/newgui.h"
#include "gui/dialog.h"


// Uncomment the following to enable the new font code:
//#define NEW_FONT_CODE



namespace GUI {

/*
 * TODO list
 * - add more widgets: edit field, popup, radio buttons, ...
 *
 * Other ideas:
 * - allow multi line (l/c/r aligned) text via StaticTextWidget ?
 * - add "close" widget to all dialogs (with a flag to turn it off) ?
 * - make dialogs "moveable" ?
 * - come up with a new look & feel / theme for the GUI 
 * - ...
 */

enum {
	kDoubleClickDelay = 500, // milliseconds
	kCursorAnimateDelay = 250,
	kKeyRepeatInitialDelay = 400,
	kKeyRepeatSustainDelay = 100
};


// Constructor
NewGui::NewGui() : _needRedraw(false),
	_stateIsSaved(false), _cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	
	_system = OSystem::instance();

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));

	// Reset key repeat
	_currentKeyDown.keycode = 0;
}

void NewGui::updateColors() {
	// Setup some default GUI colors.
	_bgcolor = _system->RGBToColor(0, 0, 0);
	_color = _system->RGBToColor(104, 104, 104);
	_shadowcolor = _system->RGBToColor(64, 64, 64);
	_textcolor = _system->RGBToColor(32, 160, 32);
	_textcolorhi = _system->RGBToColor(0, 255, 0);
}

void NewGui::runLoop() {
	Dialog *activeDialog = _dialogStack.top();
	bool didSaveState = false;

	if (activeDialog == 0)
		return;
	
	// Setup some default GUI colors. Normally this will be done whenever an
	// EVENT_SCREEN_CHANGED is received. However, not yet all backends support
	// that event, so we also do it "manually" whenever a run loop is entered.
	updateColors();

	if (!_stateIsSaved) {
		saveState();
		didSaveState = true;
	}

	while (!_dialogStack.empty() && activeDialog == _dialogStack.top()) {
		activeDialog->handleTickle();

		if (_needRedraw) {
			// Restore the overlay to its initial state, then draw all dialogs.
			// This is necessary to get the blending right.
			_system->clearOverlay();
			_system->grabOverlay((OverlayColor *)_screen.pixels, _screenPitch);
			for (int i = 0; i < _dialogStack.size(); i++)
				_dialogStack[i]->drawDialog();
			_needRedraw = false;
		}

		animateCursor();
		_system->updateScreen();		

		OSystem::Event event;
		uint32 time = _system->getMillis();

		while (_system->pollEvent(event)) {
			switch (event.event_code) {
			case OSystem::EVENT_KEYDOWN:
#if !defined(__PALM_OS__)
				// init continuous event stream
				// not done on PalmOS because keyboard is emulated and keyup is not generated
				_currentKeyDown.ascii = event.kbd.ascii;
				_currentKeyDown.keycode = event.kbd.keycode;
				_currentKeyDown.flags = event.kbd.flags;
				_keyRepeatTime = time + kKeyRepeatInitialDelay;
#endif
				activeDialog->handleKeyDown(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
				break;
			case OSystem::EVENT_KEYUP:
				activeDialog->handleKeyUp(event.kbd.ascii, event.kbd.keycode, event.kbd.flags);
				if (event.kbd.keycode == _currentKeyDown.keycode)
					// only stop firing events if it's the current key
					_currentKeyDown.keycode = 0;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				activeDialog->handleMouseMoved(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 0);
				break;
			// We don't distinguish between mousebuttons (for now at least)
			case OSystem::EVENT_LBUTTONDOWN:
			case OSystem::EVENT_RBUTTONDOWN:
				if (_lastClick.count && (time < _lastClick.time + kDoubleClickDelay)
							&& ABS(_lastClick.x - event.mouse.x) < 3
							&& ABS(_lastClick.y - event.mouse.y) < 3) {
					_lastClick.count++;
				} else {
					_lastClick.x = event.mouse.x;
					_lastClick.y = event.mouse.y;
					_lastClick.count = 1;
				}
				_lastClick.time = time;
				activeDialog->handleMouseDown(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 1, _lastClick.count);
				break;
			case OSystem::EVENT_LBUTTONUP:
			case OSystem::EVENT_RBUTTONUP:
				activeDialog->handleMouseUp(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 1, _lastClick.count);
				break;
			case OSystem::EVENT_WHEELUP:
				activeDialog->handleMouseWheel(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, -1);
				break;
			case OSystem::EVENT_WHEELDOWN:
				activeDialog->handleMouseWheel(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y, 1);
				break;
			case OSystem::EVENT_QUIT:
				_system->quit();
				return;
			case OSystem::EVENT_SCREEN_CHANGED:
				updateColors();
				activeDialog->handleScreenChanged();
				break;
			}
		}

		// check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0) {
			if (_keyRepeatTime < time) {
				// fire event
				activeDialog->handleKeyDown(_currentKeyDown.ascii, _currentKeyDown.keycode, _currentKeyDown.flags);
				_keyRepeatTime = time + kKeyRepeatSustainDelay;
			}
		}

		// Delay for a moment
		_system->delayMillis(10);
	}
	
	if (didSaveState)
		restoreState();
}

#pragma mark -

void NewGui::saveState() {

	// Backup old cursor
	_oldCursorMode = _system->showMouse(true);

	// Enable the overlay
	_system->showOverlay();

	// Create a screen buffer for the overlay data, and fill it with
	// whatever is visible on the screen rught now.
	_screen.h = _system->getOverlayHeight();
	_screen.w = _system->getOverlayWidth();
	_screen.bytesPerPixel = sizeof(OverlayColor);
	_screen.pitch = _screen.w * _screen.bytesPerPixel;
	_screenPitch = _screen.w;
	_screen.pixels = (OverlayColor*)calloc(_screen.w * _screen.h, sizeof(OverlayColor));

	_system->grabOverlay((OverlayColor *)_screen.pixels, _screenPitch);

	_currentKeyDown.keycode = 0;
	_lastClick.x = _lastClick.y = 0;
	_lastClick.time = 0;
	_lastClick.count = 0;

	_stateIsSaved = true;
}

void NewGui::restoreState() {
	_system->showMouse(_oldCursorMode);

	_system->hideOverlay();
	if (_screen.pixels) {
		free(_screen.pixels);
		_screen.pixels = 0;
	}

	_system->updateScreen();
	
	_stateIsSaved = false;
}

void NewGui::openDialog(Dialog *dialog) {
	_dialogStack.push(dialog);
	_needRedraw = true;
}

void NewGui::closeTopDialog() {
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;

	// Remove the dialog from the stack
	_dialogStack.pop();
	_needRedraw = true;
}

#pragma mark -

const Graphics::Font &NewGui::getFont() const {
#ifdef NEW_FONT_CODE
	return Graphics::g_sysfont;
#else
	return Graphics::g_scummfont;
#endif
}

OverlayColor *NewGui::getBasePtr(int x, int y) {
	return (OverlayColor *)_screen.getBasePtr(x, y);
}

void NewGui::box(int x, int y, int width, int height, OverlayColor colorA, OverlayColor colorB) {
	hLine(x + 1, y, x + width - 2, colorA);
	hLine(x, y + 1, x + width - 1, colorA);
	vLine(x, y + 1, y + height - 2, colorA);
	vLine(x + 1, y, y + height - 1, colorA);

	hLine(x + 1, y + height - 2, x + width - 1, colorB);
	hLine(x + 1, y + height - 1, x + width - 2, colorB);
	vLine(x + width - 1, y + 1, y + height - 2, colorB);
	vLine(x + width - 2, y + 1, y + height - 1, colorB);
}

void NewGui::hLine(int x, int y, int x2, OverlayColor color) {
	_screen.hLine(x, y, x2, color);
}

void NewGui::vLine(int x, int y, int y2, OverlayColor color) {
	_screen.vLine(x, y, y2, color);
}

void NewGui::blendRect(int x, int y, int w, int h, OverlayColor color, int level) {
#ifdef NEWGUI_256
	fillRect(x, y, w, h, color);
#else
	int r, g, b;
	uint8 ar, ag, ab;
	_system->colorToRGB(color, ar, ag, ab);
	r = ar * level;
	g = ag * level;
	b = ab * level;

	OverlayColor *ptr = getBasePtr(x, y);

	while (h--) {
		for (int i = 0; i < w; i++) {
			_system->colorToRGB(ptr[i], ar, ag, ab);
			ptr[i] = _system->RGBToColor((ar + r) / (level+1),
										 (ag + g) / (level+1),
										 (ab + b) / (level+1));
		}
		ptr += _screenPitch;
	}
#endif
}

void NewGui::fillRect(int x, int y, int w, int h, OverlayColor color) {
	_screen.fillRect(Common::Rect(x, y, x+w, y+h), color);
}

void NewGui::frameRect(int x, int y, int w, int h, OverlayColor color) {
	_screen.frameRect(Common::Rect(x, y, x+w, y+h), color);
}

void NewGui::addDirtyRect(int x, int y, int w, int h) {
	// For now we don't keep yet another list of dirty rects but simply
	// blit the affected area directly to the overlay. At least for our current
	// GUI/widget/dialog code that is just fine.
	OverlayColor *buf = getBasePtr(x, y);
	_system->copyRectToOverlay(buf, _screenPitch, x, y, w, h);
}

void NewGui::drawChar(byte chr, int xx, int yy, OverlayColor color, const Graphics::Font *font) {
	if (font == 0)
		font = &getFont();
	font->drawChar(&_screen, chr, xx, yy, color);
}

int NewGui::getStringWidth(const String &str) {
	return getFont().getStringWidth(str);
}

int NewGui::getCharWidth(byte c) {
	return getFont().getCharWidth(c);
}

void NewGui::drawString(const String &s, int x, int y, int w, OverlayColor color, TextAlignment align, int deltax, bool useEllipsis) {
	getFont().drawString(&_screen, s, x, y, w, color, align, deltax, useEllipsis);
}

//
// Draw an 8x8 bitmap at location (x,y)
//
void NewGui::drawBitmap(uint32 *bitmap, int x, int y, OverlayColor color, int h) {
	OverlayColor *ptr = getBasePtr(x, y);

	for (y = 0; y < h; y++) {
		uint32 mask = 0xF0000000;
		for (x = 0; x < 8; x++) {
			if (bitmap[y] & mask)
				ptr[x] = color;
			mask >>= 4;
		}
		ptr += _screenPitch;
	}
}

//
// Draw the mouse cursor (animated). This is mostly ripped from the cursor code in gfx.cpp
// We could plug in a different cursor here if we like to.
//
void NewGui::animateCursor() {
	int time = _system->getMillis(); 
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		const byte colors[4] = { 15, 15, 7, 8 };
		const byte color = colors[_cursorAnimateCounter];
		int i;
		
		for (i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = color;
				_cursor[16 * i + 7] = color;
			}
		}
	
		_system->setMouseCursor(_cursor, 16, 16, 7, 7);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

} // End of namespace GUI
