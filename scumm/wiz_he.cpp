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

#include "stdafx.h"

#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/wiz_he.h"

namespace Scumm {

Wiz::Wiz() {
	_imagesNum = 0;
	memset(&_images, 0, sizeof(_images));
	memset(&_polygons, 0, sizeof(_polygons));
	_rectOverrideEnabled = false;
}

void Wiz::imageNumClear() {
	_imagesNum = 0;
}

void Wiz::polygonClear() {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].flag == 1)
			memset(&_polygons[i], 0, sizeof(WizPolygon));
	}
}

void Wiz::polygonLoad(const uint8 *polData) {
	int slots = READ_LE_UINT32(polData);
	polData += 4;
	debug(1, "Loading %d polygon slots", slots);

	bool flag = 1;
	int id, points, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	while (slots--) {
		id = READ_LE_UINT32(polData);
		points = READ_LE_UINT32(polData + 4);
		if (points != 4)
			error("Illegal polygon with %d points", points);
		vert1x = READ_LE_UINT32(polData + 8);
		vert1y = READ_LE_UINT32(polData + 12);
		vert2x = READ_LE_UINT32(polData + 16);
		vert2y = READ_LE_UINT32(polData + 20);
		vert3x = READ_LE_UINT32(polData + 24);
		vert3y = READ_LE_UINT32(polData + 28);
		vert4x = READ_LE_UINT32(polData + 32);
		vert4y = READ_LE_UINT32(polData + 36);

		polData += 40;
		polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
	}
}

void Wiz::polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y) {
	WizPolygon *wp = NULL;
	for (int i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == 0) {
			wp = &_polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Wiz::polygonStore: out of polygon slot, max = %d", ARRAYSIZE(_polygons));
	}

	wp->vert[0].x = vert1x;
	wp->vert[0].y = vert1y;
	wp->vert[1].x = vert2x;
	wp->vert[1].y = vert2y;
	wp->vert[2].x = vert3x;
	wp->vert[2].y = vert3y;
	wp->vert[3].x = vert4x;
	wp->vert[3].y = vert4y;
	wp->vert[4].x = vert1x;
	wp->vert[4].y = vert1y;
	wp->id = id;
	wp->numVerts = 5;
	wp->flag = flag;	

	polygonCalcBoundBox(wp->vert, wp->numVerts, wp->bound);
}

void Wiz::polygonRotatePoints(Common::Point *pts, int num, int angle) {
	double alpha = angle * PI / 180.;
	double cos_alpha = cos(alpha);
	double sin_alpha = sin(alpha);

	for (int i = 0; i < num; ++i) {
		int16 x = pts[i].x;
		int16 y = pts[i].y;
		pts[i].x = (int16)(x * cos_alpha - y * sin_alpha);
		pts[i].y = (int16)(y * cos_alpha + x * sin_alpha);
	}
}

void Wiz::polygonCalcBoundBox(Common::Point *vert, int numVerts, Common::Rect &bound) {
	bound.left = 10000;
	bound.top = 10000;
	bound.right = -10000;
	bound.bottom = -10000;

	// compute bounding box
	for (int j = 0; j < numVerts; j++) {
		Common::Rect r(vert[j].x, vert[j].y, vert[j].x + 1, vert[j].y + 1);
		bound.extend(r);
	}
}

void Wiz::polygonErase(int fromId, int toId) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id >= fromId && _polygons[i].id <= toId)
			memset(&_polygons[i], 0, sizeof(WizPolygon));
	}
}

int Wiz::polygonHit(int id, int x, int y) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if ((id == 0 || _polygons[i].id == id) && _polygons[i].bound.contains(x, y)) {
			if (polygonContains(_polygons[i], x, y)) {
				return _polygons[i].id;
			}
		}
	}
	return 0;
}

bool Wiz::polygonDefined(int id) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++)
		if (_polygons[i].id == id)
			return true;
	return false;
}

bool Wiz::polygonContains(const WizPolygon &pol, int x, int y) {
	int pi = pol.numVerts - 1;
	bool diry = (y < pol.vert[pi].y);
	bool curdir;
	bool r = false;

	for (int i = 0; i < pol.numVerts; i++) {
		curdir = (y < pol.vert[i].y);

		if (curdir != diry) {
			if (((pol.vert[pi].y - pol.vert[i].y) * (pol.vert[i].x - x) <
				 (pol.vert[pi].x - pol.vert[i].x) * (pol.vert[i].y - y)) == diry)
				r = !r;
		}

		pi = i;
		diry = curdir;
	}

	// HE80+
	int a, b;
	pi = pol.numVerts - 1;
	if (r == 0) {
		for (int i = 0; i < pol.numVerts; i++) {
			if (pol.vert[i].y == y && pol.vert[i].y == pol.vert[pi].y) {

				a = pol.vert[i].x;
				b = pol.vert[pi].x;

				if (pol.vert[i].x >= pol.vert[pi].x)
					a = pol.vert[pi].x;

				if (pol.vert[i].x > pol.vert[pi].x)
					b = pol.vert[i].x;

				if (x >= a && x <= b)
					return 1;

			} else if (pol.vert[i].x == x && pol.vert[i].x == pol.vert[pi].x) {

				a = pol.vert[i].y;
				b = pol.vert[i].y;

				if (pol.vert[i].y >= pol.vert[pi].y)
					a = pol.vert[pi].y;

				if (pol.vert[i].y <= pol.vert[pi].y)
					b = pol.vert[pi].y;

				if (y >= a && y <= b)
					return 1;
			}
			pi = i;
		}
	}

	return r;
}

void Wiz::copyAuxImage(uint8 *dst1, uint8 *dst2, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch) {
	Common::Rect dstRect(srcx, srcy, srcx + srcw, srcy + srch);
	dstRect.clip(dstw, dsth);
	
	int rw = dstRect.width();
	int rh = dstRect.height();
	if (rh <= 0 || rw <= 0)
		return;
	
	uint8 *dst1Ptr = dst1 + dstRect.left + dstRect.top * dstw;
	uint8 *dst2Ptr = dst2 + dstRect.left + dstRect.top * dstw;
	const uint8 *dataPtr = src;

  	while (rh--) {
 		uint16 off = READ_LE_UINT16(dataPtr); dataPtr += 2;
		const uint8 *dataPtrNext = off + dataPtr;
		uint8 *dst1PtrNext = dst1Ptr + dstw;
		uint8 *dst2PtrNext = dst2Ptr + dstw;
		if (off != 0) {
			int w = rw;
			while (w > 0) {
				uint8 code = *dataPtr++;
				if (code & 1) {
					code >>= 1;
					dst1Ptr += code;
					dst2Ptr += code;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					w -= code;
					if (w >= 0) {
						memset(dst1Ptr, *dataPtr++, code);
						dst1Ptr += code;
						dst2Ptr += code;
					} else {
						code += w;
						memset(dst1Ptr, *dataPtr, code);
					}
				} else {
					code = (code >> 2) + 1;
					w -= code;
					if (w >= 0) {
						memcpy(dst1Ptr, dst2Ptr, code);
						dst1Ptr += code;
						dst2Ptr += code;
					} else {
						code += w;
						memcpy(dst1Ptr, dst2Ptr, code);
					}
				}
			}
		}
		dataPtr = dataPtrNext;
		dst1Ptr = dst1PtrNext;
		dst2Ptr = dst2PtrNext;
	}
}

