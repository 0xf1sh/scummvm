/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Misc. graphics routines

// Line drawing code utilizes Bresenham's run-length slice algorithm 
// described in "Michael Abrash's Graphics Programming Black Book", 
// Coriolis Group Books, 1997

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/interface.h"

#include "common/system.h"


namespace Saga {

Gfx::Gfx(OSystem *system, int width, int height, GameDetector &detector) : _system(system) {
	SURFACE back_buf;

	_system->beginGFXTransaction();
		_vm->initCommonGFX(detector);
		_system->initSize(width, height);
	_system->endGFXTransaction();

	debug(0, "Init screen %dx%d", width, height);
	// Convert surface data to R surface data
	back_buf.pixels = calloc(1, width * height);
	back_buf.w = width;
	back_buf.h = height;
	back_buf.pitch = width;
	back_buf.bytesPerPixel = 1;

	back_buf.clip_rect.left = 0;
	back_buf.clip_rect.top = 0;
	back_buf.clip_rect.right = width;
	back_buf.clip_rect.bottom = height;

	// Set module data
	_back_buf = back_buf;
	_init = 1;
	_white_index = -1;
	_black_index = -1;

	// For now, always show the mouse cursor.
	setCursor();
	_system->showMouse(true);
}

/*
~Gfx() {
  free(GfxModule.r_back_buf->pixels);
}
 */

int drawPalette(SURFACE *dst_s) {
	int x;
	int y;
	int color = 0;

	Rect pal_rect;

	for (y = 0; y < 16; y++) {
		pal_rect.top = (y * 8) + 4;
		pal_rect.bottom = pal_rect.top + 8;

		for (x = 0; x < 16; x++) {
			pal_rect.left = (x * 8) + 4;
			pal_rect.right = pal_rect.left + 8;

			drawRect(dst_s, &pal_rect, color);
			color++;
		}
	}

	return 0;
}

// TODO: I've fixed at least one clipping bug here, but I have a feeling there
//       are several more. 

// * Copies a rectangle from a raw 8 bit pixel buffer to the specified surface.
// The buffer is of width 'src_w' and height 'src_h'. The rectangle to be 
// copied is defined by 'src_rect'.  
// The rectangle is copied to the destination surface at point 'dst_pt'.
// - If dst_pt is NULL, the buffer is rectangle is copied to the destination 
//    origin.
// - If src_rect is NULL, the entire buffer is copied./
// - The surface must match the logical dimensions of the buffer exactly.
// - Returns FAILURE on error
int bufToSurface(SURFACE *ds, const byte *src, int src_w, int src_h, 
					 Rect *src_rect, Point *dst_pt) {
	const byte *read_p;
	byte *write_p;

	int row;

	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	// Clamp source rectangle to source buffer
	if (src_rect != NULL) {
		src_rect->clip(src_w, src_h);

		s = *src_rect;
	} else {
		s.left = 0;
		s.top = 0;
		s.right = src_w;
		s.bottom = src_h;
	}

	if (s.width() <= 0 || s.height() <= 0) {
		// Empty or negative region
		return FAILURE;
	}

	// Get destination origin and clip rectangle
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip = ds->clip_rect;

	if (clip.left == clip.right) {
		clip.left = 0;
		clip.right = ds->w;
	}

	if (clip.top == clip.bottom) {
		clip.top = 0;
		clip.bottom = ds->h;
	}

	// Clip source rectangle to destination surface
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s.left;
	src_off_y = s.top;
	src_draw_w = s.width();
	src_draw_h = s.height();

	// Clip to left edge

	if (d_x < clip.left) {
		if (d_x <= (-src_draw_w)) {
			// dst rect completely off left edge
			return SUCCESS;
		}

		src_off_x += (clip.left - d_x);
		src_draw_w -= (clip.left - d_x);

		dst_off_x = clip.left;
	}

	// Clip to top edge

	if (d_y < clip.top) {
		if (d_y >= (-src_draw_h)) {
			// dst rect completely off top edge
			return SUCCESS;
		}

		src_off_y += (clip.top - d_y);
		src_draw_h -= (clip.top - d_y);

		dst_off_y = clip.top;
	}

	// Clip to right edge

	if (d_x >= clip.right) {
		// dst rect completely off right edge
		return SUCCESS;
	}

	if ((d_x + src_draw_w) > clip.right) {
		src_draw_w = clip.right - d_x;
	}

	// Clip to bottom edge

	if (d_y > clip.bottom) {
		// dst rect completely off bottom edge
		return SUCCESS;
	}

	if ((d_y + src_draw_h) > clip.bottom) {
		src_draw_h = clip.bottom - d_y;
	}

	// Transfer buffer data to surface
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = ((byte *)ds->pixels + dst_off_x) + (ds->pitch * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {
		memcpy(write_p, read_p, src_draw_w);

		write_p += ds->pitch;
		read_p += src_w;
	}

	return SUCCESS;
}

int bufToBuffer(byte *dst_buf, int dst_w, int dst_h, const byte *src,
					int src_w, int src_h, Rect *src_rect, Point *dst_pt) {
	const byte *read_p;
	byte *write_p;
	int row;

	Common::Rect s;
	int d_x, d_y;
	Common::Rect clip;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	// Clamp source rectangle to source buffer
	if (src_rect != NULL) {
		src_rect->clip(src_w, src_h);

		s.left = src_rect->left;
		s.top = src_rect->top;
		s.right = src_rect->right;
		s.bottom = src_rect->bottom;
	} else {
		s.left = 0;
		s.top = 0;
		s.right = src_w;
		s.bottom = src_h;
	}

	if (s.width() <= 0 || s.height() <= 0) {
		// Empty or negative region
		return FAILURE;
	}

	// Get destination origin and clip rectangle
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip.left = 0;
	clip.top = 0;
	clip.right = dst_w;
	clip.bottom = dst_h;

	// Clip source rectangle to destination surface
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s.left;
	src_off_y = s.top;
	src_draw_w = s.width();
	src_draw_h = s.height();

	// Clip to left edge

	if (d_x < clip.left) {
		if (d_x <= (-src_draw_w)) {
			// dst rect completely off left edge
			return SUCCESS;
		}

		src_off_x += (clip.left - d_x);
		src_draw_w -= (clip.left - d_x);

		dst_off_x = clip.left;
	}

	// Clip to top edge

	if (d_y < clip.top) {
		if (d_y >= (-src_draw_h)) {
			// dst rect completely off top edge
			return SUCCESS;
		}

		src_off_y += (clip.top - d_y);
		src_draw_h -= (clip.top - d_y);

		dst_off_y = clip.top;
	}

	// Clip to right edge

	if (d_x >= clip.right) {
		// dst rect completely off right edge
		return SUCCESS;
	}

	if ((d_x + src_draw_w) > clip.right) {
		src_draw_w = clip.right - d_x;
	}

	// Clip to bottom edge

	if (d_y >= clip.bottom) {
		// dst rect completely off bottom edge
		return SUCCESS;
	}

	if ((d_y + src_draw_h) > clip.bottom) {
		src_draw_h = clip.bottom - d_y;
	}

	// Transfer buffer data to surface
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = (dst_buf + dst_off_x) + (dst_w * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {
		memcpy(write_p, read_p, src_draw_w);

		write_p += dst_w;
		read_p += src_w;
	}

	return SUCCESS;
}

// Fills a rectangle in the surface ds from point 'p1' to point 'p2' using
// the specified color.
int drawRect(SURFACE *ds, const Rect *dst_rect, int color) {
	Rect r(ds->w, ds->h);

	if (dst_rect != NULL) {
		r = *dst_rect;
		r.clip(ds->w, ds->h);

		if (!r.isValidRect()) {
			// Empty or negative region
			return FAILURE;
		}
	}
	
	ds->fillRect(r, color);

	return SUCCESS;
}

int drawFrame(SURFACE *ds, const Point *p1, const Point *p2, int color) {
	int min_x;
	int max_x;
	int min_y;
	int max_y;

	assert((ds != NULL) && (p1 != NULL) && (p2 != NULL));
	
	min_x = MIN(p1->x, p2->x);
	max_x = MAX(p1->x, p2->x);
	min_y = MIN(p1->y, p2->y);
	max_y = MAX(p1->y, p2->y);

	ds->frameRect(Common::Rect(min_x, min_y, max_x+1, max_y+1), color);

	return SUCCESS;
}

int drawPolyLine(SURFACE *ds, const Point *pts, int pt_ct, int draw_color) {
	const Point *first_pt = pts;
	int last_i = 1;
	int i;

	assert((ds != NULL) & (pts != NULL));

	if (pt_ct < 3) {
		return FAILURE;
	}

	for (i = 1; i < pt_ct; i++) {
		drawLine(ds, &pts[i], &pts[i - 1], draw_color);
		last_i = i;
	}

	drawLine(ds, &pts[last_i], first_pt, draw_color);

	return SUCCESS;
}

int getClipInfo(CLIPINFO *clipinfo) {
	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	if (clipinfo == NULL) {
		return FAILURE;
	}

	if (clipinfo->dst_pt != NULL) {
		d_x = clipinfo->dst_pt->x;
		d_y = clipinfo->dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	s = *clipinfo->src_rect;

	clip = *clipinfo->dst_rect;

	// Clip source rectangle to destination surface
	clipinfo->dst_draw_x = d_x;
	clipinfo->dst_draw_y = d_y;
	clipinfo->src_draw_x = s.left;
	clipinfo->src_draw_y = s.top;
	clipinfo->draw_w = s.right - s.left;
	clipinfo->draw_h = s.bottom - s.top;

	clipinfo->nodraw = 0;

	// Clip to left edge
	if (d_x < clip.left) {
		if (d_x <= -(clipinfo->draw_w)) {
			// dst rect completely off left edge
			clipinfo->nodraw = 1;
			return SUCCESS;
		}

		clipinfo->src_draw_x += (clip.left - d_x);
		clipinfo->draw_w -= (clip.left - d_x);
		clipinfo->dst_draw_x = clip.left;
	}

	// Clip to top edge
	if (d_y < clip.top) {
		if (d_y <= -(clipinfo->draw_h)) {
			// dst rect completely off top edge
			clipinfo->nodraw = 1;
			return SUCCESS;
		}

		clipinfo->src_draw_y += (clip.top - d_y);
		clipinfo->draw_h -= (clip.top - d_y);

		clipinfo->dst_draw_y = clip.top;
	}

	// Clip to right edge
	if (d_x >= clip.right) {
		// dst rect completely off right edge
		clipinfo->nodraw = 1;
		return SUCCESS;
	}

	if ((d_x + clipinfo->draw_w) > clip.right) {
		clipinfo->draw_w = clip.right - d_x;
	}

	// Clip to bottom edge
	if (d_y >= clip.bottom) {
		// dst rect completely off bottom edge
		clipinfo->nodraw = 1;
		return SUCCESS;
	}

	if ((d_y + clipinfo->draw_h) > clip.bottom) {
		clipinfo->draw_h = clip.bottom - d_y;
	}

	return SUCCESS;
}

int clipLine(SURFACE *ds, const Point *src_p1, const Point *src_p2, 
				 Point *dst_p1, Point *dst_p2) {
	const Point *n_p1;
	const Point *n_p2;

	Common::Rect clip;
	int left, top, right, bottom;
	int dx, dy;

	float m;
	int y_icpt_l, y_icpt_r;

	clip = ds->clip_rect;

	// Normalize points by x
		if (src_p1->x < src_p2->x) {
		n_p1 = src_p1;
		n_p2 = src_p2;
	} else {
		n_p1 = src_p2;
		n_p2 = src_p1;
	}

	dst_p1->x = n_p1->x;
	dst_p1->y = n_p1->y;
	dst_p2->x = n_p2->x;
	dst_p2->y = n_p2->y;

	left = n_p1->x;
	top = n_p1->y;
	right = n_p2->x;
	bottom = n_p2->y;

	dx = right - left;
	dy = bottom - top;

	if (left < 0) {
		if (right < 0) {
			// Line completely off left edge
			return -1;
		}

		// Clip to left edge
		m = ((float)bottom - top) / (right - left);
		y_icpt_l = (int)(top - (left * m) + 0.5f);

		dst_p1->x = 0;
		dst_p1->y = y_icpt_l;
	}

	if (bottom > clip.right) {
		if (left > clip.right) {
			// Line completely off right edge
			return -1;
		}

		// Clip to right edge
		m = ((float)top - bottom) / (right - left);
		y_icpt_r = (int)(top - ((clip.right - left) * m) + 0.5f);

		dst_p1->y = y_icpt_r;
		dst_p2->x = clip.right;
	}

	return 1;
}

// Utilizes Bresenham's run-length slice algorithm described in
// "Michael Abrash's Graphics Programming Black Book", 
// Coriolis Group Books, 1997
//
// Performs no clipping
void drawLine(SURFACE *ds, const Point *p1, const Point *p2, int color) {
	byte *write_p;
	int clip_result;
	int temp;
	int error_up, error_down;
	int error;
	int x_vector;
	int dx, dy;
	int min_run;
	int init_run;
	int run;
	int end_run;
	Point clip_p1, clip_p2;
	int left, top, right, bottom;
	int i, k;

	clip_result = clipLine(ds, p1, p2, &clip_p1, &clip_p2);
	if (clip_result < 0) {
		// Line not visible
		return;
	}

	left = clip_p1.x;
	top = clip_p1.y;
	right = clip_p2.x;
	bottom = clip_p2.y;

	if ((left < ds->clip_rect.left) || (right < ds->clip_rect.left) || (left > ds->clip_rect.right) || (right > ds->clip_rect.right)) {
		return;
	}

	if ((top < ds->clip_rect.top) || (bottom < ds->clip_rect.top) || (top > ds->clip_rect.bottom) || (bottom > ds->clip_rect.bottom)) {
		return;
	}

	if (top > bottom) {
		temp = top;
		top = bottom;
		bottom = temp;
		temp = left;
		left = right;
		right = temp;
	}

	write_p = (byte *)ds->pixels + (top * ds->pitch) + left;
	dx = right - left;

	if (dx < 0) {
		x_vector = -1;
		dx = -dx;
	} else {
		x_vector = 1;
	}

	dy = bottom - top;

	if (dx == 0) {
		for (i = 0; i <= dy; i++) {
			*write_p = (byte) color;
			write_p += ds->pitch;
		}
		return;
	}
	if (dy == 0) {
		for (i = 0; i <= dx; i++) {
			*write_p = (byte) color;
			write_p += x_vector;
		}
		return;
	}
	if (dx == dy) {
		for (i = 0; i <= dx; i++) {
			*write_p = (byte) color;
			write_p += x_vector + ds->pitch;
		}
		return;
	}

	if (dx >= dy) {

		min_run = dx / dy;
		error_up = (dx % dy) * 2;
		error_down = dy * 2;
		error = (dx % dy) - (dy * 2);
		init_run = (min_run / 2) + 1;
		end_run = init_run;

		if ((error_up == 0) && (min_run & 0x01) == 0) {
			init_run--;
		}

		error += dy;

		// Horiz. seg
		for (k = 0; k < init_run; k++) {
			*write_p = (byte) color;
			write_p += x_vector;
		}
		write_p += ds->pitch;

		for (i = 0; i < (dy - 1); i++) {
			run = min_run;
			if ((error += error_up) > 0) {

				run++;
				error -= error_down;
			}

			// Horiz. seg
			for (k = 0; k < run; k++) {
				*write_p = (byte) color;
				write_p += x_vector;
			}
			write_p += ds->pitch;
		}

		// Horiz. seg
		for (k = 0; k < end_run; k++) {
			*write_p = (byte) color;
			write_p += x_vector;
		}
		write_p += ds->pitch;
		return;

	} else {

		min_run = dy / dx;
		error_up = (dy % dx) * 2;
		error_down = dx * 2;
		error = (dy % dx) - (dx * 2);
		init_run = (min_run / 2) + 1;
		end_run = init_run;

		if ((error_up == 0) && ((min_run & 0x01) == 0)) {
			init_run--;
		}

		if ((min_run & 0x01) != 0) {
			error += dx;
		}

		// Vertical seg
		for (k = 0; k < init_run; k++) {
			*write_p = (byte) color;
			write_p += ds->pitch;
		}
		write_p += x_vector;

		for (i = 0; i < (dx - 1); i++) {
			run = min_run;
			if ((error += error_up) > 0) {
				run++;
				error -= error_down;
			}

			// Vertical seg
			for (k = 0; k < run; k++) {
				*write_p = (byte) color;
				write_p += ds->pitch;
			}
			write_p += x_vector;
		}

		// Vertical seg
		for (k = 0; k < end_run; k++) {
			*write_p = (byte) color;
			write_p += ds->pitch;
		}
		write_p += x_vector;
		return;
	}

	return;
}

SURFACE *Gfx::getBackBuffer() {
	return &_back_buf;
}

int Gfx::getWhite(void) {
	return _white_index;
}

int Gfx::getBlack(void) {
	return _black_index;
}

int Gfx::matchColor(unsigned long colormask) {
	int i;
	int red = (colormask & 0x0FF0000UL) >> 16;
	int green = (colormask & 0x000FF00UL) >> 8;
	int blue = colormask & 0x00000FFUL;
	int dr;
	int dg;
	int db;
	long color_delta;
	long best_delta = LONG_MAX;
	int best_index = 0;
	byte *ppal;

	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		dr = ppal[0] - red;
		dr = ABS(dr);
		dg = ppal[1] - green;
		dg = ABS(dg);
		db = ppal[2] - blue;
		db = ABS(db);
		ppal[3] = 0;

		color_delta = (long)(dr * RED_WEIGHT + dg * GREEN_WEIGHT + db * BLUE_WEIGHT);

		if (color_delta == 0) {
			return i;
		}

		if (color_delta < best_delta) {
			best_delta = color_delta;
			best_index = i;
		}
	}

	return best_index;
}

int Gfx::setPalette(SURFACE *surface, PALENTRY *pal) {
	byte red;
	byte green;
	byte blue;
	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;
	int i;
	byte *ppal;

	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		red = pal[i].red;
		ppal[0] = red;
		color_delta = red;
		green = pal[i].green;
		ppal[1] = green;
		color_delta += green;
		blue = pal[i].blue;
		ppal[2] = blue;
		color_delta += blue;
		ppal[3] = 0;

		if (color_delta < best_bdelta) {
			best_bindex = i;
			best_bdelta = color_delta;
		}

		if (color_delta > best_wdelta) {
			best_windex = i;
			best_wdelta = color_delta;
		}
	}

	// Set whitest and blackest color indices
	_white_index = best_windex;
	_black_index = best_bindex;

	_system->setPalette(_cur_pal, 0, PAL_ENTRIES);

	return SUCCESS;
}

int Gfx::getCurrentPal(PALENTRY *src_pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		src_pal[i].red = ppal[0];
		src_pal[i].green = ppal[1];
		src_pal[i].blue = ppal[2];
	}

	return SUCCESS;
}

int Gfx::palToBlack(SURFACE *surface, PALENTRY *src_pal, double percent) {
	int i;
	//int fade_max = 255;
	int new_entry;
	byte *ppal;

	double fpercent;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry 
	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	_system->setPalette(_cur_pal, 0, PAL_ENTRIES);

	return SUCCESS;
}

int Gfx::blackToPal(SURFACE *surface, PALENTRY *src_pal, double percent) {
	int new_entry;
	double fpercent;
	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;
	byte *ppal;
	int i;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red - src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green - src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue - src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	// Find the best white and black color indices again
	if (percent >= 1.0) {
		for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
			color_delta = ppal[0];
			color_delta += ppal[1];
			color_delta += ppal[2];

			if (color_delta < best_bdelta) {
				best_bindex = i;
				best_bdelta = color_delta;
			}

			if (color_delta > best_wdelta) {
				best_windex = i;
				best_wdelta = color_delta;
			}
		}
	}

	_system->setPalette(_cur_pal, 0, PAL_ENTRIES);

	return SUCCESS;
}

void Gfx::showCursor(bool state) {
	updateCursor();
	g_system->showMouse(state);
}

void Gfx::setCursor() {
	// Set up the mouse cursor
	const byte A = kITEColorLightGrey;
	const byte B = kITEColorWhite;

	const byte cursor_img[CURSOR_W * CURSOR_H] = {
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		A, A, A, B, A, A, A,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
	};

	_system->setMouseCursor(cursor_img, CURSOR_W, CURSOR_H, 3, 3, 0);
}

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point) {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	const Point *vtx0 = &points[npoints - 1];
	const Point *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);
	for (pt = 0; pt < npoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= test_point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

} // End of namespace Saga
