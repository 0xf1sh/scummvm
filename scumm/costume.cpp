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
#include "costume.h"
#include "scumm/sound.h"

const byte revBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void CostumeRenderer::ignorePakCols(int num) {
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

const byte cost_scaleTable[256] = {
	255, 253, 125, 189, 61, 221, 93, 157, 29, 237,
	109, 173, 45, 205, 77, 141, 13, 245, 117, 181,
	53, 213, 85, 149, 21, 229, 101, 165, 37, 197, 69,
	133, 5, 249, 121, 185, 57, 217, 89, 153, 25, 233,
	105, 169, 41, 201, 73, 137, 9, 241, 113, 177, 49,
	209, 81, 145, 17, 225, 97, 161, 33, 193, 65, 129,
	1, 251, 123, 187, 59, 219, 91, 155, 27, 235, 107,
	171, 43, 203, 75, 139, 11, 243, 115, 179, 51, 211,
	83, 147, 19, 227, 99, 163, 35, 195, 67, 131, 3,
	247, 119, 183, 55, 215, 87, 151, 23, 231, 103,
	167, 39, 199, 71, 135, 7, 239, 111, 175, 47, 207,
	79, 143, 15, 223, 95, 159, 31, 191, 63, 127, 0,
	128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208,
	48, 176, 112, 240, 8, 136, 72, 200, 40, 168, 104,
	232, 24, 152, 88, 216, 56, 184, 120, 248, 4, 132,
	68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52,
	180, 116, 244, 12, 140, 76, 204, 44, 172, 108,
	236, 28, 156, 92, 220, 60, 188, 124, 252, 2, 130,
	66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50,
	178, 114, 242, 10, 138, 74, 202, 42, 170, 106,
	234, 26, 154, 90, 218, 58, 186, 122, 250, 6, 134,
	70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54,
	182, 118, 246, 14, 142, 78, 206, 46, 174, 110,
	238, 30, 158, 94, 222, 62, 190, 126, 254
};

byte CostumeRenderer::mainRoutine() {
	int xmoveCur, ymoveCur, i, skip;
	byte drawFlag = 1;
	uint scal;
	bool use_scaling;
	byte startScaleIndexX;
	int ex1, ex2;
	int y_top, y_bottom;
	int x_left, x_right;
	int step;
	const CostumeInfo *costumeInfo;
	
	CHECK_HEAP
	v1.mask = 0xF;
	v1.shr = 4;
	if (_loaded._numColors == 32) {
		v1.mask = 7;
		v1.shr = 3;
	}

	// FIXME: those are here just in case... you never now...
	assert(_srcptr[1] == 0);
	assert(_srcptr[3] == 0);

	costumeInfo = (const CostumeInfo *)_srcptr;
	_width = _width2 = READ_LE_UINT16(&costumeInfo->width);
	_height = READ_LE_UINT16(&costumeInfo->height);
	xmoveCur = _xmove + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
	ymoveCur = _ymove + (int16)READ_LE_UINT16(&costumeInfo->rel_y);
	_xmove += (int16)READ_LE_UINT16(&costumeInfo->move_x);
	_ymove -= (int16)READ_LE_UINT16(&costumeInfo->move_y);
	_srcptr += 12;

	switch (_loaded._ptr[7] & 0x7F) {
	case 0x60:
	case 0x61:
		ex1 = _srcptr[0];
		ex2 = _srcptr[1];
		_srcptr += 2;
		if (ex1 != 0xFF || ex2 != 0xFF) {
			ex1 = READ_LE_UINT16(_loaded._ptr + _loaded._numColors + 10 + ex1 * 2);
			_srcptr = _loaded._baseptr + READ_LE_UINT16(_loaded._ptr + ex1 + ex2 * 2) + 14;
		}
	}

	v1.x = _actorX;
	v1.y = _actorY;

	use_scaling = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	skip = 0;

	if (use_scaling) {
		v1.scaleXstep = -1;
		if (xmoveCur < 0) {
			xmoveCur = -xmoveCur;
			v1.scaleXstep = 1;
		}

		if (_mirror) {
			startScaleIndexX = _scaleIndexX = 128 - xmoveCur;
			for (i = 0; i < xmoveCur; i++) {
				if (cost_scaleTable[_scaleIndexX++] < _scaleX)
					v1.x -= v1.scaleXstep;
			}
			x_right = x_left = v1.x;
			_scaleIndexX = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (x_right < 0) {
					skip++;
					startScaleIndexX = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX++];
				if (scal < _scaleX)
					x_right++;
			}
		} else {
			startScaleIndexX = _scaleIndexX = xmoveCur + 128;
			for (i = 0; i < xmoveCur; i++) {
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					v1.x += v1.scaleXstep;
			}
			x_right = x_left = v1.x;
			_scaleIndexX = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (x_left > (_vm->_screenWidth - 1)) {
					skip++;
					startScaleIndexX = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					x_left--;
			}
		}
		_scaleIndexX = startScaleIndexX;
		if (skip)
			skip--;

		step = -1;
		if (ymoveCur < 0) {
			ymoveCur = -ymoveCur;
			step = 1;
		}
		_scaleIndexY = 128 - ymoveCur;
		for (i = 0; i < ymoveCur; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				v1.y -= step;
		}
		y_top = y_bottom = v1.y;
		_scaleIndexY = 128 - ymoveCur;
		for (i = 0; i < _height; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				y_bottom++;
		}
		_scaleIndexY = _scaleIndexYTop = 128 - ymoveCur;
	} else {
		if (!_mirror)
			xmoveCur = -xmoveCur;
		v1.x += xmoveCur;
		v1.y += ymoveCur;
		if (_mirror) {
			x_left = v1.x;
			x_right = v1.x + _width;
		} else {
			x_left = v1.x - _width;
			x_right = v1.x;
		}
		y_top = v1.y;
		y_bottom = y_top + _height;
	}

	v1.scaleXstep = -1;
	if (_mirror)
		v1.scaleXstep = 1;

	_vm->updateDirtyRect(0, x_left, x_right + 1, y_top, y_bottom, _dirty_id);

	if (y_top >= (int)_outheight || y_bottom <= 0)
		return 0;

	_docontinue = 0;
	if (x_left >= _vm->_screenWidth || x_right <= 0)
		return 1;

	if (_mirror) {
		if (!use_scaling)
			skip = -v1.x;
		if (skip > 0) {
			_width2 -= skip;
			ignorePakCols(skip);
			v1.x = 0;
			_docontinue = 1;
		} else {
			skip = x_right - _vm->_screenWidth;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				_width2 -= skip;
			}
		}
	} else {
		if (!use_scaling)
			skip = x_right - _vm->_screenWidth;
		if (skip > 0) {
			_width2 -= skip;
			ignorePakCols(skip);
			v1.x = _vm->_screenWidth - 1;
			_docontinue = 1;
		} else {
			skip = -1 - x_left;
			if (skip <= 0)
				drawFlag = 2;
			else
				_width2 -= skip;
		}
	}

	if (_width2 == 0)
		return 0;

	if ((uint) y_top > (uint) _outheight)
		y_top = 0;

	if (x_left < 0)
		x_left = 0;

	if ((uint) y_bottom > _outheight)
		y_bottom = _outheight;

	if (_draw_top > y_top)
		_draw_top = y_top;
	if (_draw_bottom < y_bottom)
		_draw_bottom = y_bottom;

	if (_height + y_top >= 256) {
		CHECK_HEAP
		return 2;
	}

	v1.destptr = _outptr + v1.y * _vm->_screenWidth + v1.x;

	v1.mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + v1.y * _numStrips + _vm->_screenStartStrip;
	v1.imgbufoffs = _vm->gdi._imgBufOffs[_zbuf];

	// FIXME: Masking used to be conditional. Will it cause regressions
	// to always do it? I don't think so, since the behaviour used to be
	// to mask if there was a mask to apply, unless _zbuf is 0.
	//
	// However, when _zbuf is 0 masking and charset masking are the same
	// thing. I believe the only thing that is ever written to the
	// frontmost mask buffer is the charset mask, except in Sam & Max
	// where it's also used for the inventory box and conversation icons.

	_use_mask = true;
	_use_charset_mask = true;

	CHECK_HEAP

	if (_vm->_features & GF_AMIGA)
		proc3_ami();
	else
		proc3();

	CHECK_HEAP
	return drawFlag;
}

