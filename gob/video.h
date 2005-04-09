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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_VIDEO_H
#define GOB_VIDEO_H

namespace Gob {

#include "stdafx.h"
#include "common/util.h"
	
#define VID_SET_CURSOR(val) { _AH = 1; _CX = (val); geninterrupt(0x10); }
#define VID_RESTORE_MODE { _AX = 3; geninterrupt(0x10); }

#define TEXT_VID_SEG 	0xB800
#define TEXT_VID_OFF 	0
#define TEXT_COL_COUNT	80
#define TEXT_ROW_COUNT	25

	
typedef struct SurfaceDesc_t {
	int16 width;
	int16 height;
	char reserved1;
	char flag;
	int16 vidMode;
	byte *vidPtr;
	int16 reserved2;
} SurfaceDesc;

typedef struct FontDesc_t {
	char *dataPtr;
	char itemWidth;
	char itemHeight;
	char startItem;
	char endItem;
	char itemSize;
	char bitWidth;
	void *extraData;
} FontDesc;


class VideoDriver {
public:
	VideoDriver() {}
	virtual ~VideoDriver() {}
	virtual void drawSprite(SurfaceDesc *source, SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) = 0;
	virtual void fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom, byte color) = 0;
	virtual void putPixel(int16 x, int16 y, byte color, SurfaceDesc *dest) = 0;		
	virtual void drawLetter(char item, int16 x, int16 y, FontDesc *fontDesc, byte color1, byte color2, byte transp, SurfaceDesc *dest) = 0;
	virtual void drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, byte color) = 0;
	virtual void drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y, byte transp, SurfaceDesc *dest) = 0;
};


typedef void (*FillRectFunc) (SurfaceDesc * desc, int16 left, int16 top, int16 right,
    int16 bottom, int16 color);
typedef void (*DrawSpriteFunc) (SurfaceDesc * source, SurfaceDesc * dest,
    int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp);
typedef void (*PutPixelFunc) (int16 x, int16 y, int16 color, SurfaceDesc * desc);
typedef void (*XorRectFunc) (SurfaceDesc * desc, int16 left, int16 top, int16 right,
    int16 bottom);
typedef void (*SetXorValFunc) (int16 val);
typedef void (*DrawLineFunc) (SurfaceDesc * desc, int16 x0, int16 y0, int16 x1,
    int16 y1, int16 color);
typedef void (*DrawLetterFunc) (char item, int16 x, int16 y, FontDesc * fontDesc,
    int16 color1, int16 color2, int16 transp, SurfaceDesc * dest);
typedef char (*DrawPackedSpriteFunc) (byte *sprBuf, int16 width, int16 height,
    int16 x, int16 y, int16 transp, SurfaceDesc * dest);

#define GDR_VERSION	4

#define SET_SEG(ptr,seg)	(((unsigned*)(ptr))[1] = (seg))


#define PRIMARY_SURFACE		0x80
#define RETURN_PRIMARY		0x01
#define DISABLE_SPR_ALLOC	0x20

typedef struct Color {
	byte red;
	byte green;
	byte blue;
} Color;

typedef struct PalDesc {
	Color *vgaPal;
	int16 *unused1;
	int16 *unused2;
} PalDesc;

char vid_initDriver(int16 vidMode);
void vid_freeDriver(void);
int32 vid_getRectSize(int16 width, int16 height, int16 flag, int16 mode);
SurfaceDesc *vid_initSurfDesc(int16 vidMode, int16 width, int16 height, int16 flags);
void vid_freeSurfDesc(SurfaceDesc * surfDesc);
int16 vid_clampValue(int16 val, int16 max);
void vid_drawSprite(SurfaceDesc * source, SurfaceDesc * dest, int16 left,
    int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp);
void vid_fillRect(SurfaceDesc * dest, int16 left, int16 top, int16 right, int16 bottom,
    int16 color);
void vid_drawLine(SurfaceDesc * dest, int16 x0, int16 y0, int16 x1, int16 y1,
    int16 color);
void vid_putPixel(int16 x, int16 y, int16 color, SurfaceDesc * dest);
void vid_drawLetter(char item, int16 x, int16 y, FontDesc * fontDesc, int16 color1,
    int16 color2, int16 transp, SurfaceDesc * dest);
void vid_clearSurf(SurfaceDesc * dest);
void vid_drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y,
    int16 transp, SurfaceDesc * dest);
void vid_setPalElem(int16 index, char red, char green, char blue, int16 unused,
    int16 vidMode);
void vid_setPalette(PalDesc * palDesc);
void vid_setFullPalette(PalDesc * palDesc);
void vid_initPrimary(int16 mode);
char vid_spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight, int16 x,
    int16 y, int16 transp, SurfaceDesc * destDesc);
void vid_setHandlers(void);

}				// End of namespace Gob

#endif
