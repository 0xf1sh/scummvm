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

#ifndef D_DRAW_H
#define D_DRAW_H

#include "common/rect.h"

namespace Sword2 {

// This is the maximum mouse cursor size in the SDL backend

#define MAX_MOUSE_W		80
#define MAX_MOUSE_H		80

#define RENDERAVERAGETOTAL	4

#define BLOCKWIDTH		64
#define BLOCKHEIGHT		64
#define MAXLAYERS		5

#define PALTABLESIZE		64 * 64 * 64

// Maximum scaled size of a sprite
#define SCALE_MAXWIDTH		512
#define SCALE_MAXHEIGHT		512

// Dirty grid cell size
#define CELLWIDE		10
#define CELLDEEP		20

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct MouseAnim {
	uint8 runTimeComp;	// type of runtime compression used for the
				// frame data
	uint8 noAnimFrames;	// number of frames in the anim
	int8 xHotSpot;		
	int8 yHotSpot;
	uint8 mousew;
	uint8 mouseh;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct BlockSurface {
	byte data[BLOCKWIDTH * BLOCKHEIGHT];
	bool transparent;
};

class Graphics {
	friend class MoviePlayer;

private:
	Sword2Engine *_vm;

	byte *_buffer;
	byte *_dirtyGrid;

	uint16 _gridWide;
	uint16 _gridDeep;

	int32 _renderCaps;
	int8 _renderLevel;

	uint8 _menuStatus[2];
	byte *_icons[2][RDMENU_MAXPOCKETS];
	uint8 _pocketStatus[2][RDMENU_MAXPOCKETS];

	uint8 _iconCount;

	bool _needFullRedraw;

	byte _paletteMatch[PALTABLESIZE];

	uint8 _fadeStatus;

	int32 _fadeStartTime;
	int32 _fadeTotalTime;

	uint8 _mouseFrame;
	byte *_mouseSprite;
	struct MouseAnim *_mouseAnim;
	struct MouseAnim *_luggageAnim;
	int32 *_mouseOffsets;
	int32 *_luggageOffset;

	// Scroll variables.  _scrollX and _scrollY hold the current scroll
	// position, and _scrollXTarget and _scrollYTarget are the target
	// position for the end of the game cycle.

	int16 _scrollX;
	int16 _scrollY;

	int16 _scrollXTarget;
	int16 _scrollYTarget;
	int16 _scrollXOld;
	int16 _scrollYOld;

	int16 _parallaxScrollX;	// current x offset to link a sprite to the
				// parallax layer
	int16 _parallaxScrollY;	// current y offset to link a sprite to the
				// parallax layer
	int16 _locationWide;
	int16 _locationDeep;

	uint16 _layer;

	int32 _initialTime;
	int32 _startTime;
	int32 _totalTime;
	int32 _renderAverageTime;
	int32 _framesPerGameCycle;
	bool _renderTooSlow;

	uint8 _xBlocks[MAXLAYERS];
	uint8 _yBlocks[MAXLAYERS];

	// An array of sub-blocks, one for each of the parallax layers.

	BlockSurface **_blockSurfaces[MAXLAYERS];

	uint16 _xScale[SCALE_MAXWIDTH];
	uint16 _yScale[SCALE_MAXHEIGHT];

	byte *_lightMask;

	void clearIconArea(int menu, int pocket, Common::Rect *r);

	void decompressMouse(byte *decomp, byte *comp, int width, int height, int pitch, int xOff = 0, int yOff = 0);

	void fadeServer(void);

	void scaleImageFast(byte *dst, uint16 dstPitch, uint16 dstWidth,
		uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth,
		uint16 srcHeight);
	void scaleImageGood(byte *dst, uint16 dstPitch, uint16 dstWidth,
		uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth,
		uint16 srcHeight, byte *backbuf);

	void updateRect(Common::Rect *r);

	void blitBlockSurface(BlockSurface *s, Common::Rect *r, Common::Rect *clipRect);

	void mirrorSprite(byte *dst, byte *src, int16 w, int16 h);
	int32 decompressRLE256(byte *dest, byte *source, int32 decompSize);
	void unwindRaw16(byte *dest, byte *source, uint8 blockSize, byte *colTable);
	int32 decompressRLE16(byte *dest, byte *source, int32 decompSize, byte *colTable);


public:
	Graphics(Sword2Engine *vm, int16 width, int16 height);
	~Graphics();

	// Game screen metrics
	int16 _screenWide;
	int16 _screenDeep;

	byte _palette[256 * 4];

	byte *getScreen(void) { return _buffer; }

	int8 getRenderLevel(void);
	void setRenderLevel(int8 level);

	void clearScene(void);

	void processMenu(void);
	int32 showMenu(uint8 menu);
	int32 hideMenu(uint8 menu);
	int32 setMenuIcon(uint8 menu, uint8 pocket, byte *icon);
	void closeMenuImmediately(void);

	void markAsDirty(int16 x0, int16 y0, int16 x1, int16 y1);
	void updateDisplay(bool redrawScene = true);
	void setNeedFullRedraw(void);

	void setPalette(int16 startEntry, int16 noEntries, byte *palette, uint8 setNow);
	void updatePaletteMatchTable(byte *data);
	uint8 quickMatch(uint8 r, uint8 g, uint8 b);
	int32 fadeUp(float time = 0.75);
	int32 fadeDown(float time = 0.75);
	uint8 getFadeStatus(void);
	void dimPalette(void);
	void waitForFade(void);

	int32 setMouseAnim(byte *ma, int32 size, int32 mouseFlash);
	int32 setLuggageAnim(byte *la, int32 size);
	int32 animateMouse(void);

	void drawMouse(void);

	void resetRenderEngine(void);

	void setScrollTarget(int16 sx, int16 sy);
	void initialiseRenderCycle(void);
	void startRenderCycle(void);
	bool endRenderCycle(void);
	void renderParallax(Parallax *p, int16 layer);
	void setLocationMetrics(uint16 w, uint16 h);
	int32 initialiseBackgroundLayer(Parallax *p);
	void closeBackgroundLayer(void);

	void plotPoint(uint16 x, uint16 y, uint8 colour);
	void drawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint8 colour);
#ifdef BACKEND_8BIT
	void plotYUV(byte *lut, int width, int height, byte *const *dat);
#endif


	int32 createSurface(SpriteInfo *s, byte **surface);
	void drawSurface(SpriteInfo *s, byte *surface, Common::Rect *clipRect = NULL);
	void deleteSurface(byte *surface);
	int32 drawSprite(SpriteInfo *s);
	int32 openLightMask(SpriteInfo *s);
	int32 closeLightMask(void);
};

} // End of namespace Sword2

#endif
