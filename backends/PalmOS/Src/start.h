/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 
#ifndef __START_H__
#define __START_H__

typedef struct {
	Char nameP[32];
	UInt16 cardNo;
	LocalID dbID;
} SkinInfoType, *SkinInfoPtr;

typedef	struct {

	//skin params
	SkinInfoType skin;	//	card where is located the skin
	Boolean soundClick;	
	//
	Boolean vibrator;
	Boolean autoOff;
	Boolean setStack;
	Boolean exitLauncher;

	UInt16 listPosition;

	struct {
		UInt16 volRefNum;
		Boolean moveDB;
		Boolean deleteDB;
		Boolean confirmMoveDB;
	} card;

	Boolean debug;
	UInt16 debugLevel;
	Boolean stdPalette;
	Boolean demoMode;
	Boolean copyProtection;
	Boolean arm;
	Boolean altIntro;
	
	struct {
		Boolean enable;
		UInt8 mode;
	} lightspeed;

} GlobalsPreferenceType, *GlobalsPreferencePtr;

extern GlobalsPreferencePtr gPrefs;

extern Boolean bDirectMode;
extern Boolean bStartScumm;
extern Boolean bLaunched;

#define appPrefID				0x00
#define appVersionNum			0x01
#define appPrefVersionNum		0x01

#define STACK_DEFAULT			8192
#define STACK_LARGER			16384
#define STACK_GET				0

Err AppStart(void);
void AppStop(void);
Boolean StartScummVM();
void PINGetScreenDimensions();
void GetMemory(UInt32* storageMemoryP, UInt32* dynamicMemoryP, UInt32 *storageFreeP, UInt32 *dynamicFreeP);
void WinScreenGetPitch();
void SavePrefs();
Err SendDatabase (UInt16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP);
#endif
