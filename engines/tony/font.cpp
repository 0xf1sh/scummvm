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
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/font.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       RMFont Methods
\****************************************************************************/

RMFont::RMFont() {
	_letter = NULL;
	_nLetters = _fontDimx = _fontDimy = _dimx = _dimy = 0;
}

RMFont::~RMFont() {
	unload();
}


/**
 * Dumps a font to a buffer
 * @param buf                   Buffer for font contents
 * @param nChars                Number of characters (max 256)
 * @param dimx                  X dimension in pixels
 * @param dimy                  Y dimension in pixels
*
\****************************************************************************/

void DumpFontBMP(const char *filename, const byte *buf, int nChars, int charX, int charY, byte *pal) {
	error("DumpFontBMP not supported in ScummVM");
}


void RMFont::load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID) {
	_letter = new RMGfxSourceBuffer8RLEByte[nChars];

	// Initialize the fonts
	for (int i = 0; i < nChars; i++) {
		// Initialize the buffer with the letters
		_letter[i].init(buf + i * (dimx * dimy + 8) + 8, dimx, dimy);
		_letter[i].loadPaletteWA(palResID);
	}

	_fontDimx = dimx;
	_fontDimy = dimy;

	_nLetters = nChars;
}

void RMFont::load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID) {
	RMRes res(resID);

	if ((int)res.size() < nChars * (dimy * dimx + 8))
		nChars = res.size() / (dimy * dimx + 8);

	load(res, nChars, dimx, dimy, palResID);
}

void RMFont::unload() {
	if (_letter != NULL) {
		delete[] _letter;
		_letter = NULL;
	}
}


RMGfxPrimitive *RMFont::makeLetterPrimitive(byte bChar, int &nLength) {
	RMFontPrimitive *prim;
	int nLett;

	// Convert from character to glyph index
	nLett = convertToLetter(bChar);
	assert(nLett < _nLetters);

	// Create primitive font
	prim = new RMFontPrimitive(this);
	prim->_nChar = nLett;

	// Get the length of the character in pixels
	nLength = letterLength(bChar);

	return prim;
}

