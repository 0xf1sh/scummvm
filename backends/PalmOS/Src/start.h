/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

#define appFileCreator 'ScVM'
#define curItemVersion sysMakeROMVersion(2,5,0,0,0)

#define itemVersion_2 sysMakeROMVersion(2,0,0,0,0)

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
	Boolean autoReset;
	Boolean demoMode;
	Boolean fullscreen;
	Boolean aspectRatio;
	Boolean copyProtection;
	Boolean arm;

	struct {
		UInt16 master;
		UInt16 music;
		UInt16 sfx;
		UInt16 speech;
	} volume;
	
	struct {
		// midi
		Boolean multiMidi;
		Boolean music;
		UInt8 drvMusic;
		UInt8 tempo;
		// sound FX
		Boolean sfx;
		// CD audio
		Boolean CD;
		UInt8 drvCD;
		Boolean setDefaultTrackLength;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
	} sound;

} GlobalsPreferenceType, *GlobalsPreferencePtr;

extern GlobalsPreferencePtr gPrefs;

extern Boolean bDirectMode;
extern Boolean bStartScumm;

#define appPrefID				0x00
#define appVersionNum			0x01
#define appPrefVersionNum		0x01

#define STACK_DEFAULT			8192
#define STACK_LARGER			16384
#define STACK_GET				0

Err AppStart(void);
void AppStop(void);
Boolean CheckVibratorExists();
Boolean StartScummVM();
void PINGetScreenDimensions();
void GetMemory(UInt32* storageMemoryP, UInt32* dynamicMemoryP, UInt32 *storageFreeP, UInt32 *dynamicFreeP);
void WinScreenGetPitch();
void SavePrefs();
Err SendDatabase (UInt16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP);
#endif
