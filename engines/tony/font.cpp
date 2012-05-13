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
/**************************************************************************
 *                                     ���������������������������������� *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ���������������������������������� *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Font.CPP.............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/mpal/stubs.h"
#include "tony/font.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       Metodi di RMFont
\****************************************************************************/

RMFont::RMFont() {
	m_letter = NULL;
}

RMFont::~RMFont() {
	Unload();
}

/****************************************************************************\
*
* Function:     void RMFont::Load(byte *buf, int nChars, int dimx, int dimy);
*
* Description:  Carica un font da buffer
*
* Input:        byte *buf              Buffer contenente il font
*               int nChars              Numero di caratteri (max 256)
*               int dimx,dimy           Dimensione in pixel di un carattere
*
\****************************************************************************/

void DumpFontBMP(const char *filename, const byte *buf, int nChars, int charX, int charY, byte *pal) {
	error("DumpFontBMP not supported in ScummVM");
}


void RMFont::Load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID) {
	m_letter = new RMGfxSourceBuffer8RLEByte[nChars];

#if 0
	if (nChars == 112 && palResID == RES_F_PAL) {
		// Font parla
		DumpFontBMP("font_parla.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	}
	else if (nChars == 102 && palResID == RES_F_PAL) {
		// Font macc
		DumpFontBMP("font_macc.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	} else if (nChars == 85 && palResID == RES_F_PAL) {
		// Font obj
		DumpFontBMP("font_obj.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	} else if (nChars == 112 && palResID == RES_F_CPAL) {
		// Font credits
		DumpFontBMP("font_credits.bmp", buf, nChars, dimx, dimy, RMRes(palResID));		
	}
#endif

	// Carichiamoce 'sto font
	for (int i = 0; i < nChars; i++) {
		// Inizializza il buffer con le lettere
		m_letter[i].Init(buf + i * (dimx * dimy + 8) + 8, dimx, dimy);
		m_letter[i].LoadPaletteWA(palResID);
	}

	m_fontDimx = dimx;
	m_fontDimy = dimy;

	nLetters=nChars;
}

void RMFont::Load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID) {
	RMRes res(resID);

	if ((int)res.Size() < nChars * (dimy * dimx + 8))
		nChars = res.Size() / (dimy * dimx + 8);

	Load(res, nChars, dimx, dimy, palResID);
}

void RMFont::Unload(void) {
	if (m_letter != NULL) {
		delete[] m_letter;
		m_letter = NULL;
	}
}


RMGfxPrimitive *RMFont::MakeLetterPrimitive(byte bChar, int &nLength) {
	RMFontPrimitive *prim;
	int nLett;

	// Converte da carattere a lettera
	nLett = ConvertToLetter(bChar);

	// Crea la primitiva per il font
	prim = new RMFontPrimitive(this);
	prim->m_nChar = nLett;

	// Si fa' dare la lunghezza della lettera in pixel
	nLength = LetterLength(bChar);

	return prim;
}

void RMFont::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim2) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	RMFontPrimitive *prim = (RMFontPrimitive *)prim2;

	CORO_BEGIN_CODE(_ctx);

	// Richiama la Draw della lettera assegnata alla primitiva
	if (prim->m_nChar != -1)
		CORO_INVOKE_2(m_letter[prim->m_nChar].Draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMFont::Close(void) {
	Unload();
}

int RMFont::StringLen(const RMString &text) {
	int len, i;

	len = 0;
	for (i = 0; i < text.Length() - 1; i++)
		len += LetterLength(text[i], text[i + 1]);
	len += LetterLength(text[i]);

	return len;
}

int RMFont::StringLen(char bChar, char bNext) {
	return LetterLength(bChar, bNext);
}

/****************************************************************************\
*       Metodi di RMFontColor
\****************************************************************************/

RMFontColor::RMFontColor() : RMFont() {
 m_r = m_g = m_b = 255;
}

RMFontColor::~RMFontColor() {

}

void RMFontColor::SetBaseColor(byte r1, byte g1, byte b1) {
	int r = (int)r1 << 16;
	int g = (int)g1 << 16;
	int b = (int)b1 << 16;

	int rstep = r / 14;
	int gstep = g / 14;
	int bstep = b / 14;

	int i;
	byte pal[768*3];

	// Controlla se siamo gia' sul colore giusto
	if (m_r == r1 && m_g == g1 && m_b == b1)
		return;

	m_r = r1;
	m_g = g1;
	m_b = b1;

	// Costruisce la nuova palette per il font
	for (i = 1; i < 16; i++) {
		pal[i * 3 + 0] = r >> 16;	
		pal[i * 3 + 1] = g >> 16;	
		pal[i * 3 + 2] = b >> 16;	

		r -= rstep;
		g -= gstep;
		b -= bstep;
	}

	pal[15*3 + 0] += 8;
	pal[15*3 + 1] += 8;
	pal[15*3 + 2] += 8;

	// La mette in tutte le lettere
	for (i = 0; i < nLetters; i++)
		m_letter[i].LoadPaletteWA(pal);
}


/***************************************************************************\
*       Metodi di RMFontParla
\****************************************************************************/

void RMFontParla::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 112	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// french
		+  5;	// deutsch

	Load(RES_F_PARL, nchars, 20, 20);

	// Inizializziamo le tabelline del cazzo
	lDefault = 13;
	hDefault = 18;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 52;

	cTable[';'] = 62;
	cTable[','] = 63;
	cTable['.'] = 64;
	cTable[':'] = 65;
	cTable['-'] = 66;
	cTable['_'] = 67;
	cTable['+'] = 68;
	cTable['<'] = 69;
	cTable['>'] = 70;
	cTable['!'] = 71;
	//cTable['!'] = 72;  Esclamativo alla rovescia
	cTable['?'] = 73;
	//cTable['?'] = 74;  Interrogativo alla rovescia
	cTable['('] = 75;
	cTable[')'] = 76;
	cTable['\"'] = 77;
	cTable['^'] = 77;
	cTable['/'] = 78;
	cTable[(byte)'�'] = 79;
	cTable['$'] = 80;
	cTable['%'] = 81;
	cTable['&'] = 82;
	cTable['='] = 83;
	cTable[(byte)'�'] = 84;
	cTable[(byte)'�'] = 85;
	cTable[(byte)'�'] = 86;
	cTable[(byte)'�'] = 87;
	cTable[(byte)'�'] = 88;
	cTable[(byte)'�'] = 89;
	cTable[(byte)'�'] = 89;
	cTable[(byte)'�'] = 90;
	cTable[(byte)'�'] = 91;
	cTable[(byte)'�'] = 92;
	cTable[(byte)'�'] = 93;
	cTable[(byte)'�'] = 94;
	cTable[(byte)'�'] = 95;
	cTable[(byte)'�'] = 96;
	cTable[(byte)'�'] = 97;
	cTable[(byte)'�'] = 98;
	cTable[(byte)'�'] = 99;
	//cTable[' '] = 100;  e cerchietto
	//cTable[' '] = 101;  i cerchietto
	//cTable[' '] = 102;  o cerchietto
	//cTable[' '] = 103;  u cerchietto
	cTable[(byte)'�'] = 104;
	cTable[(byte)'�'] = 105;
	cTable[(byte)'�'] = 106;
	cTable[(byte)'�'] = 107;
	cTable[(byte)'�'] = 108;
	cTable[(byte)'�'] = 109;
	//cTable['�'] = 110;  integrale 
	cTable['\''] = 111; 

	// Un po' di lunghezze
	lTable[' '] = 9;
	lTable['\''] = 5;
	lTable['.'] = 5;
	lTable[','] = 5;
	lTable[':'] = 5;
	lTable[';'] = 5;
	lTable['!'] = 5;
	lTable['?'] = 10;
	lTable['\"'] = 5;
	lTable['^'] = 5;
	lTable['('] = 7;
	lTable[')'] = 7;

	lTable['4'] = 10;

	lTable['a'] = 14;
	lTable['b'] = 15;
	lTable['c'] = 12;
	lTable['e'] = 12;
	lTable['i'] = 6;
	lTable['�'] = 6;
	lTable['l'] = 5;
	lTable['m'] = 16;
	lTable['n'] = 12;
	lTable['o'] = 11;
	lTable['p'] = 11;
	lTable['s'] = 12;
	lTable['u'] = 12;

	lTable['E'] = 10;
	lTable['F'] = 11;


	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//�����ꣳ���󌜯���
	
	cTable[(byte)'�'] = 112;
	cTable[(byte)'�'] = 113;
	cTable[(byte)'�'] = 114;
	cTable[(byte)'�'] = 115;
	cTable[(byte)'�'] = 116;
	cTable[(byte)'�'] = 117;
	cTable[(byte)'�'] = 118;
	cTable[(byte)'�'] = 119;
	cTable[(byte)'�'] = 120;
	cTable[(byte)'�'] = 121;
	cTable[(byte)'�'] = 122;
	cTable[(byte)'�'] = 123;
	cTable[(byte)'�'] = 124;
	cTable[(byte)'�'] = 125;
	cTable[(byte)'�'] = 126;
	cTable[(byte)'�'] = 127;
	cTable[(byte)'�'] = 128;
	cTable[(byte)'�'] = 129;

	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 13;

#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable[(byte)'�'] = 130;
	cTable[(byte)'�'] = 131;
	cTable[(byte)'�'] = 132;
	cTable[(byte)'�'] = 133;
	cTable[(byte)'�'] = 134;
	cTable[(byte)'�'] = 135;
	cTable[(byte)'�'] = 136;
	cTable[(byte)'�'] = 137;
	cTable[(byte)'�'] = 138;
	cTable[(byte)'�'] = 139;
	cTable[(byte)'�'] = 140;
	cTable[(byte)'�'] = 141;
	cTable[(byte)'�'] = 142;
	cTable[(byte)'�'] = 143;
	cTable[(byte)'�'] = 144;
	cTable[(byte)'�'] = 145;
	cTable[(byte)'�'] = 146;
	cTable[(byte)'�'] = 147;
	cTable[(byte)'�'] = 148;
	cTable[(byte)'�'] = 149;
	cTable[(byte)'�'] = 150;
	cTable[(byte)'�'] = 151;
	cTable[(byte)'�'] = 152;
	cTable[(byte)'�'] = 153;
	cTable[(byte)'�'] = 154;
	cTable[(byte)'�'] = 155;
	cTable[(byte)'�'] = 156;
	cTable[(byte)'�'] = 157;
	cTable[(byte)'�'] = 158;
	cTable[(byte)'�'] = 159;
	cTable[(byte)'�'] = 160;
	cTable[(byte)'�'] = 161;
	cTable[(byte)'�'] = 162;

	cTable[(byte)'�'] = 163;
	cTable[(byte)'�'] = 164;
	cTable[(byte)'�'] = 165;
	cTable[(byte)'�'] = 166;
	cTable[(byte)'�'] = 167;
	cTable[(byte)'�'] = 168;
	cTable[(byte)'�'] = 169;
	cTable[(byte)'�'] = 170;
	cTable[(byte)'�'] = 171;
	cTable[(byte)'�'] = 172;
	cTable[(byte)'�'] = 173;
	cTable[(byte)'�'] = 174;
	cTable[(byte)'�'] = 175;
	cTable[(byte)'�'] = 176;
	cTable[(byte)'�'] = 177;
	cTable[(byte)'�'] = 178;
	cTable[(byte)'�'] = 179;
	cTable[(byte)'�'] = 180;
	cTable[(byte)'�'] = 181;
	cTable[(byte)'�'] = 182;
	cTable[(byte)'�'] = 183;
	cTable[(byte)'�'] = 184;
	cTable[(byte)'�'] = 185;
	cTable[(byte)'�'] = 186;
	cTable[(byte)'�'] = 187;
	cTable[(byte)'�'] = 188;
	cTable[(byte)'�'] = 189;
	cTable[(byte)'�'] = 190;
	cTable[(byte)'�'] = 191;
	cTable[(byte)'�'] = 192;
	cTable[(byte)'�'] = 193;
	cTable[(byte)'�'] = 194;
	cTable[(byte)'�'] = 195;

	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 17;
	lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 18;	
	lTable[(byte)'�'] = 19;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 14;

	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] =  8;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] =  9;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 14;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH

	cTable[(byte)'�'] = 196;
	cTable[(byte)'�'] = 197;
	cTable[(byte)'�'] = 198;
	cTable[(byte)'�'] = 199;
	cTable[(byte)'�'] = 200;
	cTable[(byte)'�'] = 201;
	cTable[(byte)'�'] = 202;
	cTable[(byte)'�'] = 203;
	cTable[(byte)'�'] = 204;
	cTable[(byte)'�'] = 205;
	cTable[(byte)'�'] = 206;
	cTable[(byte)'�'] = 207;
	cTable[(byte)'�'] = 208;
	cTable[(byte)'�'] = 209;
	cTable[(byte)'�'] = 210;

	cTable[(byte)'�'] = 211;
	cTable[(byte)'�'] = 212;
	cTable[(byte)'�'] = 213;
	cTable[(byte)'�'] = 214;
	cTable[(byte)'�'] = 215;
	cTable[(byte)'�'] = 216;
	cTable[(byte)'�'] = 217;
	cTable[(byte)'�'] = 218;
	cTable[(byte)'�'] = 219;
	cTable[(byte)'�'] = 220;
	cTable[(byte)'�'] = 221;
	cTable[(byte)'�'] = 222;
	cTable[(byte)'�'] = 223;
	cTable[(byte)'�'] = 224;
	cTable[(byte)'�'] = 225;

	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 19;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;

	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 7;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 17;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;

#endif // FONT_CZECH

#ifdef FONT_FRENCH
	cTable[(byte)'�'] = 226;
	cTable[(byte)'�'] = 227;
	cTable[(byte)'�'] = 228;
	cTable[(byte)'�'] = 229;
	cTable[(byte)'�'] = 230;
	cTable[(byte)'�'] = 231;
	cTable[(byte)'�'] = 232;
	cTable[(byte)'�'] = 233;

	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] =  9;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable[(byte)'�'] = 234;
	// 'SS' = 235
	cTable[(byte)'�'] = 236;
	cTable[(byte)'�'] = 237;
	cTable[(byte)'�'] = 238;

	lTable[(byte)'�'] = 15;