void RMFont::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim2) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	RMFontPrimitive *prim = (RMFontPrimitive *)prim2;

	CORO_BEGIN_CODE(_ctx);

	// Call the draw method of the letter assigned to the primitive
	if (prim->_nChar != -1)
		CORO_INVOKE_2(_letter[prim->_nChar].draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMFont::close() {
	unload();
}

int RMFont::stringLen(const RMString &text) {
	int len, i;

	len = 0;
	for (i = 0; i < text.length() - 1; i++)
		len += letterLength(text[i], text[i + 1]);
	len += letterLength(text[i]);

	return len;
}

int RMFont::stringLen(char bChar, char bNext) {
	return letterLength(bChar, bNext);
}

/****************************************************************************\
*       Metodi di RMFontColor
\****************************************************************************/

RMFontColor::RMFontColor() : RMFont() {
	_fontR = _fontG = _fontB = 255;
}

RMFontColor::~RMFontColor() {
}

void RMFontColor::setBaseColor(byte r1, byte g1, byte b1) {
	int r = (int)r1 << 16;
	int g = (int)g1 << 16;
	int b = (int)b1 << 16;

	int rstep = r / 14;
	int gstep = g / 14;
	int bstep = b / 14;

	int i;
	byte pal[768 * 3];

	// Check if we are already on the right color
	if (_fontR == r1 && _fontG == g1 && _fontB == b1)
		return;

	_fontR = r1;
	_fontG = g1;
	_fontB = b1;

	// Constructs a new paletter for the font
	for (i = 1; i < 16; i++) {
		pal[i * 3 + 0] = r >> 16;
		pal[i * 3 + 1] = g >> 16;
		pal[i * 3 + 2] = b >> 16;

		r -= rstep;
		g -= gstep;
		b -= bstep;
	}

	pal[15 * 3 + 0] += 8;
	pal[15 * 3 + 1] += 8;
	pal[15 * 3 + 2] += 8;

	// Puts in all the letters
	for (i = 0; i < _nLetters; i++)
		_letter[i].loadPaletteWA(pal);
}

/***************************************************************************\
*       RMFontDialog Methods
\****************************************************************************/

void RMFontDialog::init() {
	int i;

	// bernie: Number of characters in the font
	int nchars =
	    112    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // french
	    +  5;   // deutsch

	load(RES_F_PARL, nchars, 20, 20);

	// Initialize the f**king table
	_lDefault = 13;
	_hDefault = 18;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');
	for (i = 0; i < 256; i++) {
		_cTable[i] = -1;
		_lTable[i] = _lDefault;
	}

	for (i = 0; i < 26; i++)
		_cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		_cTable['a' + i] = i + 26;

	for (i = 0; i < 10; i++)
		_cTable['0' + i] = i + 52;

	_cTable[';'] = 62;
	_cTable[','] = 63;
	_cTable['.'] = 64;
	_cTable[':'] = 65;
	_cTable['-'] = 66;
	_cTable['_'] = 67;
	_cTable['+'] = 68;
	_cTable['<'] = 69;
	_cTable['>'] = 70;
	_cTable['!'] = 71;
	//_cTable['!'] = 72;  Exclamation countdown
	_cTable['?'] = 73;
	//_cTable['?'] = 74;  Question down
	_cTable['('] = 75;
	_cTable[')'] = 76;
	_cTable['\"'] = 77;
	_cTable['^'] = 77;
	_cTable['/'] = 78;
	_cTable[(byte)'�'] = 79;
	_cTable['$'] = 80;
	_cTable['%'] = 81;
	_cTable['&'] = 82;
	_cTable['='] = 83;
	_cTable[(byte)'�'] = 84;
	_cTable[(byte)'�'] = 85;
	_cTable[(byte)'�'] = 86;
	_cTable[(byte)'�'] = 87;
	_cTable[(byte)'�'] = 88;
	_cTable[(byte)'�'] = 89;
	_cTable[(byte)'�'] = 89;
	_cTable[(byte)'�'] = 90;
	_cTable[(byte)'�'] = 91;
	_cTable[(byte)'�'] = 92;
	_cTable[(byte)'�'] = 93;
	_cTable[(byte)'�'] = 94;
	_cTable[(byte)'�'] = 95;
	_cTable[(byte)'�'] = 96;
	_cTable[(byte)'�'] = 97;
	_cTable[(byte)'�'] = 98;
	_cTable[(byte)'�'] = 99;
	//_cTable[' '] = 100;  e circlet
	//_cTable[' '] = 101;  i circlet
	//_cTable[' '] = 102;  o circlet
	//_cTable[' '] = 103;  u circlet
	_cTable[(byte)'�'] = 104;
	_cTable[(byte)'�'] = 105;
	_cTable[(byte)'�'] = 106;
	_cTable[(byte)'�'] = 107;
	_cTable[(byte)'�'] = 108;
	_cTable[(byte)'�'] = 109;
	//_cTable['�'] = 110;  integral
	_cTable['\''] = 111;

	// Little lengths
	_lTable[' '] = 9;
	_lTable['\''] = 5;
	_lTable['.'] = 5;
	_lTable[','] = 5;
	_lTable[':'] = 5;
	_lTable[';'] = 5;
	_lTable['!'] = 5;
	_lTable['?'] = 10;
	_lTable['\"'] = 5;
	_lTable['^'] = 5;
	_lTable['('] = 7;
	_lTable[')'] = 7;

	_lTable['4'] = 10;

	_lTable['a'] = 14;
	_lTable['b'] = 15;
	_lTable['c'] = 12;
	_lTable['e'] = 12;
	_lTable['i'] = 6;
	_lTable['�'] = 6;
	_lTable['l'] = 5;
	_lTable['m'] = 16;
	_lTable['n'] = 12;
	_lTable['o'] = 11;
	_lTable['p'] = 11;
	_lTable['s'] = 12;
	_lTable['u'] = 12;

	_lTable['E'] = 10;
	_lTable['F'] = 11;

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//AaCcEeLlNnOoSsZzZz
		//�����ꣳ���󌜯���

		_cTable[(byte)'�'] = 112;
		_cTable[(byte)'�'] = 113;
		_cTable[(byte)'�'] = 114;
		_cTable[(byte)'�'] = 115;
		_cTable[(byte)'�'] = 116;
		_cTable[(byte)'�'] = 117;
		_cTable[(byte)'�'] = 118;
		_cTable[(byte)'�'] = 119;
		_cTable[(byte)'�'] = 120;
		_cTable[(byte)'�'] = 121;
		_cTable[(byte)'�'] = 122;
		_cTable[(byte)'�'] = 123;
		_cTable[(byte)'�'] = 124;
		_cTable[(byte)'�'] = 125;
		_cTable[(byte)'�'] = 126;
		_cTable[(byte)'�'] = 127;
		_cTable[(byte)'�'] = 128;
		_cTable[(byte)'�'] = 129;

		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 13;

	} else if (_vm->getLanguage() == Common::RU_RUS) {

		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis

		_cTable[(byte)'�'] = 130;
		_cTable[(byte)'�'] = 131;
		_cTable[(byte)'�'] = 132;
		_cTable[(byte)'�'] = 133;
		_cTable[(byte)'�'] = 134;
		_cTable[(byte)'�'] = 135;
		_cTable[(byte)'�'] = 136;
		_cTable[(byte)'�'] = 137;
		_cTable[(byte)'�'] = 138;
		_cTable[(byte)'�'] = 139;
		_cTable[(byte)'�'] = 140;
		_cTable[(byte)'�'] = 141;
		_cTable[(byte)'�'] = 142;
		_cTable[(byte)'�'] = 143;
		_cTable[(byte)'�'] = 144;
		_cTable[(byte)'�'] = 145;
		_cTable[(byte)'�'] = 146;
		_cTable[(byte)'�'] = 147;
		_cTable[(byte)'�'] = 148;
		_cTable[(byte)'�'] = 149;
		_cTable[(byte)'�'] = 150;
		_cTable[(byte)'�'] = 151;
		_cTable[(byte)'�'] = 152;
		_cTable[(byte)'�'] = 153;
		_cTable[(byte)'�'] = 154;
		_cTable[(byte)'�'] = 155;
		_cTable[(byte)'�'] = 156;
		_cTable[(byte)'�'] = 157;
		_cTable[(byte)'�'] = 158;
		_cTable[(byte)'�'] = 159;
		_cTable[(byte)'�'] = 160;
		_cTable[(byte)'�'] = 161;
		_cTable[(byte)'�'] = 162;

		_cTable[(byte)'�'] = 163;
		_cTable[(byte)'�'] = 164;
		_cTable[(byte)'�'] = 165;
		_cTable[(byte)'�'] = 166;
		_cTable[(byte)'�'] = 167;
		_cTable[(byte)'�'] = 168;
		_cTable[(byte)'�'] = 169;
		_cTable[(byte)'�'] = 170;
		_cTable[(byte)'�'] = 171;
		_cTable[(byte)'�'] = 172;
		_cTable[(byte)'�'] = 173;
		_cTable[(byte)'�'] = 174;
		_cTable[(byte)'�'] = 175;
		_cTable[(byte)'�'] = 176;
		_cTable[(byte)'�'] = 177;
		_cTable[(byte)'�'] = 178;
		_cTable[(byte)'�'] = 179;
		_cTable[(byte)'�'] = 180;
		_cTable[(byte)'�'] = 181;
		_cTable[(byte)'�'] = 182;
		_cTable[(byte)'�'] = 183;
		_cTable[(byte)'�'] = 184;
		_cTable[(byte)'�'] = 185;
		_cTable[(byte)'�'] = 186;
		_cTable[(byte)'�'] = 187;
		_cTable[(byte)'�'] = 188;
		_cTable[(byte)'�'] = 189;
		_cTable[(byte)'�'] = 190;
		_cTable[(byte)'�'] = 191;
		_cTable[(byte)'�'] = 192;
		_cTable[(byte)'�'] = 193;
		_cTable[(byte)'�'] = 194;
		_cTable[(byte)'�'] = 195;

		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 17;
		_lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 14;

		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] =  8;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] =  9;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 14;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// Czech
		_cTable[(byte)'�'] = 196;
		_cTable[(byte)'�'] = 197;
		_cTable[(byte)'�'] = 198;
		_cTable[(byte)'�'] = 199;
		_cTable[(byte)'�'] = 200;
		_cTable[(byte)'�'] = 201;
		_cTable[(byte)'�'] = 202;
		_cTable[(byte)'�'] = 203;
		_cTable[(byte)'�'] = 204;
		_cTable[(byte)'�'] = 205;
		_cTable[(byte)'�'] = 206;
		_cTable[(byte)'�'] = 207;
		_cTable[(byte)'�'] = 208;
		_cTable[(byte)'�'] = 209;
		_cTable[(byte)'�'] = 210;

		_cTable[(byte)'�'] = 211;
		_cTable[(byte)'�'] = 212;
		_cTable[(byte)'�'] = 213;
		_cTable[(byte)'�'] = 214;
		_cTable[(byte)'�'] = 215;
		_cTable[(byte)'�'] = 216;
		_cTable[(byte)'�'] = 217;
		_cTable[(byte)'�'] = 218;
		_cTable[(byte)'�'] = 219;
		_cTable[(byte)'�'] = 220;
		_cTable[(byte)'�'] = 221;
		_cTable[(byte)'�'] = 222;
		_cTable[(byte)'�'] = 223;
		_cTable[(byte)'�'] = 224;
		_cTable[(byte)'�'] = 225;

		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;

		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 7;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 17;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		_cTable[(byte)'�'] = 226;
		_cTable[(byte)'�'] = 227;
		_cTable[(byte)'�'] = 228;
		_cTable[(byte)'�'] = 229;
		_cTable[(byte)'�'] = 230;
		_cTable[(byte)'�'] = 231;
		_cTable[(byte)'�'] = 232;
		_cTable[(byte)'�'] = 233;

		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] =  9;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		_cTable[(byte)'�'] = 234;
		// 'SS' = 235
		_cTable[(byte)'�'] = 236;
		_cTable[(byte)'�'] = 237;
		_cTable[(byte)'�'] = 238;

		_lTable[(byte)'�'] = 15;
	}
}


/***************************************************************************\
*       RMFontMacc Methods
\****************************************************************************/