static bool calcClipRects(int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, const Common::Rect *rect, Common::Rect &srcRect, Common::Rect &dstRect) {
	srcRect = Common::Rect(src_w, src_h);
	dstRect = Common::Rect(src_x, src_y, src_x + src_w, src_y + src_h);
	Common::Rect r3;
	int diff;

	if (rect) {
		r3 = *rect;
		Common::Rect r4(dst_w, dst_h);
		if (r3.intersects(r4)) {
			r3.clip(r4);
		} else {
			return false;
		}
	} else {
		r3 = Common::Rect(dst_w, dst_h);
	}
	diff = dstRect.left - r3.left;
	if (diff < 0) {
		srcRect.left -= diff;
		dstRect.left -= diff;
	}
	diff = dstRect.right - r3.right;
	if (diff > 0) {
		srcRect.right -= diff;
		dstRect.right -= diff;
	}
	diff = dstRect.top - r3.top;
	if (diff < 0) {
		srcRect.top -= diff;
		dstRect.top -= diff;
	}
	diff = dstRect.bottom - r3.bottom;
	if (diff > 0) {
		srcRect.bottom -= diff;
		dstRect.bottom -= diff;
	}

	return srcRect.isValidRect() && dstRect.isValidRect();
}

void Wiz::copyWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, const uint8 *palPtr) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		dst += r2.left + r2.top * dstw;
		decompressWizImage(dst, dstw, r2, src, r1, palPtr);
	}
}

void Wiz::copyRaw16BitWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor) {
	// RAW 16 bits in 555 format

	// HACK: Skip every second bit for now
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		if (flags & kWIFFlipX) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & kWIFFlipY) {
			int t = r1.top;
			int b = r1.bottom;
			r1.top = srch - b;
			r1.bottom = srch - t;
		}
		byte imagePal[256];
		if (!palPtr) {
			for (int i = 0; i < 256; i++) {
				imagePal[i] = i;
			}
			palPtr = imagePal;
		}

		int h = r1.height();
		int w = r1.width();
		src += r1.left + r1.top * srcw * 2;
		dst += r2.left + r2.top * dstw;

		while (h--) {
			const uint8 *p = src;
			for (int i = 0; i < w; ++i) {
				uint8 col = *p;
				if (transColor == -1 || transColor != col) {
					dst[i] = palPtr[col];
				}
				p += 2;
			}
			src += srcw * 2;
			dst += dstw;
		}

	}
}

void Wiz::copyRawWizImage(uint8 *dst, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor) {
	Common::Rect r1, r2;
	if (calcClipRects(dstw, dsth, srcx, srcy, srcw, srch, rect, r1, r2)) {
		if (flags & kWIFFlipX) {
			int l = r1.left;
			int r = r1.right;
			r1.left = srcw - r;
			r1.right = srcw - l;
		}
		if (flags & kWIFFlipY) {
			int t = r1.top;
			int b = r1.bottom;
			r1.top = srch - b;
			r1.bottom = srch - t;
		}
		byte imagePal[256];
		if (!palPtr) {
			for (int i = 0; i < 256; i++) {
				imagePal[i] = i;
			}
			palPtr = imagePal;
		}
		int h = r1.height();
		int w = r1.width();
		src += r1.left + r1.top * srcw;
		dst += r2.left + r2.top * dstw;
		while (h--) {
			const uint8 *p = src;
			for (int i = 0; i < w; ++i) {
				uint8 col = *p++;
				if (transColor == -1 || transColor != col) {
					dst[i] = palPtr[col];
				}
			}
			src += srcw;
			dst += dstw;
		}
	}
}

void Wiz::decompressWizImage(uint8 *dst, int dstPitch, const Common::Rect &dstRect, const uint8 *src, const Common::Rect &srcRect, const uint8 *palPtr) {
	const uint8 *dataPtr, *dataPtrNext;
	uint8 *dstPtr, *dstPtrNext;
	uint32 code;
	uint8 databit;
	int h, w, xoff;
	uint16 off;
	
	byte imagePal[256];
	if (!palPtr) {
		for (int i = 0; i < 256; i++) {
			imagePal[i] = i;
		}
		palPtr = imagePal;
	}

	dstPtr = dst;
	dataPtr = src;
	
	// Skip over the first 'srcRect->top' lines in the data
	h = srcRect.top;
	while (h--) {
		dataPtr += READ_LE_UINT16(dataPtr) + 2;
	}
	h = srcRect.height();
	w = srcRect.width();
	if (h <= 0 || w <= 0)
		return;
		
	while (h--) {
		xoff = srcRect.left;
		off = READ_LE_UINT16(dataPtr);
		w = srcRect.right - srcRect.left;
		dstPtrNext = dstPitch + dstPtr;
		dataPtrNext = off + 2 + dataPtr;
		dataPtr += 2;
		if (off == 0)
			goto dec_next;

		// Skip over the leftmost 'srcRect->left' pixels.
		// TODO: This code could be merged (at a loss of efficency) with the
		// loop below which does the actual drawing.
		while (xoff > 0) {
			code = *dataPtr++;
			databit = code & 1;
			code >>= 1;
			if (databit) {
				xoff -= code;
				if (xoff < 0) {
					code = -xoff;
					goto dec_sub1;
				}
			} else {
				databit = code & 1;
				code = (code >> 1) + 1;
				if (databit) {
					++dataPtr;
					xoff -= code;
					if (xoff < 0) {
						code = -xoff;
						--dataPtr;
						goto dec_sub2;
					}
				} else {
					dataPtr += code;
					xoff -= code;
					if (xoff < 0) {
						dataPtr += xoff;
						code = -xoff;
						goto dec_sub3;
					}
				}
			}
		}

		while (w > 0) {
			code = *dataPtr++;
			databit = code & 1;
			code >>= 1;
			if (databit) {
dec_sub1:		dstPtr += code;
				w -= code;
			} else {
				databit = code & 1;
				code = (code >> 1) + 1;
				if (databit) {
dec_sub2:			w -= code;
					if (w < 0) {
						code += w;
					}
					uint8 color = palPtr[*dataPtr++];
					memset(dstPtr, color, code);
					dstPtr += code;
				} else {
dec_sub3:			w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						*dstPtr++ = palPtr[*dataPtr++];
					}
				}
			}
		}
dec_next:
		dataPtr = dataPtrNext;
		dstPtr = dstPtrNext;
	}
}

int Wiz::isWizPixelNonTransparent(const uint8 *data, int x, int y, int w, int h) {
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return 0;
	}
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	uint16 off = READ_LE_UINT16(data); data += 2;
	if (off == 0) {
		return 0;
	}
	while (x > 0) {
		uint8 code = *data++;
		if (code & 1) {
			code >>= 1;
			if (code > x) {
				return 0;
			}
			x -= code;
		} else if (code & 2) {
			code = (code >> 2) + 1;
			if (code > x) {
				return 1;
			}
			x -= code;
			++data;
		} else {
			code = (code >> 2) + 1;
			if (code > x) {
				return 1;
			}
			x -= code;
			data += code;
		}
	}
	return (~data[0]) & 1;
}

uint8 Wiz::getWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 color) {
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return color;
	}
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	uint16 off = READ_LE_UINT16(data); data += 2;
	if (off == 0) {
		return color;
	}
	while (x > 0) {
		uint8 code = *data++;
		if (code & 1) {
			code >>= 1;
			if (code > x) {
				return color;
			}
			x -= code;
		} else if (code & 2) {
			code = (code >> 2) + 1;
			if (code > x) {
				return data[0];
			}
			x -= code;
			++data;
		} else {
			code = (code >> 2) + 1;
			if (code > x) {
				return data[x];
			}
			x -= code;
			data += code;
		}
	}
	return (data[0] & 1) ? color : data[1];
}

uint8 Wiz::getRawWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 color) {
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return color;
	}
	return data[y * w + x];
}