#endif // FONT_GERMAN
}


/***************************************************************************\
*       Metodi di RMFontMacc
\****************************************************************************/

void RMFontMacc::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 102	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// francais
		+  5;	// deutsch


	Load(RES_F_MACC, nchars, 11, 16);

	// Default
	lDefault = 10;
	hDefault = 17;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A'+i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a'+i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0'+i] = i + 52;

	cTable['!'] = 62;
	//cTable['!'] = 63;			// ! rovescia
	cTable['\"'] = 64;		
	cTable['$'] = 65;		
	cTable['%'] = 66;		
	cTable['&'] = 67;		
	cTable['/'] = 68;		
	cTable['('] = 69;		
	cTable[')'] = 70;		
	cTable['='] = 71;		
	cTable['?'] = 72;		
	//cTable['?'] = 73;		   // ? rovescia
	cTable['*'] = 74;		
	cTable['+'] = 75;		
	cTable[(byte)'�'] = 76;		
	cTable[';'] = 77;		
	cTable[','] = 78;		
	cTable['.'] = 79;
	cTable[':'] = 80;		
	cTable['-'] = 81;		
	cTable['<'] = 82;
	cTable['>'] = 83;
	cTable['/'] = 84;
	cTable[(byte)'�'] = 85;
	cTable[(byte)'�'] = 86;
	cTable[(byte)'�'] = 87;
	cTable[(byte)'�'] = 88;
	cTable[(byte)'�'] = 89;
	cTable[(byte)'�'] = 90;
	//cTable[(byte)''] = 91;		  // e col pallino
	cTable[(byte)'�'] = 92;		
	cTable[(byte)'�'] = 93;		
	//cTable[(byte)''] = 94;			// i col pallino
	cTable[(byte)'�'] = 95;		
	cTable[(byte)'�'] = 96;		
	//cTable[(byte)''] = 97;		  // o col pallino
	cTable[(byte)'�'] = 98;		
	cTable[(byte)'�'] = 99;		
	//cTable[(byte)''] = 100;		  // u col pallino
	cTable[(byte)'�'] = 101;		

	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//�����ꣳ���󌜯���
	
	cTable[(byte)'�'] = 102;
	cTable[(byte)'�'] = 103;
	cTable[(byte)'�'] = 104;
	cTable[(byte)'�'] = 105;
	cTable[(byte)'�'] = 106;
	cTable[(byte)'�'] = 107;
	cTable[(byte)'�'] = 108;
	cTable[(byte)'�'] = 109;
	cTable[(byte)'�'] = 110;
	cTable[(byte)'�'] = 111;
	cTable[(byte)'�'] = 112;
	cTable[(byte)'�'] = 113;
	cTable[(byte)'�'] = 114;
	cTable[(byte)'�'] = 115;
	cTable[(byte)'�'] = 116;
	cTable[(byte)'�'] = 117;
	cTable[(byte)'�'] = 118;
	cTable[(byte)'�'] = 119;
	
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 13;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable[(byte)'�'] = 120;
	cTable[(byte)'�'] = 121;
	cTable[(byte)'�'] = 122;
	cTable[(byte)'�'] = 123;
	cTable[(byte)'�'] = 124;
	cTable[(byte)'�'] = 125;
	cTable[(byte)'�'] = 126;
	cTable[(byte)'�'] = 127;
	cTable[(byte)'�'] = 128;
	cTable[(byte)'�'] = 129;
	cTable[(byte)'�'] = 130;
	cTable[(byte)'�'] = 131;
	cTable[(byte)'�'] = 132;
	cTable[(byte)'�'] = 133;
	cTable[(byte)'�'] = 134;
	cTable[(byte)'�'] = 135;
	cTable[(byte)'�'] = 136;
	cTable[(byte)'�'] = 137;
	cTable[(byte)'�'] = 138;
	cTable[(byte)'�'] = 139;
	cTable[(byte)'�'] = 140;
	cTable[(byte)'�'] = 141;
	cTable[(byte)'�'] = 142;
	cTable[(byte)'�'] = 143;
	cTable[(byte)'�'] = 144;
	cTable[(byte)'�'] = 145;
	cTable[(byte)'�'] = 146;
	cTable[(byte)'�'] = 147;
	cTable[(byte)'�'] = 148;
	cTable[(byte)'�'] = 149;
	cTable[(byte)'�'] = 150;
	cTable[(byte)'�'] = 151;
	cTable[(byte)'�'] = 152;

	cTable[(byte)'�'] = 153;
	cTable[(byte)'�'] = 154;
	cTable[(byte)'�'] = 155;
	cTable[(byte)'�'] = 156;
	cTable[(byte)'�'] = 157;
	cTable[(byte)'�'] = 158;
	cTable[(byte)'�'] = 159;
	cTable[(byte)'�'] = 160;
	cTable[(byte)'�'] = 161;
	cTable[(byte)'�'] = 162;
	cTable[(byte)'�'] = 163;
	cTable[(byte)'�'] = 164;
	cTable[(byte)'�'] = 165;
	cTable[(byte)'�'] = 166;
	cTable[(byte)'�'] = 167;
	cTable[(byte)'�'] = 168;
	cTable[(byte)'�'] = 169;
	cTable[(byte)'�'] = 170;
	cTable[(byte)'�'] = 171;
	cTable[(byte)'�'] = 172;
	cTable[(byte)'�'] = 173;
	cTable[(byte)'�'] = 174;
	cTable[(byte)'�'] = 175;
	cTable[(byte)'�'] = 176;
	cTable[(byte)'�'] = 177;
	cTable[(byte)'�'] = 178;
	cTable[(byte)'�'] = 179;
	cTable[(byte)'�'] = 180;
	cTable[(byte)'�'] = 181;
	cTable[(byte)'�'] = 182;
	cTable[(byte)'�'] = 183;
	cTable[(byte)'�'] = 184;
	cTable[(byte)'�'] = 185;

	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 8;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;

	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] =  9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH
	
	cTable[(byte)'�'] = 186;
	cTable[(byte)'�'] = 187;
	cTable[(byte)'�'] = 188;
	cTable[(byte)'�'] = 189;
	cTable[(byte)'�'] = 190;
	cTable[(byte)'�'] = 191;
	cTable[(byte)'�'] = 192;
	cTable[(byte)'�'] = 193;
	cTable[(byte)'�'] = 194;
	cTable[(byte)'�'] = 195;
	cTable[(byte)'�'] = 196;
	cTable[(byte)'�'] = 197;
	cTable[(byte)'�'] = 198;
	cTable[(byte)'�'] = 199;
	cTable[(byte)'�'] = 200;

	cTable[(byte)'�'] = 201;
	cTable[(byte)'�'] = 202;
	cTable[(byte)'�'] = 203;
	cTable[(byte)'�'] = 204;
	cTable[(byte)'�'] = 205;
	cTable[(byte)'�'] = 206;
	cTable[(byte)'�'] = 207;
	cTable[(byte)'�'] = 208;
	cTable[(byte)'�'] = 209;
	cTable[(byte)'�'] = 210;
	cTable[(byte)'�'] = 211;
	cTable[(byte)'�'] = 212;
	cTable[(byte)'�'] = 213;
	cTable[(byte)'�'] = 214;
	cTable[(byte)'�'] = 215;

	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;

	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 9;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	cTable[(byte)'�'] = 226;
	cTable[(byte)'�'] = 227;
	cTable[(byte)'�'] = 228;
	cTable[(byte)'�'] = 229;
	cTable[(byte)'�'] = 230;
	cTable[(byte)'�'] = 231;
	cTable[(byte)'�'] = 232;
	cTable[(byte)'�'] = 233;

	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 8;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable[(byte)'�'] = 234;
	// 'SS' = 235
	cTable[(byte)'�'] = 236;
	cTable[(byte)'�'] = 237;
	cTable[(byte)'�'] = 238;

	lTable[(byte)'�'] = 11;