void RMFontMacc::init() {
	int i;

	// bernie: Number of characters in the font
	int nchars =
	    102    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // francais
	    +  5;   // deutsch


	load(RES_F_MACC, nchars, 11, 16);

	// Default
	_lDefault = 10;
	_hDefault = 17;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		_cTable[i] = -1;
		_lTable[i] = _lDefault;
	}

	for (i = 0; i < 26; i++)
		_cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		_cTable['a' + i] = i + 26;

	for (i = 0; i < 10; i++)
		_cTable['0' + i] = i + 52;

	_cTable['!'] = 62;
	//_cTable['!'] = 63;         // ! rovescia
	_cTable['\"'] = 64;
	_cTable['$'] = 65;
	_cTable['%'] = 66;
	_cTable['&'] = 67;
	_cTable['/'] = 68;
	_cTable['('] = 69;
	_cTable[')'] = 70;
	_cTable['='] = 71;
	_cTable['?'] = 72;
	//_cTable['?'] = 73;        // ? rovescia
	_cTable['*'] = 74;
	_cTable['+'] = 75;
	_cTable[(byte)'�'] = 76;
	_cTable[';'] = 77;
	_cTable[','] = 78;
	_cTable['.'] = 79;
	_cTable[':'] = 80;
	_cTable['-'] = 81;
	_cTable['<'] = 82;
	_cTable['>'] = 83;
	_cTable['/'] = 84;
	_cTable[(byte)'�'] = 85;
	_cTable[(byte)'�'] = 86;
	_cTable[(byte)'�'] = 87;
	_cTable[(byte)'�'] = 88;
	_cTable[(byte)'�'] = 89;
	_cTable[(byte)'�'] = 90;
	//_cTable[(byte)''] = 91;          // e with ball
	_cTable[(byte)'�'] = 92;
	_cTable[(byte)'�'] = 93;
	//_cTable[(byte)''] = 94;            // i with ball
	_cTable[(byte)'�'] = 95;
	_cTable[(byte)'�'] = 96;
	//_cTable[(byte)''] = 97;          // o with ball
	_cTable[(byte)'�'] = 98;
	_cTable[(byte)'�'] = 99;
	//_cTable[(byte)''] = 100;         // u with ball
	_cTable[(byte)'�'] = 101;

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//AaCcEeLlNnOoSsZzZz
		//�����ꣳ���󌜯���

		_cTable[(byte)'�'] = 102;
		_cTable[(byte)'�'] = 103;
		_cTable[(byte)'�'] = 104;
		_cTable[(byte)'�'] = 105;
		_cTable[(byte)'�'] = 106;
		_cTable[(byte)'�'] = 107;
		_cTable[(byte)'�'] = 108;
		_cTable[(byte)'�'] = 109;
		_cTable[(byte)'�'] = 110;
		_cTable[(byte)'�'] = 111;
		_cTable[(byte)'�'] = 112;
		_cTable[(byte)'�'] = 113;
		_cTable[(byte)'�'] = 114;
		_cTable[(byte)'�'] = 115;
		_cTable[(byte)'�'] = 116;
		_cTable[(byte)'�'] = 117;
		_cTable[(byte)'�'] = 118;
		_cTable[(byte)'�'] = 119;

		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 13;

	} else if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis
		_cTable[(byte)'�'] = 120;
		_cTable[(byte)'�'] = 121;
		_cTable[(byte)'�'] = 122;
		_cTable[(byte)'�'] = 123;
		_cTable[(byte)'�'] = 124;
		_cTable[(byte)'�'] = 125;
		_cTable[(byte)'�'] = 126;
		_cTable[(byte)'�'] = 127;
		_cTable[(byte)'�'] = 128;
		_cTable[(byte)'�'] = 129;
		_cTable[(byte)'�'] = 130;
		_cTable[(byte)'�'] = 131;
		_cTable[(byte)'�'] = 132;
		_cTable[(byte)'�'] = 133;
		_cTable[(byte)'�'] = 134;
		_cTable[(byte)'�'] = 135;
		_cTable[(byte)'�'] = 136;
		_cTable[(byte)'�'] = 137;
		_cTable[(byte)'�'] = 138;
		_cTable[(byte)'�'] = 139;
		_cTable[(byte)'�'] = 140;
		_cTable[(byte)'�'] = 141;
		_cTable[(byte)'�'] = 142;
		_cTable[(byte)'�'] = 143;
		_cTable[(byte)'�'] = 144;
		_cTable[(byte)'�'] = 145;
		_cTable[(byte)'�'] = 146;
		_cTable[(byte)'�'] = 147;
		_cTable[(byte)'�'] = 148;
		_cTable[(byte)'�'] = 149;
		_cTable[(byte)'�'] = 150;
		_cTable[(byte)'�'] = 151;
		_cTable[(byte)'�'] = 152;

		_cTable[(byte)'�'] = 153;
		_cTable[(byte)'�'] = 154;
		_cTable[(byte)'�'] = 155;
		_cTable[(byte)'�'] = 156;
		_cTable[(byte)'�'] = 157;
		_cTable[(byte)'�'] = 158;
		_cTable[(byte)'�'] = 159;
		_cTable[(byte)'�'] = 160;
		_cTable[(byte)'�'] = 161;
		_cTable[(byte)'�'] = 162;
		_cTable[(byte)'�'] = 163;
		_cTable[(byte)'�'] = 164;
		_cTable[(byte)'�'] = 165;
		_cTable[(byte)'�'] = 166;
		_cTable[(byte)'�'] = 167;
		_cTable[(byte)'�'] = 168;
		_cTable[(byte)'�'] = 169;
		_cTable[(byte)'�'] = 170;
		_cTable[(byte)'�'] = 171;
		_cTable[(byte)'�'] = 172;
		_cTable[(byte)'�'] = 173;
		_cTable[(byte)'�'] = 174;
		_cTable[(byte)'�'] = 175;
		_cTable[(byte)'�'] = 176;
		_cTable[(byte)'�'] = 177;
		_cTable[(byte)'�'] = 178;
		_cTable[(byte)'�'] = 179;
		_cTable[(byte)'�'] = 180;
		_cTable[(byte)'�'] = 181;
		_cTable[(byte)'�'] = 182;
		_cTable[(byte)'�'] = 183;
		_cTable[(byte)'�'] = 184;
		_cTable[(byte)'�'] = 185;

		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 8;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;

		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] =  9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// Czech

		_cTable[(byte)'�'] = 186;
		_cTable[(byte)'�'] = 187;
		_cTable[(byte)'�'] = 188;
		_cTable[(byte)'�'] = 189;
		_cTable[(byte)'�'] = 190;
		_cTable[(byte)'�'] = 191;
		_cTable[(byte)'�'] = 192;
		_cTable[(byte)'�'] = 193;
		_cTable[(byte)'�'] = 194;
		_cTable[(byte)'�'] = 195;
		_cTable[(byte)'�'] = 196;
		_cTable[(byte)'�'] = 197;
		_cTable[(byte)'�'] = 198;
		_cTable[(byte)'�'] = 199;
		_cTable[(byte)'�'] = 200;

		_cTable[(byte)'�'] = 201;
		_cTable[(byte)'�'] = 202;
		_cTable[(byte)'�'] = 203;
		_cTable[(byte)'�'] = 204;
		_cTable[(byte)'�'] = 205;
		_cTable[(byte)'�'] = 206;
		_cTable[(byte)'�'] = 207;
		_cTable[(byte)'�'] = 208;
		_cTable[(byte)'�'] = 209;
		_cTable[(byte)'�'] = 210;
		_cTable[(byte)'�'] = 211;
		_cTable[(byte)'�'] = 212;
		_cTable[(byte)'�'] = 213;
		_cTable[(byte)'�'] = 214;
		_cTable[(byte)'�'] = 215;

		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;

		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 9;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		_cTable[(byte)'�'] = 226;
		_cTable[(byte)'�'] = 227;
		_cTable[(byte)'�'] = 228;
		_cTable[(byte)'�'] = 229;
		_cTable[(byte)'�'] = 230;
		_cTable[(byte)'�'] = 231;
		_cTable[(byte)'�'] = 232;
		_cTable[(byte)'�'] = 233;

		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 8;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		// German

		_cTable[(byte)'�'] = 234;
		// 'SS' = 235
		_cTable[(byte)'�'] = 236;
		_cTable[(byte)'�'] = 237;
		_cTable[(byte)'�'] = 238;

		_lTable[(byte)'�'] = 11;
	}
}

/***************************************************************************\
*       RMFontCredits Methods
\****************************************************************************/

void RMFontCredits::init() {
	int i;

	// bernie: Number of characters in the font
	int nchars =
	    112    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // french
	    +  2;   // deutsch


	load(RES_F_CREDITS, nchars, 27, 28, RES_F_CPAL);

	// Default
	_lDefault = 10;
	_hDefault = 28;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		_cTable[i] = -1;
		_lTable[i] = _lDefault;
	}

	for (i = 0; i < 26; i++)
		_cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		_cTable['a' + i] = i + 26;



	_cTable[(byte)'�'] = 52;
	_cTable[(byte)'�'] = 53;
//	_cTable[''] = 54; // a ^
//	_cTable[''] = 55; // a pallini
	_cTable[(byte)'�'] = 56;
	_cTable[(byte)'�'] = 57;
