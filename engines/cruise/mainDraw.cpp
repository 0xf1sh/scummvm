/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "cruise/cruise_main.h"
#include "cruise/polys.h"
#include "common/util.h"

namespace Cruise {

int currentTransparent;

struct autoCellStruct {
	struct autoCellStruct *next;
	short int ovlIdx;
	short int objIdx;
	short int type;
	short int newValue;
	cellStruct *pCell;
};

autoCellStruct autoCellHead;

void addAutoCell(int overlayIdx, int idx, int type, int newVal, cellStruct *pObject) {
	autoCellStruct *pNewEntry;

	pNewEntry = new autoCellStruct;

	pNewEntry->next = autoCellHead.next;
	autoCellHead.next = pNewEntry;

	pNewEntry->ovlIdx = overlayIdx;
	pNewEntry->objIdx = idx;
	pNewEntry->type = type;
	pNewEntry->newValue = newVal;
	pNewEntry->pCell = pObject;
}

void freeAutoCell(void) {
	autoCellStruct *pCurrent = autoCellHead.next;

	while (pCurrent) {
		autoCellStruct *next = pCurrent->next;

		if (pCurrent->type == 5) {
			objInit(pCurrent->ovlIdx, pCurrent->objIdx, pCurrent->newValue);
		} else {
			setObjectPosition(pCurrent->ovlIdx, pCurrent->objIdx, pCurrent->type, pCurrent->newValue);
		}

		if (pCurrent->pCell->animWait < 0) {
			objectParamsQuery params;

			getMultipleObjectParam(pCurrent->ovlIdx, pCurrent->objIdx, &params);

			pCurrent->pCell->animCounter = params.var6 - 1;
		}

		delete pCurrent;

		pCurrent = next;
	}
}

void flipScreen(void) {
	uint8 *swapPtr;

	swapPtr = gfxModuleData.pPage00;
	gfxModuleData.pPage00 = gfxModuleData.pPage10;
	gfxModuleData.pPage10 = swapPtr;

	gfxModuleData_flipScreen();

	/*memcpy(globalAtariScreen, gfxModuleData.pPage00, 16000);
	 * convertAtariToRaw(gfxModuleData.pPage00,globalScreen,200,320); */
}

int spriteX1;
int spriteX2;
int spriteY1;
int spriteY2;

char *polyOutputBuffer;

void pixel(int x, int y, char color) {
	if (x >= 0 && x < 320 && y >= 0 && y < 200)
		polyOutputBuffer[320 * y + x] = color;
}

// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
void flipPoly(int fileId, int16 *dataPtr, int scale, char** newFrame, int X, int Y, int *outX, int *outY, int *outScale)
{
	if (*dataPtr == 0)
	{
		int16 offset;
		int16 newX;
		int16 newY;

		dataPtr ++;

		offset = *(dataPtr++);
		flipShort(&offset);

		newX = *(dataPtr++);
		flipShort(&newX);

		newY = *(dataPtr++);
		flipShort(&newY);

		offset += fileId;

		if (offset >= 0 )
		{
			if (filesDatabase[offset].resType == 0 && filesDatabase[offset].subData.ptr)
			{
				dataPtr = (int16 *)filesDatabase[offset].subData.ptr;
			}
		}

		scale = -scale;
		X -= newX;
		Y -= newY;
	}

	*newFrame = (char*)dataPtr;
	*outX = X;
	*outY = Y;
	*outScale = scale;
}

int upscaleValue(int value, int scale) {
	return (((value * scale) << 8) / 2);
}

int m_flipLeftRight;
int m_useSmallScale;
int m_lowerX;
int m_lowerY;
int m_coordCount;
int m_first_X;
int m_first_Y;
int m_scaleValue;
int m_color;

int16 DIST_3D[512];
int16 polyBuffer2[512];
int16 XMIN_XMAX[404];
int16 polyBuffer4[512];

// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
void getPolySize(int positionX, int positionY, int scale, int sizeTable[4], unsigned char *dataPtr)
{
	int upperBorder;
	int lowerBorder;
	m_flipLeftRight = 0;

	if (scale < 0) {		// flip left right
		m_flipLeftRight = 1;
		scale = -scale;
	}
	// X1

	upperBorder = *(dataPtr + 3);

	if (m_flipLeftRight) {
		upperBorder = -upperBorder;
	}

	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;
	upperBorder = -upperBorder;
	lowerBorder = upperBorder;

	// X2

	upperBorder = *(dataPtr + 1);
	upperBorder -= *(dataPtr + 3);

	if (m_flipLeftRight) {
		upperBorder = -upperBorder;
	}

	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;

	if (upperBorder < lowerBorder) {	// exchange borders if lower > upper
		SWAP(upperBorder, lowerBorder);
	}

	sizeTable[0] = lowerBorder;	// left
	sizeTable[1] = upperBorder;	// right

	// Y1

	upperBorder = *(dataPtr + 4);
	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;
	upperBorder = -upperBorder;
	lowerBorder = upperBorder;

	// Y2

	upperBorder = *(dataPtr + 2);
	upperBorder -= *(dataPtr + 4);
	upperBorder = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;

	if (upperBorder < lowerBorder) {	// exchange borders if lower > upper
		SWAP(upperBorder, lowerBorder);
	}

	sizeTable[2] = lowerBorder;	// bottom
	sizeTable[3] = upperBorder;	// top
}

int polyVar1;

void blitPolyMode1(char *dest, char *ptr, int16 * buffer, char color)
{
	ASSERT(0);
}

void blitPolyMode2(char *dest, int16 * buffer, char color)
{
	int i;

	for (i = 0; i < polyVar1; i++) {
		line(buffer[i * 2], buffer[i * 2 + 1], buffer[(i + 1) * 2], buffer[(i + 1) * 2 + 1], color);
	} 

	fillpoly(buffer, polyVar1, color);
}

int polySize1;
int polySize2;
int polySize3;
int polySize4;

int16 *polyVar2;

char *drawPolyMode1(char *dataPointer, int linesToDraw) {
	int index;
	int16 *pBufferDest;

	polyVar1 = linesToDraw;
	pBufferDest = &polyBuffer4[polyVar1 * 2];
	index = *(dataPointer++);

	polySize1 = polySize2 = pBufferDest[-2] = pBufferDest[-2 + linesToDraw * 2] = polyBuffer2[index * 2];
	polySize1 = polySize2 = pBufferDest[-1] = pBufferDest[-1 + linesToDraw * 2] = polyBuffer2[(index * 2) + 1];

	linesToDraw--;

	pBufferDest -= 2;

	polyVar2 = pBufferDest;

	do {
		int value;

		index = *(dataPointer++);
		value = pBufferDest[-2] = pBufferDest[-2 + polyVar1 * 2] = polyBuffer2[index * 2];

		if (value < polySize1) {
			polySize1 = value;
		}
		if (value > polySize2) {
			polySize2 = value;
		}

		value = pBufferDest[-1] = pBufferDest[-1 + polyVar1 * 2] = polyBuffer2[(index * 2) + 1];

		if (value > polySize4) {
			polySize4 = value;
		}
		if (value < polySize3) {
			polySize3 = value;
			polyVar2 = pBufferDest - 4;
		}
		pBufferDest -= 2;

	} while (--linesToDraw);

	return dataPointer;
}

char *drawPolyMode2(char *dataPointer, int linesToDraw)
{
	int index;
	int16 *pBufferDest;

	pBufferDest = polyBuffer4;
	polyVar1 = linesToDraw;
	polyVar2 = polyBuffer4;
	index = *(dataPointer++);

	polySize1 = polySize2 = pBufferDest[0] = pBufferDest[linesToDraw * 2] = polyBuffer2[index * 2];
	polySize1 = polySize2 = pBufferDest[1] = pBufferDest[linesToDraw * 2 + 1] = polyBuffer2[(index * 2) + 1];

	linesToDraw--;

	pBufferDest += 2;

	do {
		int value;

		index = *(dataPointer++);
		value = pBufferDest[0] = pBufferDest[polyVar1 * 2] = polyBuffer2[index * 2];

		if (value < polySize1) {
			polySize1 = value;
		}
		if (value > polySize2) {
			polySize2 = value;
		}

		value = pBufferDest[1] = pBufferDest[polyVar1 * 2 + 1] = polyBuffer2[(index * 2) + 1];

		if (value > polySize4) {
			polySize4 = value;
		}
		if (value < polySize3) {
			polySize3 = value;
			polyVar2 = pBufferDest;
		}

		pBufferDest += 2;

	} while (--linesToDraw);

	return dataPointer;
}

// this function builds the poly model and then calls the draw functions (OLD: mainDrawSub1Sub5)
void buildPolyModel(int positionX, int positionY, int scale, char *pMask, char *destBuffer, char *dataPtr) {
	int counter = 0;	// numbers of coordinates to process
	int startX = 0;		// first X in model
	int startY = 0;		// first Y in model
	int x = 0;		// current X
	int y = 0;		// current Y
	int offsetXinModel = 0;	// offset of the X value in the model
	int offsetYinModel = 0;	// offset of the Y value in the model
	unsigned char *dataPointer = (unsigned char *)dataPtr;
	int16 *ptrPoly_1_Buf = DIST_3D;
	int16 *ptrPoly_2_Buf;
	polyOutputBuffer = destBuffer;	// global

	m_flipLeftRight = 0;
	m_useSmallScale = 0;
	m_lowerX = *(dataPointer + 3);
	m_lowerY = *(dataPointer + 4);

	if (scale < 0) {
		scale = -scale;	// flip left right
		m_flipLeftRight = 1;
	}

	if (scale < 0x180) {	// If scale is smaller than 384
		m_useSmallScale = 1;
		m_scaleValue = scale << 1;	// double scale
	} else {
		m_scaleValue = scale;
	}

	dataPointer += 5;

	m_coordCount = (*(dataPointer++)) + 1;	// original uses +1 here but its later substracted again, we could skip it
	m_first_X = *(dataPointer++);
	m_first_Y = *(dataPointer++);
	startX = m_lowerX - m_first_X;
	startY = m_lowerY - m_first_Y;

	if (m_useSmallScale) {
		startX >>= 1;
		startY >>= 1;
	}

	if (m_flipLeftRight) {
		startX = -startX;
	}

	/*
	 * NOTE:
	 * 
	 * The original code continues here with using X, Y instead of startX and StartY.
	 * 
	 * Original code:
	 * positionX -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	 * positionY -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	 */

	// get coordinates from data

	startX = positionX - ((upscaleValue(startX, m_scaleValue) + 0x8000) >> 16);
	startY = positionY - ((upscaleValue(startY, m_scaleValue) + 0x8000) >> 16);

	ptrPoly_1_Buf[0] = 0;
	ptrPoly_1_Buf[1] = 0;
	ptrPoly_1_Buf += 2;
	counter = m_coordCount - 1 - 1;	// skip the first pair, we already have the values

	// dpbcl0
	do {
		x = *(dataPointer++) - m_first_X;
		if (m_useSmallScale) {	// shrink all coordinates by factor 2 if a scale smaller than 384 is used
			x >>= 1;
		}
		ptrPoly_1_Buf[0] = offsetXinModel - x;
		ptrPoly_1_Buf++;
		offsetXinModel = x;

		y = *(dataPointer++) - m_first_Y;
		if (m_useSmallScale) {
			y >>= 1;
		}
		ptrPoly_1_Buf[0] = -(offsetYinModel - y);
		ptrPoly_1_Buf++;
		offsetYinModel = y;

	} while (--counter);

	// scale and adjust coordinates with offset (using two polybuffers by doing that)
	ptrPoly_2_Buf = DIST_3D;
	ptrPoly_1_Buf = polyBuffer2;
	counter = m_coordCount - 1;	// reset counter // process first pair two
	int m_current_X = 0;
	int m_current_Y = 0;

	do {
		x = ptrPoly_2_Buf[0];

		if (m_flipLeftRight == 0) {
			x = -x;
		}
		//////////////////

		m_current_X += upscaleValue(x, m_scaleValue);
		ptrPoly_1_Buf[0] = ((m_current_X + 0x8000) >> 16) + startX;	// adjust X value with start offset

		m_current_Y += upscaleValue(ptrPoly_2_Buf[1], m_scaleValue);
		ptrPoly_1_Buf[1] = ((m_current_Y + 0x8000) >> 16) + startY;	// adjust Y value with start offset

		/////////////////

		ptrPoly_1_Buf += 2;
		ptrPoly_2_Buf += 2;

	} while (--counter);

	// position of the dataPointer is m_coordCount * 2

	do {
		int linesToDraw = *dataPointer++;

		if (linesToDraw > 1) {	// if value not zero
			uint16 minimumScale;

			m_color = *dataPointer;	// color
			dataPointer += 2;

			minimumScale = *(uint16 *) (dataPointer);
			dataPointer += 2;

			flipShort(&minimumScale);

			if (minimumScale <= scale)
			{	
				if (m_flipLeftRight) {
					drawPolyMode1((char *)dataPointer, linesToDraw);
				} else {
					drawPolyMode2((char *)dataPointer, linesToDraw);
				}

				if (destBuffer) {
					if (pMask) {
						blitPolyMode1(destBuffer, pMask, polyBuffer4, m_color & 0xF);
					} else {
						blitPolyMode2(destBuffer, polyBuffer4, m_color & 0xF);
					}
				}
			}

			dataPointer += linesToDraw;
		} else {
			dataPointer += 4;
		}
	} while (*dataPointer != 0xFF);
}

// draw poly sprite (OLD: mainDrawSub1)
void mainDrawPolygons(int fileIndex, cellStruct *pObject, int X, int scale, int Y, char *destBuffer, char *dataPtr) {
	int newX;
	int newY;
	int newScale;
	char *newFrame;
	
	int var_8;		// unused

	int sizeTable[4];	// 0 = left, 1 = right, 2 = bottom, 3 = top

	// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
	flipPoly(fileIndex, (int16*)dataPtr, scale, &newFrame, X, Y, &newX, &newY, &newScale);

	// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
	getPolySize(newX, newY, newScale, sizeTable, (unsigned char *)newFrame);

	spriteX2 = sizeTable[0] - 2;	// left   border
	spriteX1 = sizeTable[1] + 18;	// right  border
	spriteY2 = sizeTable[2] - 2;	// bottom border
	spriteY1 = sizeTable[3] + 2;	// top    border

	if (spriteX2 >= 320)
		return;
	if (spriteX1 < 0)
		return;
	if (spriteY2 >= 200)
		return;
	if (spriteY1 < 0)
		return;

	if (spriteX2 < 0) {
		spriteX2 = 0;
	}
	if (spriteX1 > 320) {
		spriteX1 = 320;
	}
	if (spriteY2 < 0) {
		spriteY2 = 0;
	}
	if (spriteY1 > 200) {
		spriteY1 = 200;
	}

	if (spriteX1 == spriteX2)
		return;
	if (spriteY1 == spriteY2)
		return;

	char *pMask = NULL;
	var_8 = 0;

	if (pObject) {
		cellStruct *pCurrentObject = pObject;

		do {
			if (pCurrentObject->type == OBJ_TYPE_BGMK)
			{
//				ASSERT(0);
			}

			pCurrentObject = pCurrentObject->next;
		} while (pCurrentObject);
	}

	// this function builds the poly model and then calls the draw functions (OLD: mainDrawSub1Sub5)
	buildPolyModel(newX, newY, newScale, pMask, destBuffer, newFrame);
}

void mainSprite(int globalX, int globalY, gfxEntryStruct *pGfxPtr,
	    uint8 *ouputPtr, int newColor, int idx) {
	// this is used for font only

	if (pGfxPtr) {
		uint8 *initialOuput;
		uint8 *output;
		int i;
		int j;
		int x;
		int y;
		uint8 *ptr = pGfxPtr->imagePtr;
		int height = pGfxPtr->height;
		int width = pGfxPtr->width;

		if (globalY < 0) {
			globalY = 0;
		}

		if (globalY + pGfxPtr->height >= 198) {
			globalY = 198 - pGfxPtr->height;
		}

		initialOuput = ouputPtr + (globalY * 320) + globalX;

		y = globalY;
		x = globalX;

		for (i = 0; i < height; i++) {
			output = initialOuput + 320 * i;

			for (j = 0; j < width; j++) {
				uint8 color = *(ptr++);

				if (color) {
					if ((x >= 0) && (x < 320) && (y >= 0)
					    && (y < 200)) {
						if (color == 1) {
							*output = (uint8) 0;
						} else {
							*output =
							    (uint8) newColor;
						}
					}
				}
				output++;
			}
		}
	}
}

void mainDrawSub4(int objX1, int var_6, cellStruct *currentObjPtr,
	    char *data1, int objY2, int objX2, char *output, char *data2) {
	int x = 0;
	int y = 0;

	for (y = 0; y < var_6; y++) {
		for (x = 0; x < (objX1 * 8); x++) {
			uint8 color = (data1[0]);
			data1++;

			if ((x + objX2) >= 0 && (x + objX2) < 320
			    && (y + objY2) >= 0 && (y + objY2) < 200) {
				if (color != currentTransparent) {
					output[320 * (y + objY2) + x + objX2] =
					    color;
				}
			}
		}
	}
}

#ifdef _DEBUG
void drawCtp(void) {
	int i;

	if (ctp_walkboxTable) {
		for (i = 0; i < 15; i++) {
			uint16 *dataPtr = &ctp_walkboxTable[i * 40];
			int type = walkboxType[i];	// show different types in different colors

			if (*dataPtr) {
				int j;
				fillpoly((short *)dataPtr + 1, *dataPtr, type);

				for (j = 0; j < (*dataPtr - 1); j++) {
					line(dataPtr[1 + j * 2],
					    dataPtr[1 + j * 2 + 1],
					    dataPtr[1 + (j + 1) * 2],
					    dataPtr[1 + (j + 1) * 2 + 1], 0);
				}

				line(dataPtr[1 + j * 2],
				    dataPtr[1 + j * 2 + 1], dataPtr[1],
				    dataPtr[2], 0);
			}
		}
	}
}
#endif

void drawMenu(menuStruct *pMenu) {
	if (pMenu && pMenu->numElements) {
		int height;
		int x;
		int y;
		int var_10;
		int bx;
		int newX;
		int var_6;
		int currentY;
		int var_8;
		int di;
		menuElementStruct *si;

		height = pMenu->gfx->height;
		x = pMenu->x;
		y = pMenu->y;

		var_10 = pMenu->gfx->width / (199 - (pMenu->gfx->width * 2));

		bx = var_10 / (pMenu->numElements + 1);	// rustine...

		if (!bx) {
			bx++;

			if ((pMenu->numElements * height) + y > 199 - height) {
				y = ((-1 - pMenu->numElements) * height) + 200;
			}
		} else {
			if (var_10 % pMenu->numElements) {
				bx++;
			}

			y = height;
		}

		newX = 320 * (2 - bx);

		if (newX < x) {
			x = newX;
		}

		if (x < 0) {
			x = 0;
		}

		var_6 = (80 * (bx - 1)) + x;

		if (var_6 <= 320) {
			mainSprite(var_6, y - height, pMenu->gfx,
			    gfxModuleData.pPage10, video4, 320);
		}

		currentY = y;
		var_8 = 0;
		di = x;

		si = pMenu->ptrNextElement;

		if (si) {
			do {
				int color;

				gfxEntryStruct *var_2 = si->gfx;

				si->x = di;
				si->y = currentY;
				si->varA = 320;

				if (si->varC) {
					color = video3;
				} else {
					if (si->color != 255) {
						color = si->color;
					} else {
						color = video2;
					}
				}

				if (di < 320) {
					mainSprite(di, currentY, var_2,
					    gfxModuleData.pPage10, color, 320);
				}

				currentY += height;
				var_8++;

				if (var_8 == var_10) {
					var_8 = 0;
					di += 320;
					currentY = y;
				}

				si = si->next;
			} while (si);
		}
	}
}

int getValueFromObjectQuerry(objectParamsQuery *params, int idx) {
	switch (idx) {
	case 0:
		return params->X;
	case 1:
		return params->Y;
	case 2:
		return params->baseFileIdx;
	case 3:
		return params->fileIdx;
	case 4:
		return params->scale;
	case 5:
		return params->var5;
	case 6:
		return params->var6;
	case 7:
		return params->var7;
	}

	assert(0);

	return 0;
}

void mainDraw(int16 param) {
	uint8 *bgPtr;
	cellStruct *currentObjPtr;
	int16 currentObjIdx;
	int16 objX1 = 0;
	int16 objY1 = 0;
	int16 objZ1 = 0;
	int16 objX2;
	int16 objY2;
	int16 objZ2;
	int16 spriteHeight;

	if (fadeVar) {
		return;
	}

	bgPtr = backgroundPtrtable[currentActiveBackgroundPlane];

	if (bgPtr) {
		gfxModuleData_gfxCopyScreen((char *)bgPtr, (char *)gfxModuleData.pPage10);
	}

	autoCellHead.next = NULL;

	currentObjPtr = cellHead.next;

#ifdef _DEBUG
/*	polyOutputBuffer = (char*)bgPtr;
	drawCtp(); */
#endif

	//-------------------------------------------------- PROCESS SPRITES -----------------------------------------//

	while (currentObjPtr) {
		if ((currentActiveBackgroundPlane == currentObjPtr->backgroundPlane) && (currentObjPtr->freeze == 0) && (currentObjPtr->type == OBJ_SPRITE)) {
			objectParamsQuery params;

			currentObjIdx = currentObjPtr->idx;

			if ((currentObjPtr->followObjectOverlayIdx != currentObjPtr->overlay) || (currentObjPtr->followObjectIdx != currentObjPtr->idx)) {
				// Declaring this twice ?
				// objectParamsQuery params;

				getMultipleObjectParam(currentObjPtr->followObjectOverlayIdx, currentObjPtr->followObjectIdx, &params);

				objX1 = params.X;
				objY1 = params.Y;
				objZ1 = params.fileIdx;
			} else {
				objX1 = 0;
				objY1 = 0;
				objZ1 = 0;
			}

			getMultipleObjectParam(currentObjPtr->overlay,
			    currentObjIdx, &params);

			objX2 = objX1 + params.X;
			objY2 = objY1 + params.Y;
			objZ2 = params.fileIdx;

			if (objZ2 >= 0) {
				objZ2 += objZ1;
			}

			if ((params.var5 >= 0) && (objZ2 >= 0) && filesDatabase[objZ2].subData.ptr) {
				if (filesDatabase[objZ2].subData.resourceType == 8) {	// Poly
					mainDrawPolygons(objZ2, currentObjPtr, objX2, params.scale, objY2, (char *)gfxModuleData.pPage10, (char *)filesDatabase[objZ2].subData.ptr);	// poly
				} else if (filesDatabase[objZ2].subData.resourceType == 6) {	// sound
				} else if (filesDatabase[objZ2].resType == 1) {	//(num plan == 1)
				} else if (filesDatabase[objZ2].subData.resourceType == 4) {
					objX1 = filesDatabase[objZ2].width;	// width
					spriteHeight = filesDatabase[objZ2].height;	// height

					if (filesDatabase[objZ2].subData.ptr) {
						currentTransparent = filesDatabase[objZ2].subData.transparency;

						mainDrawSub4(objX1, spriteHeight, currentObjPtr, (char *)filesDatabase[objZ2].subData.ptr, objY2, objX2,(char *)gfxModuleData.pPage10,(char *)filesDatabase[objZ2].subData.ptr);
					}
				}
			}

			// automatic animation process
			if (currentObjPtr->animStep && !param) {
				if (currentObjPtr->animCounter <= 0) {

					bool change = true;

					int newVal = getValueFromObjectQuerry(&params, currentObjPtr->animChange) + currentObjPtr->animStep;

					if (currentObjPtr->animStep > 0) {
						if (newVal > currentObjPtr->animEnd) {
							if (currentObjPtr->animLoop) {
								newVal = currentObjPtr->animStart;
								if (currentObjPtr->animLoop>0)
									currentObjPtr->animLoop--;
							} else {
								int16 data2;
								data2 = currentObjPtr->animStart;

								change = false;
								currentObjPtr->animStep = 0;

								if (currentObjPtr->animType) {	// should we resume the script ?
									if (currentObjPtr->parentType == 20) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &procHead, -1, 0);
									} else if (currentObjPtr->parentType == 30) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &relHead, -1, 0);
									}
								}
							}
						}
					} else {
						if (newVal < currentObjPtr->animEnd) {
							if (currentObjPtr->animLoop) {
								newVal = currentObjPtr->animStart;
								if (currentObjPtr->animLoop>0)
									currentObjPtr->animLoop--;
							} else {
								int16 data2;
								data2 = currentObjPtr->animStart;

								change = false;
								currentObjPtr->animStep = 0;

								if (currentObjPtr->animType) {	// should we resume the script ?
									if (currentObjPtr->parentType == 20) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &procHead, -1, 0);
									} else if (currentObjPtr->parentType == 30) {
										changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &relHead, -1, 0);
									}
								}
							}
						}
					}

					if (currentObjPtr->animWait >= 0) {
						currentObjPtr->animCounter = currentObjPtr->animWait;
					}

					if ((currentObjPtr->animSignal >= 0) && (currentObjPtr->animSignal == newVal) && (currentObjPtr->animType != 0)) {
						if (currentObjPtr->parentType == 20) {
							changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &procHead, -1, 0);
						} else if (currentObjPtr->parentType == 30) {
							changeScriptParamInList(currentObjPtr->parentOverlay, currentObjPtr->parent, &relHead, -1, 0);
						}

						currentObjPtr->animType = 0;
					}

					if (change) {
						addAutoCell(currentObjPtr->overlay, currentObjPtr->idx, currentObjPtr->animChange, newVal, currentObjPtr);
					}
				} else {
					currentObjPtr->animCounter--;
				}
			}
		}

		currentObjPtr = currentObjPtr->next;
	}

	//----------------------------------------------------------------------------------------------------------------//

	freeAutoCell();
	var20 = 0;

	//-------------------------------------------------- DRAW OBJECTS TYPE 5 (MSG)-----------------------------------------//

	currentObjPtr = cellHead.next;

	while (currentObjPtr) {
		if (currentObjPtr->type == 5 && currentObjPtr->freeze == 0) {
			mainSprite(currentObjPtr->x, currentObjPtr->field_C, currentObjPtr->gfxPtr, gfxModuleData.pPage10, currentObjPtr->color, currentObjPtr->spriteIdx);
			var20 = 1;
		}
		currentObjPtr = currentObjPtr->next;
	}

	//----------------------------------------------------------------------------------------------------------------//

	if (currentActiveMenu != -1) {
		if (menuTable[currentActiveMenu]) {
			drawMenu(menuTable[currentActiveMenu]);
			return;
		}
	}
	else
	if ((linkedRelation) && (linkedMsgList))
	{
		ASSERT(0);
		// TODO: draw mouse here
	}
}

} // End of namespace Cruise
