/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "Toolbar.h"

#include "SDL_ImageResource.h"

namespace CEGUI {

	GUIElement::GUIElement(int x, int y, int width, int height) :
	_background(0), _drawn(false), _visible(true), _x(x), _y(y), _width(width), _height(height)
	{
	}

	bool GUIElement::setBackground(WORD backgroundReference) {
		_background = new SDL_ImageResource();
		if (!_background->load(backgroundReference)) {
			delete _background;
			_background = NULL;
			return false;
		}
		if (!_height && !_width) {
			_height = _background->height();
			_width = _background->width();
		}
		else 
		if (_background->height() != _height || _background->width() != _width) {
			delete _background;
			_background = NULL;
			return false;
		}
		return true;
	}
	 
	void GUIElement::move(int x, int y) {
		_x = x;
		_y = y;
	}

	bool GUIElement::draw(SDL_Surface *surface) {
		if (_background && !_drawn && _visible) {
			SDL_Rect rect;

			rect.x = _x;
			rect.y = _y;
			rect.w = _width;
			rect.h = _height;

			SDL_BlitSurface(_background->get(), NULL, surface, &rect);

			_drawn = true;

			return true;
		}
		else
			return false;
	}

	bool GUIElement::checkInside(int x, int y) {
		if (x >= _x && x <= _x + _width && y >= _y && y <= _y + _height)
			return true;
		else
			return false; 
	}

	void GUIElement::setVisible(bool visibility) {
		if (visibility && !_visible)
			_drawn = false;
		_visible = visibility;
	}

	bool GUIElement::visible() {
		return _visible;
	}

	void GUIElement::forceRedraw() {
		_drawn = false;
	}

	bool GUIElement::drawn() {
		return _drawn;
	}

	int GUIElement::x() {
		return _x;
	}

	int GUIElement::y() {
		return _y;
	}

	int GUIElement::width() {
		return _width;
	}

	int GUIElement::height() {
		return _height;
	}

	GUIElement::~GUIElement() {
		if (_background)
			delete _background;
	}

}