void Wiz::computeWizHistogram(uint32 *histogram, const uint8 *data, const Common::Rect *srcRect) {
	int y = srcRect->top;
	while (y != 0) {
		data += READ_LE_UINT16(data) + 2;
		--y;
	}
	int ih = srcRect->height();
	while (ih--) {
		uint16 off = READ_LE_UINT16(data); data += 2;
		if (off != 0) {
			const uint8 *p = data;
			int x1 = srcRect->left;
			int x2 = srcRect->right;
			uint8 code;
			while (x1 > 0) {
				code = *p++;
				if (code & 1) {
					code >>= 1;
					if (code > x1) {
						code -= x1;
						x2 -= code;
						break;
					}
					x1 -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (code > x1) {
						code -= x1;
						goto dec_sub2;
					}
					x1 -= code;
					++p;
				} else {
					code = (code >> 2) + 1;
					if (code > x1) {
						code -= x1;
						p += x1;
						goto dec_sub3;
					}
					x1 -= code;
					p += code;
				}
			}
			while (x2 > 0) {
				code = *p++;
				if (code & 1) {
					code >>= 1;
					x2 -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
dec_sub2:			x2 -= code;
					if (x2 < 0) {
						code += x2;
					}
					histogram[*p++] += code;
				} else {
					code = (code >> 2) + 1;
dec_sub3:			x2 -= code;
					if (x2 < 0) {
						code += x2;
					}
					int n = code;
					while (n--) {
						++histogram[*p++];
					}
				}
			}
			data += off;
		}
	}
}

void Wiz::computeRawWizHistogram(uint32 *histogram, const uint8 *data, int srcPitch, const Common::Rect *srcRect) {
	data += srcRect->top * srcPitch + srcRect->left;
	int iw = srcRect->width();
	int ih = srcRect->height();
	while (ih--) {
		for (int i = 0; i < iw; ++i) {
			++histogram[data[i]];
		}
		data += srcPitch;
	}
}

struct wizPackCtx {
	uint32 len;
	uint8 saveCode;
	uint8 saveBuf[256];
};

static void wizPackType1Helper1(uint8 *&dst, int len, byte newColor, byte prevColor, wizPackCtx *ctx) {
	assert(len > 0);
	if (newColor == prevColor) {
		do {
			int blockLen = MIN(len, 0x7F);
			len -= blockLen;
			if (dst) {
				*dst++ = (blockLen * 2) | 1;
			}
			++ctx->len;
		} while (len > 0);
	} else {
		do {
			int blockLen = MIN(len, 0x40);
			len -= blockLen;
			if (dst) {
				*dst++ = ((blockLen - 1) * 4) | 2;
			}
			++ctx->len;
			if (dst) {
				*dst++ = newColor;
			}
			++ctx->len;
		} while (len > 0);
	}
}

static void wizPackType1Helper2(uint8 *&dst, int len, wizPackCtx *ctx) {
	assert(len > 0);
	const uint8 *src = ctx->saveBuf;
	do {
		int blockLen = MIN(len, 0x40);
		len -= blockLen;
		if (dst) {
			*dst++ = (blockLen - 1) * 4;
		}
		++ctx->len;
		while (blockLen--) {
			if (dst) {
				*dst++ = *src++;
			}
			++ctx->len;
		}
	} while (len > 0);
}

static int wizPackType1(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt, uint8 tColor) {
	debug(1, "wizPackType1(%d, [%d,%d,%d,%d])", tColor, rCapt.left, rCapt.top, rCapt.right, rCapt.bottom);
	wizPackCtx ctx;
	memset(&ctx, 0, sizeof(ctx));
	
	src += rCapt.top * srcPitch + rCapt.left;
	int w = rCapt.width();
	int h = rCapt.height();
	
	uint8 *nextDstPtr, *curDstPtr;
	uint8 curColor, prevColor;
	int saveBufPos;
	
	nextDstPtr = curDstPtr = 0;
	
	int dataSize = 0;
	while (h--) {
		if (dst) {
			curDstPtr = dst;
			nextDstPtr = dst;
			dst += 2;
		}
		dataSize += 2;
		int numBytes = 0;
		
		int i, code;
		for (i = 0; i < w; ++i) {
			if (src[i] != tColor)
				break;
		}
		if (i != w) {
			curDstPtr = dst;
			ctx.len = 0;
			prevColor = ctx.saveBuf[0] = *src;
			const uint8 *curSrcPtr = src + 1;
			saveBufPos = 1;
			code = (tColor - ctx.saveBuf[0] == 0) ? 1 : 0;
			int curw = w;
			while (curw--) {
				ctx.saveBuf[saveBufPos] = curColor = *curSrcPtr++;
				++saveBufPos;
				if (code == 0) {
					if (curColor == tColor) {
						--saveBufPos;
						wizPackType1Helper2(curDstPtr, saveBufPos, &ctx);
						code = saveBufPos = 1;
						ctx.saveBuf[0] = curColor;
						numBytes = 0;
						prevColor = curColor;
						continue;
					}
					if (saveBufPos > 0x80) {
						--saveBufPos;
						wizPackType1Helper2(curDstPtr, saveBufPos, &ctx);
						saveBufPos = 1;
						ctx.saveBuf[0] = curColor;
						numBytes = 0;
						prevColor = curColor;
						continue;
					}
					if (prevColor != curColor) {
						numBytes = saveBufPos - 1;
						prevColor = curColor;
						continue;
					}
					code = 1;
					if (numBytes != 0) {
						if (saveBufPos - numBytes < 3) {
							code = 0;
						} else {
							wizPackType1Helper2(curDstPtr, numBytes, &ctx);
						}
					}
				}
				if (prevColor != curColor || saveBufPos - numBytes > 0x80) {
					saveBufPos -= numBytes;
					--saveBufPos;
					wizPackType1Helper1(curDstPtr, saveBufPos, prevColor, tColor, &ctx);
					saveBufPos = 1;
					numBytes = 0;
					ctx.saveBuf[0] = curColor;
					code = (tColor - ctx.saveBuf[0] == 0) ? 1 : 0;
				}
				prevColor = curColor;
			}
			if (code == 0) {
				wizPackType1Helper2(curDstPtr, saveBufPos, &ctx);
			} else {
				saveBufPos -= numBytes;
				wizPackType1Helper1(curDstPtr, saveBufPos, prevColor, tColor, &ctx);
			}
			dataSize += ctx.len;
			src += srcPitch;
			if (dst) {
				dst += ctx.len;
				*(uint16 *)nextDstPtr = TO_LE_16(ctx.len);
			}
		}
	}
	return dataSize;
}

static int wizPackType0(uint8 *dst, const uint8 *src, int srcPitch, const Common::Rect& rCapt, uint8 tColor) {
	int w = rCapt.width();
	int h = rCapt.height();
	int size = w * h;
	if (dst) {
		src += rCapt.top * srcPitch + rCapt.left;
		while (h--) {
			memcpy(dst, src, w);
			dst += w;
			src += srcPitch;
		}
	}
	return size;
}