void CostumeRenderer::proc3() {
	const byte *mask, *src;
	byte *dst;
	byte maskbit, len, height, pcolor, width;
	int color;
	uint y;
	bool masked;

	mask = v1.mask_ptr + (v1.x >> 3);
	maskbit = revBitMask[v1.x & 7];
	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.repcolor;
	color = v1.repcolor;
	height = _height;
	width = _width2;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || cost_scaleTable[_scaleIndexY++] < _scaleY) {
				masked = (_use_mask && (mask[v1.imgbufoffs] & maskbit)) || (_use_charset_mask && (mask[0] & maskbit));
				
				if (color && y < _outheight && !masked) {
					// FIXME: Fully implement _shadow_mode.
					// For now, it's enough for Sam & Max
					// transparency.
					if (_shadow_mode & 0x20) {
						pcolor = _vm->_proc_special_palette[*dst];
					} else {
						pcolor = _palette[color];
						if (pcolor == 13 && _shadow_table)
							pcolor = _shadow_table[*dst];
					}
					*dst = pcolor;
				}
				dst += _vm->_screenWidth;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--width)
					return;
				height = _height;
				y = v1.y;

				_scaleIndexY = _scaleIndexYTop;
				if (_scaleX == 255 || cost_scaleTable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _vm->_screenWidth)
						return;
					maskbit = revBitMask[v1.x & 7];
					v1.destptr += v1.scaleXstep;
				}
				_scaleIndexX += v1.scaleXstep;
				dst = v1.destptr;
				mask = v1.mask_ptr + (v1.x >> 3);
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc3_ami() {
	const byte *mask, *src;
	byte *dst;
	byte maskbit, len, height, width;
	int color;
	uint y;
	bool masked;
	int oldXpos, oldScaleIndexX;

	mask = v1.mask_ptr + (v1.x >> 3);
	dst = v1.destptr;
	height = _height;
	width = _width;
	src = _srcptr;
	maskbit = revBitMask[v1.x & 7];
	y = v1.y;
	oldXpos = v1.x;
	oldScaleIndexX = _scaleIndexX;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;
		do {
			if (_scaleY == 255 || cost_scaleTable[_scaleIndexY] < _scaleY) {
				masked = (_use_mask && (mask[v1.imgbufoffs] & maskbit)) || (_use_charset_mask && (mask[0] & maskbit));
				
				if (color && v1.x >= 0 && v1.x < _vm->_screenWidth && !masked) {
					*dst = _palette[color];
				}

				if (_scaleX == 255 || cost_scaleTable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					dst += v1.scaleXstep;
					maskbit = revBitMask[v1.x & 7];
				}
				_scaleIndexX += v1.scaleXstep;
				mask = v1.mask_ptr + (v1.x >> 3);
			}
			if (!--width) {
				if (!--height)
					return;

				if (y >= _outheight)
					return;

				if (v1.x != oldXpos) {
					dst += _vm->_screenWidth - (v1.x - oldXpos);
					v1.mask_ptr += _numStrips;
					mask = v1.mask_ptr;
					y++;
				}
				width = _width;
				v1.x = oldXpos;
				_scaleIndexX = oldScaleIndexX;
				_scaleIndexY++;
			}
		} while (--len);
	} while (1);
}