#endif // FONT_GERMAN
}

/***************************************************************************\
*       Metodi di RMFontCredits
\****************************************************************************/

void RMFontCredits::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 112	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// french
		+  2;	// deutsch


	Load(RES_F_CREDITS, nchars, 27, 28, RES_F_CPAL);

	// Default
	lDefault=10;
	hDefault=28;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;



	cTable[(byte)'�'] = 52;
	cTable[(byte)'�'] = 53;
//	cTable[''] = 54; // a ^
//	cTable[''] = 55; // a pallini
	cTable[(byte)'�'] = 56;
	cTable[(byte)'�'] = 57; 
//	cTable[''] = 58; // e ^
//	cTable[''] = 59; // e pallini
	cTable[(byte)'�'] = 60;
	cTable[(byte)'�'] = 61;
//	cTable[''] = 62; // i ^
//	cTable[''] = 63; // i pallini
	cTable[(byte)'�'] = 64;
	cTable[(byte)'�'] = 65;
//	cTable[''] = 66; // o ^
//	cTable[''] = 67; // o pallini
	cTable[(byte)'�'] = 68;
	cTable[(byte)'�'] = 69;
//	cTable[''] = 70; // u ^
//	cTable[''] = 71; // u pallini
//	cTable[''] = 72; // y pallini
	cTable[(byte)'�'] = 73;
	cTable[(byte)'�'] = 74; 
//	cTable[''] = 75; // o barrato
//	cTable[''] = 76; // ac
	cTable[(byte)'�'] = 77;
//	cTable[''] = 78; // ? rovesciato
	cTable['?'] = 79; 
//	cTable[''] = 80; // ! rovesciato
	cTable['!'] = 81; 
//	cTable[''] = 82; // 1/2
//	cTable[''] = 83; // 1/4
	cTable['('] = 84;
	cTable[')'] = 85;
	cTable[(byte)'�'] = 86;
	cTable[(byte)'�'] = 87;
