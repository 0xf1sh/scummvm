/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef SKYTEXT_H
#define SKYTEXT_H

#include "sky/disk.h"

struct HuffTree {
	unsigned char lChild;
	unsigned char rChild;
	char value;
};

class SkyText {
public:
	SkyText(SkyDisk *skyDisk, uint32 gameVersion, uint16 _language);
	~SkyText(void);
	void getText(uint32 textNr);
	struct displayText_t displayText(uint8 *dest, bool centre, uint16 pixelWidth, uint8 color);
	struct displayText_t displayText(char *textPtr, uint8 *dest, bool centre, uint16 pixelWidth, uint8 color);
	void makeGameCharacter(char textChar, uint8 *charSetPtr, uint8 *&data, uint8 color);
	struct lowTextManager_t lowTextManager(uint32 textNum, uint16 width, uint16 logicNum, uint8 color, bool centre);

protected:
	bool getTBit();
	void fnSetFont(uint32 fontNr);
	void initHuffTree();
	char getTextChar();

	SkyDisk *_skyDisk;
	uint16 	_language;
	uint32	_gameVersion;
	uint8	_inputValue;
	uint8	_shiftBits;
	uint8	*_textItemPtr;

	const HuffTree *_huffTree;

	struct charSet {
		uint8 *addr;
		uint32 charHeight;
		uint32 charSpacing;
	} _mainCharacterSet, _linkCharacterSet, _controlCharacterSet;	
	
	uint32	_curCharSet;
	uint8	*_characterSet;
	uint8	_charHeight;
	uint8	*_preAfterTableArea;

	char _textBuffer[1024];
	uint8 _centreTable[40];
	
	uint8	*_mouseTextData;	//space for the mouse text
	uint8	_dtCol;
	uint16	_dtLineWidth;	//width of line in pixels
	uint32	_dtLines;	//no of lines to do
	uint32	_dtLineSize;	//size of one line in bytes
	uint8	*_dtData;	//address of textdata
	uint32	_dtLetters;	//no of chars in message
	char	*_dtText;	//pointer to text
	uint32	_dtCharSpacing;	//character seperation adjustment
	uint32	_dtWidth;	//width of chars in last line (for editing (?))
	uint32	_dtLastWidth;
	bool	_dtCentre;	//set for centre text
	uint32	_lowTextWidth;
};

#endif
