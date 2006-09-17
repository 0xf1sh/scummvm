/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * $URL$
 * $Id$
 *
 */

#include "be_os5.h"

void OSystem_PalmOS5::setCursorPalette(const byte *colors, uint start, uint num) {
	for(uint i = 0; i < num; i++) {
		_mousePal[i + start] = gfxMakeDisplayRGB(colors[0], colors[1], colors[2]);
		colors += 4;
	}
	
	_cursorPaletteDisabled = false;
}

void OSystem_PalmOS5::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
}

void OSystem_PalmOS5::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	if (w == 0 || h == 0)
		return;

	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColor = keycolor;

	// copy new cursor
	byte *dst = _mouseDataP;
	memset(dst, MAX_MOUSE_W * MAX_MOUSE_H, keycolor);
	while (h--) {
		memcpy(dst, buf, w);
		dst += MAX_MOUSE_W;
		buf += w;
	}
}

void OSystem_PalmOS5::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible)
		return;

	byte *src = _mouseDataP;		// Image representing the mouse
	byte color;
	int width;

	_mouseCurState.y = _mouseCurState.y >= _screenHeight ? _screenHeight - 1 : _mouseCurState.y;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;

	int draw_x = x;
	int draw_y = y;

	// clip the mouse rect
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * MAX_MOUSE_W;
		y = 0;
	}
	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

	// Backup the covered area draw the mouse cursor
	if (_overlayVisible) {
		int16 *bak = (int16 *)_mouseBackupP;			// Surface used to backup the area obscured by the mouse
		int16 *dst = _overlayP + y * _screenWidth + x;
		int16 *pal = _cursorPaletteDisabled ? _nativePal : _mousePal;

		do {
			width = w;
			do {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					*dst = pal[color];
				dst++;
			} while (--width);

			src += MAX_MOUSE_W - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
		} while (--h);

	} else {
		byte *bak = _mouseBackupP;						// Surface used to backup the area obscured by the mouse
		byte *dst =_offScreenP + y * _screenWidth + x;	// Surface we are drawing into

		do {
			width = w;
			do {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					*dst = color;
				dst++;
			} while (--width);

			src += MAX_MOUSE_W - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
		} while (--h);
	}

	_mouseDrawn = true;
}

void OSystem_PalmOS5::undraw_mouse() {
	if (!_mouseDrawn)
		return;

	int h = _mouseOldState.h;
	// No need to do clipping here, since draw_mouse() did that already
	if (_overlayVisible) {
		int16 *bak = (int16 *)_mouseBackupP;
		int16 *dst = _overlayP + _mouseOldState.y * _screenWidth + _mouseOldState.x;

		do {
			memcpy(dst, bak, _mouseOldState.w * 2);
			bak += MAX_MOUSE_W;
			dst += _screenWidth;
		} while (--h);

	} else {
		byte *dst, *bak = _mouseBackupP;
		dst = _offScreenP + _mouseOldState.y * _screenWidth + _mouseOldState.x;
		
		do {
			memcpy(dst, bak, _mouseOldState.w);
			bak += MAX_MOUSE_W;
			dst += _screenWidth;
		} while (--h);
	}

	_mouseDrawn = false;
}