void ScummEngine_v72he::captureWizImage(int resNum, const Common::Rect& r, bool backBuffer, int compType) {
	debug(1, "ScummEngine_v72he::captureWizImage(%d, %d, [%d,%d,%d,%d])", resNum, compType, r.left, r.top, r.right, r.bottom);
	uint8 *src = NULL;
	VirtScreen *pvs = &virtscr[kMainVirtScreen];
	if (backBuffer) {
		src = pvs->getBackPixels(0, 0);
	} else {
		src = pvs->getPixels(0, 0);
	}
	Common::Rect rCapt(pvs->w, pvs->h);
	if (rCapt.intersects(r)) {
		rCapt.clip(r);
		const uint8 *palPtr;
		if (_heversion >= 99) {
			palPtr = _hePalettes + 1024;
		} else {
			palPtr = _currentPalette;
		}

		int w = rCapt.width();
		int h = rCapt.height();
		int tColor = (VAR_WIZ_TCOLOR != 0xFF) ? VAR(VAR_WIZ_TCOLOR) : 5;

		// compute compressed size
		int dataSize = 0;
		int headerSize = palPtr ? 1080 : 36;
		switch (compType) {
		case 0:
			dataSize = wizPackType0(0, src, pvs->pitch, rCapt, tColor);
			break;
		case 1:
			dataSize = wizPackType1(0, src, pvs->pitch, rCapt, tColor);
			break;
		default:
			error("unhandled compression type %d", compType);
			break;
		}

		// alignment
		dataSize = (dataSize + 1) & ~1;
		int wizSize = headerSize + dataSize;
		// write header
		uint8 *wizImg = res.createResource(rtImage, resNum, dataSize + headerSize);
		WRITE_BE_UINT32(wizImg + 0x00, 'AWIZ');
		WRITE_BE_UINT32(wizImg + 0x04, wizSize);
		WRITE_BE_UINT32(wizImg + 0x08, 'WIZH');
		WRITE_BE_UINT32(wizImg + 0x0C, 0x14);
		WRITE_LE_UINT32(wizImg + 0x10, compType);
		WRITE_LE_UINT32(wizImg + 0x14, w);
		WRITE_LE_UINT32(wizImg + 0x18, h);
		int curSize = 0x1C;
		if (palPtr) {
			WRITE_BE_UINT32(wizImg + 0x1C, 'RGBS');
			WRITE_BE_UINT32(wizImg + 0x20, 0x308);
			memcpy(wizImg + 0x24, palPtr, 0x300);
			WRITE_BE_UINT32(wizImg + 0x324, 'RMAP');
			WRITE_BE_UINT32(wizImg + 0x328, 0x10C);
			WRITE_BE_UINT32(wizImg + 0x32C, 0);
			curSize = 0x330;
			for (int i = 0; i < 256; ++i) {
				wizImg[curSize] = i;
				++curSize;
			}
		}
		WRITE_BE_UINT32(wizImg + curSize + 0x0, 'WIZD');
		WRITE_BE_UINT32(wizImg + curSize + 0x4, dataSize + 8);
		curSize += 8;

		// write compressed data
		switch (compType) {
		case 0:
			wizPackType0(wizImg + headerSize, src, pvs->pitch, rCapt, tColor);
			break;
		case 1:
			wizPackType1(wizImg + headerSize, src, pvs->pitch, rCapt, tColor);
			break;
		default:
			break;
		}
	}
}

void ScummEngine_v72he::getWizImageDim(int resNum, int state, int32 &w, int32 &h) {
	uint8 *dataPtr = getResourceAddress(rtImage, resNum);
	assert(dataPtr);
	uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, state, 0);
	assert(wizh);
	w = READ_LE_UINT32(wizh + 0x4);
	h = READ_LE_UINT32(wizh + 0x8);
}

void ScummEngine_v72he::displayWizImage(WizImage *pwi) {
	if (_fullRedraw) {
		assert(_wiz._imagesNum < ARRAYSIZE(_wiz._images));
		memcpy(&_wiz._images[_wiz._imagesNum], pwi, sizeof(WizImage));
		++_wiz._imagesNum;
	} else if (pwi->flags & kWIFIsPolygon) {
		drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags, pwi->xmapNum, 0, 0);
	} else {
		const Common::Rect *r = NULL;
		drawWizImage(pwi->resNum, pwi->state, pwi->x1, pwi->y1, pwi->zorder, pwi->xmapNum, pwi->field_390, r, pwi->flags, 0, 0);
	}
}

uint8 *ScummEngine_v72he::drawWizImage(int resNum, int state, int x1, int y1, int zorder, int xmapNum, int field_390, const Common::Rect *clipBox, int flags, int dstResNum, int paletteNum) {
	debug(2, "drawWizImage(resNum %d, x1 %d y1 %d flags 0x%X zorder %d xmapNum %d field_390 %d dstResNum %d paletteNum %d)", resNum, x1, y1, flags, zorder, xmapNum, field_390, dstResNum, paletteNum);
	uint8 *dst = NULL;
	const uint8 *palPtr = NULL;
	if (_heversion >= 99) {
		if (paletteNum) {
			palPtr = _hePalettes + paletteNum * 1024 + 768;
		} else {
			palPtr = _hePalettes + 1792;
		}
	}
	uint8 *dataPtr = getResourceAddress(rtImage, resNum);
	if (dataPtr) {
		uint8 *rmap = NULL;
		uint8 *xmap = findWrappedBlock(MKID('XMAP'), dataPtr, state, 0);
		
		uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, state, 0);
		assert(wizh);
		uint32 comp   = READ_LE_UINT32(wizh + 0x0);
		uint32 width  = READ_LE_UINT32(wizh + 0x4);
		uint32 height = READ_LE_UINT32(wizh + 0x8);
		debug(2, "wiz_header.comp = %d wiz_header.w = %d wiz_header.h = %d", comp, width, height);
		
		uint8 *wizd = findWrappedBlock(MKID('WIZD'), dataPtr, state, 0);
		assert(wizd);
		if (flags & kWIFHasPalette) {
			uint8 *pal = findWrappedBlock(MKID('RGBS'), dataPtr, state, 0);
			assert(pal);
			setPaletteFromPtr(pal, 256);
		}
		if (flags & kWIFRemapPalette) {
			rmap = findWrappedBlock(MKID('RMAP'), dataPtr, state, 0);
			assert(rmap);
			if (_heversion <= 80 || READ_BE_UINT32(rmap) != 0x01234567) {
				uint8 *rgbs = findWrappedBlock(MKID('RGBS'), dataPtr, state, 0);
				assert(rgbs);
				remapHEPalette(rgbs, rmap + 4);
			}
		}
		if (flags & kWIFPrint) {
			warning("WizImage printing is unimplemented");
			return NULL;
		}

		int32 cw, ch;
		if (flags & kWIFBlitToMemBuffer) {
			dst = (uint8 *)malloc(width * height);
			int color = 255; // FIXME: should be (VAR_WIZ_TCOLOR != 0xFF) ? VAR(VAR_WIZ_TCOLOR) : 5;
			memset(dst, color, width * height);
			cw = width;
			ch = height;
		} else {
			if (dstResNum) {
				uint8 *dstPtr = getResourceAddress(rtImage, dstResNum);
				assert(dstPtr);
				dst = findWrappedBlock(MKID('WIZD'), dstPtr, 0, 0);
				assert(dst);

				getWizImageDim(dstResNum, 0, cw, ch);
			} else {
				VirtScreen *pvs = &virtscr[kMainVirtScreen];
				if (flags & kWIFMarkBufferDirty) {
					dst = pvs->getPixels(0, pvs->topline);
				} else {
					dst = pvs->getBackPixels(0, pvs->topline);
				}
				cw = pvs->w;
				ch = pvs->h;
			}
		}
		Common::Rect rScreen(cw, ch);
		if (clipBox) {
			Common::Rect clip(clipBox->left, clipBox->top, clipBox->right, clipBox->bottom);
			if (rScreen.intersects(clip)) {
				rScreen.clip(clip);
			} else {
				return 0;
			}
		} else if (_wiz._rectOverrideEnabled) {
			if (rScreen.intersects(_wiz._rectOverride)) {
				rScreen.clip(_wiz._rectOverride);
			} else {
				return 0;
			}
		}

		// XXX handle 'XMAP' data
		if (xmap) {
			palPtr = xmap;
		}
		if (flags & kWIFRemapPalette) {
			palPtr = rmap + 4;
		}

		uint8 *trns = findWrappedBlock(MKID('TRNS'), dataPtr, state, 0);
		int color = (trns == NULL) ? VAR(VAR_WIZ_TCOLOR) : -1;

		switch (comp) {
		case 0:
			_wiz.copyRawWizImage(dst, wizd, cw, ch, x1, y1, width, height, &rScreen, flags, palPtr, color);
			break;
		case 1:
			// TODO Adding masking for flags 0x80 and 0x100
			if (flags & 0x80) {
				// Used in maze
				warning("drawWizImage: Unhandled flag 0x80");
			} else if (flags & 0x100) {
				// Used in readdemo
				warning("drawWizImage: Unhandled flag 0x100");
			}
			_wiz.copyWizImage(dst, wizd, cw, ch, x1, y1, width, height, &rScreen, palPtr);
			break;
		case 2:
			_wiz.copyRaw16BitWizImage(dst, wizd, cw, ch, x1, y1, width, height, &rScreen, flags, palPtr, color);
			break;
		case 5:
			// Used in Moonbase Commander
			warning("drawWizImage: Unhandled wiz compression type %d", comp);
			break;
		default:
			error("drawWizImage: Unhandled wiz compression type %d", comp);
		}

		if (!(flags & kWIFBlitToMemBuffer) && dstResNum == 0) {
			Common::Rect rImage(x1, y1, x1 + width, y1 + height);
			if (rImage.intersects(rScreen)) {
				rImage.clip(rScreen);
				if (!(flags & kWIFBlitToFrontVideoBuffer) && (flags & (kWIFBlitToFrontVideoBuffer | kWIFMarkBufferDirty))) {
					++rImage.bottom;
					markRectAsDirty(kMainVirtScreen, rImage);
				} else {
					gdi.copyVirtScreenBuffers(rImage);
				}
			}
		}
	}
	return dst;
}

