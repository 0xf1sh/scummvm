/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "akos.h"
#include "imuse.h"
#include "sound.h"

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct AkosHeader {
	byte unk_1[2];
	byte flags;
	byte unk_2;
	uint16 num_anims;
	uint16 unk_3;
	uint16 codec;
} GCC_PACK;

struct AkosOffset {
	uint32 akcd;
	uint16 akci;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif



enum AkosOpcodes {
	AKC_Return = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_CmdQue3 = 0xC015,
	AKC_ComplexChan = 0xC020,
	AKC_Jump = 0xC030,
	AKC_JumpIfSet = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_Ignore = 0xC050,
	AKC_IncVar = 0xC060,
	AKC_CmdQue3Quick = 0xC061,
	AKC_JumpStart = 0xC070,
	AKC_JumpE = 0xC070,
	AKC_JumpNE = 0xC071,
	AKC_JumpL = 0xC072,
	AKC_JumpLE = 0xC073,
	AKC_JumpG = 0xC074,
	AKC_JumpGE = 0xC075,
	AKC_StartAnim = 0xC080,
	AKC_StartVarAnim = 0xC081,
	AKC_Random = 0xC082,
	AKC_SetActorClip = 0xC083,
	AKC_StartAnimInActor = 0xC084,
	AKC_SetVarInActor = 0xC085,
	AKC_HideActor = 0xC086,
	AKC_SetDrawOffs = 0xC087,
	AKC_JumpTable = 0xC088,
	AKC_SoundStuff = 0xC089,
	AKC_Flip = 0xC08A,
	AKC_Cmd3 = 0xC08B,
	AKC_Ignore3 = 0xC08C,
	AKC_Ignore2 = 0xC08D,
	AKC_SkipStart = 0xC090,
	AKC_SkipE = 0xC090,
	AKC_SkipNE = 0xC091,
	AKC_SkipL = 0xC092,
	AKC_SkipLE = 0xC093,
	AKC_SkipG = 0xC094,
	AKC_SkipGE = 0xC095,
	AKC_ClearFlag = 0xC09F,
	AKC_EndSeq = 0xC0FF
};

bool Scumm::akos_hasManyDirections(Actor *a) {
	byte *akos;
	const AkosHeader *akhd;

	akos = getResourceAddress(rtCostume, a->costume);
	assert(akos);

	akhd = (const AkosHeader *)findResourceData(MKID('AKHD'), akos);
	return (akhd->flags & 2) != 0;
}

int Scumm::akos_frameToAnim(Actor *a, int frame) {
	if (akos_hasManyDirections(a))
		return toSimpleDir(1, a->facing) + frame * 8;
	else
		return newDirToOldDir(a->facing) + frame * 4;
}

void Scumm::akos_decodeData(Actor *a, int frame, uint usemask) {
	uint anim;
	const byte *akos, *r;
	const AkosHeader *akhd;
	uint offs;
	int i;
	byte code;
	uint16 start, len;
	uint16 mask;

	if (a->costume == 0)
		return;

	anim = akos_frameToAnim(a, frame);

	akos = getResourceAddress(rtCostume, a->costume);
	assert(akos);

	akhd = (const AkosHeader *)findResourceData(MKID('AKHD'), akos);

	if (anim >= READ_LE_UINT16(&akhd->num_anims))
		return;

	r = findResourceData(MKID('AKCH'), akos);
	assert(r);

	offs = READ_LE_UINT16(r + anim * sizeof(uint16));
	if (offs == 0)
		return;
	r += offs;

	i = 0;
	mask = READ_LE_UINT16(r);
	r += sizeof(uint16);
	do {
		if (mask & 0x8000) {
			code = *r++;
			if (usemask & 0x8000) {
				switch (code) {
				case 1:
					a->cost.active[i] = 0;
					a->cost.frame[i] = frame;
					a->cost.end[i] = 0;
					a->cost.start[i] = 0;
					a->cost.curpos[i] = 0;
					break;
				case 4:
					a->cost.stopped |= 1 << i;
					break;
				case 5:
					a->cost.stopped &= ~(1 << i);
					break;
				default:
					start = READ_LE_UINT16(r);
					len = READ_LE_UINT16(r + sizeof(uint16));
					r += sizeof(uint16) * 2;

					a->cost.active[i] = code;
					a->cost.frame[i] = frame;
					a->cost.end[i] = start + len;
					a->cost.start[i] = start;
					a->cost.curpos[i] = start;
					break;
				}
			} else {
				if (code != 1 && code != 4 && code != 5)
					r += sizeof(uint16) * 2;
			}
		}
		i++;
		mask <<= 1;
		usemask <<= 1;
	} while ((uint16)mask);
}

void AkosRenderer::setPalette(byte *new_palette) {
	const byte *the_akpl;
	uint size, i;

	the_akpl = _vm->findResourceData(MKID('AKPL'), akos);
	size = _vm->getResourceDataSize(akpl);

	if (size > 256)
		error("akos_setPalette: %d is too many colors", size);

	for (i = 0; i < size; i++) {
		palette[i] = new_palette[i] != 0xFF ? new_palette[i] : the_akpl[i];
	}

	if (size == 256) {
		byte color = new_palette[0];
		if (color == 255) {
			palette[0] = color;
		} else {
			_vm->_bompActorPalettePtr = palette;
		}
	}
}

void AkosRenderer::setCostume(int costume) {
	akos = _vm->getResourceAddress(rtCostume, costume);
	assert(akos);

	akhd = (const AkosHeader *) _vm->findResourceData(MKID('AKHD'), akos);
	akof = (const AkosOffset *) _vm->findResourceData(MKID('AKOF'), akos);
	akci = _vm->findResourceData(MKID('AKCI'), akos);
	aksq = _vm->findResourceData(MKID('AKSQ'), akos);
	akcd = _vm->findResourceData(MKID('AKCD'), akos);
	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	codec = READ_LE_UINT16(&akhd->codec);
}

void AkosRenderer::setFacing(Actor *a) {
	_mirror = (newDirToOldDir(a->facing) != 0 || akhd->flags & 1);
	if (a->flip)
		_mirror = !_mirror;
}

byte AkosRenderer::drawLimb(const CostumeData &cost, int limb) {
	uint code;
	const byte *p;
	const AkosOffset *off;
	const CostumeInfo *costumeInfo;
	uint i, extra;
	byte result = 0;

	if (!cost.active[limb] || cost.stopped & (1 << limb))
		return 0;

	p = aksq + cost.curpos[limb];

	code = p[0];
	if (code & 0x80)
		code = (code << 8) | p[1];

	if (code == AKC_Return || code == AKC_EndSeq)
		return 0;

	if (code != AKC_ComplexChan) {
		off = akof + (code & 0xFFF);

		assert((code & 0xFFF) * 6 < READ_BE_UINT32_UNALIGNED((const byte *)akof - 4) - 8);
		assert((code & 0x7000) == 0);

		_srcptr = akcd + READ_LE_UINT32(&off->akcd);
		costumeInfo = (const CostumeInfo *) (akci + READ_LE_UINT16(&off->akci));

		_width = READ_LE_UINT16(&costumeInfo->width);
		_height = READ_LE_UINT16(&costumeInfo->height);
		_xmoveCur = _xmove + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
		_ymoveCur = _ymove + (int16)READ_LE_UINT16(&costumeInfo->rel_y);
		_xmove += (int16)READ_LE_UINT16(&costumeInfo->move_x);
		_ymove -= (int16)READ_LE_UINT16(&costumeInfo->move_y);

		switch (codec) {
		case 1:
			result |= codec1();
			break;
		case 5:
			result |= codec5();
			break;
		case 16:
			result |= codec16();
			break;
		default:
			error("akos_drawCostumeChannel: invalid codec %d", codec);
		}
	} else {
		extra = p[2];
		p += 3;

		for (i = 0; i != extra; i++) {
			code = p[4];
			if (code & 0x80)
				code = ((code & 0xF) << 8) | p[5];
			off = akof + code;

			_srcptr = akcd + READ_LE_UINT32(&off->akcd);
			costumeInfo = (const CostumeInfo *) (akci + READ_LE_UINT16(&off->akci));

			_width = READ_LE_UINT16(&costumeInfo->width);
			_height = READ_LE_UINT16(&costumeInfo->height);

			_xmoveCur = _xmove + (int16)READ_LE_UINT16(p + 0);
			_ymoveCur = _ymove + (int16)READ_LE_UINT16(p + 2);

			p += (p[4] & 0x80) ? 6 : 5;

			switch (codec) {
			case 1:
				result |= codec1();
				break;
			case 5:
				result |= codec5();
				break;
			case 16:
				result |= codec16();
				break;
			default:
				error("akos_drawCostumeChannel: invalid codec %d", codec);
			}
		}
	}

	return result;
}

void AkosRenderer::codec1_genericDecode() {
	const byte *src;
	byte *dst;
	byte len, maskbit;
	uint y, color, height;
	const byte *scaleytab, *mask;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[v1.scaleYindex];
	maskbit = revBitMask[v1.x & 7];
	mask = v1.mask_ptr + (v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < _scaleY) {
				if (color && y < _outheight
						&& (!v1.mask_ptr || !((mask[0] | mask[v1.imgbufoffs]) & maskbit))) {
					*dst = palette[color];
				}
				mask += _numStrips;
				dst += _outwidth;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[v1.scaleYindex];

				if (v1.scaletable[v1.scaleXindex] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _vm->_screenWidth)
						return;
					maskbit = revBitMask[v1.x & 7];
					v1.destptr += v1.scaleXstep;
				}
				mask = v1.mask_ptr + (v1.x >> 3);
				v1.scaleXindex += v1.scaleXstep;
				dst = v1.destptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void AkosRenderer::codec1_spec1() {
	const byte *src;
	byte *dst;
	byte len, maskbit;
	uint y, color, height;
	byte pcolor;
	const byte *scaleytab, *mask;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[v1.scaleYindex];
	maskbit = revBitMask[v1.x & 7];
	mask = v1.mask_ptr + (v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < _scaleY) {
				if (color && y < _outheight
						&& (!v1.mask_ptr || !((mask[0] | mask[v1.imgbufoffs]) & maskbit))) {
					pcolor = palette[color];
					if (pcolor == 13)
						pcolor = _shadow_table[*dst];
					*dst = pcolor;
				}
				mask += _numStrips;
				dst += _outwidth;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[v1.scaleYindex];

				if (v1.scaletable[v1.scaleXindex] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _vm->_screenWidth)
						return;
					maskbit = revBitMask[v1.x & 7];
					v1.destptr += v1.scaleXstep;
				}
				mask = v1.mask_ptr + (v1.x >> 3);
				v1.scaleXindex += v1.scaleXstep;
				dst = v1.destptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void AkosRenderer::codec1_spec2() {
	warning("codec1_spec2"); // TODO
}

void AkosRenderer::codec1_spec3() {
	const byte *src;
	byte *dst;
	byte len, maskbit;
	uint y, color, height;
	uint pcolor;
	const byte *scaleytab, *mask;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[v1.scaleYindex];
	maskbit = revBitMask[v1.x & 7];
	mask = v1.mask_ptr + (v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (*scaleytab++ < _scaleY) {
				if (color && y < _outheight
						&& (!v1.mask_ptr || !((mask[0] | mask[v1.imgbufoffs]) & maskbit))) {
					pcolor = palette[color];
					if (pcolor < 8) {
						pcolor = (pcolor << 8) + *dst;
						*dst = _shadow_table[pcolor];
					} else {
						*dst = pcolor;
					}
				}
				mask += _numStrips;
				dst += _outwidth;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[v1.scaleYindex];

				if (v1.scaletable[v1.scaleXindex] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _vm->_screenWidth)
						return;
					maskbit = revBitMask[v1.x & 7];
					v1.destptr += v1.scaleXstep;
				}
				mask = v1.mask_ptr + (v1.x >> 3);
				v1.scaleXindex += v1.scaleXstep;
				dst = v1.destptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

#ifdef __PALM_OS__
const byte *default_scale_table;
#else
const byte default_scale_table[768] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};
#endif

byte AkosRenderer::codec1() {
	int num_colors;
	bool use_scaling;
	int i, j;
	int skip = 0, startScaleIndexX, tmp_y;
	int cur_x, x_right, x_left;
	int cur_y, y_top, y_bottom;
	bool y_clipping;
	bool charsetmask, masking;
	int step;
	byte drawFlag = 1;

	/* implement custom scale table */

	v1.scaletable = default_scale_table;

	// FIXME - which value for VAR_CUSTOMSCALETABLE in V8 ?
	if (_vm->VAR_CUSTOMSCALETABLE != 0xFF && _vm->isGlobInMemory(rtString, _vm->VAR(_vm->VAR_CUSTOMSCALETABLE))) {
		v1.scaletable = _vm->getStringAddressVar(_vm->VAR_CUSTOMSCALETABLE);
	}

	/* Setup color decoding variables */
	num_colors = _vm->getResourceDataSize(akpl);
	if (num_colors == 32) {
		v1.mask = (1 << 3) - 1;
		v1.shr = 3;
	} else if (num_colors == 64) {
		v1.mask = (1 << 2) - 1;
		v1.shr = 2;
	} else {
		v1.mask = (1 << 4) - 1;
		v1.shr = 4;
	}

	use_scaling = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	cur_x = _actorX;
	cur_y = _actorY;

	if (use_scaling) {

		/* Scale direction */
		v1.scaleXstep = -1;
		if (_xmoveCur < 0) {
			_xmoveCur = -_xmoveCur;
			v1.scaleXstep = 1;
		}

		if (_mirror) {
			/* Adjust X position */
			startScaleIndexX = 0x180 - _xmoveCur;
			j = startScaleIndexX;
			for (i = 0; i < _xmoveCur; i++) {
				if (v1.scaletable[j++] < _scaleX)
					cur_x -= v1.scaleXstep;
			}

			x_left = x_right = cur_x;

			j = startScaleIndexX;
			for (i = 0, skip = 0; i < _width; i++) {
				if (x_right < 0) {
					skip++;
					startScaleIndexX = j;
				}
				if (v1.scaletable[j++] < _scaleX)
					x_right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			startScaleIndexX = 0x180 + _xmoveCur;
			j = startScaleIndexX;
			for (i = 0; i < _xmoveCur; i++) {
				if (v1.scaletable[j++] < _scaleX)
					cur_x += v1.scaleXstep;
			}

			x_left = x_right = cur_x;

			j = startScaleIndexX;
			for (i = 0, skip = 0; i < _width; i++) {
				if (x_left >= (int)_outwidth) {
					startScaleIndexX = j;
					skip++;
				}
				if (v1.scaletable[j--] < _scaleX)
					x_left--;
			}
		}

		if (skip)
			skip--;

		step = -1;
		if (_ymoveCur < 0) {
			_ymoveCur = -_ymoveCur;
			step = -step;
		}

		tmp_y = 0x180 - _ymoveCur;
		for (i = 0; i < _ymoveCur; i++) {
			if (v1.scaletable[tmp_y++] < _scaleY)
				cur_y -= step;
		}

		y_top = y_bottom = cur_y;
		tmp_y = 0x180 - _ymoveCur;
		for (i = 0; i < _height; i++) {
			if (v1.scaletable[tmp_y++] < _scaleY)
				y_bottom++;
		}

		tmp_y = 0x180 - _ymoveCur;
	} else {
		if (!_mirror)
			_xmoveCur = -_xmoveCur;

		cur_x += _xmoveCur;
		cur_y += _ymoveCur;

		if (_mirror) {
			x_left = cur_x;
			x_right = cur_x + _width;
		} else {
			x_right = cur_x;
			x_left = cur_x - _width;
		}

		y_top = cur_y;
		y_bottom = cur_y + _height;

		startScaleIndexX = 0x180;
		tmp_y = 0x180;
	}

	v1.scaleXindex = startScaleIndexX;
	v1.scaleYindex = tmp_y;
	v1.skip_width = _width;

	v1.scaleXstep = -1;
	if (_mirror)
		v1.scaleXstep = -v1.scaleXstep;

	if ((int) y_top >= (int)_outheight || y_bottom <= 0)
		return 0;

	if ((int)x_left >= (int)_outwidth || x_right <= 0)
		return 1;

	v1.replen = 0;

	if (_mirror) {
		if (!use_scaling)
			skip = -cur_x;
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(skip);
			cur_x = 0;
		} else {
			skip = x_right - _outwidth;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	} else {
		if (!use_scaling)
			skip = x_right - _outwidth + 1;
		if (skip > 0) {
			v1.skip_width -= skip;
			codec1_ignorePakCols(skip);
			cur_x = _outwidth - 1;
		} else {
			skip = -1 - x_left;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	}

	v1.x = cur_x;
	v1.y = cur_y;

	if (v1.skip_width <= 0 || _height <= 0)
		return 0;

	_vm->updateDirtyRect(0, x_left, x_right, y_top, y_bottom, _dirty_id);

	y_clipping = ((uint) y_bottom > _outheight || y_top < 0);

	if ((uint) y_top > (uint) _outheight)
		y_top = 0;

	if ((uint) y_bottom > (uint) _outheight)
		y_bottom = _outheight;

	if (_draw_top > y_top)
		_draw_top = y_top;
	if (_draw_bottom < y_bottom)
		_draw_bottom = y_bottom;

	if (cur_x == -1)
		cur_x = 0;									/* ?? */

	v1.destptr = _outptr + cur_x + cur_y * _outwidth;

	charsetmask =
		_vm->hasCharsetMask(x_left, y_top + _vm->virtscr[0].topline, x_right,
												_vm->virtscr[0].topline + y_bottom);
	masking = false;
	if (_zbuf != 0) {
		masking = _vm->isMaskActiveAt(x_left, y_top, x_right, y_bottom,
										_vm->getResourceAddress(rtBuffer, 9) +
										_vm->gdi._imgBufOffs[_zbuf] + _vm->_screenStartStrip) != 0;
	}

	v1.mask_ptr = NULL;
	if (masking || charsetmask || _shadow_mode) {
		v1.mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + cur_y * _numStrips + _vm->_screenStartStrip;
		v1.imgbufoffs = _vm->gdi._imgBufOffs[_zbuf];
		if (!charsetmask && masking) {
			v1.mask_ptr += v1.imgbufoffs;
			v1.imgbufoffs = 0;
		}
	}

	switch (_shadow_mode) {
	case 1:
		codec1_spec1();
		break;
	case 2:
		codec1_spec2();
		break;
	case 3:
		codec1_spec3();
		break;
	default:
		codec1_genericDecode();
		break;
	}
	
	return drawFlag;
}


void AkosRenderer::codec1_ignorePakCols(int num) {
	int n;

	n = _height;
	if (num > 1)
		n *= num;

	do {
		v1.repcolor = *_srcptr++;
		v1.replen = v1.repcolor & v1.mask;
		if (v1.replen == 0) {
			v1.replen = *_srcptr++;
		}
		do {
			if (!--n) {
				v1.repcolor >>= v1.shr;
				return;
			}
		} while (--v1.replen);
	} while (1);
}

byte AkosRenderer::codec5() {
	int32 clip_left, clip_right, clip_top, clip_bottom, maxw, maxh, tmp_x, tmp_y;

	if (!_mirror) {
		clip_left = (_actorX - _xmoveCur - _width) + 1;
	} else {
		clip_left = _actorX + _xmoveCur - 1;
	}

	clip_right = (clip_left + _width) - 1;
	clip_top = _actorY + _ymoveCur;
	clip_bottom = (clip_top + _height) - 1;
	maxw = _outwidth - 1;
	maxh = _outheight - 1;

	if (clip_left < 0) {
		clip_left = 0;
	}

	tmp_x = clip_right - maxw;
	if (tmp_x > 0) {
		clip_right -= tmp_x;
	}

	tmp_y = clip_top;
	if (tmp_y < 0) {
		clip_top -= tmp_y;
	}

	tmp_y = clip_bottom - maxh;
	if (tmp_y > 0) {
		clip_bottom -= tmp_y;
	}

	if ((clip_right <= clip_left) || (clip_top >= clip_bottom))
		return 1;

	_vm->updateDirtyRect(0, clip_left, clip_right + 1, clip_top, clip_bottom + 1, _dirty_id);

	if (_draw_top > clip_top)
		_draw_top = clip_top;
	if (_draw_bottom < clip_bottom)
		_draw_bottom = clip_bottom + 1;

	BompDrawData bdd;

	bdd.srcwidth = _width;
	bdd.srcheight = _height;
	bdd.out = _outptr;
	bdd.outwidth = _outwidth;
	bdd.outheight = _outheight;
	bdd.dataptr = _srcptr;
	bdd.scale_x = 255;
	bdd.scale_y = 255;
	bdd.shadowMode = _shadow_mode;

	_vm->_bompScallingXPtr = NULL;
	_vm->_bompScallingYPtr = NULL;

	int decode_mode;

	if (!_mirror) {
		bdd.x = (_actorX - _xmoveCur - _width) + 1;
		decode_mode = 3;
	} else {
		bdd.x = _actorX + _xmoveCur;
		decode_mode = 1;
	}

	bdd.y = _actorY + _ymoveCur;

	if (_zbuf != 0) {
		_vm->_bompMaskPtr = _vm->getResourceAddress(rtBuffer, 9) + _vm->gdi._imgBufOffs[_zbuf];
		_vm->drawBomp(&bdd, decode_mode, 1);
	} else {
		_vm->drawBomp(&bdd, decode_mode, 0);
	}

	_vm->_bompActorPalettePtr = NULL;
	
	return 0;
}

void AkosRenderer::akos16SetupBitReader(const byte *src) {
	akos16.unk5 = 0;
	akos16.numbits = 16;
	akos16.mask = (1 << *src) - 1;
	akos16.shift = *(src);
	akos16.color = *(src + 1);
	akos16.bits = (*(src + 2) | *(src + 3) << 8);
	akos16.dataptr = src + 4;
}

void AkosRenderer::akos16PutOnScreen(byte *dest, const byte *src, byte transparency, int32 count) {
	byte tmp_data;

	if (count == 0)
		return;

	switch(_shadow_mode) {
	case 0:
		do {
			tmp_data = *(src++);
			if (tmp_data != transparency) {
				*(dest) = tmp_data;
			}
			dest++;
		} while (--count != 0);
		break;

	case 1:
		do {
			tmp_data = *(src++);
			if (tmp_data != transparency) {
				if (tmp_data == 13) {
					tmp_data = _shadow_table[*(dest)];
				}
				*(dest) = tmp_data;
			}
			dest++;
		} while (--count != 0);
		break;

	case 3:
		do {
			tmp_data = *(src++);
			if (tmp_data != transparency) {
				if (tmp_data < 8) {
					tmp_data = _shadow_table[*(dest) + (tmp_data << 8)];
				}
				*(dest) = tmp_data;
			}
			dest++;
		} while (--count != 0);
		break;
	}
}

#define AKOS16_FILL_BITS()                                        \
        if (akos16.numbits <= 8) {                                \
          akos16.bits |= (*akos16.dataptr++) << akos16.numbits;   \
          akos16.numbits += 8;                                    \
        }

#define AKOS16_EAT_BITS(n)                                        \
		akos16.numbits -= (n);                                    \
		akos16.bits >>= (n);


void AkosRenderer::akos16SkipData(int32 numbytes) {
	akos16DecodeLine(0, numbytes, 0);
}

void AkosRenderer::akos16DecodeLine(byte *buf, int32 numbytes, int32 dir) {
	uint16 bits, tmp_bits;

	while (numbytes != 0) {
		if (buf) {
			*buf = akos16.color;
			buf += dir;
		}
		
		if (akos16.unk5 == 0) {
			AKOS16_FILL_BITS()
			bits = akos16.bits & 3;
			if (bits & 1) {
				AKOS16_EAT_BITS(2)
				if (bits & 2) {
					tmp_bits = akos16.bits & 7;
					AKOS16_EAT_BITS(3)
					if (tmp_bits != 4) {
						akos16.color += (tmp_bits - 4);
					} else {
						akos16.unk5 = 1;
						AKOS16_FILL_BITS()
						akos16.unk6 = (akos16.bits & 0xff) - 1;
						AKOS16_EAT_BITS(8)
						AKOS16_FILL_BITS()
					}
				} else {
					AKOS16_FILL_BITS()
					akos16.color = ((byte)akos16.bits) & akos16.mask;
					AKOS16_EAT_BITS(akos16.shift)
					AKOS16_FILL_BITS()
				}
			} else {
				AKOS16_EAT_BITS(1);
			}
		} else {
			if (--akos16.unk6 == 0) {
				akos16.unk5 = 0;
			}
		}
		numbytes--;
	}
}

void AkosRenderer::akos16ApplyMask(byte *dest, byte *maskptr, byte bits, int32 count, byte fillwith) {
	byte tmp;
	byte tmp_data = *(maskptr++);
	byte bitpos = 1 << (7 - bits);

	if (count == 0)
		return;

	for(;;) {
		tmp = tmp_data;
		do {
			if (tmp & bitpos) {
				*(dest) = fillwith;
			}
			dest++;

			if (--count == 0)
				return;
		} while ((bitpos>>=1) != 0);

		bitpos = 0x80;
		tmp_data = *(maskptr++);
	}
}

void AkosRenderer::akos16Decompress(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency) {
	byte *tmp_buf = akos16.buffer;

	if (dir < 0) {
		dest -= (t_width - 1);
		tmp_buf += (t_width - 1);
	}

	akos16SetupBitReader(src);

	if (numskip_before != 0) {
		akos16SkipData(numskip_before);
	}

	while (t_height != 0) {
		akos16DecodeLine(tmp_buf, t_width, dir);
		akos16PutOnScreen(dest, akos16.buffer, transparency, t_width);

		if (numskip_after != 0)	{
			akos16SkipData(numskip_after);
		}
		dest += pitch;

		t_height--;
	}
}

void AkosRenderer::akos16DecompressMask(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, byte * maskptr, int32 bitpos_start) {
	byte *tmp_buf = akos16.buffer;
	int maskpitch;

	if (dir < 0) {
		dest -= (t_width - 1);
		tmp_buf += (t_width - 1);
	}

	akos16SetupBitReader(src);

	if (numskip_before != 0) {
		akos16SkipData(numskip_before);
	}

	maskpitch = _numStrips ;

	while (t_height != 0) {
		akos16DecodeLine(tmp_buf, t_width, dir);
		akos16ApplyMask(akos16.buffer, maskptr, (byte)bitpos_start, t_width, transparency);
		akos16PutOnScreen(dest, akos16.buffer, transparency, t_width);

		if (numskip_after != 0)	{
			akos16SkipData(numskip_after);
		}
		dest += pitch;
		maskptr += maskpitch;

		t_height--;
	}
}

byte AkosRenderer::codec16() {
	int32 clip_left;

	if(!_mirror) {
		clip_left = (_actorX - _xmoveCur - _width) + 1;
	} else {
		clip_left = _actorX + _xmoveCur;
	}

	int32 clip_top = _ymoveCur + _actorY;
	int32 clip_right = (clip_left + _width) - 1;
	int32 clip_bottom = (clip_top + _height) - 1;
	int32 skip_x = 0;
	int32 skip_y = 0;
	int32 cur_x = _width - 1;
	int32 cur_y = _height - 1;
	int32 maxw = _outwidth - 1;
	int32 maxh = _outheight - 1;
	int32 tmp_x, tmp_y;
	byte transparency = (_vm->_features & GF_HUMONGOUS) ? 0 : 255;

/*
	tmp_x = clip_left;
	if(tmp_x < 0) {
		tmp_x = -tmp_x;
		clip_left -= tmp_x;
		skip_x = tmp_x;
	}
*/

	// Modified by ludde
	if (clip_left < 0) {
		skip_x = -clip_left;
		clip_left = 0;
	}

	tmp_x = clip_right - maxw;
	if(tmp_x > 0) {
		cur_x -= tmp_x;
		clip_right -= tmp_x;
	}

	tmp_y = clip_top;
	if(tmp_y < 0) {
		skip_y -= tmp_y;
		clip_top -= tmp_y;
	}

	tmp_y = clip_bottom - maxh;
	if(tmp_y > 0) {
		cur_y -= tmp_y;
		clip_bottom -= tmp_y;
	}

	if ((clip_left >= clip_right) || (clip_top >= clip_bottom))
		return 0;

	_vm->updateDirtyRect(0, clip_left, clip_right + 1, clip_top, clip_bottom + 1, _dirty_id);

	if (_draw_top > clip_top)
		_draw_top = clip_top;
	if (_draw_bottom < clip_bottom)
		_draw_bottom = clip_bottom + 1;

	int32 width_unk, height_unk;

	height_unk = clip_top;
	int32 pitch = _vm->_screenWidth;

	int32 dir;

	if (!_mirror) {
		dir = -1;

		int tmp_skip_x = skip_x;
		skip_x = _width - 1 - cur_x;
		cur_x = _width - 1 - tmp_skip_x;
		width_unk = clip_right;
	} else {
		dir = 1;
		width_unk = clip_left;
	}

	tmp_y = cur_y - skip_y;
	if(tmp_y < 0) {
		tmp_y = -tmp_y;
	}

	int32 out_height = tmp_y + 1;

	cur_x -= skip_x;
	if(cur_x < 0) {
		cur_x = -cur_x;
	}

	cur_x++;

	int32 numskip_before = skip_x + (skip_y * _width);
	int32 numskip_after = _width - cur_x;

	byte *dest = _outptr + width_unk + height_unk * _outwidth;

	if (_zbuf == 0) {
		akos16Decompress(dest, pitch, _srcptr, cur_x, out_height, dir, numskip_before, numskip_after, transparency);
		return 0;
	}

	byte *ptr = _vm->_screenStartStrip + _vm->getResourceAddress(rtBuffer, 9) + _vm->gdi._imgBufOffs[_zbuf];
	ptr += _numStrips * clip_top + (clip_left / 8);
	akos16DecompressMask(dest, pitch, _srcptr, cur_x, out_height, dir, numskip_before, numskip_after, transparency, ptr, clip_left / 8);

	return 0;
}

bool Scumm::akos_increaseAnims(const byte *akos, Actor *a) {
	const byte *aksq, *akfo;
	int i;
	uint size;
	bool result;

	aksq = findResourceData(MKID('AKSQ'), akos);
	akfo = findResourceData(MKID('AKFO'), akos);

	size = getResourceDataSize(akfo) >> 1;

	result = false;
	for (i = 0; i < 16; i++) {
		if (a->cost.active[i] != 0)
			result |= akos_increaseAnim(a, i, aksq, (const uint16 *)akfo, size);
	}
	return result;
}

#define GW(o) ((int16)READ_LE_UINT16(aksq+curpos+(o)))
#define GUW(o) READ_LE_UINT16(aksq+curpos+(o))
#define GB(o) aksq[curpos+(o)]

bool Scumm::akos_increaseAnim(Actor *a, int chan, const byte *aksq, const uint16 *akfo, int numakfo) {
	byte active;
	uint old_curpos, curpos, end;
	uint code;
	bool flag_value;
	int tmp, tmp2;

	active = a->cost.active[chan];
	end = a->cost.end[chan];
	old_curpos = curpos = a->cost.curpos[chan];
	flag_value = false;

	do {

		code = aksq[curpos];
		if (code & 0x80)
			code = (code << 8) | aksq[curpos + 1];

		switch (active) {
		case 6:
			switch (code) {
			case AKC_JumpIfSet:
			case AKC_AddVar:
			case AKC_SetVar:
			case AKC_SkipGE:
			case AKC_SkipG:
			case AKC_SkipLE:
			case AKC_SkipL:

			case AKC_SkipNE:
			case AKC_SkipE:
				curpos += 5;
				break;
			case AKC_JumpTable:
			case AKC_SetActorClip:
			case AKC_Ignore3:
			case AKC_Ignore2:
			case AKC_Ignore:
			case AKC_StartAnim:
			case AKC_StartVarAnim:
			case AKC_CmdQue3:
				curpos += 3;
				break;
			case AKC_SoundStuff:
				curpos += 8;		// in Putt is 6
				break;
			case AKC_Cmd3:
			case AKC_SetVarInActor:
			case AKC_SetDrawOffs:
				curpos += 6;
				break;
			case AKC_ClearFlag:
			case AKC_HideActor:
			case AKC_IncVar:
			case AKC_CmdQue3Quick:
			case AKC_Return:
			case AKC_EndSeq:
				curpos += 2;
				break;
			case AKC_JumpGE:
			case AKC_JumpG:
			case AKC_JumpLE:
			case AKC_JumpL:
			case AKC_JumpNE:
			case AKC_JumpE:
			case AKC_Random:
				curpos += 7;
				break;
			case AKC_Flip:
			case AKC_Jump:
			case AKC_StartAnimInActor:
				curpos += 4;
				break;
			case AKC_ComplexChan:
				curpos += 3;
				tmp = aksq[curpos - 1];
				while (--tmp >= 0) {
					curpos += 4;
					curpos += (aksq[curpos] & 0x80) ? 2 : 1;
				}
				break;
			default:
				if ((code & 0xC000) == 0xC000)
					error("akos_increaseAnim: invalid code %x", code);
				curpos += (code & 0x8000) ? 2 : 1;
			}
			break;
		case 2:
			curpos += (code & 0x8000) ? 2 : 1;
			if (curpos > end)
				curpos = a->cost.start[chan];
			break;
		case 3:
			if (curpos != end)
				curpos += (code & 0x8000) ? 2 : 1;
			break;
		}

		code = aksq[curpos];
		if (code & 0x80)
			code = (code << 8) | aksq[curpos + 1];

		if (flag_value && code != AKC_ClearFlag)
			continue;

		switch (code) {
		case AKC_StartAnimInActor:
			akos_queCommand(4, derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:29"), a->getAnimVar(GB(3)), 0);
			continue;

		case AKC_Random:
			a->setAnimVar(GB(6), _rnd.getRandomNumberRng(GW(2), GW(4)));
			continue;
		case AKC_JumpGE:
		case AKC_JumpG:
		case AKC_JumpLE:
		case AKC_JumpL:
		case AKC_JumpNE:
		case AKC_JumpE:
			if (akos_compare(a->getAnimVar(GB(4)), GW(5), code - AKC_JumpStart) != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_IncVar:
			a->setAnimVar(0, a->getAnimVar(0) + 1);
			continue;
		case AKC_SetVar:
			a->setAnimVar(GB(4), GW(2));
			continue;
		case AKC_AddVar:
			a->setAnimVar(GB(4), a->getAnimVar(GB(4)) + GW(2));
			continue;
		case AKC_Flip:
			a->flip = GW(2) != 0;
			continue;
		case AKC_CmdQue3:
//			tmp = GB(2);	// previous
			tmp = GB(2) - 1;
			if ((uint) tmp < 8)
				akos_queCommand(3, a, a->sound[tmp], 0);
			continue;
		case AKC_CmdQue3Quick:
//			akos_queCommand(3, a, a->sound[1], 0);	//previous
			akos_queCommand(3, a, a->sound[0], 0);
			continue;
		case AKC_StartAnim:
			akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			akos_queCommand(4, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_SetVarInActor:
			derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:9")->setAnimVar(GB(3), GW(4));
			continue;
		case AKC_HideActor:
			akos_queCommand(1, a, 0, 0);
			continue;
		case AKC_SetActorClip:
			akos_queCommand(5, a, GB(2), 0);
			continue;
		case AKC_SoundStuff:
			tmp = GB(2);
			if (tmp >= 8)
				continue;
			tmp2 = GB(4);
			if (tmp2 < 1 || tmp2 > 3)
				error("akos_increaseAnim:8 invalid code %d", tmp2);
			akos_queCommand(tmp2 + 6, a, a->sound[tmp], GB(6));
			continue;
		case AKC_SetDrawOffs:
			akos_queCommand(6, a, GW(2), GW(4));
			continue;
		case AKC_JumpTable:
			if (akfo == NULL)
				error("akos_increaseAnim: no AKFO table");
			tmp = a->getAnimVar(GB(2)) - 1;
			if (tmp < 0 || tmp >= numakfo - 1)
				error("akos_increaseAnim: invalid jump value %d", tmp);
			curpos = READ_LE_UINT16(&akfo[tmp]);
			break;
		case AKC_JumpIfSet:
			if (!a->getAnimVar(GB(4)))
				continue;
			a->setAnimVar(GB(4), 0);
			curpos = GUW(2);
			break;

		case AKC_ClearFlag:
			flag_value = false;
			continue;

		case AKC_Jump:
			curpos = GUW(2);
			break;

		case AKC_Return:
		case AKC_EndSeq:
		case AKC_ComplexChan:
			break;

		case AKC_Ignore:
		case AKC_Ignore2:
		case AKC_Ignore3:
			continue;

		case AKC_SkipE:
		case AKC_SkipNE:
		case AKC_SkipL:
		case AKC_SkipLE:
		case AKC_SkipG:
		case AKC_SkipGE:
			if (akos_compare(a->getAnimVar(GB(4)), GW(2), code - AKC_SkipStart) == 0)
				flag_value = true;
			continue;

		default:
			if ((code & 0xC000) == 0xC000)
				error("Undefined uSweat token %X", code);
		}
		break;
	} while (1);

	int code2 = aksq[curpos];
	if (code2 & 0x80)
		code2 = (code2 << 8) | aksq[curpos + 1];
	assert((code2 & 0xC000) != 0xC000 || code2 == AKC_ComplexChan || code2 == AKC_Return || code2 == AKC_EndSeq);

	a->cost.curpos[chan] = curpos;

	return curpos != old_curpos;
}

void Scumm::akos_queCommand(byte cmd, Actor *a, int param_1, int param_2) {
	switch (cmd) {
	case 1:
		a->putActor(0, 0, 0);
		break;
	case 2:
		warning("unimplemented akos_queCommand(2,%d,%d,%d)", a->number, param_1, param_2);
		// start script token in actor
		break;
	case 3:
		if (param_1 != 0) {
			_sound->addSoundToQueue(param_1);
		}
		break;
	case 4:
		a->startAnimActor(param_1);
		// param_2 ?
		break;
	case 5:
		a->forceClip = param_1;
		break;
	case 6:
		warning("unimplemented akos_queCommand(6,%d,%d,%d)", a->number, param_1, param_2);
//		a->offs_x = param_1;
//		a->offs_y = param_2;
		break;
	case 7:
		if (param_1 != 0) {
			if (_imuseDigital) {
//				_imuseDigital->doCommand(12, 0x600, param_1, 0, 0, 0, 0, 0);
			}
		}
		break;
	case 8:
		if (param_1 != 0) {
			if (_imuseDigital) {
//				_imuseDigital->doCommand(12, 0x700, param_1, 0, 0, 0, 0, 0);
			}
		}
		break;
	case 9:
		if (param_1 != 0) {
			if (_imuseDigital) {
//				_imuseDigital->doCommand(12, 0x500, param_1, 0, 0, 0, 0, 0);
			}
		}
		break;
	default:
		warning("akos_queCommand(%d,%d,%d,%d)", cmd, a->number, param_1, param_2);
	}
}

bool Scumm::akos_compare(int a, int b, byte cmd) {
	switch (cmd) {
	case 0:
		return a == b;
	case 1:
		return a != b;
	case 2:
		return a < b;
	case 3:
		return a <= b;
	case 4:
		return a > b;
	default:
		return a >= b;
	}
}

#ifdef __PALM_OS__
#include "scumm_globals.h" // init globals
void Akos_initGlobals()		{	
	GSETPTR(default_scale_table, GBVARS_DEFAULTSCALETABLE_INDEX, byte, GBVARS_SCUMM)
}
void Akos_releaseGlobals()	{
	GRELEASEPTR(GBVARS_DEFAULTSCALETABLE_INDEX, GBVARS_SCUMM)
}
#endif