//	cTable[''] = 88; // AE
	cTable[':'] = 89;
	cTable['%'] = 90;
	cTable['&'] = 91;
	cTable['/'] = 92;
	cTable['+'] = 93;
	cTable[';'] = 94;
	cTable[','] = 95;
	cTable['^'] = 96;
	cTable['='] = 97;
	cTable['_'] = 98; 
	cTable['*'] = 99; 
	cTable['.'] = 100;  

	for (i = 0; i < 10; i++)
		cTable['0'+i] = i+101;
	cTable['\''] = 111;  

	lTable[' '] = 11;
	lTable[(byte)'�'] = lTable['A'] = 19;
	lTable['B'] = 15;
	lTable['C'] = 14;
	lTable['D'] = 13;
	lTable['E'] = 14;
	lTable['F'] = 13;
	lTable['G'] = 16;
	lTable['H'] = 15;
	lTable['I'] = 5;
	lTable['J'] = 8;
	lTable['K'] = 15;
	lTable['L'] = 13;
	lTable['M'] = 17;
	lTable['N'] = 15;
	lTable['�'] = lTable['O'] = 14;
	lTable['P'] = 12;
	lTable['Q'] = 14;
	lTable['R'] = 14;
	lTable['S'] = 15;
	lTable['T'] = 11;
	lTable['�'] = lTable['U'] = 12;
	lTable['V'] = 12;
	lTable['W'] = 16;
	lTable['X'] = 12;
	lTable['Y'] = 13;
	lTable['Z'] = 14;

	lTable['a'] = 11;
	lTable['b'] = 9;
	lTable['c'] = 9;
	lTable['d'] = 10;
	lTable['e'] = 9;
	lTable['f'] = 8;
	lTable['g'] = 9;
	lTable['h'] = 10;
	lTable['i'] = 5;
	lTable['j'] = 6;
	lTable['k'] = 12;
	lTable['l'] = 6;
	lTable['m'] = 14;
	lTable['n'] = 10;
	lTable['o'] = 11;
	lTable['p'] = 11;
	lTable['q'] = 9;
	lTable['r'] = 9;
	lTable['s'] = 9;
	lTable['t'] = 6;
	lTable['u'] = 9;
	lTable['v'] = 10;
	lTable['w'] = 14;
	lTable['x'] = 9;
	lTable['y'] = 10;
	lTable['z'] = 9;

	lTable['0'] = 12;
	lTable['1'] = 8;
	lTable['2'] = 10;
	lTable['3'] = 11;
	lTable['4'] = 12;
	lTable['5'] = 11;
	lTable['6'] = 12;
	lTable['7'] = 10;
	lTable['8'] = 11;
	lTable['9'] = 10;

	lTable['/'] = 10;
	lTable['^'] = 9;
	lTable[','] = 5;
	lTable['.'] = 5;
	lTable[';'] = 5;
	lTable[':'] = 5;
	lTable['\''] = 5;



	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//�����ꣳ���󌜯���
	
	cTable[(byte)'�'] = 112;
	cTable[(byte)'�'] = 113;
	cTable[(byte)'�'] = 114;
	cTable[(byte)'�'] = 115;
	cTable[(byte)'�'] = 116;
	cTable[(byte)'�'] = 117;
	cTable[(byte)'�'] = 118;
	cTable[(byte)'�'] = 119;
	cTable[(byte)'�'] = 120;
	cTable[(byte)'�'] = 121;
	cTable[(byte)'�'] = 122;
	cTable[(byte)'�'] = 123;
	cTable[(byte)'�'] = 124;
	cTable[(byte)'�'] = 125;
	cTable[(byte)'�'] = 126;
	cTable[(byte)'�'] = 127;
	cTable[(byte)'�'] = 128;
	cTable[(byte)'�'] = 129;
	
	lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable[(byte)'�'] = 130;
	cTable[(byte)'�'] = 131;
	cTable[(byte)'�'] = 132;
	cTable[(byte)'�'] = 133;
	cTable[(byte)'�'] = 134;
	cTable[(byte)'�'] = 135;
	cTable[(byte)'�'] = 136;
	cTable[(byte)'�'] = 137;
	cTable[(byte)'�'] = 138;
	cTable[(byte)'�'] = 139;
	cTable[(byte)'�'] = 140;
	cTable[(byte)'�'] = 141;
	cTable[(byte)'�'] = 142;
	cTable[(byte)'�'] = 143;
	cTable[(byte)'�'] = 144;
	cTable[(byte)'�'] = 145;
	cTable[(byte)'�'] = 146;
	cTable[(byte)'�'] = 147;
	cTable[(byte)'�'] = 148;
	cTable[(byte)'�'] = 149;
	cTable[(byte)'�'] = 150;
	cTable[(byte)'�'] = 151;
	cTable[(byte)'�'] = 152;
	cTable[(byte)'�'] = 153;
	cTable[(byte)'�'] = 154;
	cTable[(byte)'�'] = 155;
	cTable[(byte)'�'] = 156;
	cTable[(byte)'�'] = 157;
	cTable[(byte)'�'] = 158;
	cTable[(byte)'�'] = 159;
	cTable[(byte)'�'] = 160;
	cTable[(byte)'�'] = 161;
	cTable[(byte)'�'] = 162;

	cTable[(byte)'�'] = 163;
	cTable[(byte)'�'] = 164;
	cTable[(byte)'�'] = 165;
	cTable[(byte)'�'] = 166;
	cTable[(byte)'�'] = 167;
	cTable[(byte)'�'] = 168;
	cTable[(byte)'�'] = 169;
	cTable[(byte)'�'] = 170;
	cTable[(byte)'�'] = 171;
	cTable[(byte)'�'] = 172;
	cTable[(byte)'�'] = 173;
	cTable[(byte)'�'] = 174;
	cTable[(byte)'�'] = 175;
	cTable[(byte)'�'] = 176;
	cTable[(byte)'�'] = 177;
	cTable[(byte)'�'] = 178;
	cTable[(byte)'�'] = 179;
	cTable[(byte)'�'] = 180;
	cTable[(byte)'�'] = 181;
	cTable[(byte)'�'] = 182;
	cTable[(byte)'�'] = 183;
	cTable[(byte)'�'] = 184;
	cTable[(byte)'�'] = 185;
	cTable[(byte)'�'] = 186;
	cTable[(byte)'�'] = 187;
	cTable[(byte)'�'] = 188;
	cTable[(byte)'�'] = 189;
	cTable[(byte)'�'] = 190;
	cTable[(byte)'�'] = 191;
	cTable[(byte)'�'] = 192;
	cTable[(byte)'�'] = 193;
	cTable[(byte)'�'] = 194;
	cTable[(byte)'�'] = 195;

	lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 22;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 22;
	lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 23;
	lTable[(byte)'�'] = 23;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 12;	
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = 15;

	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 11;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH

	cTable[(byte)'�'] = 196;
	cTable[(byte)'�'] = 197;
	cTable[(byte)'�'] = 198;
	cTable[(byte)'�'] = 199;
	cTable[(byte)'�'] = 200;
	cTable[(byte)'�'] = 201;
	cTable[(byte)'�'] = 202;
	cTable[(byte)'�'] = 203;
	cTable[(byte)'�'] = 204;
	cTable[(byte)'�'] = 205;
	cTable[(byte)'�'] = 206;
	cTable[(byte)'�'] = 207;
	cTable[(byte)'�'] = 208;
	cTable[(byte)'�'] = 209;
	cTable[(byte)'�'] = 210;

	cTable[(byte)'�'] = 211;
	cTable[(byte)'�'] = 212;
	cTable[(byte)'�'] = 213;
	cTable[(byte)'�'] = 214;
	cTable[(byte)'�'] = 215;
	cTable[(byte)'�'] = 216;
	cTable[(byte)'�'] = 217;
	cTable[(byte)'�'] = 218;
	cTable[(byte)'�'] = 219;
	cTable[(byte)'�'] = 220;
	cTable[(byte)'�'] = 221;
	cTable[(byte)'�'] = 222;
	cTable[(byte)'�'] = 223;
	cTable[(byte)'�'] = 224;
	cTable[(byte)'�'] = 225;

	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 14;
	lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = 7;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = 19;
	lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 13;
	lTable[(byte)'�'] = 13;

	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 6;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	cTable[(byte)'�'] = 226;
	cTable[(byte)'�'] = 227;
	cTable[(byte)'�'] = 228;
	cTable[(byte)'�'] = 229;
	cTable[(byte)'�'] = 230;
	cTable[(byte)'�'] = 231;
	cTable[(byte)'�'] = 232;
	cTable[(byte)'�'] = 233;

	lTable[(byte)'�'] = 12;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 6;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 10;
	lTable[(byte)'�'] = 11;
	lTable[(byte)'�'] = 11;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable[(byte)'�'] = 234;
	// 'SS' = 235

	// old chars overrides
	cTable[(byte)'�'] = cTable[(byte)'�'] = 55;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 67;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 71;

	lTable[(byte)'�'] = 11;

#endif // FONT_GERMAN
}



