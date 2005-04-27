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

#include "common/scaler/intern.h"
#include "common/scaler/scalebit.h"
#include "common/util.h"


int gBitFormat = 565;

// RGB-to-YUV lookup table
extern "C" {

#ifdef USE_NASM
// NOTE: if your compiler uses different mangled names, add another
//       condition here

#ifndef _WIN32
#define RGBtoYUV _RGBtoYUV
#define LUT16to32 _LUT16to32
#endif

#endif

// FIXME/TODO: The following two tables suck up 512 KB.
// They should at least be allocated on the heap, to reduce the size of the
// binary. 
//
// Note: a memory lookup table is *not* necessarily faster than computing
// these things on the fly, because of its size. Both tables together, plus 
// the code, plus the input/output GFX data, won't fit in the cache on many
// systems, so main memory has to be accessed, which is about the worst thing
// that can happen to code which tries to be fast...
//
// So we should think about ways to get these smaller / removed. The LUT16to32
// is only used by the HQX asm right now; maybe somebody can modify the code
// there to work w/o it (and do some benchmarking, too?). To do that, just
// do the conversion on the fly, or even do w/o it (as the C++ code manages to),
// by making different versions of the code based on gBitFormat (or by writing
// bit masks into registers which are computed based on gBitFormat).
//
// RGBtoYUV is also used by the C(++) version of the HQX code. Maybe we can
// use the same technique which is employed by our MPEG code to reduce the
// size of the lookup tables at the cost of some additional computations? That
// might actually result in a speedup, too, if done right (and the code code
// might actually be suitable for AltiVec/MMX/SSE speedup).
// 
// Of course, the above is largely a conjecture, and the actual speed
// differences are likely to vary a lot between different architectures and
// CPUs.
uint RGBtoYUVstorage[65536];
uint *RGBtoYUV = RGBtoYUVstorage;
uint LUT16to32[65536];
}

static const uint16 dotmatrix_565[16] = {
	0x01E0, 0x0007, 0x3800, 0x0000,
	0x39E7, 0x0000, 0x39E7, 0x0000,
	0x3800, 0x0000, 0x01E0, 0x0007,
	0x39E7, 0x0000, 0x39E7, 0x0000
};
static const uint16 dotmatrix_555[16] = {
	0x00E0, 0x0007, 0x1C00, 0x0000,
	0x1CE7, 0x0000, 0x1CE7, 0x0000,
	0x1C00, 0x0000, 0x00E0, 0x0007,
	0x1CE7, 0x0000, 0x1CE7, 0x0000
};
static const uint16 *dotmatrix;

static void InitLUT(uint32 BitFormat);

void InitScalers(uint32 BitFormat) {
	if (BitFormat == 565) {
		dotmatrix = dotmatrix_565;
	} else if (BitFormat == 555) {
		dotmatrix = dotmatrix_555;
	} else {
		error("Unknown bit format %d", BitFormat);
	}

	gBitFormat = BitFormat;
	InitLUT(BitFormat);
}

void InitLUT(uint32 BitFormat) {
	int r, g, b;
	int Y, u, v;
	int gInc, gShift;
	
	for (int i = 0; i < 65536; i++) {
		LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	}

	if (BitFormat == 565) {
		gInc = 256 >> 6;
		gShift = 6 - 3;
	} else {
		gInc = 256 >> 5;
		gShift = 5 - 3;
	}

	for (r = 0; r < 256; r += 8) {
		for (g = 0; g < 256; g += gInc) {
			for (b = 0; b < 256; b += 8) {
				Y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2 * g - b) >> 3);
				RGBtoYUV[ (r << (5 + gShift)) + (g << gShift) + (b >> 3) ] = (Y << 16) + (u << 8) + v;
			}
		}
	}
}

/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destionation.
 */
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	while (height--) {
		memcpy(dstPtr, srcPtr, 2 * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

/**
 * Trivial nearest-neighbour 2x scaler.
 */
void Normal2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;

	assert(((int)dstPtr & 3) == 0);
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 4) {
			uint32 color = *(((const uint16 *)srcPtr) + i);
			
			color |= color << 16;

			*(uint32 *)(r) = color;
			*(uint32 *)(r + dstPitch) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
	}
}

/**
 * Trivial nearest-neighbour 3x scaler.
 */