//	_cTable[''] = 58; // e ^
//	_cTable[''] = 59; // e pallini
	_cTable[(byte)'�'] = 60;
	_cTable[(byte)'�'] = 61;
//	_cTable[''] = 62; // i ^
//	_cTable[''] = 63; // i pallini
	_cTable[(byte)'�'] = 64;
	_cTable[(byte)'�'] = 65;
//	_cTable[''] = 66; // o ^
//	_cTable[''] = 67; // o pallini
	_cTable[(byte)'�'] = 68;
	_cTable[(byte)'�'] = 69;
//	_cTable[''] = 70; // u ^
//	_cTable[''] = 71; // u pallini
//	_cTable[''] = 72; // y pallini
	_cTable[(byte)'�'] = 73;
	_cTable[(byte)'�'] = 74;
//	_cTable[''] = 75; // o barrato
//	_cTable[''] = 76; // ac
	_cTable[(byte)'�'] = 77;
//	_cTable[''] = 78; // ? rovesciato
	_cTable['?'] = 79;
//	_cTable[''] = 80; // ! rovesciato
	_cTable['!'] = 81;
//	_cTable[''] = 82; // 1/2
//	_cTable[''] = 83; // 1/4
	_cTable['('] = 84;
	_cTable[')'] = 85;
	_cTable[(byte)'�'] = 86;
	_cTable[(byte)'�'] = 87;
//	_cTable[''] = 88; // AE
	_cTable[':'] = 89;
	_cTable['%'] = 90;
	_cTable['&'] = 91;
	_cTable['/'] = 92;
	_cTable['+'] = 93;
	_cTable[';'] = 94;
	_cTable[','] = 95;
	_cTable['^'] = 96;
	_cTable['='] = 97;
	_cTable['_'] = 98;
	_cTable['*'] = 99;
	_cTable['.'] = 100;

	for (i = 0; i < 10; i++)
		_cTable['0' + i] = i + 101;
	_cTable['\''] = 111;

	_lTable[' '] = 11;
	_lTable[(byte)'�'] = _lTable['A'] = 19;
	_lTable['B'] = 15;
	_lTable['C'] = 14;
	_lTable['D'] = 13;
	_lTable['E'] = 14;
	_lTable['F'] = 13;
	_lTable['G'] = 16;
	_lTable['H'] = 15;
	_lTable['I'] = 5;
	_lTable['J'] = 8;
	_lTable['K'] = 15;
	_lTable['L'] = 13;
	_lTable['M'] = 17;
	_lTable['N'] = 15;
	_lTable['�'] = _lTable['O'] = 14;
	_lTable['P'] = 12;
	_lTable['Q'] = 14;
	_lTable['R'] = 14;
	_lTable['S'] = 15;
	_lTable['T'] = 11;
	_lTable['�'] = _lTable['U'] = 12;
	_lTable['V'] = 12;
	_lTable['W'] = 16;
	_lTable['X'] = 12;
	_lTable['Y'] = 13;
	_lTable['Z'] = 14;

	_lTable['a'] = 11;
	_lTable['b'] = 9;
	_lTable['c'] = 9;
	_lTable['d'] = 10;
	_lTable['e'] = 9;
	_lTable['f'] = 8;
	_lTable['g'] = 9;
	_lTable['h'] = 10;
	_lTable['i'] = 5;
	_lTable['j'] = 6;
	_lTable['k'] = 12;
	_lTable['l'] = 6;
	_lTable['m'] = 14;
	_lTable['n'] = 10;
	_lTable['o'] = 11;
	_lTable['p'] = 11;
	_lTable['q'] = 9;
	_lTable['r'] = 9;
	_lTable['s'] = 9;
	_lTable['t'] = 6;
	_lTable['u'] = 9;
	_lTable['v'] = 10;
	_lTable['w'] = 14;
	_lTable['x'] = 9;
	_lTable['y'] = 10;
	_lTable['z'] = 9;

	_lTable['0'] = 12;
	_lTable['1'] = 8;
	_lTable['2'] = 10;
	_lTable['3'] = 11;
	_lTable['4'] = 12;
	_lTable['5'] = 11;
	_lTable['6'] = 12;
	_lTable['7'] = 10;
	_lTable['8'] = 11;
	_lTable['9'] = 10;

	_lTable['/'] = 10;
	_lTable['^'] = 9;
	_lTable[','] = 5;
	_lTable['.'] = 5;
	_lTable[';'] = 5;
	_lTable[':'] = 5;
	_lTable['\''] = 5;

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//AaCcEeLlNnOoSsZzZz
		//�����ꣳ���󌜯���

		_cTable[(byte)'�'] = 112;
		_cTable[(byte)'�'] = 113;
		_cTable[(byte)'�'] = 114;
		_cTable[(byte)'�'] = 115;
		_cTable[(byte)'�'] = 116;
		_cTable[(byte)'�'] = 117;
		_cTable[(byte)'�'] = 118;
		_cTable[(byte)'�'] = 119;
		_cTable[(byte)'�'] = 120;
		_cTable[(byte)'�'] = 121;
		_cTable[(byte)'�'] = 122;
		_cTable[(byte)'�'] = 123;
		_cTable[(byte)'�'] = 124;
		_cTable[(byte)'�'] = 125;
		_cTable[(byte)'�'] = 126;
		_cTable[(byte)'�'] = 127;
		_cTable[(byte)'�'] = 128;
		_cTable[(byte)'�'] = 129;

		_lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;

	} else if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis
		_cTable[(byte)'�'] = 130;
		_cTable[(byte)'�'] = 131;
		_cTable[(byte)'�'] = 132;
		_cTable[(byte)'�'] = 133;
		_cTable[(byte)'�'] = 134;
		_cTable[(byte)'�'] = 135;
		_cTable[(byte)'�'] = 136;
		_cTable[(byte)'�'] = 137;
		_cTable[(byte)'�'] = 138;
		_cTable[(byte)'�'] = 139;
		_cTable[(byte)'�'] = 140;
		_cTable[(byte)'�'] = 141;
		_cTable[(byte)'�'] = 142;
		_cTable[(byte)'�'] = 143;
		_cTable[(byte)'�'] = 144;
		_cTable[(byte)'�'] = 145;
		_cTable[(byte)'�'] = 146;
		_cTable[(byte)'�'] = 147;
		_cTable[(byte)'�'] = 148;
		_cTable[(byte)'�'] = 149;
		_cTable[(byte)'�'] = 150;
		_cTable[(byte)'�'] = 151;
		_cTable[(byte)'�'] = 152;
		_cTable[(byte)'�'] = 153;
		_cTable[(byte)'�'] = 154;
		_cTable[(byte)'�'] = 155;
		_cTable[(byte)'�'] = 156;
		_cTable[(byte)'�'] = 157;
		_cTable[(byte)'�'] = 158;
		_cTable[(byte)'�'] = 159;
		_cTable[(byte)'�'] = 160;
		_cTable[(byte)'�'] = 161;
		_cTable[(byte)'�'] = 162;

		_cTable[(byte)'�'] = 163;
		_cTable[(byte)'�'] = 164;
		_cTable[(byte)'�'] = 165;
		_cTable[(byte)'�'] = 166;
		_cTable[(byte)'�'] = 167;
		_cTable[(byte)'�'] = 168;
		_cTable[(byte)'�'] = 169;
		_cTable[(byte)'�'] = 170;
		_cTable[(byte)'�'] = 171;
		_cTable[(byte)'�'] = 172;
		_cTable[(byte)'�'] = 173;
		_cTable[(byte)'�'] = 174;
		_cTable[(byte)'�'] = 175;
		_cTable[(byte)'�'] = 176;
		_cTable[(byte)'�'] = 177;
		_cTable[(byte)'�'] = 178;
		_cTable[(byte)'�'] = 179;
		_cTable[(byte)'�'] = 180;
		_cTable[(byte)'�'] = 181;
		_cTable[(byte)'�'] = 182;
		_cTable[(byte)'�'] = 183;
		_cTable[(byte)'�'] = 184;
		_cTable[(byte)'�'] = 185;
		_cTable[(byte)'�'] = 186;
		_cTable[(byte)'�'] = 187;
		_cTable[(byte)'�'] = 188;
		_cTable[(byte)'�'] = 189;
		_cTable[(byte)'�'] = 190;
		_cTable[(byte)'�'] = 191;
		_cTable[(byte)'�'] = 192;
		_cTable[(byte)'�'] = 193;
		_cTable[(byte)'�'] = 194;
		_cTable[(byte)'�'] = 195;

		_lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 23;
		_lTable[(byte)'�'] = 23;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = 15;

		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 11;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// CZECH Language

		_cTable[(byte)'�'] = 196;
		_cTable[(byte)'�'] = 197;
		_cTable[(byte)'�'] = 198;
		_cTable[(byte)'�'] = 199;
		_cTable[(byte)'�'] = 200;
		_cTable[(byte)'�'] = 201;
		_cTable[(byte)'�'] = 202;
		_cTable[(byte)'�'] = 203;
		_cTable[(byte)'�'] = 204;
		_cTable[(byte)'�'] = 205;
		_cTable[(byte)'�'] = 206;
		_cTable[(byte)'�'] = 207;
		_cTable[(byte)'�'] = 208;
		_cTable[(byte)'�'] = 209;
		_cTable[(byte)'�'] = 210;

		_cTable[(byte)'�'] = 211;
		_cTable[(byte)'�'] = 212;
		_cTable[(byte)'�'] = 213;
		_cTable[(byte)'�'] = 214;
		_cTable[(byte)'�'] = 215;
		_cTable[(byte)'�'] = 216;
		_cTable[(byte)'�'] = 217;
		_cTable[(byte)'�'] = 218;
		_cTable[(byte)'�'] = 219;
		_cTable[(byte)'�'] = 220;
		_cTable[(byte)'�'] = 221;
		_cTable[(byte)'�'] = 222;
		_cTable[(byte)'�'] = 223;
		_cTable[(byte)'�'] = 224;
		_cTable[(byte)'�'] = 225;

		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 14;
		_lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = 7;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 13;
		_lTable[(byte)'�'] = 13;

		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 6;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		_cTable[(byte)'�'] = 226;
		_cTable[(byte)'�'] = 227;
		_cTable[(byte)'�'] = 228;
		_cTable[(byte)'�'] = 229;
		_cTable[(byte)'�'] = 230;
		_cTable[(byte)'�'] = 231;
		_cTable[(byte)'�'] = 232;
		_cTable[(byte)'�'] = 233;

		_lTable[(byte)'�'] = 12;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 6;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 10;
		_lTable[(byte)'�'] = 11;
		_lTable[(byte)'�'] = 11;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		// German

		_cTable[(byte)'�'] = 234;
		// 'SS' = 235

		// old chars overrides
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 55;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 67;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 71;

		_lTable[(byte)'�'] = 11;
	}
}