/***************************************************************************\
*       Metodi di RMFontObj
\****************************************************************************/

#define TOUPPER(a)	((a) >='a'&&(a)<='z'?(a)+'A'-'a':(a))
#define TOLOWER(a)	((a) >='A'&&(a)<='Z'?(a)+'a'-'A':(a))

void RMFontObj::SetBothCase(int nChar, int nNext, signed char spiazz) {
	l2Table[TOUPPER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOUPPER(nChar)][TOLOWER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOLOWER(nNext)] = spiazz;
}


void RMFontObj::Init(void) {
	int i;

	//bernie: numero di caratteri nel font (solo maiuscolo)
	int nchars =
		  85	// base
		+  9	// polish
		+ 33	// russian
		+ 15	// czech
		+  0	// francais (no uppercase chars)
		+  1;	// deutsch


	Load(RES_F_OBJ, nchars, 25, 30);

	// Inizializziamo le tabelline del cazzo
	lDefault = 26;
	hDefault = 30;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++) {
		cTable['A' + i] = i+0;
		cTable['a' + i] = i+0;
	}

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 26;

	cTable[','] = 36;
	cTable[';'] = 37;
	cTable['.'] = 38;
	cTable[':'] = 39;
	cTable['-'] = 40;
	cTable['+'] = 41;
	cTable['!'] = 42;
	// cTable['!'] = 43; Esclamativo alla rovescia
	cTable['?'] = 44;
	//cTable['?'] = 45;  Interrogativo alla rovescia
	cTable['/'] = 46;
	cTable['('] = 47;
	cTable[')'] = 48;
	cTable['='] = 49;
	cTable['\''] = 50;
	cTable['\"'] = 51;
	cTable[(byte)'�'] = 52;
	cTable[(byte)'$'] = 53;
	cTable[(byte)'%'] = 54;
	cTable[(byte)'&'] = 55;
	cTable[(byte)'^'] = 56;
	cTable[(byte)'*'] = 57;
	cTable[(byte)'<'] = 58;
	cTable[(byte)'>'] = 59;
	cTable[(byte)'�'] = 60;
	cTable[(byte)'�'] = 61;
	cTable[(byte)'�'] = 62;
	cTable[(byte)'�'] = 63;
	//cTable[(byte)'�'] = 64;   integrale
	cTable[(byte)'�'] = 65;
	cTable[(byte)'�'] = 66;
	cTable[(byte)'�'] = 67;
	cTable[(byte)'�'] = 68;
	cTable[(byte)'�'] = 69;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 70;
	cTable[(byte)'�'] = 71;
	cTable[(byte)'�'] = 72;
	cTable[(byte)'�'] = 73;
	//cTable[(byte)' '] = 74;   e cerchietto
	cTable[(byte)'�'] = 75;
	cTable[(byte)'�'] = 76;
	//cTable[(byte)' '] = 77;	  i cerchietto
	cTable[(byte)'�'] = 78;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 79;
	//cTable[(byte)' '] = 80;		o cerchietto
	cTable[(byte)'�'] = 81;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 82;
	//cTable[' '] = 83;		u cerchietto
	//cTable[' '] = 84;   y dieresi

	/* Un po' di lunghezze */
	lTable[' '] = 11;
	lTable['.'] = 8;
	lTable['-'] = 12;
	lTable['\''] = 8;
	lTable['0'] = 20;
	lTable['1'] = 20;
	lTable['2'] = 15;
	lTable['3'] = 20;
	lTable['4'] = 20;
	lTable['5'] = 20;
	lTable['6'] = 20;
	lTable['7'] = 20;
	lTable['8'] = 20;
	lTable['9'] = 20;

	
	lTable['a'] = lTable['A'] = lTable['�'] = lTable['�'] = 17;
	lTable['b'] = lTable['B'] = 17;
	lTable['c'] = lTable['C'] = 19;
	lTable['d'] = lTable['D'] = 17;
	lTable['e'] = lTable['E'] = 15;
	lTable['f'] = lTable['F'] = 17;
	lTable['g'] = lTable['G'] = 19;
	lTable['i'] = lTable['I'] = 16;
	lTable['h'] = lTable['H'] = 17;
	lTable['k'] = lTable['K'] = 17;
	lTable['l'] = lTable['L'] = 14;
	lTable['m'] = lTable['M'] = 19;
	lTable['n'] = lTable['N'] = 17;
	lTable['o'] = lTable['O'] = lTable['�'] = lTable['�'] = 19;
	lTable['p'] = lTable['P'] = 17;
	lTable['q'] = lTable['Q'] = 19;
	lTable['r'] = lTable['R'] = 14;
	lTable['s'] = lTable['S'] = 13;
	lTable['t'] = lTable['T'] = 15;
	lTable['u'] = lTable['U'] = lTable['�'] = lTable['�'] = 15;
	lTable['v'] = lTable['V'] = 13;
	lTable['x'] = lTable['X'] = 15;
	lTable['y'] = lTable['Y'] = 13;
	lTable['w'] = lTable['W'] = 19;
	lTable['z'] = lTable['Z'] = 20;
	lTable[(byte)'�'] = 17;

	/* Casi particolari */
	SetBothCase('C','C',2);
	SetBothCase('A','T',-2);
	SetBothCase('R','S',2);
	SetBothCase('H','I',-2);
	SetBothCase('T','S',2);
	SetBothCase('O','R',2);
	SetBothCase('O','L',2);
	SetBothCase('O','G',2);
	SetBothCase('Z','A',-1);
	SetBothCase('R','R',1);
	SetBothCase('R','U',3);


	// Polish characters
	//�����ꣳ���󌜯���
	//AaCcEeLlNnOoSsZzZz
	cTable[(byte)'�'] = cTable[(byte)'�'] = 85;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 20;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 86;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 22;
	
	cTable[(byte)'�'] = cTable[(byte)'�'] = 87;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 17;
	
	cTable[(byte)'�'] = cTable[(byte)'�'] = 88;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 19;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 89;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 17;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 90;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 22;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 91;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 15;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 92;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 21;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 93;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 21;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable[(byte)'�'] = cTable[(byte)'�'] = 85;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 20;

	cTable[(byte)'�'] = cTable[(byte)'�'] =94;
	cTable[(byte)'�'] = cTable[(byte)'�'] =95;
	cTable[(byte)'�'] = cTable[(byte)'�'] =96;
	cTable[(byte)'�'] = cTable[(byte)'�'] =97;
	cTable[(byte)'�'] = cTable[(byte)'�'] =98;
	cTable[(byte)'�'] = cTable[(byte)'�'] =99;
	cTable[(byte)'�'] = cTable[(byte)'�'] =100;
	cTable[(byte)'�'] = cTable[(byte)'�'] =101;
	cTable[(byte)'�'] = cTable[(byte)'�'] =102;
	cTable[(byte)'�'] = cTable[(byte)'�'] =103;
	cTable[(byte)'�'] = cTable[(byte)'�'] =104;
	cTable[(byte)'�'] = cTable[(byte)'�'] =105;
	cTable[(byte)'�'] = cTable[(byte)'�'] =106;
	cTable[(byte)'�'] = cTable[(byte)'�'] =107;
	cTable[(byte)'�'] = cTable[(byte)'�'] =108;
	cTable[(byte)'�'] = cTable[(byte)'�'] =109;
	cTable[(byte)'�'] = cTable[(byte)'�'] =110;
	cTable[(byte)'�'] = cTable[(byte)'�'] =111;
	cTable[(byte)'�'] = cTable[(byte)'�'] =112;
	cTable[(byte)'�'] = cTable[(byte)'�'] =113;
	cTable[(byte)'�'] = cTable[(byte)'�'] =114;
	cTable[(byte)'�'] = cTable[(byte)'�'] =115;
	cTable[(byte)'�'] = cTable[(byte)'�'] =116;
	cTable[(byte)'�'] = cTable[(byte)'�'] =117;
	cTable[(byte)'�'] = cTable[(byte)'�'] =118;
	cTable[(byte)'�'] = cTable[(byte)'�'] =119;
	cTable[(byte)'�'] = cTable[(byte)'�'] =120;
	cTable[(byte)'�'] = cTable[(byte)'�'] =121;
	cTable[(byte)'�'] = cTable[(byte)'�'] =122;
	cTable[(byte)'�'] = cTable[(byte)'�'] =123;
	cTable[(byte)'�'] = cTable[(byte)'�'] =124;
	cTable[(byte)'�'] = cTable[(byte)'�'] =125;
	cTable[(byte)'�'] = cTable[(byte)'�'] =126;


	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 17;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 22;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 22;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 19;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 21;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 20;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 16;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 19;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 22;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 19;	
	lTable[(byte)'�'] = lTable[(byte)'�'] = 19;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 15;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;
	lTable[(byte)'�'] = lTable[(byte)'�'] = 18;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH
	// rep. ceca characters	

	cTable[(byte)'�'] = cTable[(byte)'�'] = 127;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 128;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 129;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 130;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 131;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 132;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 133;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 134;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 135;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 136;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 137;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 138;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 139;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 140;
	cTable[(byte)'�'] = cTable[(byte)'�'] = 141;

	lTable[(byte)'�'] = lTable[(byte)'�'] =17;
	lTable[(byte)'�'] = lTable[(byte)'�'] =15;
	lTable[(byte)'�'] = lTable[(byte)'�'] =22;
	lTable[(byte)'�'] = lTable[(byte)'�'] =18;
	lTable[(byte)'�'] = lTable[(byte)'�'] =21;
	lTable[(byte)'�'] = lTable[(byte)'�'] =16;
	lTable[(byte)'�'] = lTable[(byte)'�'] =18;
	lTable[(byte)'�'] = lTable[(byte)'�'] =19;
	lTable[(byte)'�'] = lTable[(byte)'�'] =17;
	lTable[(byte)'�'] = lTable[(byte)'�'] =23;
	lTable[(byte)'�'] = lTable[(byte)'�'] =24;
	lTable[(byte)'�'] = lTable[(byte)'�'] =17;
	lTable[(byte)'�'] = lTable[(byte)'�'] =22;
	lTable[(byte)'�'] = lTable[(byte)'�'] =16;
	lTable[(byte)'�'] = lTable[(byte)'�'] =16;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	// traduci le lettere accentate in lettere normali

	cTable[(byte)'�'] = cTable[(byte)'�'] = cTable[(byte)'�'] = 0; // a
	lTable[(byte)'�'] = lTable[(byte)'�'] = lTable[(byte)'�'] = 17;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 4; // e
	lTable[(byte)'�'] = lTable[(byte)'�'] = 15;

	cTable[(byte)'�'] = cTable[(byte)'�'] = cTable[(byte)'�'] = 8; // i
	lTable[(byte)'�'] = lTable[(byte)'�'] = lTable[(byte)'�'] = 16;

	cTable[(byte)'�'] = cTable[(byte)'�'] = cTable[(byte)'�'] = cTable[(byte)'�'] = 14; // o
	lTable[(byte)'�'] = lTable[(byte)'�'] = lTable[(byte)'�'] = lTable[(byte)'�'] = 19;

	cTable[(byte)'�'] = cTable[(byte)'�'] = 20; // u
	lTable[(byte)'�'] = lTable[(byte)'�'] = 15;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['�'] = 142;
	// SS = 143

	lTable['�'] = 24;