struct PolygonDrawData {
	struct InterArea {
		bool valid;
		int32 xmin;
		int32 xmax;
		int32 x1;
		int32 y1;
		int32 x2;
		int32 y2;
	};
	Common::Point pto;
	InterArea *ia;
	int areasNum;
	
	PolygonDrawData(int n) {
		areasNum = n;
		ia = new InterArea[areasNum];
		memset(ia, 0, sizeof(InterArea) * areasNum);
	}
	
	~PolygonDrawData() {
		delete[] ia;
	}
	
	void calcIntersection(const Common::Point *p1, const Common::Point *p2, const Common::Point *p3, const Common::Point *p4) {
		int32 x1_acc = p1->x << 0x10;
		int32 x3_acc = p3->x << 0x10;
		int32 y3_acc = p3->y << 0x10;
  		uint16 dy = ABS(p2->y - p1->y) + 1;
  		int32 x1_step = ((p2->x - p1->x) << 0x10) / dy;
  		int32 x3_step = ((p4->x - p3->x) << 0x10) / dy;
  		int32 y3_step = ((p4->y - p3->y) << 0x10) / dy;

  		int iaidx = p1->y - pto.y;
  		while (dy--) {
  			assert(iaidx >= 0 && iaidx < areasNum);
  			InterArea *pia = &ia[iaidx];
  			int32 tx1 = x1_acc >> 0x10;
  			int32 tx3 = x3_acc >> 0x10;
  			int32 ty3 = y3_acc >> 0x10;
  			
  			if (!pia->valid || pia->xmin > tx1) {
  				pia->xmin = tx1;
  				pia->x1 = tx3;
  				pia->y1 = ty3;
			}
  			if (!pia->valid || pia->xmax < tx1) {
  				pia->xmax = tx1;
  				pia->x2 = tx3;
  				pia->y2 = ty3;
			}
  			pia->valid = true;

  			x1_acc += x1_step;
  			x3_acc += x3_step;
  			y3_acc += y3_step;
  			
  			if (p2->y <= p1->y) {
  				--iaidx;
  			} else {
  				++iaidx;
  			}  			
  		}
	}
};

void ScummEngine_v72he::drawWizComplexPolygon(int resNum, int state, int po_x, int po_y, int xmapNum, int angle, int zoom, const Common::Rect *r, int flags, int dstResNum, int paletteNum) {
	Common::Point pts[4];
	int32 w, h;
	getWizImageDim(resNum, state, w, h);

	pts[1].x = pts[2].x = w / 2 - 1;
	pts[0].x = pts[0].y = pts[1].y = pts[3].x = -w / 2;
	pts[2].y = pts[3].y = h / 2 - 1;

	// transform points
	if (zoom != 256) {
		for (int i = 0; i < 4; ++i) {
			pts[i].x = pts[i].x * zoom / 256;
			pts[i].y = pts[i].y * zoom / 256;
		}
	}
	if (angle)
		_wiz.polygonRotatePoints(pts, 4, angle);

	for (int i = 0; i < 4; ++i) {
		pts[i].x += po_x;
		pts[i].y += po_y;
	}

	if (zoom != 256) {
		warning("drawWizComplexPolygon() zoom not implemented");

		//drawWizPolygonTransform(resNum, state, pts, flags, VAR(VAR_WIZ_TCOLOR), r, dstPtr, paletteNum, xmapPtr);
	} else {
		warning("drawWizComplexPolygon() angle partially implemented");

		angle %= 360;
		if (angle < 0) {
			angle += 360;
		}

		Common::Rect bounds;
		_wiz.polygonCalcBoundBox(pts, 4, bounds);
		int x1 = bounds.left;
		int y1 = bounds.top;

		switch(angle) {
		case 270:
			flags |= kWIFFlipX | kWIFFlipY;
			//drawWizComplexPolygonHelper(resNum, state, x1, y1, r, flags, dstResNum, paletteNum);
			break;
		case 180:
			flags |= kWIFFlipX | kWIFFlipY;
			drawWizImage(resNum, state, x1, y1, 0, xmapNum, 0, r, flags, dstResNum, paletteNum);
			break;
		case 90:
			//drawWizComplexPolygonHelper(resNum, state, x1, y1, r, flags, dstResNum, paletteNum);
			break;
		case 0:
			drawWizImage(resNum, state, x1, y1, 0, xmapNum, 0, r, flags, dstResNum, paletteNum);
			break;
		default:
			//drawWizPolygonTransform(resNum, state, pts, flags, VAR(VAR_WIZ_TCOLOR), r, dstResNum, paletteNum, xmapPtr);
			break;
		}
	}
}