void LoadedCostume::loadCostume(int id) {
	_ptr = _vm->getResourceAddress(rtCostume, id);

	if (_vm->_features & GF_AFTER_V6)
		_ptr += 8;
	else if (_vm->_features & GF_OLD_BUNDLE)
		_ptr += -2;
	else if (_vm->_features & GF_SMALL_HEADER)
		_ptr += 0;
	else
		_ptr += 2;

	_baseptr = _ptr;

	switch (_ptr[7] & 0x7F) {
	case 0x58:
		_numColors = 16;
		break;
	case 0x59:
		_numColors = 32;
		break;
	case 0x60:										/* New since version 6 */
		_numColors = 16;
		break;
	case 0x61:										/* New since version 6 */
		_numColors = 32;
		break;
	default:
		error("Costume %d is invalid", id);
	}
	
	// In GF_OLD_BUNDLE games, there is no actual palette, just a single color byte. 
	// Don't forget, these games were designed around a fixed 16 color HW palette :-)
	// In addition, all offsets are shifted by 2; we accomodate that via a seperate
	// _baseptr value (instead of adding tons of if's throughout the code).
	if (_vm->_features & GF_OLD_BUNDLE) {
		_numColors = 1;
		_baseptr += 2;
	}
	_dataptr = _baseptr + READ_LE_UINT16(_ptr + _numColors + 8);
}

byte CostumeRenderer::drawLimb(const CostumeData &cost, int limb) {
	int i;
	int code;
	const byte *frameptr;

	// If the specified limb is stopped or not existing, do nothing.
	if (cost.curpos[limb] == 0xFFFF || cost.stopped & (1 << limb))
		return 0;

	// Determine the position the limb is at
	i = cost.curpos[limb] & 0x7FFF;
	
	// Get the base pointer for that limb
	frameptr = _loaded._baseptr + READ_LE_UINT16(_loaded._ptr + _loaded._numColors + limb * 2 + 10);
	
	// Determine the offset to the costume data for the limb at position i
	code = _loaded._dataptr[i] & 0x7F;

	// Code 0x7B indicates a limb for which there is nothing to draw
	if (code != 0x7B) {
		_srcptr = _loaded._baseptr + READ_LE_UINT16(frameptr + code * 2);
		if (!(_vm->_features & GF_OLD256) || code < 0x79)
			return mainRoutine();
	}

	return 0;

}

int Scumm::cost_frameToAnim(Actor *a, int frame) {
	return newDirToOldDir(a->facing) + frame * 4;
}