#endif // FONT_GERMAN
}


/****************************************************************************\
*       Metodi di RMText
\****************************************************************************/

RMFontColor *RMText::m_fonts[4] = { NULL, NULL, NULL, NULL };
OSystem::MutexRef RMText::m_cs;
RMGfxClearTask RMText::m_clear;

RMText::RMText() {
	// Colore di default: bianco
	m_r = m_g = m_b = 255;

	// Lunghezza di default
	maxLineLength = 350;

	m_bTrasp0 = true;
	aHorType = HCENTER;
	aVerType = VTOP;
	SetPriority(150);
}

RMText::~RMText() {
	
}

void RMText::Unload() {
	if (m_fonts[0] != NULL) {
		delete m_fonts[0];
		delete m_fonts[1];
		delete m_fonts[2];
		delete m_fonts[3];
		m_fonts[0] =  m_fonts[1] = m_fonts[2] = m_fonts[3] = 0;

		g_system->unlockMutex(m_cs);
	}
}

void RMText::SetMaxLineLength(int max) {
	maxLineLength = max;
}

void RMText::RemoveThis(CORO_PARAM, bool &result) {
 // Qui possiamo fare i controlli sul numero di frame, sul tempo trascorso
 // etc.
	result = true;
}


void RMText::WriteText(const RMString &text, int nFont, int *time) {
	// Inizializza i font (una volta sola)	
	if (m_fonts[0] == NULL) {
		m_fonts[0] = new RMFontParla; m_fonts[0]->Init();
		m_fonts[1] = new RMFontObj;   m_fonts[1]->Init();
		m_fonts[2] = new RMFontMacc;  m_fonts[2]->Init();
		m_fonts[3] = new RMFontCredits;  m_fonts[3]->Init();

		m_cs = g_system->createMutex();
	}

	g_system->lockMutex(m_cs);
	WriteText(text,m_fonts[nFont],time);
	g_system->unlockMutex(m_cs);
}


void RMText::WriteText(const RMString &text, RMFontColor *font, int *time) {
	RMGfxPrimitive *prim;
	char *p, *old_p;
	int i, j, x, y;
	int len;
	int numchar;
	int width, height;
	char *string;
	int numlines;

	// Setta il colore di base
	font->SetBaseColor(m_r, m_g, m_b);

	// Si autodistrugge il buffer prima di iniziare
	Destroy();

	// Se la stringa � vuota, non fare nulla	
	if (text == NULL || text[0] == '\0')
		return;
	
	// Divide la frase in linee. In questo ciclo, X contiene la lunghezza massima raggiunta da una linea
	// e I il numero delle linee
	string=p = text;
	i = j = x = 0;
	while (*p != '\0') {
		j += font->StringLen(*p);
		if (j > (((aHorType == HLEFTPAR) && (i > 0)) ? maxLineLength - 25 : maxLineLength)) {
			j -= font->StringLen(*p, p[1]);
			if (j > x) x = j;

			// Torna indietro al primo spazio utile
			//
			// BERNIE: nella versione originale le frasi contenenti
			// parole che superano la larghezza di una riga causavano
			// l'interruzione dell'intera frase.
			// Questo workaround e' parziale: la parola troppo lunga
			// viene spezzata bruscamente e si perde una lettera.
			// Oltre allo spazio e' ammesso il wrap sul carattere '-'.
			//
			old_p = p;
			while (*p != ' ' && *p != '-' && p > string) p--;

			if (p == string)
				p = old_p;

			// Controlla se sono tutti spazi fino alla fine
			while (*p == ' ' && *p != '\0') p++;
			if (*p == '\0')
				break;
			p--;
			i++;
			*p = '\0';
			j = 0;
		}
		p++;
	}

	if (j > x) x = j;
 
	i++;
	numlines = i;

	// X=Lunghezza della linea piu' lunga. Controlla se puo' essere puttata a X1
	//x+=font->StringLen(-1)+1;          // Meglio esagerare per sicurezza
	x += 8;

	// Posizione di partenza per la surface: X1,Y
	width = x;
	height = (numlines - 1) * font->LetterHeight() + font->m_fontDimy;

	// Crea la surface
	Create(width, height);
	//AddPrim(new RMGfxPrimitive(&m_clear));
	Common::fill(m_buf, m_buf + width * height * 2, 0);

	p = string;

	y = 0;
	numchar = 0;
	for (; i > 0; i--) {
		// Misura la lunghezza della linea
		x = 0;
		j = font->StringLen(RMString(p));

		switch (aHorType) {
		case HLEFT:
			x = 0;
			break;

		case HLEFTPAR:
			if (i == numlines)
				x=0;
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
				x += font->StringLen(*p);
				p++;
				continue;
			}

			prim = font->MakeLetterPrimitive(*p, len);
			prim->Dst().x1 = x;
			prim->Dst().y1 = y;
			AddPrim(prim);

			numchar++;

			x += font->StringLen(*p, p[1]);
			p++;
		}
		p++;
		y += font->LetterHeight();
	}

	if (time != NULL)
		*time = 1000 + numchar * (11 - nCfgTextSpeed) * 14;
}