void ScummEngine_v72he::drawWizPolygon(int resNum, int state, int id, int flags, int xmapNum, int dstResNum, int paletteNum) {
	debug(1, "drawWizPolygon(resNum %d, id %d, flags 0x%X, xmapNum %d paletteNum %d)", resNum, id, flags, xmapNum, paletteNum);
	int i;
	WizPolygon *wp = NULL;
	for (i = 0; i < ARRAYSIZE(_wiz._polygons); ++i) {
		if (_wiz._polygons[i].id == id) {
			wp = &_wiz._polygons[i];
			break;
		}
	}
	if (!wp) {
		error("Polygon %d is not defined", id);
	}
	if (wp->numVerts != 5) {
		error("Invalid point count %d for Polygon %d", wp->numVerts, id);
	}
	const Common::Rect *r = NULL;
	uint8 *srcWizBuf = drawWizImage(resNum, state, 0, 0, 0, xmapNum, 0, r, kWIFBlitToMemBuffer, 0, paletteNum);
	if (srcWizBuf) {
		uint8 *dst;
		int32 wizW, wizH;
		VirtScreen *pvs = &virtscr[kMainVirtScreen];

		if (dstResNum) {
			uint8 *dstPtr = getResourceAddress(rtImage, dstResNum);
			assert(dstPtr);
			dst = findWrappedBlock(MKID('WIZD'), dstPtr, 0, 0);
			assert(dst);

			getWizImageDim(dstResNum, 0, wizW, wizH);
		} else {
			if (flags & kWIFMarkBufferDirty) {
				dst = pvs->getPixels(0, 0);
			} else {
				dst = pvs->getBackPixels(0, 0);
			}

			getWizImageDim(resNum, state, wizW, wizH);
		}
		if (wp->bound.left < 0 || wp->bound.top < 0 || wp->bound.right >= pvs->w || wp->bound.bottom >= pvs->h) {
			error("Invalid coords polygon %d", wp->id);
		}

		Common::Point bbox[4];
		bbox[0].x = 0;
		bbox[0].y = 0;
		bbox[1].x = wizW - 1;
		bbox[1].y = 0;
		bbox[2].x = wizW - 1;
		bbox[2].y = wizH - 1;
		bbox[3].x = 0;
		bbox[3].y = wizH - 1;

  		int16 xmin_p, xmax_p, ymin_p, ymax_p;
  		xmin_p = xmax_p = wp->vert[0].x;
  		ymin_p = ymax_p = wp->vert[0].y;
  		for (i = 1; i < 4; ++i) {
  			xmin_p = MIN(wp->vert[i].x, xmin_p);
  			xmax_p = MAX(wp->vert[i].x, xmax_p);
  			ymin_p = MIN(wp->vert[i].y, ymin_p);
  			ymax_p = MAX(wp->vert[i].y, ymax_p);
  		}
  		
  		int16 xmin_b, xmax_b, ymin_b, ymax_b;
  		xmin_b = 0;
  		xmax_b = wizW - 1;
  		ymin_b = 0;
  		ymax_b = wizH - 1;

		PolygonDrawData pdd(ymax_p - ymin_p + 1);
		pdd.pto.x = xmin_p;
		pdd.pto.y = ymin_p;
		
		for (i = 0; i < 3; ++i) {
			pdd.calcIntersection(&wp->vert[i], &wp->vert[i + 1], &bbox[i], &bbox[i + 1]);
		}
		pdd.calcIntersection(&wp->vert[3], &wp->vert[0], &bbox[3], &bbox[0]);
				
		uint yoff = pdd.pto.y * pvs->w;
		for (i = 0; i < pdd.areasNum; ++i) {
			PolygonDrawData::InterArea *pia = &pdd.ia[i];
			uint16 dx = pia->xmax - pia->xmin + 1;
			uint8 *dstPtr = dst + pia->xmin + yoff;
			int32 x_acc = pia->x1 << 0x10;
			int32 y_acc = pia->y1 << 0x10;
			int32 x_step = ((pia->x2 - pia->x1) << 0x10) / dx;
			int32 y_step = ((pia->y2 - pia->y1) << 0x10) / dx;
			while (dx--) {
				uint srcWizOff = (y_acc >> 0x10) * wizW + (x_acc >> 0x10);
				assert(srcWizOff < (uint32)(wizW * wizH));
				x_acc += x_step;
				y_acc += y_step;
				*dstPtr++ = srcWizBuf[srcWizOff];
			}
			yoff += pvs->pitch;
		}

		if (flags & kWIFMarkBufferDirty) {
			markRectAsDirty(kMainVirtScreen, wp->bound);
		} else {
			gdi.copyVirtScreenBuffers(wp->bound);
		}

		free(srcWizBuf);
	}
}

void ScummEngine_v72he::flushWizBuffer() {
	for (int i = 0; i < _wiz._imagesNum; ++i) {
		WizImage *pwi = &_wiz._images[i];
		if (pwi->flags & kWIFIsPolygon) {
			drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags, pwi->xmapNum, 0, pwi->paletteNum);
		} else {
			const Common::Rect *r = NULL;
			drawWizImage(pwi->resNum, pwi->state, pwi->x1, pwi->y1, pwi->zorder, pwi->xmapNum, pwi->field_390, r, pwi->flags, 0, pwi->paletteNum);
		}
	}
	_wiz._imagesNum = 0;
}

void ScummEngine_v80he::loadImgSpot(int resId, int state, int16 &x, int16 &y) {
	uint8 *dataPtr = getResourceAddress(rtImage, resId);
	assert(dataPtr);
	uint8 *spotPtr = findWrappedBlock(MKID('SPOT'), dataPtr, state, 0);
	if (spotPtr) {
		x = (int16)READ_LE_UINT32(spotPtr + 0);
		y = (int16)READ_LE_UINT32(spotPtr + 4);
	} else {
		x = 0;
		y = 0;
	}
}

void ScummEngine_v80he::loadWizCursor(int resId) {
	int16 x, y;
	loadImgSpot(resId, 0, x, y);
	if (x < 0) {
		x = 0;
	} else if (x > 32) {
		x = 32;
	}
	if (y < 0) {
		y = 0;
	} else if (y > 32) {
		y = 32;
	}

	const Common::Rect *r = NULL;
	uint8 *cursor = drawWizImage(resId, 0, 0, 0, 0, 0, 0, r, kWIFBlitToMemBuffer, 0, 0);
	int32 cw, ch;	
	getWizImageDim(resId, 0, cw, ch);
	setCursorFromBuffer(cursor, cw, ch, cw);
	setCursorHotspot(x, y);
	free(cursor);
}

void ScummEngine_v72he::displayWizComplexImage(const WizParameters *params) {
	int maskImgResNum = 0;
	if (params->processFlags & kWPFMaskImg) {
		maskImgResNum = params->maskImgResNum;
		warning("displayWizComplexImage() unhandled flag 0x80000");
	}
	int paletteNum = 0;
	if (params->processFlags & kWPFPaletteNum) {
		paletteNum = params->img.paletteNum;
	}
	int zoom = 256;
	if (params->processFlags & kWPFZoom) {
		zoom = params->zoom;
	}
	int rotationAngle = 0;
	if (params->processFlags & kWPFRotate) {
		rotationAngle = params->angle;
	}
	int state = 0;
	if (params->processFlags & kWPFNewState) {
		state = params->img.state;
	}
	int flags = 0;
	if (params->processFlags & kWPFNewFlags) {
		flags = params->img.flags;
	}
	int po_x = 0;
	int po_y = 0;
	if (params->processFlags & kWPFSetPos) {
		po_x = params->img.x1;
		po_y = params->img.y1;
	}
	int xmapNum = 0;
	if (params->processFlags & kWPFXmapNum) {
		xmapNum = params->xmapNum;
	}
	int field_390 = 0;
	if (params->processFlags & 0x200000) {
		field_390 = params->img.field_390;
		warning("displayWizComplexImage() unhandled flag 0x200000");
	}
	const Common::Rect *r = NULL;
	if (params->processFlags & kWPFClipBox) {
		r = &params->box;
	}
	int dstResNum = 0;
	if (params->processFlags & kWPFDstResNum) {
		dstResNum = params->dstResNum;
	}
	if (params->processFlags & kWPFRemapPalette) {
		int st = (params->processFlags & kWPFNewState) ? params->img.state : 0;
		int num = params->remapNum;
		const uint8 *index = params->remapIndex;
		uint8 *iwiz = getResourceAddress(rtImage, params->img.resNum);
		assert(iwiz);
		uint8 *rmap = findWrappedBlock(MKID('RMAP'), iwiz, st, 0) ;
		assert(rmap);
		WRITE_BE_UINT32(rmap, 0x01234567);
		while (num--) {
			uint8 idx = *index++;
			rmap[4 + idx] = params->remapColor[idx];
		}
		flags |= kWIFRemapPalette;
	}

	if (_fullRedraw && dstResNum == 0) {
		if (maskImgResNum != 0 || (params->processFlags & (kWPFZoom | kWPFRotate)))
			error("Can't do this command in the enter script.");

		assert(_wiz._imagesNum < ARRAYSIZE(_wiz._images));
		WizImage *pwi = &_wiz._images[_wiz._imagesNum];
		pwi->resNum = params->img.resNum;
		pwi->x1 = po_x;
		pwi->y1 = po_y;
		pwi->zorder = params->img.zorder;
		pwi->state = state;
		pwi->flags = flags;
		pwi->xmapNum = xmapNum;
		pwi->field_390 = field_390;
		pwi->paletteNum = paletteNum;
		++_wiz._imagesNum;
	} else {
		if (maskImgResNum != 0) {
			// TODO
		} else if (params->processFlags & (kWPFZoom | kWPFRotate)) {
			drawWizComplexPolygon(params->img.resNum, state, po_x, po_y, xmapNum, rotationAngle, zoom, r, flags, dstResNum, paletteNum);
		} else {
			if (flags & kWIFIsPolygon) {
				drawWizPolygon(params->img.resNum, state, po_x, flags, xmapNum, dstResNum, paletteNum); // XXX , VAR(VAR_WIZ_TCOLOR));
			} else {
				drawWizImage(params->img.resNum, state, po_x, po_y, params->img.zorder, xmapNum, field_390, r, flags, dstResNum, paletteNum);
			}
		}
	}
}