void Normal3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;

	assert(((int)dstPtr & 1) == 0);
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 6) {
			uint16 color = *(((const uint16 *)srcPtr) + i);

			*(uint16 *)(r + 0) = color;
			*(uint16 *)(r + 2) = color;
			*(uint16 *)(r + 4) = color;
			*(uint16 *)(r + 0 + dstPitch) = color;
			*(uint16 *)(r + 2 + dstPitch) = color;
			*(uint16 *)(r + 4 + dstPitch) = color;
			*(uint16 *)(r + 0 + dstPitch2) = color;
			*(uint16 *)(r + 2 + dstPitch2) = color;
			*(uint16 *)(r + 4 + dstPitch2) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch3;
	}
}

#define INTERPOLATE		INTERPOLATE<bitFormat>
#define Q_INTERPOLATE	Q_INTERPOLATE<bitFormat>

/**
 * Trivial nearest-neighbour 1.5x scaler.
 */
template<int bitFormat>
void Normal1o5xTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;
	const uint32 srcPitch2 = srcPitch * 2;

	assert(((int)dstPtr & 1) == 0);
	while (height > 0) {
		r = dstPtr;
		for (int i = 0; i < width; i += 2, r += 6) {
			uint16 color0 = *(((const uint16 *)srcPtr) + i);
			uint16 color1 = *(((const uint16 *)srcPtr) + i + 1);
			uint16 color2 = *(((const uint16 *)(srcPtr + srcPitch)) + i);
			uint16 color3 = *(((const uint16 *)(srcPtr + srcPitch)) + i + 1);

			*(uint16 *)(r + 0) = color0;
			*(uint16 *)(r + 2) = INTERPOLATE(color0, color1);
			*(uint16 *)(r + 4) = color1;
			*(uint16 *)(r + 0 + dstPitch) = INTERPOLATE(color0, color2);
			*(uint16 *)(r + 2 + dstPitch) = Q_INTERPOLATE(color0, color1, color2, color3);
			*(uint16 *)(r + 4 + dstPitch) = INTERPOLATE(color1, color3);
			*(uint16 *)(r + 0 + dstPitch2) = color2;
			*(uint16 *)(r + 2 + dstPitch2) = INTERPOLATE(color2, color3);
			*(uint16 *)(r + 4 + dstPitch2) = color3;
		}
		srcPtr += srcPitch2;
		dstPtr += dstPitch3;
		height -= 2;
	}
}
MAKE_WRAPPER(Normal1o5x)

/**
 * The Scale2x filter, also known as AdvMame2x.
 * See also http://scale2x.sourceforge.net
 */
void AdvMame2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	scale(2, dstPtr, dstPitch, srcPtr - srcPitch, srcPitch, 2, width, height);
}

/**
 * The Scale3x filter, also known as AdvMame3x.
 * See also http://scale2x.sourceforge.net
 */
void AdvMame3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	scale(3, dstPtr, dstPitch, srcPtr - srcPitch, srcPitch, 2, width, height);
}

template<int bitFormat>
void TV2xTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, 
					int width, int height) {
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	while (height--) {
		for (int i = 0, j = 0; i < width; ++i, j += 2) {
			uint16 p1 = *(p + i);
			uint32 pi;

			pi = (((p1 & redblueMask) * 7) >> 3) & redblueMask;
			pi |= (((p1 & greenMask) * 7) >> 3) & greenMask;

			*(q + j) = p1;
			*(q + j + 1) = p1;
			*(q + j + nextlineDst) = (uint16)pi;
			*(q + j + nextlineDst + 1) = (uint16)pi;
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}
MAKE_WRAPPER(TV2x)

static inline uint16 DOT_16(uint16 c, int j, int i) {
	return c - ((c >> 2) & *(dotmatrix + ((j & 3) << 2) + (i & 3)));
}

// FIXME: This scaler doesn't quite work. Either it needs to know where on the
// screen it's drawing, or the dirty rects will have to be adjusted so that
// access to the dotmatrix array are made in a consistent way. (Doing that in
// a way that also works together with aspect-ratio correction is left as an
// exercise for the reader.)

void DotMatrix(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height) {
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	for (int j = 0, jj = 0; j < height; ++j, jj += 2) {
		for (int i = 0, ii = 0; i < width; ++i, ii += 2) {
			uint16 c = *(p + i);
			*(q + ii) = DOT_16(c, jj, ii);
			*(q + ii + 1) = DOT_16(c, jj, ii + 1);
			*(q + ii + nextlineDst) = DOT_16(c, jj + 1, ii);
			*(q + ii + nextlineDst + 1) = DOT_16(c, jj + 1, ii + 1);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}