void RMText::ClipOnScreen(RMGfxPrimitive *prim) {
	// Cerca di non farlo uscire dallo schermo
	if (prim->Dst().x1 < 5) prim->Dst().x1 = 5;
	if (prim->Dst().y1 < 5) prim->Dst().y1 = 5;
	if (prim->Dst().x1+m_dimx > 635) prim->Dst().x1 = 635 - m_dimx;
	if (prim->Dst().y1+m_dimy > 475) prim->Dst().y1 = 475 - m_dimy;
}

void RMText::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Allinea orizzontalmente
	if (aHorType == HCENTER)
		prim->Dst().TopLeft() -= RMPoint(m_dimx / 2, 0);
	else if (aHorType == HRIGHT)
		prim->Dst().TopLeft() -= RMPoint(m_dimx, 0);


	// Alinea verticalemente
	if (aVerType == VTOP) {

	} else if (aVerType == VCENTER) {
		prim->Dst().y1 -= m_dimy / 2;

	} else if (aVerType == VBOTTOM) {
		prim->Dst().y1 -= m_dimy;
	}

	ClipOnScreen(prim);

	CORO_INVOKE_2(RMGfxWoodyBuffer::Draw, bigBuf, prim);

	CORO_END_CODE;
}

/****************************************************************************\
*       Metodi di RMTextDialog
\****************************************************************************/

RMTextDialog::RMTextDialog() : RMText() {
	m_startTime = 0;
	dst = RMPoint(0,0);

	m_bSkipStatus = true;
	m_bShowed = true;
	m_bForceTime = false;
	m_bForceNoTime = false;
	m_bAlwaysDisplay = false;
	m_bNoTab = false;
	hCustomSkip = CORO_INVALID_PID_VALUE;
	hCustomSkip2 = CORO_INVALID_PID_VALUE;
	m_input = NULL;

	// Crea l'evento di fine displaying
	hEndDisplay = CoroScheduler.createEvent(false, false);
}

RMTextDialog::~RMTextDialog() {
	CoroScheduler.closeEvent(hEndDisplay);
}

void RMTextDialog::Show(void) {
	m_bShowed = true;
}

void RMTextDialog::Hide(CORO_PARAM) {
	m_bShowed = false;
}

void RMTextDialog::WriteText(const RMString &text, int font, int *time) {
	RMText::WriteText(text,font,&m_time);

	if (time != NULL)
		*time = m_time;
}

void RMTextDialog::WriteText(const RMString &text, RMFontColor *font, int *time) {
	RMText::WriteText(text,font,&m_time);

	if (time != NULL)
		*time = m_time;
}


void RMTextDialog::SetSkipStatus(bool bEnabled) {
	m_bSkipStatus = bEnabled;
}

void RMTextDialog::ForceTime(void) {
	m_bForceTime = true;
}

void RMTextDialog::ForceNoTime(void) {
	m_bForceNoTime = true;
}

void RMTextDialog::SetNoTab(void) {
	m_bNoTab = true;
}

void RMTextDialog::SetForcedTime(uint32 dwTime) {
	m_time = dwTime;	
}

void RMTextDialog::SetAlwaysDisplay(void) {
	m_bAlwaysDisplay = true;
}

void RMTextDialog::RemoveThis(CORO_PARAM, bool &result) {
	CORO_BEGIN_CONTEXT;
		bool expired;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Presume successful result
	result = true;

	// Frase NON di background
	if (m_bSkipStatus) {
		if (!(bCfgDubbing && hCustomSkip2 != CORO_INVALID_PID_VALUE))
			if (bCfgTimerizedText) {
				if (!m_bForceNoTime)
					if (_vm->GetTime() > (uint32)m_time + m_startTime)
						return;
			}

		if (!m_bNoTab)
			if (_vm->GetEngine()->GetInput().GetAsyncKeyState(Common::KEYCODE_TAB))
				return;

		if (!m_bNoTab)
			if (m_input)
				if (m_input->MouseLeftClicked() || m_input->MouseRightClicked())
					return;
	}
	// Frase di background
	else {
		if (!(bCfgDubbing && hCustomSkip2 != CORO_INVALID_PID_VALUE))
			if (!m_bForceNoTime)
				if (_vm->GetTime() > (uint32)m_time + m_startTime)
					return;
	}

	// Se il tempo � forzato
	if (m_bForceTime)
		if (_vm->GetTime() > (uint32)m_time + m_startTime)
			return;

	if (hCustomSkip != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, hCustomSkip, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	if (bCfgDubbing && hCustomSkip2 != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, hCustomSkip2, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	result = false;

	CORO_END_CODE;
}

void RMTextDialog::Unregister(void) {
	RMGfxTask::Unregister();
	assert(m_nInList == 0);
	CoroScheduler.setEvent(hEndDisplay);
}

void RMTextDialog::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_startTime == 0)
		m_startTime = _vm->GetTime();
	
	if (m_bShowed) {
		if (bCfgSottotitoli || m_bAlwaysDisplay) {
			prim->Dst().TopLeft() = dst;
			CORO_INVOKE_2(RMText::Draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMTextDialog::SetCustomSkipHandle(uint32 hCustom) {
	hCustomSkip = hCustom;
}

void RMTextDialog::SetCustomSkipHandle2(uint32 hCustom) {
	hCustomSkip2 = hCustom;
}

void RMTextDialog::WaitForEndDisplay(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, hEndDisplay, CORO_INFINITE);
}

void RMTextDialog::SetInput(RMInput *input) {
	m_input = input;
}

/****************************************************************************\
*       Metodi di RMTextDialogScrolling
\****************************************************************************/

RMTextDialogScrolling::RMTextDialogScrolling() {
	curLoc = NULL;
}

RMTextDialogScrolling::RMTextDialogScrolling(RMLocation *loc) {
	curLoc = loc;
	startScroll = loc->ScrollPosition();
}

RMTextDialogScrolling::~RMTextDialogScrolling() {
}

void RMTextDialogScrolling::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
		RMPoint curDst;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curDst = dst;

	if (curLoc != NULL)
		dst -= curLoc->ScrollPosition() - startScroll;

	CORO_INVOKE_2(RMTextDialog::Draw, bigBuf, prim);

	dst = _ctx->curDst;

	CORO_END_CODE;
}

void RMTextDialogScrolling::ClipOnScreen(RMGfxPrimitive *prim) {
	// Non dobbiamo fare nulla!
}


/****************************************************************************\
*       RMTextItemName Methods
\****************************************************************************/

RMTextItemName::RMTextItemName() : RMText() {
	m_item = NULL;
	SetPriority(220);
}

RMTextItemName::~RMTextItemName() {

}

void RMTextItemName::DoFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv) {
	CORO_BEGIN_CONTEXT;
		RMItem *lastItem;
		uint32 hThread;
	CORO_END_CONTEXT(_ctx);

	RMString itemName;

	CORO_BEGIN_CODE(_ctx);

	_ctx->lastItem = m_item;

	// Adds to the list if there is need
	if (!m_nInList)
		bigBuf.AddPrim(new RMGfxPrimitive(this));
	
	// Update the scrolling co-ordinates
	m_curscroll = loc.ScrollPosition();

	// Check if we are on the inventory
	if (inv.ItemInFocus(m_mpos))
		m_item = inv.WhichItemIsIn(m_mpos);
	else
		m_item = loc.WhichItemIsIn(m_mpos);
	
	itemName = "";

	// If there an item, get it's name
	if (m_item != NULL)
		m_item->GetName(itemName);

	// Write it
	WriteText(itemName, 1);

	// Handle the change If the selected item is different from the previous one
	if (_ctx->lastItem != m_item) {
		if (m_item == NULL)
			ptr.SetSpecialPointer(RMPointer::PTR_NONE);
		else {
			_ctx->hThread = mpalQueryDoAction(20, m_item->MpalCode(), 0);		
			if (_ctx->hThread == CORO_INVALID_PID_VALUE)
				ptr.SetSpecialPointer(RMPointer::PTR_NONE);
			else
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->hThread, CORO_INFINITE);
		}
	}

	CORO_END_CODE;
}


void RMTextItemName::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Se non c'e' testo, e' inutile continuare
	if (m_buf == NULL)
		return;

	// Setta come coordinate destinazione quelle del mouse
	prim->Dst().TopLeft() = m_mpos-RMPoint(0, 30);

	CORO_INVOKE_2(RMText::Draw, bigBuf, prim);

	CORO_END_CODE;
}

RMPoint RMTextItemName::GetHotspot() { 
	if (m_item == NULL) 
		return m_mpos + m_curscroll; 
	else 
		return m_item->Hotspot();  
}