/***************************************************************************\
*       Metodi di RMFontObj
\****************************************************************************/

#define TOUPPER(a)  ((a) >= 'a' && (a) <= 'z' ? (a) + 'A' - 'a' : (a))
#define TOLOWER(a)  ((a) >= 'A' && (a) <= 'Z' ? (a) + 'a' - 'A' : (a))

void RMFontObj::setBothCase(int nChar, int nNext, signed char spiazz) {
	_l2Table[TOUPPER(nChar)][TOUPPER(nNext)] = spiazz;
	_l2Table[TOUPPER(nChar)][TOLOWER(nNext)] = spiazz;
	_l2Table[TOLOWER(nChar)][TOUPPER(nNext)] = spiazz;
	_l2Table[TOLOWER(nChar)][TOLOWER(nNext)] = spiazz;
}


void RMFontObj::init() {
	int i;

	//bernie: Number of characters in the font (solo maiuscolo)
	int nchars =
	    85    // base
	    +  9    // polish
	    + 33    // russian
	    + 15    // czech
	    +  0    // francais (no uppercase chars)
	    +  1;   // deutsch


	load(RES_F_OBJ, nchars, 25, 30);

	// Initialize the f**king table
	_lDefault = 26;
	_hDefault = 30;
	Common::fill(&_l2Table[0][0], &_l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		_cTable[i] = -1;
		_lTable[i] = _lDefault;
	}

	for (i = 0; i < 26; i++) {
		_cTable['A' + i] = i + 0;
		_cTable['a' + i] = i + 0;
	}

	for (i = 0; i < 10; i++)
		_cTable['0' + i] = i + 26;

	_cTable[','] = 36;
	_cTable[';'] = 37;
	_cTable['.'] = 38;
	_cTable[':'] = 39;
	_cTable['-'] = 40;
	_cTable['+'] = 41;
	_cTable['!'] = 42;
	// _cTable['!'] = 43; Exclamation countdown
	_cTable['?'] = 44;
	// _cTable['?'] = 45;  Interrogativo alla rovescia
	_cTable['/'] = 46;
	_cTable['('] = 47;
	_cTable[')'] = 48;
	_cTable['='] = 49;
	_cTable['\''] = 50;
	_cTable['\"'] = 51;
	_cTable[(byte)'�'] = 52;
	_cTable[(byte)'$'] = 53;
	_cTable[(byte)'%'] = 54;
	_cTable[(byte)'&'] = 55;
	_cTable[(byte)'^'] = 56;
	_cTable[(byte)'*'] = 57;
	_cTable[(byte)'<'] = 58;
	_cTable[(byte)'>'] = 59;
	_cTable[(byte)'�'] = 60;
	_cTable[(byte)'�'] = 61;
	_cTable[(byte)'�'] = 62;
	_cTable[(byte)'�'] = 63;
	//_cTable[(byte)'�'] = 64;   integral
	_cTable[(byte)'�'] = 65;
	_cTable[(byte)'�'] = 66;
	_cTable[(byte)'�'] = 67;
	_cTable[(byte)'�'] = 68;
	_cTable[(byte)'�'] = 69;
	_cTable[(byte)'�'] = _cTable[(byte)'�'] = 70;
	_cTable[(byte)'�'] = 71;
	_cTable[(byte)'�'] = 72;
	_cTable[(byte)'�'] = 73;
	//_cTable[(byte)' '] = 74;   e circlet
	_cTable[(byte)'�'] = 75;
	_cTable[(byte)'�'] = 76;
	//_cTable[(byte)' '] = 77;     i circlet
	_cTable[(byte)'�'] = 78;
	_cTable[(byte)'�'] = _cTable[(byte)'�'] = 79;
	//_cTable[(byte)' '] = 80;       o circlet
	_cTable[(byte)'�'] = 81;
	_cTable[(byte)'�'] = _cTable[(byte)'�'] = 82;
	//_cTable[' '] = 83;     u circlet
	//_cTable[' '] = 84;   y dieresi

	/* Little lengths */
	_lTable[' '] = 11;
	_lTable['.'] = 8;
	_lTable['-'] = 12;
	_lTable['\''] = 8;
	_lTable['0'] = 20;
	_lTable['1'] = 20;
	_lTable['2'] = 15;
	_lTable['3'] = 20;
	_lTable['4'] = 20;
	_lTable['5'] = 20;
	_lTable['6'] = 20;
	_lTable['7'] = 20;
	_lTable['8'] = 20;
	_lTable['9'] = 20;


	_lTable['a'] = _lTable['A'] = _lTable['�'] = _lTable['�'] = 17;
	_lTable['b'] = _lTable['B'] = 17;
	_lTable['c'] = _lTable['C'] = 19;
	_lTable['d'] = _lTable['D'] = 17;
	_lTable['e'] = _lTable['E'] = 15;
	_lTable['f'] = _lTable['F'] = 17;
	_lTable['g'] = _lTable['G'] = 19;
	_lTable['i'] = _lTable['I'] = 16;
	_lTable['h'] = _lTable['H'] = 17;
	_lTable['k'] = _lTable['K'] = 17;
	_lTable['l'] = _lTable['L'] = 14;
	_lTable['m'] = _lTable['M'] = 19;
	_lTable['n'] = _lTable['N'] = 17;
	_lTable['o'] = _lTable['O'] = _lTable['�'] = _lTable['�'] = 19;
	_lTable['p'] = _lTable['P'] = 17;
	_lTable['q'] = _lTable['Q'] = 19;
	_lTable['r'] = _lTable['R'] = 14;
	_lTable['s'] = _lTable['S'] = 13;
	_lTable['t'] = _lTable['T'] = 15;
	_lTable['u'] = _lTable['U'] = _lTable['�'] = _lTable['�'] = 15;
	_lTable['v'] = _lTable['V'] = 13;
	_lTable['x'] = _lTable['X'] = 15;
	_lTable['y'] = _lTable['Y'] = 13;
	_lTable['w'] = _lTable['W'] = 19;
	_lTable['z'] = _lTable['Z'] = 20;
	_lTable[(byte)'�'] = 17;

	/* Casi particolari */
	setBothCase('C', 'C', 2);
	setBothCase('A', 'T', -2);
	setBothCase('R', 'S', 2);
	setBothCase('H', 'I', -2);
	setBothCase('T', 'S', 2);
	setBothCase('O', 'R', 2);
	setBothCase('O', 'L', 2);
	setBothCase('O', 'G', 2);
	setBothCase('Z', 'A', -1);
	setBothCase('R', 'R', 1);
	setBothCase('R', 'U', 3);

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//�����ꣳ���󌜯���
		//AaCcEeLlNnOoSsZzZz
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 85;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 20;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 86;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 87;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 88;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 89;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 90;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 91;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 92;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 21;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 93;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 21;

	} else if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 85;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 20;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 94;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 95;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 96;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 97;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 98;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 99;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 100;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 101;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 102;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 103;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 104;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 105;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 106;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 107;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 108;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 109;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 110;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 111;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 112;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 113;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 114;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 115;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 116;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 117;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 118;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 119;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 120;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 121;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 122;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 123;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 124;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 125;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 126;


		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 21;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 20;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// Czech

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 127;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 128;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 129;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 130;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 131;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 132;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 133;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 134;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 135;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 136;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 137;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 138;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 139;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 140;
		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 141;

		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 21;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 18;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 23;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 24;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 22;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		// Translate accented characters as normal letters

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = _cTable[(byte)'�'] = 0; // a
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = _lTable[(byte)'�'] = 17;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 4; // e
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = _cTable[(byte)'�'] = 8; // i
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = _lTable[(byte)'�'] = 16;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = _cTable[(byte)'�'] = _cTable[(byte)'�'] = 14; // o
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = _lTable[(byte)'�'] = _lTable[(byte)'�'] = 19;

		_cTable[(byte)'�'] = _cTable[(byte)'�'] = 20; // u
		_lTable[(byte)'�'] = _lTable[(byte)'�'] = 15;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		// German

		_cTable['�'] = 142;
		// SS = 143

		_lTable['�'] = 24;
	}
}