void ScummEngine_v90he::createWizEmptyImage(const WizParameters *params) {
	debug(1, "ScummEngine_v90he::createWizEmptyImage(%d, %d, %d)", params->img.resNum, params->resDefImgW, params->resDefImgH);
	int img_w = 640;
	if (params->processFlags & kWPFUseDefImgWidth) {
		img_w = params->resDefImgW;
	}
	int img_h = 480;
	if (params->processFlags & kWPFUseDefImgHeight) {
		img_h = params->resDefImgH;
	}
	int img_x = 0;
	int img_y = 0;
	if (params->processFlags & 1) {
		img_x = params->img.x1;
		img_y = params->img.y1;
	}
	const uint16 flags = 0xB;
	int res_size = 0x1C;
	if (flags & 1) {
		res_size += 0x308;
	}
	if (flags & 2) {
		res_size += 0x10;
	}
	if (flags & 8) {
		res_size += 0x10C;
	}
	res_size += 8 + img_w * img_h;
	
	const uint8 *palPtr;
	if (_heversion >= 99) {
		palPtr = _hePalettes + 1024;
	} else {
		palPtr = _currentPalette;
	}
	uint8 *res_data = res.createResource(rtImage, params->img.resNum, res_size);
	if (!res_data) {
		VAR(119) = -1;
	} else {
		VAR(119) = 0;
		WRITE_BE_UINT32(res_data, 'AWIZ'); res_data += 4;
		WRITE_BE_UINT32(res_data, res_size); res_data += 4;
		WRITE_BE_UINT32(res_data, 'WIZH'); res_data += 4;
		WRITE_BE_UINT32(res_data, 0x14); res_data += 4;
		WRITE_LE_UINT32(res_data, 0); res_data += 4;
		WRITE_LE_UINT32(res_data, img_w); res_data += 4;
		WRITE_LE_UINT32(res_data, img_h); res_data += 4;
		if (flags & 1) {
			WRITE_BE_UINT32(res_data, 'RGBS'); res_data += 4;
			WRITE_BE_UINT32(res_data, 0x308); res_data += 4;
			memcpy(res_data, palPtr, 0x300); res_data += 0x300;			
		}
		if (flags & 2) {
			WRITE_BE_UINT32(res_data, 'SPOT'); res_data += 4;
			WRITE_BE_UINT32(res_data, 0x10); res_data += 4;
			WRITE_BE_UINT32(res_data, img_x); res_data += 4;
			WRITE_BE_UINT32(res_data, img_y); res_data += 4;
		}
		if (flags & 8) {
			WRITE_BE_UINT32(res_data, 'RMAP'); res_data += 4;
			WRITE_BE_UINT32(res_data, 0x10C); res_data += 4;
			WRITE_BE_UINT32(res_data, 0); res_data += 4;
			for (int i = 0; i < 256; ++i) {
				*res_data++ = i;
			}
		}
		WRITE_BE_UINT32(res_data, 'WIZD'); res_data += 4;
		WRITE_BE_UINT32(res_data, 8 + img_w * img_h); res_data += 4;
	}
}

void ScummEngine_v90he::fillWizRect(const WizParameters *params) {
	int state = 0;
	if (params->processFlags & kWPFNewState) {
		state = params->img.state;
	}
	uint8 *dataPtr = getResourceAddress(rtImage, params->img.resNum);
	if (dataPtr) {	
		uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, state, 0);
		assert(wizh);
		int c = READ_LE_UINT32(wizh + 0x0);
		int w = READ_LE_UINT32(wizh + 0x4);
		int h = READ_LE_UINT32(wizh + 0x8);
		assert(c == 0);
		Common::Rect r1(w, h);
		if (params->processFlags & kWPFClipBox) {
			if (!r1.intersects(params->box)) {
				return;
			}
			r1.clip(params->box);
		}
		if (params->processFlags & kWPFClipBox2) {
			r1.clip(params->box2);
		}
		uint8 color = VAR(93);
		if (params->processFlags & kWPFFillColor) {
			color = params->fillColor;
		}
		uint8 *wizd = findWrappedBlock(MKID('WIZD'), dataPtr, state, 0);
		assert(wizd);
		int dx = r1.width();
		int dy = r1.height();
		wizd += r1.top * w + r1.left;
		while (dy--) {
			memset(wizd, color, dx);
			wizd += w;
		}
	}
}

void ScummEngine_v90he::fillWizParallelogram(const WizParameters *params) {	
	if (params->processFlags & kWPFClipBox2) {
		int state = 0;
		if (params->processFlags & kWPFNewState) {
			state = params->img.state;
		}
		uint8 *dataPtr = getResourceAddress(rtImage, params->img.resNum);
		if (dataPtr) {
			uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, state, 0);
			assert(wizh);
			int c = READ_LE_UINT32(wizh + 0x0);
			int w = READ_LE_UINT32(wizh + 0x4);
			int h = READ_LE_UINT32(wizh + 0x8);
			assert(c == 0);
			Common::Rect r1(w, h);
			if (params->processFlags & kWPFClipBox) {
				if (!r1.intersects(params->box)) {
					return;
				}
				r1.clip(params->box);
			}
			uint8 color = VAR(93);
			if (params->processFlags & kWPFFillColor) {
				color = params->fillColor;
			}
			uint8 *wizd = findWrappedBlock(MKID('WIZD'), dataPtr, state, 0);
			assert(wizd);
			int x1 = params->box2.left;
			int y1 = params->box2.top;
			int x2 = params->box2.right;
			int y2 = params->box2.bottom;
			
			int dx = x2 - x1;
			int incx = 0;
			if (dx > 0) {
				incx = 1;
			} else if (dx < 0) {
				incx = -1;
			}
			int dy = y2 - y1;
			int incy = 0;
			if (dy > 0) {
				incy = 1;
			} else if (dy < 0) {
				incy = -1;
			}
			
			if (r1.contains(x1, y1)) {
				*(wizd + y1 * w + x1) = color;
			}
			
			if (dx >= dy) {
				int step1_y = (dy - dx) * 2;
				int step2_y = dy * 2;
				int accum_y = dy * 2 - dx;
				while (x1 != x2) {
					if (accum_y <= 0) {
						accum_y += step2_y;
					} else {
						accum_y += step1_y;
						y1 += incy;
					}
					x1 += incx;
					if (r1.contains(x1, y1)) {
						*(wizd + y1 * w + x1) = color;
					}
				}
			} else {
				int step1_x = (dx - dy) * 2;
				int step2_x = dx * 2;
				int accum_x = dx * 2 - dy;
				while (y1 != y2) {
					if (accum_x <= 0) {
						accum_x += step2_x;
					} else {
						accum_x += step1_x;
						x1 += incx;
					}
					y1 += incy;
					if (r1.contains(x1, y1)) {
						*(wizd + y1 * w + x1) = color;
					}					
				}
			}
		}
	}
}