void Scumm::cost_decodeData(Actor *a, int frame, uint usemask) {
	const byte *r;
	uint mask, j;
	int i;
	byte extra, cmd;
	const byte *dataptr;
	int anim;
	LoadedCostume lc(this);

	lc.loadCostume(a->costume);

	anim = cost_frameToAnim(a, frame);

	if (anim > lc._ptr[6]) {
		return;
	}

	r = lc._baseptr + READ_LE_UINT16(lc._ptr + anim * 2 + lc._numColors + 42);

	if (r == lc._baseptr) {
		return;
	}

	dataptr = lc._dataptr;
	mask = READ_LE_UINT16(r);
	r += 2;
	i = 0;
	do {
		if (mask & 0x8000) {
			if ((_features & GF_AFTER_V3) || (_features & GF_AFTER_V2)) {
				j = *r++;

				if (j == 0xFF)
					j = 0xFFFF;
			} else {
				j = READ_LE_UINT16(r);
				r += 2;
			}
			if (usemask & 0x8000) {
				if (j == 0xFFFF) {
					a->cost.curpos[i] = 0xFFFF;
					a->cost.start[i] = 0;
					a->cost.frame[i] = frame;
				} else {
					extra = *r++;
					cmd = dataptr[j];
					if (cmd == 0x7A) {
						a->cost.stopped &= ~(1 << i);
					} else if (cmd == 0x79) {
						a->cost.stopped |= (1 << i);
					} else {
						a->cost.curpos[i] = a->cost.start[i] = j;
						a->cost.end[i] = j + (extra & 0x7F);
						if (extra & 0x80)
							a->cost.curpos[i] |= 0x8000;
						a->cost.frame[i] = frame;
					}
				}
			} else {
				if (j != 0xFFFF)
					r++;
			}
		}
		i++;
		usemask <<= 1;
		mask <<= 1;
	} while ((uint16)mask);
}

void CostumeRenderer::setPalette(byte *palette) {
	int i;
	byte color;

	if (_vm->_features & GF_OLD_BUNDLE) {
		if ((_vm->VAR(_vm->VAR_CURRENT_LIGHTS) & LIGHTMODE_actor_color)) {
			memcpy(_palette, palette, 16);
		} else {
			memset(_palette, 8, 16);
			_palette[12] = 0;
		}
		_palette[_loaded._ptr[8]] = _palette[0];
	} else {
		if ((_vm->_features & GF_AFTER_V6) || (_vm->VAR(_vm->VAR_CURRENT_LIGHTS) & LIGHTMODE_actor_color)) {
			for (i = 0; i < _loaded._numColors; i++) {
				color = palette[i];
				if (color == 255)
					color = _loaded._ptr[8 + i];
				_palette[i] = color;
			}
		} else {
			memset(_palette, 8, _loaded._numColors);
			_palette[12] = 0;
		}
	}
}

void CostumeRenderer::setFacing(Actor *a) {
	_mirror = newDirToOldDir(a->facing) != 0 || (_loaded._ptr[7] & 0x80);
}

void CostumeRenderer::setCostume(int costume) {
	_loaded.loadCostume(costume);
}

byte LoadedCostume::increaseAnims(Actor *a) {
	int i;
	byte r = 0;

	for (i = 0; i != 16; i++) {
		if (a->cost.curpos[i] != 0xFFFF)
			r += increaseAnim(a, i);
	}
	return r;
}

byte LoadedCostume::increaseAnim(Actor *a, int slot) {
	int highflag;
	int i, end;
	byte code, nc;

	if (a->cost.curpos[slot] == 0xFFFF)
		return 0;

	highflag = a->cost.curpos[slot] & 0x8000;
	i = a->cost.curpos[slot] & 0x7FFF;
	end = a->cost.end[slot];
	code = _dataptr[i] & 0x7F;

	do {
		if (!highflag) {
			if (i++ >= end)
				i = a->cost.start[slot];
		} else {
			if (i != end)
				i++;
		}
		nc = _dataptr[i];

		if (nc == 0x7C) {
			a->cost.animCounter1++;
			if (a->cost.start[slot] != end)
				continue;
		} else {
			if (_vm->_features & GF_AFTER_V6) {
				if (nc >= 0x71 && nc <= 0x78) {
					_vm->_sound->addSoundToQueue2(a->sound[nc - 0x71]);
					if (a->cost.start[slot] != end)
						continue;
				}
			} else {
				if (nc == 0x78) {
					a->cost.animCounter2++;
					if (a->cost.start[slot] != end)
						continue;
				}
			}
		}

		a->cost.curpos[slot] = i | highflag;
		return (_dataptr[i] & 0x7F) != code;
	} while (1);
}

bool Scumm::isCostumeInUse(int cost) {
	int i;
	Actor *a;

	if (_roomResource != 0)
		for (i = 1; i < _numActors; i++) {
			a = derefActor(i);
			if (a->isInCurrentRoom() && a->costume == cost)
				return true;
		}

	return false;
}