/****************************************************************************\
*       RMText Methods
\****************************************************************************/

RMFontColor *RMText::_fonts[4] = { NULL, NULL, NULL, NULL };

void RMText::initStatics() {
	Common::fill(&_fonts[0], &_fonts[4], (RMFontColor *)NULL);
}

RMText::RMText() {
	// Default color: white
	_textR = _textG = _textB = 255;

	// Default length
	_maxLineLength = 350;

	_bTrasp0 = true;
	_aHorType = HCENTER;
	_aVerType = VTOP;
	setPriority(150);
}

RMText::~RMText() {

}

void RMText::unload() {
	if (_fonts[0] != NULL) {
		delete _fonts[0];
		delete _fonts[1];
		delete _fonts[2];
		delete _fonts[3];
		_fonts[0] =  _fonts[1] = _fonts[2] = _fonts[3] = 0;
	}
}

void RMText::setMaxLineLength(int max) {
	_maxLineLength = max;
}

void RMText::removeThis(CORO_PARAM, bool &result) {
	// Here we can do checks on the number of frames, time spent, etc.
	result = true;
}

void RMText::writeText(const RMString &text, int nFont, int *time) {
	// Initializes the font (only once)
	if (_fonts[0] == NULL) {
		_fonts[0] = new RMFontDialog;
		_fonts[0]->init();
		_fonts[1] = new RMFontObj;
		_fonts[1]->init();
		_fonts[2] = new RMFontMacc;
		_fonts[2]->init();
		_fonts[3] = new RMFontCredits;
		_fonts[3]->init();
	}

	writeText(text, _fonts[nFont], time);
}

void RMText::writeText(const RMString &text, RMFontColor *font, int *time) {
	RMGfxPrimitive *prim;
	char *p, *old_p;
	int i, j, x, y;
	int len;
	int numchar;
	int width, height;
	char *string;
	int numlines;

	// Set the base color
	font->setBaseColor(_textR, _textG, _textB);

	// Destroy the buffer before starting
	destroy();

	// If the string is empty, do nothing
	if (text == NULL || text[0] == '\0')
		return;

	// Divide the words into lines. In this cycle, X contains the maximum length reached by a line,
	// and the number of lines
	string = p = text;
	i = j = x = 0;
	while (*p != '\0') {
		j += font->stringLen(*p);
		if (j > (((_aHorType == HLEFTPAR) && (i > 0)) ? _maxLineLength - 25 : _maxLineLength)) {
			j -= font->stringLen(*p, p[1]);
			if (j > x)
				x = j;

			// Back to the first usable space
			//
			// BERNIE: In the original, sentences containing words that exceed the
			// width of a line caused discontinuation of the whole sentence.
			// This workaround has the partial word broken up so it will still display
			//
			old_p = p;
			while (*p != ' ' && *p != '-' && p > string)
				p--;

			if (p == string)
				p = old_p;

			// Check if there are any blanks to end
			while (*p == ' ' && *p != '\0')
				p++;
			if (*p == '\0')
				break;
			p--;
			i++;
			*p = '\0';
			j = 0;
		}
		p++;
	}

	if (j > x)
		x = j;

	i++;
	numlines = i;

	x += 8;

	// Starting position for the surface: X1, Y
	width = x;
	height = (numlines - 1) * font->letterHeight() + font->_fontDimy;

	// Create the surface
	create(width, height);
	Common::fill(_buf, _buf + width * height * 2, 0);

	p = string;

	y = 0;
	numchar = 0;
	for (; i > 0; i--) {
		// Measure the length of the line
		x = 0;
		j = font->stringLen(RMString(p));

		switch (_aHorType) {
		case HLEFT:
			x = 0;
			break;

		case HLEFTPAR:
			if (i == numlines)
				x = 0;
			else
				x = 25;
			break;

		case HCENTER:
			x = width / 2 - j / 2;
			break;

		case HRIGHT:
			x = width - j - 1;
			break;
		}

		while (*p != '\0') {
			if (*p == ' ') {
				x += font->stringLen(*p);
				p++;
				continue;
			}

			prim = font->makeLetterPrimitive(*p, len);
			prim->getDst()._x1 = x;
			prim->getDst()._y1 = y;
			addPrim(prim);

			numchar++;

			x += font->stringLen(*p, p[1]);
			p++;
		}
		p++;
		y += font->letterHeight();
	}

	if (time != NULL)
		*time = 1000 + numchar * (11 - GLOBALS._nCfgTextSpeed) * 14;
}

void RMText::clipOnScreen(RMGfxPrimitive *prim) {
	// Don't let it go outside the screen
	if (prim->getDst()._x1 < 5)
		prim->getDst()._x1 = 5;
	if (prim->getDst()._y1 < 5)
		prim->getDst()._y1 = 5;
	if (prim->getDst()._x1 + _dimx > 635)
		prim->getDst()._x1 = 635 - _dimx;
	if (prim->getDst()._y1 + _dimy > 475)
		prim->getDst()._y1 = 475 - _dimy;
}

