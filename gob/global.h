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
#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "gob/dataio.h"
#include "gob/video.h"

#include "common/file.h"

namespace Gob {

extern char pressedKeys[128];

extern char useMouse;
extern int16 mousePresent;

extern int16 presentCGA;
extern int16 presentEGA;
extern int16 presentVGA;
extern int16 presentHER;

extern int16 videoMode;

extern int16 disableVideoCfg;

#define VIDMODE_CGA	0x05
#define VIDMODE_EGA	0x0d
#define VIDMODE_VGA 0x13
#define VIDMODE_HER	7

extern uint16 presentSound;
extern uint16 soundFlags;
extern int16 disableSoundCfg;
//extern int16 blasterPort;

#define PROAUDIO_FLAG	0x10
#define ADLIB_FLAG		0x08
#define BLASTER_FLAG	0x04
#define INTERSOUND_FLAG	0x02
#define SPEAKER_FLAG	0x01
#define MIDI_FLAG		0x4000

extern uint16 disableLangCfg;
extern uint16 language;

#define NO	0
#define YES	1
#define UNDEF	2

#define F1_KEY	0x3b00
#define F2_KEY	0x3c00
#define F3_KEY	0x3d00
#define F4_KEY	0x3e00
#define F5_KEY	0x3f00
#define F6_KEY	0x4000
#define ESCAPE	0x001b
#define ENTER	0x000d

/* Configuration */
extern char batFileName[8];

/* Init */
extern int16 requiredSpace;

/* Timer variables */
extern int32 startTime;
extern char timer_enabled;
extern int16 timer_delta;

extern int16 frameWaitTime;
extern int32 startFrameTime;

/* Mouse */
extern int16 disableMouseCfg;

extern int16 mouseXShift;
extern int16 mouseYShift;
extern int16 mouseMaxCol;
extern int16 mouseMaxRow;

/* Memory */
extern int16 allocatedBlocks[2];
extern char *heapHeads[2];
extern int32 heapSize;
extern char *heapFence;
extern char inAllocSub;

/* Timer and delays */
extern int16 delayTime;
extern int16 fastComputer;

/* Joystick */
extern char useJoystick;

/* Files */
#define MAX_FILES	30

extern int16 filesCount;
extern File filesHandles[MAX_FILES];

/* Data files */
extern struct ChunkDesc *dataFiles[MAX_DATA_FILES];
extern int16 numDataChunks[MAX_DATA_FILES];
extern int16 dataFileHandles[MAX_DATA_FILES];
extern int32 chunkPos[MAX_SLOT_COUNT * MAX_DATA_FILES];
extern int32 chunkOffset[MAX_SLOT_COUNT * MAX_DATA_FILES];
extern int32 chunkSize[MAX_SLOT_COUNT * MAX_DATA_FILES];
extern char isCurrentSlot[MAX_SLOT_COUNT * MAX_DATA_FILES];
extern int32 packedSize;

/* Video drivers */
#define UNK_DRIVER	0
#define VGA_DRIVER	1
#define EGA_DRIVER	2
#define CGA_DRIVER	3
#define HER_DRIVER	4

extern SurfaceDesc primarySurfDesc;
extern SurfaceDesc *pPrimarySurfDesc;
extern int16 sprAllocated;

extern int16 primaryWidth;
extern int16 primaryHeight;

extern int16 doRangeClamp;

extern DrawPackedSpriteFunc pDrawPacked;

extern char redPalette[256];
extern char greenPalette[256];
extern char bluePalette[256];

extern int16 setAllPalette;

extern SurfaceDesc *curPrimaryDesc;
extern SurfaceDesc *allocatedPrimary;

extern int16 oldMode;
extern int16 needDriverInit;
extern char dontSetPalette;

extern PalDesc *pPaletteDesc;

typedef void (*FileHandler) (char *path);
extern FileHandler pFileHandler;

	//extern void interrupt(*oldInt5Handler) (void);
	//extern void interrupt(*oldInt0Handler) (void);
	//extern void interrupt(*oldInt1bHandler) (void);

extern int16 unusedPalette1[18];
extern int16 unusedPalette2[16];
extern Color vgaPalette[16];
extern PalDesc paletteStruct;

extern int16 debugFlag;
extern int16 breakSet;
extern int16 inVM;
extern int16 colorCount;

extern int16 trySmallForBig;
extern int16 checkMemFlag;

extern char inter_resStr[200];
extern int32 inter_resVal;

extern char *inter_variables;
extern char *inter_execPtr;
extern int16 inter_animDataSize;

extern int16 inter_mouseX;
extern int16 inter_mouseY;

extern char *tmpPalBuffer;

typedef struct Rectangle {
	int16 left;
	int16 top;
	int16 width;
	int16 height;
} Rectangle;

}				// End of namespace Gob

#endif
