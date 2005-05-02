/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
#include "common/util.h"
#include "graphics/surface.h"

namespace Graphics {

void Surface::hLine(int x, int y, int x2, uint32 color) {
	// Clipping
	if (y < 0 || y >= h)
		return;

	if (x2 < x)
		SWAP(x2, x);
	
	if (x < 0)
		x = 0;
	if (x2 >= w)
		x2 = w - 1;
	
	if (bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(x, y);
		if (x2 >= x)
			memset(ptr, (byte)color, x2-x+1);
	} else if (bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(x, y);
		while (x++ <= x2) {
			*ptr++ = (uint16)color;
		}
	} else {
		error("Surface::hLine: bytesPerPixel must be 1 or 2");
	}
}

void Surface::vLine(int x, int y, int y2, uint32 color) {
	// Clipping
	if (x < 0 || x >= w)
		return;

	if (y2 < y)
		SWAP(y2, y);
	
	if (y < 0)
		y = 0;
	if (y2 >= h)
		y2 = h - 1;
	
	if (bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = (byte)color;
			ptr += pitch;
		}
	} else if (bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = (uint16)color;
			ptr += pitch/2;
		}
	} else {
		error("Surface::vLine: bytesPerPixel must be 1 or 2");
	}
}

void Surface::fillRect(const Common::Rect &rOld, uint32 color) {
	Common::Rect r(rOld);
	r.clip(w, h);
	
	if (!r.isValidRect())
		return;

	int width = r.width();
	int height = r.height();
	int i;

	if (bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(r.left, r.top);
		while (height--) {
			memset(ptr, (byte)color, width);
			ptr += pitch;
		}
	} else if (bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(r.left, r.top);
		while (height--) {
			for (i = 0; i < width; i++) {
				ptr[i] = (uint16)color;
			}
			ptr += pitch/2;
		}
	} else {
		error("Surface::fillRect: bytesPerPixel must be 1 or 2");
	}
}

void Surface::frameRect(const Common::Rect &r, uint32 color) {
	hLine(r.left, r.top, r.right-1, color);
	hLine(r.left, r.bottom-1, r.right-1, color);
	vLine(r.left, r.top, r.bottom-1, color);
	vLine(r.right-1, r.top, r.bottom-1, color);
}

} // End of namespace Graphics