void RMText::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Horizontally
	if (_aHorType == HCENTER)
		prim->getDst().topLeft() -= RMPoint(_dimx / 2, 0);
	else if (_aHorType == HRIGHT)
		prim->getDst().topLeft() -= RMPoint(_dimx, 0);


	// Vertically
	if (_aVerType == VTOP) {

	} else if (_aVerType == VCENTER) {
		prim->getDst()._y1 -= _dimy / 2;

	} else if (_aVerType == VBOTTOM) {
		prim->getDst()._y1 -= _dimy;
	}

	clipOnScreen(prim);

	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);

	CORO_END_CODE;
}

/****************************************************************************\
*       RMTextDialog Methods
\****************************************************************************/

RMTextDialog::RMTextDialog() : RMText() {
	_time = _startTime = 0;
	_dst = RMPoint(0, 0);

	_bSkipStatus = true;
	_bShowed = true;
	_bForceTime = false;
	_bForceNoTime = false;
	_bAlwaysDisplay = false;
	_bNoTab = false;
	_hCustomSkip = CORO_INVALID_PID_VALUE;
	_hCustomSkip2 = CORO_INVALID_PID_VALUE;
	_input = NULL;

	// Create the event for displaying the end
	_hEndDisplay = CoroScheduler.createEvent(false, false);
}

RMTextDialog::~RMTextDialog() {
	CoroScheduler.closeEvent(_hEndDisplay);
}

void RMTextDialog::show() {
	_bShowed = true;
}

void RMTextDialog::hide(CORO_PARAM) {
	_bShowed = false;
}

void RMTextDialog::writeText(const RMString &text, int font, int *time) {
	RMText::writeText(text, font, &_time);

	if (time != NULL)
		*time = _time;
}

void RMTextDialog::writeText(const RMString &text, RMFontColor *font, int *time) {
	RMText::writeText(text, font, &_time);

	if (time != NULL)
		*time = _time;
}


void RMTextDialog::setSkipStatus(bool bEnabled) {
	_bSkipStatus = bEnabled;
}

void RMTextDialog::forceTime() {
	_bForceTime = true;
}

void RMTextDialog::forceNoTime() {
	_bForceNoTime = true;
}

void RMTextDialog::setNoTab() {
	_bNoTab = true;
}

void RMTextDialog::setForcedTime(uint32 dwTime) {
	_time = dwTime;
}

void RMTextDialog::setAlwaysDisplay() {
	_bAlwaysDisplay = true;
}

void RMTextDialog::removeThis(CORO_PARAM, bool &result) {
	CORO_BEGIN_CONTEXT;
	bool expired;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Presume successful result
	result = true;

	// Don't erase the background
	if (_bSkipStatus) {
		if (!(GLOBALS._bCfgDubbing && _hCustomSkip2 != CORO_INVALID_PID_VALUE)) {
			if (GLOBALS._bCfgTimerizedText) {
				if (!_bForceNoTime) {
					if (_vm->getTime() > (uint32)_time + _startTime)
						return;
				}
			}
		}

		if (!_bNoTab) {
			if (_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_TAB))
				return;
		}

		if (!_bNoTab) {
			if (_input) {
				if (_input->mouseLeftClicked() || _input->mouseRightClicked())
					return;
			}
		}
	}
	// Erase the background
	else if (!(GLOBALS._bCfgDubbing && _hCustomSkip2 != CORO_INVALID_PID_VALUE)) {
		if (!_bForceNoTime) {
			if (_vm->getTime() > (uint32)_time + _startTime)
				return;
		}
	}

	// If time is forced
	if (_bForceTime) {
		if (_vm->getTime() > (uint32)_time + _startTime)
			return;
	}

	if (_hCustomSkip != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _hCustomSkip, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	if (GLOBALS._bCfgDubbing && _hCustomSkip2 != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _hCustomSkip2, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	result = false;

	CORO_END_CODE;
}

void RMTextDialog::Unregister() {
	RMGfxTask::Unregister();
	assert(_nInList == 0);
	CoroScheduler.setEvent(_hEndDisplay);
}

void RMTextDialog::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_startTime == 0)
		_startTime = _vm->getTime();

	if (_bShowed) {
		if (GLOBALS._bShowSubtitles || _bAlwaysDisplay) {
			prim->getDst().topLeft() = _dst;
			CORO_INVOKE_2(RMText::draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMTextDialog::setCustomSkipHandle(uint32 hCustom) {
	_hCustomSkip = hCustom;
}

void RMTextDialog::setCustomSkipHandle2(uint32 hCustom) {
	_hCustomSkip2 = hCustom;
}

void RMTextDialog::waitForEndDisplay(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, _hEndDisplay, CORO_INFINITE);
}

void RMTextDialog::setInput(RMInput *input) {
	_input = input;
}

/****************************************************************************\
*       RMTextDialogScrolling Methods
\****************************************************************************/

RMTextDialogScrolling::RMTextDialogScrolling() {
	_curLoc = NULL;
}

RMTextDialogScrolling::RMTextDialogScrolling(RMLocation *loc) {
	_curLoc = loc;
	_startScroll = loc->scrollPosition();
}

RMTextDialogScrolling::~RMTextDialogScrolling() {
}

void RMTextDialogScrolling::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	RMPoint curDst;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curDst = _dst;

	if (_curLoc != NULL)
		_dst -= _curLoc->scrollPosition() - _startScroll;

	CORO_INVOKE_2(RMTextDialog::draw, bigBuf, prim);

	_dst = _ctx->curDst;

	CORO_END_CODE;
}

void RMTextDialogScrolling::clipOnScreen(RMGfxPrimitive *prim) {
	// We must not do anything!
}


/****************************************************************************\
*       RMTextItemName Methods
\****************************************************************************/

RMTextItemName::RMTextItemName() : RMText() {
	_item = NULL;
	setPriority(220);
}

RMTextItemName::~RMTextItemName() {

}

void RMTextItemName::doFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv) {
	CORO_BEGIN_CONTEXT;
	RMItem *lastItem;
	uint32 hThread;
	CORO_END_CONTEXT(_ctx);

	RMString itemName;

	CORO_BEGIN_CODE(_ctx);

	_ctx->lastItem = _item;

	// Adds to the list if there is need
	if (!_nInList)
		bigBuf.addPrim(new RMGfxPrimitive(this));

	// Update the scrolling co-ordinates
	_curscroll = loc.scrollPosition();

	// Check if we are on the inventory
	if (inv.itemInFocus(_mpos))
		_item = inv.whichItemIsIn(_mpos);
	else
		_item = loc.whichItemIsIn(_mpos);

	itemName = "";

	// If there an item, get its name
	if (_item != NULL)
		_item->getName(itemName);

	// Write it
	writeText(itemName, 1);

	// Handle the change If the selected item is different from the previous one
	if (_ctx->lastItem != _item) {
		if (_item == NULL)
			ptr.setSpecialPointer(RMPointer::PTR_NONE);
		else {
			_ctx->hThread = mpalQueryDoAction(20, _item->mpalCode(), 0);
			if (_ctx->hThread == CORO_INVALID_PID_VALUE)
				ptr.setSpecialPointer(RMPointer::PTR_NONE);
			else
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->hThread, CORO_INFINITE);
		}
	}

	CORO_END_CODE;
}


void RMTextItemName::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If there is no text, it's pointless to continue
	if (_buf == NULL)
		return;

	// Set the destination coordinates of the mouse
	prim->getDst().topLeft() = _mpos - RMPoint(0, 30);

	CORO_INVOKE_2(RMText::draw, bigBuf, prim);

	CORO_END_CODE;
}

RMPoint RMTextItemName::getHotspot() {
	if (_item == NULL)
		return _mpos + _curscroll;
	else
		return _item->hotspot();
}

RMItem *RMTextItemName::getSelectedItem() {
	return _item;
}