void ScummEngine_v90he::processWizImage(const WizParameters *params) {
	char buf[512];
	unsigned int i;

	debug(2, "processWizImage: processMode %d", params->processMode);
	switch (params->processMode) {
	case 0:
		// Used in racedemo
		break;
	case 1:
		displayWizComplexImage(params);
		break;
	case 2:
 		captureWizImage(params->img.resNum, params->box, (params->img.flags & kWIFBlitToFrontVideoBuffer) != 0, params->compType);
		break;
	case 3:
		if (params->processFlags & kWPFUseFile) {
			File f;

			// Convert Windows path separators to something more portable
			strncpy(buf, (const char *)params->filename, 512);
			for (i = 0; i < strlen(buf); i++) {
				if (buf[i] == '\\')
					buf[i] = '/';
			}

			if (f.open((const char *)buf, File::kFileReadMode)) {
				uint32 id = f.readUint32LE();
				if (id == TO_LE_32(MKID('AWIZ')) || id == TO_LE_32(MKID('MULT'))) {
					uint32 size = f.readUint32BE();
					f.seek(0, SEEK_SET);
					byte *p = res.createResource(rtImage, params->img.resNum, size);
					if (f.read(p, size) != size) {
						res.nukeResource(rtImage, params->img.resNum);
						warning("i/o error when reading '%s'", buf);
						VAR(VAR_GAME_LOADED) = -2;
						VAR(119) = -2;
					} else {
						VAR(VAR_GAME_LOADED) = 0;
						VAR(119) = 0;
					}
				} else {
					VAR(VAR_GAME_LOADED) = -1;
					VAR(119) = -1;
				}
				f.close();
			} else {
				VAR(VAR_GAME_LOADED) = -3;
				VAR(119) = -3;
				warning("Unable to open for read '%s'", buf);
			}
		}
		break;
	case 4:
		if (params->processFlags & kWPFUseFile) {
			File f;

			switch(params->fileWriteMode) {
			case 2:
				VAR(119) = -1;
				break;
			case 1:
				// TODO Write image to file
				break;
			case 0:
				// Convert Windows path separators to something more portable
				strncpy(buf, (const char *)params->filename, 512);
				for (i = 0; i < strlen(buf); i++) {
					if (buf[i] == '\\')
						buf[i] = '/';
				}

				if (!f.open((const char *)buf, File::kFileWriteMode)) {
					warning("Unable to open for write '%s'", buf);
					VAR(119) = -3;
				} else {
					byte *p = getResourceAddress(rtImage, params->img.resNum);
					uint32 size = READ_BE_UINT32(p + 4);
					if (f.write(p, size) != size) {
						warning("i/o error when writing '%s'", params->filename);
						VAR(119) = -2;
					} else {
						VAR(119) = 0;
					}
					f.close();
				}
				break;
			default:
				error("processWizImage: processMode 4 unhandled fileWriteMode %d", params->fileWriteMode);
			}
		}
		break;
	case 6:
		if (params->processFlags & kWPFRemapPalette) {
			int state = (params->processFlags & kWPFNewState) ? params->img.state : 0;
			int num = params->remapNum;
			const uint8 *index = params->remapIndex;
			uint8 *iwiz = getResourceAddress(rtImage, params->img.resNum);
			assert(iwiz);
			uint8 *rmap = findWrappedBlock(MKID('RMAP'), iwiz, state, 0) ;
			assert(rmap);
			WRITE_BE_UINT32(rmap, 0x01234567);
			while (num--) {
				uint8 idx = *index++;
				rmap[4 + idx] = params->remapColor[idx];
			}
		}
		break;
	// HE 99+
	case 7:
		// Used in soccer2004
		// TODO
		break;
	case 8:
		createWizEmptyImage(params);
		break;
	case 9:
		fillWizRect(params);
		break;
	case 10:
		fillWizParallelogram(params);
		break;
	default:
		error("Unhandled processWizImage mode %d", params->processMode);
	}
}

int ScummEngine_v90he::getWizImageStates(int resNum) {
	const uint8 *dataPtr = getResourceAddress(rtImage, resNum);
	assert(dataPtr);
	if (READ_UINT32(dataPtr) == MKID('MULT')) {
		const byte *offs, *wrap;

		wrap = findResource(MKID('WRAP'), dataPtr);
		if (wrap == NULL)
			return 1;

		offs = findResourceData(MKID('OFFS'), wrap);
		if (offs == NULL)
			return 1;

		return getResourceDataSize(offs) / 4;
	} else {
		return 1;
	}
}

int ScummEngine_v90he::isWizPixelNonTransparent(int resNum, int state, int x, int y, int flags) {
	int ret = 0;
	uint8 *data = getResourceAddress(rtImage, resNum);
	assert(data);
	uint8 *wizh = findWrappedBlock(MKID('WIZH'), data, state, 0);
	assert(wizh);
	int c = READ_LE_UINT32(wizh + 0x0);
	int w = READ_LE_UINT32(wizh + 0x4);
	int h = READ_LE_UINT32(wizh + 0x8);
	uint8 *wizd = findWrappedBlock(MKID('WIZD'), data, state, 0);
	assert(wizd);
	if (x >= 0 && x < w && y >= 0 && y < h) {
		if (flags & kWIFFlipX) {
			x = w - x - 1;
		}
		if (flags & kWIFFlipY) {
			y = h - y - 1;
		}
		switch (c) {
		case 0:
			ret = _wiz.getRawWizPixelColor(wizd, x, y, w, h, VAR(VAR_WIZ_TCOLOR)) != VAR(VAR_WIZ_TCOLOR) ? 1 : 0;
			break;
		case 1:
			ret = _wiz.isWizPixelNonTransparent(wizd, x, y, w, h);
			break;
		default:
			error("isWizPixelNonTransparent: Unhandled wiz compression type %d", c);
			break;
		}
	}
	return ret;
}

uint8 ScummEngine_v90he::getWizPixelColor(int resNum, int state, int x, int y, int flags) {
	uint8 color;
	uint8 *data = getResourceAddress(rtImage, resNum);
	assert(data);
	uint8 *wizh = findWrappedBlock(MKID('WIZH'), data, state, 0);
	assert(wizh);
	int c = READ_LE_UINT32(wizh + 0x0);
	int w = READ_LE_UINT32(wizh + 0x4);
	int h = READ_LE_UINT32(wizh + 0x8);
	uint8 *wizd = findWrappedBlock(MKID('WIZD'), data, state, 0);
	assert(wizd);
	switch (c) {
	case 0:
		color = _wiz.getRawWizPixelColor(wizd, x, y, w, h, VAR(VAR_WIZ_TCOLOR));
		break;
	case 1:
		color = _wiz.getWizPixelColor(wizd, x, y, w, h, VAR(VAR_WIZ_TCOLOR));
		break;
	case 5:
		// Used in Moonbase Commander
		color = 1;
		warning("getWizPixelColor: Unhandled wiz compression type %d", c);
		break;
	default:
		error("getWizPixelColor: Unhandled wiz compression type %d", c);
		break;
	}
	return color;
}

int ScummEngine_v90he::computeWizHistogram(int resNum, int state, int x, int y, int w, int h) {
	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, 255);
	if (readVar(0) != 0) {
		Common::Rect rCap(x, y, w + 1, h + 1);
		uint8 *data = getResourceAddress(rtImage, resNum);
		assert(data);
		uint8 *wizh = findWrappedBlock(MKID('WIZH'), data, state, 0);
		assert(wizh);
		int c = READ_LE_UINT32(wizh + 0x0);
		w = READ_LE_UINT32(wizh + 0x4);
		h = READ_LE_UINT32(wizh + 0x8);
		Common::Rect rWiz(w, h);
		uint8 *wizd = findWrappedBlock(MKID('WIZD'), data, state, 0);
		assert(wizd);
		if (rCap.intersects(rWiz)) {
			rCap.clip(rWiz);
			uint32 histogram[256];
			memset(histogram, 0, sizeof(histogram));
			switch (c) {
			case 0:
				_wiz.computeRawWizHistogram(histogram, wizd, w, &rCap);
				break;
			case 1:
				_wiz.computeWizHistogram(histogram, wizd, &rCap);
				break;
			default:
				error("computeWizHistogram: Unhandled wiz compression type %d", c);
				break;
			}
			for (int i = 0; i < 256; ++i) {
				writeArray(0, 0, i, histogram[i]);
			}
		}
	}
	return readVar(0);
}

} // End of namespace Scumm