RMItem *RMTextItemName::GetSelectedItem() { 
	return m_item; 
}

bool RMTextItemName::IsItemSelected() { 
	return m_item != NULL; 
}

bool RMTextItemName::IsNormalItemSelected() { 
	return m_item != NULL && m_itemName.Length() > 0; 
}


/****************************************************************************\
*       Metodi di RMDialogChoice
\****************************************************************************/

RMDialogChoice::RMDialogChoice() {
	RMResRaw dlg1(RES_I_DLGTEXT);
	RMResRaw dlg2(RES_I_DLGTEXTLINE);
	RMRes dlgpal(RES_I_DLGTEXTPAL);
	
	DlgText.Init(dlg1, dlg1.Width(), dlg1.Height());
	DlgTextLine.Init(dlg2, dlg2.Width(), dlg2.Height());

	DlgText.LoadPaletteWA(dlgpal);
	DlgTextLine.LoadPaletteWA(dlgpal);
	
	hUnreg = CoroScheduler.createEvent(false, false);
	bRemoveFromOT = false;
}

RMDialogChoice::~RMDialogChoice() {
	CoroScheduler.closeEvent(hUnreg);
}

void RMDialogChoice::Unregister(void) {
	RMGfxWoodyBuffer::Unregister();
	assert(!m_nInList);
	CoroScheduler.pulseEvent(hUnreg);

	bRemoveFromOT = false;
}

void RMDialogChoice::Init(void)
{
	m_numChoices = 0;
	m_drawedStrings = NULL;
	m_ptDrawStrings = NULL;
	m_curSelection = -1;

	Create(640, 477);
	SetPriority(140);
}


void RMDialogChoice::Close(void) {
	if (m_drawedStrings != NULL) {
		delete[] m_drawedStrings;
		m_drawedStrings = NULL;
	}

	if (m_ptDrawStrings != NULL) {
		delete[] m_ptDrawStrings;
		m_ptDrawStrings = NULL;
	}

	Destroy();
}

void RMDialogChoice::SetNumChoices(int num) {
	int i;

	m_numChoices = num;
	m_curAdded = 0;
	
	// Alloca lo spazio per le stringhe disegnate
	m_drawedStrings = new RMText[num];
	m_ptDrawStrings = new RMPoint[num];

	// Le inizializza
	for (i = 0; i < m_numChoices; i++) {
		m_drawedStrings[i].SetColor(0, 255, 0);
		m_drawedStrings[i].SetAlignType(RMText::HLEFTPAR, RMText::VTOP);
		m_drawedStrings[i].SetMaxLineLength(600);
		m_drawedStrings[i].SetPriority(10);
	}
}

void RMDialogChoice::AddChoice(const RMString &string) {
	// Si disegna la stringa
	assert(m_curAdded < m_numChoices);
	m_drawedStrings[m_curAdded++].WriteText(string, 0);	
}

void RMDialogChoice::Prepare(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
		int i;
		RMPoint ptPos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	AddPrim(new RMGfxPrimitive(&DlgText,RMPoint(0,0)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83+83)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83+83+83)));

	_ctx->ptPos.Set(20,90);

	for (_ctx->i = 0; _ctx->i < m_numChoices; _ctx->i++) {
		AddPrim(new RMGfxPrimitive(&m_drawedStrings[_ctx->i], _ctx->ptPos));
		m_ptDrawStrings[_ctx->i] = _ctx->ptPos;
		_ctx->ptPos.Offset(0,m_drawedStrings[_ctx->i].Dimy() + 15);
	}

	CORO_INVOKE_0(DrawOT);
	ClearOT();

	m_ptDrawPos.Set(0,480-_ctx->ptPos.y);

	CORO_END_CODE;
}

void RMDialogChoice::SetSelected(CORO_PARAM, int pos) {
	CORO_BEGIN_CONTEXT;
		RMGfxBox box;
		RMRect rc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (pos == m_curSelection)
		return;

	_ctx->box.SetPriority(5);

	if (m_curSelection != -1) {
		_ctx->box.SetColor(0xCC, 0xCC, 0xFF);
		_ctx->rc.TopLeft()=RMPoint(18, m_ptDrawStrings[m_curSelection].y); 
		_ctx->rc.BottomRight() = _ctx->rc.TopLeft() + RMPoint(597, m_drawedStrings[m_curSelection].Dimy());
		AddPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));

		AddPrim(new RMGfxPrimitive(&m_drawedStrings[m_curSelection], m_ptDrawStrings[m_curSelection]));
		CORO_INVOKE_0(DrawOT);
		ClearOT();
	}

	if (pos != -1) {
		_ctx->box.SetColor(100, 100, 100);
		_ctx->rc.TopLeft()=RMPoint(18, m_ptDrawStrings[pos].y); 
		_ctx->rc.BottomRight() = _ctx->rc.TopLeft()+RMPoint(597, m_drawedStrings[pos].Dimy());
		AddPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));
		AddPrim(new RMGfxPrimitive(&m_drawedStrings[pos], m_ptDrawStrings[pos]));
	}

	CORO_INVOKE_0(DrawOT);
	ClearOT();

	m_curSelection = pos;

	CORO_END_CODE;
}

void RMDialogChoice::Show(CORO_PARAM, RMGfxTargetBuffer *bigBuf) {
	CORO_BEGIN_CONTEXT;
		RMPoint destpt;
		int deltay;
		int starttime;
		int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(Prepare);
	m_bShow = false;

	if (!m_nInList && bigBuf != NULL)
		bigBuf->AddPrim(new RMGfxPrimitive(this));

	if (0) {
		m_bShow = true;
	} else {
		_ctx->starttime = _vm->GetTime();
		_ctx->deltay = 480 - m_ptDrawPos.y;
		_ctx->destpt = m_ptDrawPos;
		m_ptDrawPos.Set(0, 480);

  	if (!m_nInList && bigBuf != NULL)
	  	bigBuf->AddPrim(new RMGfxPrimitive(this));
		m_bShow = true;

		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_0(MainWaitFrame);
			MainFreeze();
			_ctx->elaps = _vm->GetTime() - _ctx->starttime;
			m_ptDrawPos.y = 480 - ((_ctx->deltay * 100) / 700 * _ctx->elaps) / 100;
			MainUnfreeze();
		}

		m_ptDrawPos.y = _ctx->destpt.y;
	}

	CORO_END_CODE;
}

void RMDialogChoice::Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_bShow == false)
		return;

	prim->SetDst(m_ptDrawPos);
	CORO_INVOKE_2(RMGfxSourceBuffer16::Draw, bigBuf, prim);

	CORO_END_CODE;
}


void RMDialogChoice::Hide(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
		int deltay;
		int starttime;
		int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (1) {
		_ctx->starttime = _vm->GetTime();

		_ctx->deltay = 480 - m_ptDrawPos.y;
		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_0(MainWaitFrame);
			MainFreeze();
			_ctx->elaps = _vm->GetTime()-_ctx->starttime;
			m_ptDrawPos.y = 480 - ((_ctx->deltay * 100) / 700 * (700 - _ctx->elaps)) / 100;
			MainUnfreeze();
		}
	}

	m_bShow = false;
	bRemoveFromOT = true;
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, hUnreg, CORO_INFINITE);

	CORO_END_CODE;
}


void RMDialogChoice::RemoveThis(CORO_PARAM, bool &result) {
	result = bRemoveFromOT;
}

void RMDialogChoice::DoFrame(CORO_PARAM, RMPoint ptMousePos) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (ptMousePos.y > m_ptDrawPos.y) {		
		for (_ctx->i = 0; _ctx->i < m_numChoices; _ctx->i++) {
			if ((ptMousePos.y >= m_ptDrawPos.y+m_ptDrawStrings[_ctx->i].y) && (ptMousePos.y < m_ptDrawPos.y+m_ptDrawStrings[_ctx->i].y+m_drawedStrings[_ctx->i].Dimy())) {
				CORO_INVOKE_1(SetSelected, _ctx->i);
				break;
			}
		}

		if (_ctx->i == m_numChoices)
			CORO_INVOKE_1(SetSelected, -1);
	}

	CORO_END_CODE;
}

int RMDialogChoice::GetSelection(void) {
	return m_curSelection;
}

} // End of namespace Tony