bool RMTextItemName::isItemSelected() {
	return _item != NULL;
}

bool RMTextItemName::isNormalItemSelected() {
	return _item != NULL && _itemName.length() > 0;
}


/****************************************************************************\
*       RMDialogChoice Methods
\****************************************************************************/

RMDialogChoice::RMDialogChoice() {
	RMResRaw dlg1(RES_I_DLGTEXT);
	RMResRaw dlg2(RES_I_DLGTEXTLINE);
	RMRes dlgpal(RES_I_DLGTEXTPAL);

	_dlgText.init(dlg1, dlg1.width(), dlg1.height());
	_dlgTextLine.init(dlg2, dlg2.width(), dlg2.height());

	_dlgText.loadPaletteWA(dlgpal);
	_dlgTextLine.loadPaletteWA(dlgpal);

	_hUnreg = CoroScheduler.createEvent(false, false);
	_bRemoveFromOT = false;
	
	_curAdded = 0;
	_bShow = false;
}

RMDialogChoice::~RMDialogChoice() {
	CoroScheduler.closeEvent(_hUnreg);
}

void RMDialogChoice::Unregister() {
	RMGfxWoodyBuffer::Unregister();
	assert(!_nInList);
	CoroScheduler.pulseEvent(_hUnreg);

	_bRemoveFromOT = false;
}

void RMDialogChoice::init() {
	_numChoices = 0;
	_drawedStrings = NULL;
	_ptDrawStrings = NULL;
	_curSelection = -1;

	create(640, 477);
	setPriority(140);
}


void RMDialogChoice::close() {
	if (_drawedStrings != NULL) {
		delete[] _drawedStrings;
		_drawedStrings = NULL;
	}

	if (_ptDrawStrings != NULL) {
		delete[] _ptDrawStrings;
		_ptDrawStrings = NULL;
	}

	destroy();
}

void RMDialogChoice::setNumChoices(int num) {
	int i;

	_numChoices = num;
	_curAdded = 0;

	// Allocate space for drawn strings
	_drawedStrings = new RMText[num];
	_ptDrawStrings = new RMPoint[num];

	// Initialization
	for (i = 0; i < _numChoices; i++) {
		_drawedStrings[i].setColor(0, 255, 0);
		_drawedStrings[i].setAlignType(RMText::HLEFTPAR, RMText::VTOP);
		_drawedStrings[i].setMaxLineLength(600);
		_drawedStrings[i].setPriority(10);
	}
}

void RMDialogChoice::addChoice(const RMString &string) {
	// Draw the string
	assert(_curAdded < _numChoices);
	_drawedStrings[_curAdded++].writeText(string, 0);
}

void RMDialogChoice::prepare(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int i;
	RMPoint ptPos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	addPrim(new RMGfxPrimitive(&_dlgText, RMPoint(0, 0)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155 + 83)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155 + 83 + 83)));
	addPrim(new RMGfxPrimitive(&_dlgTextLine, RMPoint(0, 155 + 83 + 83 + 83)));

	_ctx->ptPos.set(20, 90);

	for (_ctx->i = 0; _ctx->i < _numChoices; _ctx->i++) {
		addPrim(new RMGfxPrimitive(&_drawedStrings[_ctx->i], _ctx->ptPos));
		_ptDrawStrings[_ctx->i] = _ctx->ptPos;
		_ctx->ptPos.offset(0, _drawedStrings[_ctx->i].getDimy() + 15);
	}

	CORO_INVOKE_0(drawOT);
	clearOT();

	_ptDrawPos.set(0, 480 - _ctx->ptPos._y);

	CORO_END_CODE;
}

void RMDialogChoice::setSelected(CORO_PARAM, int pos) {
	CORO_BEGIN_CONTEXT;
	RMGfxBox box;
	RMRect rc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (pos == _curSelection)
		return;

	_ctx->box.setPriority(5);

	if (_curSelection != -1) {
		_ctx->box.setColor(0xCC, 0xCC, 0xFF);
		_ctx->rc.topLeft() = RMPoint(18, _ptDrawStrings[_curSelection]._y);
		_ctx->rc.bottomRight() = _ctx->rc.topLeft() + RMPoint(597, _drawedStrings[_curSelection].getDimy());
		addPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));

		addPrim(new RMGfxPrimitive(&_drawedStrings[_curSelection], _ptDrawStrings[_curSelection]));
		CORO_INVOKE_0(drawOT);
		clearOT();
	}

	if (pos != -1) {
		_ctx->box.setColor(100, 100, 100);
		_ctx->rc.topLeft() = RMPoint(18, _ptDrawStrings[pos]._y);
		_ctx->rc.bottomRight() = _ctx->rc.topLeft() + RMPoint(597, _drawedStrings[pos].getDimy());
		addPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));
		addPrim(new RMGfxPrimitive(&_drawedStrings[pos], _ptDrawStrings[pos]));
	}

	CORO_INVOKE_0(drawOT);
	clearOT();

	_curSelection = pos;

	CORO_END_CODE;
}

void RMDialogChoice::show(CORO_PARAM, RMGfxTargetBuffer *bigBuf) {
	CORO_BEGIN_CONTEXT;
	RMPoint destpt;
	int deltay;
	int starttime;
	int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(prepare);
	_bShow = false;

	if (!_nInList && bigBuf != NULL)
		bigBuf->addPrim(new RMGfxPrimitive(this));

	if (0) {
		_bShow = true;
	} else {
		_ctx->starttime = _vm->getTime();
		_ctx->deltay = 480 - _ptDrawPos._y;
		_ctx->destpt = _ptDrawPos;
		_ptDrawPos.set(0, 480);

		if (!_nInList && bigBuf != NULL)
			bigBuf->addPrim(new RMGfxPrimitive(this));
		_bShow = true;

		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_0(mainWaitFrame);
			mainFreeze();
			_ctx->elaps = _vm->getTime() - _ctx->starttime;
			_ptDrawPos._y = 480 - ((_ctx->deltay * 100) / 700 * _ctx->elaps) / 100;
			mainUnfreeze();
		}

		_ptDrawPos._y = _ctx->destpt._y;
	}

	CORO_END_CODE;
}

void RMDialogChoice::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bShow == false)
		return;

	prim->setDst(_ptDrawPos);
	CORO_INVOKE_2(RMGfxSourceBuffer16::draw, bigBuf, prim);

	CORO_END_CODE;
}


void RMDialogChoice::hide(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int deltay;
	int starttime;
	int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (1) {
		_ctx->starttime = _vm->getTime();

		_ctx->deltay = 480 - _ptDrawPos._y;
		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_0(mainWaitFrame);
			mainFreeze();
			_ctx->elaps = _vm->getTime() - _ctx->starttime;
			_ptDrawPos._y = 480 - ((_ctx->deltay * 100) / 700 * (700 - _ctx->elaps)) / 100;
			mainUnfreeze();
		}
	}

	_bShow = false;
	_bRemoveFromOT = true;
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _hUnreg, CORO_INFINITE);

	CORO_END_CODE;
}


void RMDialogChoice::removeThis(CORO_PARAM, bool &result) {
	result = _bRemoveFromOT;
}

void RMDialogChoice::doFrame(CORO_PARAM, RMPoint ptMousePos) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (ptMousePos._y > _ptDrawPos._y) {
		for (_ctx->i = 0; _ctx->i < _numChoices; _ctx->i++) {
			if ((ptMousePos._y >= _ptDrawPos._y + _ptDrawStrings[_ctx->i]._y) && (ptMousePos._y < _ptDrawPos._y + _ptDrawStrings[_ctx->i]._y + _drawedStrings[_ctx->i].getDimy())) {
				CORO_INVOKE_1(setSelected, _ctx->i);
				break;
			}
		}

		if (_ctx->i == _numChoices)
			CORO_INVOKE_1(setSelected, -1);
	}

	CORO_END_CODE;
}

int RMDialogChoice::getSelection() {
	return _curSelection;
}

} // End of namespace Tony
