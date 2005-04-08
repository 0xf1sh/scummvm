/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/driver_vga.h"

#ifdef _MSC_VER
#define STUB_FUNC	printf("STUB:")
#else
#define STUB_FUNC	printf("STUB: %s\n", __PRETTY_FUNCTION__)
#endif

namespace Gob {

void VGAVideoDriver::drawSprite(SurfaceDesc *source, SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height) {
		int16 width = (right - left) + 1;
		int16 height = (bottom - top) + 1;
	
		byte *srcPos = source->vidPtr + (top * source->width) + left;
		byte *destPos = dest->vidPtr + (y * dest->width) + x;
		while (height--) {
			for (int16 i = 0; i < width; ++i) {
				if (srcPos[i])
					destPos[i] = srcPos[i];
			}

			srcPos += source->width; //width ?
			destPos += dest->width;
		}
	}
}

void VGAVideoDriver::fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color) {
	if (left < dest->width && right < dest->width && top < dest->height && bottom < dest->height) {
		byte *pos = dest->vidPtr + (top * dest->width) + left;
		int16 width = (right - left) + 1;
		int16 height = (bottom - top) + 1;
		while (height--) {
			for (int16 i = 0; i < width; ++i) {
				pos[i] = color;
			}

			pos += dest->width;
		}
	}
}

void VGAVideoDriver::putPixel(int16 x, int16 y, byte color, SurfaceDesc *dest) {
	if (x >= 0 && x < dest->width && y >= 0 && y < dest->height)
		dest->vidPtr[(y * dest->width) + x] = color;
}

void VGAVideoDriver::drawLetter(char item, int16 x, int16 y, FontDesc *fontDesc, byte color1, byte color2, byte transp, SurfaceDesc *dest) {
	byte *src, *dst;
	uint16 data;
	int i, j;
	
	src = (byte *)fontDesc->dataPtr + (item - fontDesc->startItem) * (fontDesc->itemSize & 0xff);
	dst = dest->vidPtr + x + dest->width * y;

	for (i = 0; i < fontDesc->itemHeight; i++) {
		data = READ_BE_UINT16(src);
		src += 2;
		if (fontDesc->itemSize <= 8)
			src--;

		for (j = 0; j < fontDesc->itemWidth; j++) {
			if (data & 0x8000) {
				*dst = color2;
			} else {
				if (color1 == 0)
					*dst = transp;
			}
			dst++;
			data <<= 1;
		}
		dst += dest->width - fontDesc->itemWidth;
	}
}

void VGAVideoDriver::drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color) {
	STUB_FUNC;
}

void VGAVideoDriver::drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, SurfaceDesc *dest) {
	STUB_FUNC;
}

}

