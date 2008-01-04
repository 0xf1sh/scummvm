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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "common/file.h"
#include "graphics/primitives.h"

#include "parallaction/parallaction.h"


namespace Parallaction {


void halfbritePixel(int x, int y, int color, void *data) {
    byte *buffer = (byte*)data;
    buffer[x + y * _vm->_screenWidth] &= ~0x20;
}

void drawCircleLine(int xCenter, int yCenter, int x, int y, int color, void (*plotProc)(int, int, int, void *), void *data){
	Graphics::drawLine(xCenter + x, yCenter + y, xCenter - x, yCenter + y, color, plotProc, data);
	Graphics::drawLine(xCenter + x, yCenter - y, xCenter - x, yCenter - y, color, plotProc, data);
	Graphics::drawLine(xCenter + y, yCenter + x, xCenter - y, yCenter + x, color, plotProc, data);
	Graphics::drawLine(xCenter + y, yCenter - x, xCenter - y, yCenter - x, color, plotProc, data);
}

void drawCircle(int xCenter, int yCenter, int radius, int color, void (*plotProc)(int, int, int, void *), void *data) {
	int x = 0;
	int y = radius;
	int p = 1 - radius;

	/* Plot first set of points */
	drawCircleLine(xCenter, yCenter, x, y, color, plotProc, data);

	while (x < y) {
		x++;
		if (p < 0)
			p += 2*x + 1;
		else {
			y--;
			p += 2 * (x-y) + 1;
		}
		drawCircleLine(xCenter, yCenter, x, y, color, plotProc, data);
	}
}




Palette::Palette() {

	int gameType = _vm->getGameType();

	if (gameType == GType_Nippon) {
		_colors = 32;
		_hb = (_vm->getPlatform() == Common::kPlatformAmiga);
	} else
	if (gameType == GType_BRA) {
		_colors = 256;
		_hb = false;
	} else
		error("can't create palette for id = '%i'", gameType);

	_size = _colors * 3;

	makeBlack();
}

Palette::Palette(const Palette &pal) {
	clone(pal);
}

void Palette::clone(const Palette &pal) {
	_colors = pal._colors;
	_hb = pal._hb;
	_size = pal._size;
	memcpy(_data, pal._data, _size);
}


void Palette::makeBlack() {
	memset(_data, 0, _size);
}

void Palette::setEntry(uint index, int red, int green, int blue) {
	assert(index < _colors);

	if (red >= 0)
		_data[index*3] = red & 0xFF;

	if (green >= 0)
		_data[index*3+1] = green & 0xFF;

	if (blue >= 0)
		_data[index*3+2] = blue & 0xFF;
}

void Palette::makeGrayscale() {
	byte v;
	for (uint16 i = 0; i < _colors; i++) {
		v = MAX(_data[i*3+1], _data[i*3+2]);
		v = MAX(v, _data[i*3]);
		setEntry(i, v, v, v);
	}
}

void Palette::fadeTo(const Palette& target, uint step) {

	if (step == 0)
		return;

	for (uint16 i = 0; i < _size; i++) {
		if (_data[i] == target._data[i]) continue;

		if (_data[i] < target._data[i])
			_data[i] = CLIP(_data[i] + step, (uint)0, (uint)target._data[i]);
		else
			_data[i] = CLIP(_data[i] - step, (uint)target._data[i], (uint)255);
	}

	return;
}

uint Palette::fillRGBA(byte *rgba) {

	byte r, g, b;
	byte *hbPal = rgba + _colors * 4;

	for (uint32 i = 0; i < _colors; i++) {
		r = (_data[i*3]   << 2) | (_data[i*3]   >> 4);
		g = (_data[i*3+1] << 2) | (_data[i*3+1] >> 4);
		b = (_data[i*3+2] << 2) | (_data[i*3+2] >> 4);

		rgba[i*4]   = r;
		rgba[i*4+1] = g;
		rgba[i*4+2] = b;
		rgba[i*4+3] = 0;

		if (_hb) {
			hbPal[i*4]   = r >> 1;
			hbPal[i*4+1] = g >> 1;
			hbPal[i*4+2] = b >> 1;
			hbPal[i*4+3] = 0;
		}

	}

	return ((_hb) ? 2 : 1) * _colors;
}

void Palette::rotate(uint first, uint last, bool forward) {

	byte tmp[3];

	if (forward) { 					// forward

		tmp[0] = _data[first * 3];
		tmp[1] = _data[first * 3 + 1];
		tmp[2] = _data[first * 3 + 2];

		memmove(_data+first*3, _data+(first+1)*3, (last - first)*3);

		_data[last * 3]	 = tmp[0];
		_data[last * 3 + 1] = tmp[1];
		_data[last * 3 + 2] = tmp[2];

	} else {											// backward

		tmp[0] = _data[last * 3];
		tmp[1] = _data[last * 3 + 1];
		tmp[2] = _data[last * 3 + 2];

		memmove(_data+(first+1)*3, _data+first*3, (last - first)*3);

		_data[first * 3]	  = tmp[0];
		_data[first * 3 + 1] = tmp[1];
		_data[first * 3 + 2] = tmp[2];

	}

}


#define BALLOON_TAIL_WIDTH	12
#define BALLOON_TAIL_HEIGHT	10


byte _resBalloonTail[2][BALLOON_TAIL_WIDTH*BALLOON_TAIL_HEIGHT] = {
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	},
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x02, 0x02
	}
};

void Gfx::drawBalloon(const Common::Rect& r, uint16 winding) {
//	printf("Gfx::drawBalloon(%i, %i, %i, %i, %i)...", left, top, width, height, winding);

	Common::Rect q = r;

	// draws balloon
	q.right += 5;
	floodFill(kBitFront, q, 0);
	q.grow(-1);
	floodFill(kBitFront, q, 1);

	// draws tail
	// TODO: this bitmap tail should only be used for Dos games. Amiga should use a polygon fill.
	winding = (winding == 0 ? 1 : 0);
	Common::Rect s(BALLOON_TAIL_WIDTH, BALLOON_TAIL_HEIGHT);
	s.moveTo(r.left + (r.width()+5)/2 - 5, r.bottom - 1);
	flatBlit(s, _resBalloonTail[winding], _buffers[kBitFront], 2);

	return;
}




void Gfx::setPalette(Palette pal) {
	byte sysPal[256*4];

	uint n = pal.fillRGBA(sysPal);
	g_system->setPalette(sysPal, 0, n);
}

void Gfx::setBlackPalette() {
	Palette pal;
	setPalette(pal);
}




void Gfx::animatePalette() {

	// avoid forcing setPalette when not needed
	bool done = false;

	for (uint16 i = 0; i < 4; i++) {

		if ((_palettefx[i]._flags & 1) == 0) continue;		// animated palette

		_palettefx[i]._timer += _palettefx[i]._step * 2;	// update timer

		if (_palettefx[i]._timer < 0x4000) continue;		// check timeout

		_palettefx[i]._timer = 0;							// reset timer

		_palette.rotate(_palettefx[i]._first, _palettefx[i]._last, (_palettefx[i]._flags & 2) != 0);

		done = true;
	}

	if (done) {
		setPalette(_palette);
	}

	return;
}





void Gfx::setHalfbriteMode(bool enable) {
	if (_vm->getPlatform() != Common::kPlatformAmiga) return;
	if (enable == _halfbrite) return;

	_halfbrite = !_halfbrite;

	if (!enable) {
	    _hbCircleRadius = 0;
	}
}

#define HALFBRITE_CIRCLE_RADIUS             48
void Gfx::setProjectorPos(int x, int y) {
    _hbCircleRadius = HALFBRITE_CIRCLE_RADIUS;
    _hbCirclePos.x = x + _hbCircleRadius;
    _hbCirclePos.y = y + _hbCircleRadius;
}


void Gfx::drawInventory() {

	if ((_engineFlags & kEngineInventory) == 0) {
		return;
	}

	Common::Rect r;
	_vm->_inventoryRenderer->getRect(r);
	byte *data = _vm->_inventoryRenderer->getData();

	g_system->copyRectToScreen(data, r.width(), r.left, r.top, r.width(), r.height());
}

void Gfx::drawItems() {
	if (_numItems == 0) {
		return;
	}

	Graphics::Surface *surf = g_system->lockScreen();
	for (uint i = 0; i < _numItems; i++) {
		flatBlit(_items[i].rect, _items[i].data->getData(_items[i].frame), surf, 0);
	}
	g_system->unlockScreen();
}

void Gfx::drawBalloons() {
	if (_numBalloons == 0) {
		return;
	}

	Graphics::Surface *surf = g_system->lockScreen();
	for (uint i = 0; i < _numBalloons; i++) {
		Common::Rect r(_balloons[i].surface.w, _balloons[i].surface.h);
		r.moveTo(_balloons[i].x, _balloons[i].y);
		flatBlit(r, (byte*)_balloons[i].surface.getBasePtr(0, 0), surf, 2);
	}
	g_system->unlockScreen();
}

void Gfx::updateScreen() {
    if (_halfbrite) {
        Graphics::Surface *surf = g_system->lockScreen();
        byte *src = (byte*)_buffers[kBitFront]->pixels;
        byte *dst = (byte*)surf->pixels;
        for (int i = 0; i < surf->w*surf->h; i++) {
            *dst++ = *src++ | 0x20;
        }
        if (_hbCircleRadius > 0) {
            drawCircle(_hbCirclePos.x, _hbCirclePos.y, _hbCircleRadius, 0, &halfbritePixel, surf->pixels);
        }
        g_system->unlockScreen();
    } else {
        g_system->copyRectToScreen((const byte*)_buffers[kBitFront]->pixels, _buffers[kBitFront]->pitch, _screenX, _screenY, _vm->_screenWidth, _vm->_screenHeight);
    }

	drawInventory();

	drawItems();
	drawBalloons();

	drawLabel();

	g_system->updateScreen();
	return;
}


void Gfx::swapBuffers() {
	Graphics::Surface *temp = _buffers[kBitFront];
	_buffers[kBitFront] = _buffers[kBitBack];
	_buffers[kBitBack] = temp;
	updateScreen();
	return;
}


//
//	graphic primitives
//
void Gfx::clearScreen(Gfx::Buffers buffer) {
	memset(_buffers[buffer]->pixels, 0, _vm->_screenSize);

	if (buffer == kBitFront) updateScreen();

	return;
}


void Gfx::copyScreen(Gfx::Buffers srcbuffer, Gfx::Buffers dstbuffer) {
	memcpy(_buffers[dstbuffer]->pixels, _buffers[srcbuffer]->pixels, _vm->_screenSize);

	return;
}

void Gfx::floodFill(Gfx::Buffers buffer, const Common::Rect& r, byte color) {

	byte *d = (byte*)_buffers[buffer]->getBasePtr(r.left, r.top);
	uint16 w = r.width() + 1;
	uint16 h = r.height() + 1;

	for (uint16 i = 0; i < h; i++) {
		memset(d, color, w);

		d += _backgroundWidth;
	}

	return;
}

void Gfx::invertRect(Gfx::Buffers buffer, const Common::Rect& r) {

	byte *d = (byte*)_buffers[buffer]->getBasePtr(r.left, r.top);

	for (int i = 0; i < r.height(); i++) {
		for (int j = 0; j < r.width(); j++) {
			*d ^= 0x1F;
			d++;
		}

		d += (_buffers[buffer]->pitch - r.width());
	}

}

void Gfx::flatBlit(const Common::Rect& r, byte *data, Graphics::Surface *surf, byte transparentColor) {

	Common::Point dp;
	Common::Rect q(r);

	Common::Rect clipper(surf->w, surf->h);

	q.clip(clipper);
	if (!q.isValidRect()) return;

	dp.x = q.left;
	dp.y = q.top;

	q.translate(-r.left, -r.top);

	byte *s = data + q.left + q.top * r.width();
	byte *d = (byte*)surf->getBasePtr(dp.x, dp.y);

	uint sPitch = r.width() - q.width();
	uint dPitch = surf->w - q.width();

	for (uint16 i = q.top; i < q.bottom; i++) {
		for (uint16 j = q.left; j < q.right; j++) {
			if (*s != transparentColor)
				*d = *s;

			s++;
			d++;
		}

		s += sPitch;
		d += dPitch;
	}

	return;


}

void Gfx::blit(const Common::Rect& r, uint16 z, byte *data, Graphics::Surface *surf) {

	Common::Point dp;
	Common::Rect q(r);

	Common::Rect clipper(surf->w, surf->h);

	q.clip(clipper);
	if (!q.isValidRect()) return;

	dp.x = q.left;
	dp.y = q.top;

	q.translate(-r.left, -r.top);


	byte *s = data + q.left + q.top * r.width();
	byte *d = (byte*)surf->getBasePtr(dp.x, dp.y);

	uint sPitch = r.width() - q.width();
	uint dPitch = surf->w - q.width();

	for (uint16 i = 0; i < q.height(); i++) {

		for (uint16 j = 0; j < q.width(); j++) {
			if (*s != 0) {
				byte v = _depthMask->getValue(dp.x + j, dp.y + i);
				if (z >= v) *d = *s;
			}

			s++;
			d++;
		}

		s += sPitch;
		d += dPitch;
	}

	return;

}

void Gfx::setLabel(Label *label) {
	_label = label;

	if (_label) {
		_label->resetPosition();
	}
}

void Gfx::drawLabel() {
	if (!_label || !_label->_text) {
		return;
	}

	int16 _si, _di;

	Common::Point	cursor;
	_vm->getCursorPos(cursor);

	if (_vm->_activeItem._id != 0) {
		_si = cursor.x + 16 - _label->_cnv.w/2;
		_di = cursor.y + 34;
	} else {
		_si = cursor.x + 8 - _label->_cnv.w/2;
		_di = cursor.y + 21;
	}

	if (_si < 0) _si = 0;
	if (_di > 190) _di = 190;

	if (_label->_cnv.w + _si > _vm->_screenWidth)
		_si = _vm->_screenWidth - _label->_cnv.w;

	_label->_pos.x = _si;
	_label->_pos.y = _di;

	Common::Rect r(_label->_cnv.w, _label->_cnv.h);
	r.moveTo(_label->_pos);

	Graphics::Surface* surf = g_system->lockScreen();
	flatBlit(r, (byte*)_label->_cnv.getBasePtr(0, 0), surf, 0);
	g_system->unlockScreen();
}


//
//	Cnv management
//
void Gfx::flatBlitCnv(Frames *cnv, uint16 frame, int16 x, int16 y, Gfx::Buffers buffer) {

	Graphics::Surface scnv;
	Common::Rect r;

	cnv->getRect(frame, r);

	scnv.w = r.width();
	scnv.h = r.height();
	scnv.pixels = cnv->getData(frame);

	flatBlitCnv(&scnv, x, y, buffer);
}

void Gfx::flatBlitCnv(Graphics::Surface *cnv, int16 x, int16 y, Gfx::Buffers buffer) {
	Common::Rect r(cnv->w, cnv->h);
	r.moveTo(x, y);

	flatBlit(r, (byte*)cnv->pixels, _buffers[buffer], 0);
	return;
}


void Gfx::blitCnv(Graphics::Surface *cnv, int16 x, int16 y, uint16 z, Gfx::Buffers buffer) {
	Common::Rect r(cnv->w, cnv->h);
	r.moveTo(x, y);

	blit(r, z, (byte*)cnv->pixels, _buffers[buffer]);
	return;
}

void Gfx::backupDoorBackground(DoorData *data, int16 x, int16 y) {
	byte *s = (byte*)_buffers[kBit2]->getBasePtr(x, y);
	Common::Rect r;
	data->_cnv->getRect(0, r);
	copyRect(r.width(), r.height(), data->_background, r.width(), s,_backgroundWidth);
	return;
}

void Gfx::backupGetBackground(GetData *data, int16 x, int16 y) {

	byte *t = (byte*)data->_cnv->getData(0);
	byte *s = (byte*)_buffers[kBitBack]->getBasePtr(x, y);
	byte *d = data->_backup;

	Common::Rect r;
	data->_cnv->getRect(0, r);

	uint pitch = _backgroundWidth - r.width();

	for (uint16 i = 0; i < r.height(); i++) {
		for (uint16 j = 0; j < r.width(); j++) {
			*d = (*t) ? *s : 0;

			d++;
			t++;
			s++;
		}

		s += pitch;
	}

	return;
}

//
//	restores background according to specified frame
//
void Gfx::restoreDoorBackground(const Common::Rect& r, byte *data, byte* background) {

	byte *t = data;
	byte *s = background;
	byte *d0 = (byte*)_buffers[kBitBack]->getBasePtr(r.left, r.top);
	byte *d1 = (byte*)_buffers[kBit2]->getBasePtr(r.left, r.top);

	uint pitch = _backgroundWidth - r.width();

	for (uint16 i = 0; i < r.height() ; i++) {
		for (uint16 j = 0; j < r.width() ; j++) {
			if (*t) {
				*d0 = *s;
				*d1 = *s;
			}

			d0++;
			d1++;
			t++;
			s++;
		}

		d0 += pitch;
		d1 += pitch;
	}


	return;
}


//
//	copies a rectangular bitmap on the background
//
void Gfx::restoreGetBackground(const Common::Rect& r, byte *data) {

	flatBlit(r, data, _buffers[kBitBack], 0);
	flatBlit(r, data, _buffers[kBit2], 0);

	return;
}



void Gfx::displayString(uint16 x, uint16 y, const char *text, byte color) {
	byte *dst = (byte*)_buffers[kBitFront]->getBasePtr(x, y);
	if (_fontShadow) {
        dst = (byte*)_buffers[kBitFront]->getBasePtr(x-2, y+2);
        _font->setColor(0);
        _font->drawString(dst, _vm->_screenWidth, text);
	}

	dst = (byte*)_buffers[kBitFront]->getBasePtr(x, y);
	_font->setColor(color);
	_font->drawString(dst, _vm->_screenWidth, text);
}

void Gfx::displayCenteredString(uint16 y, const char *text) {
	uint16 x = (_vm->_screenWidth - getStringWidth(text)) / 2;
	displayString(x, y, text, 1);
}

bool Gfx::displayWrappedString(char *text, uint16 x, uint16 y, byte color, int16 wrapwidth) {

	uint16 lines = 0;
	bool rv = false;
	uint16 linewidth = 0;

	uint16 rx = x + 10;
	uint16 ry = y + 4;

	char token[40];

	if (wrapwidth == -1)
		wrapwidth = _vm->_screenWidth;

	while (strlen(text) > 0) {

		text = parseNextToken(text, token, 40, "   ", true);

		if (!scumm_stricmp(token, "%p")) {
			lines++;
			rx = x + 10;
			ry = y + 4 + lines*10;	// y

			strcpy(token, "> .......");
			strncpy(token+2, _password, strlen(_password));
			rv = true;
		} else {

			linewidth += getStringWidth(token);

			if (linewidth > wrapwidth) {
				// wrap line
				lines++;
				rx = x + 10;			// x
				ry = y + 4 + lines*10;	// y
				linewidth = getStringWidth(token);
			}

			if (!scumm_stricmp(token, "%s")) {
				sprintf(token, "%d", _score);
			}

		}

		displayString(rx, ry, token, color);

		rx += getStringWidth(token) + getStringWidth(" ");
		linewidth += getStringWidth(" ");

		text = Common::ltrim(text);
	}

	return rv;

}

uint16 Gfx::getStringWidth(const char *text) {
	return _font->getStringWidth(text);
}

void Gfx::getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height) {

	uint16 lines = 0;
	uint16 w = 0;
	*width = 0;

	char token[40];

	while (strlen(text) != 0) {

		text = parseNextToken(text, token, 40, "   ", true);
		w += getStringWidth(token);

		if (!scumm_stricmp(token, "%p")) {
			lines++;
		} else {
			if (w > maxwidth) {
				w -= getStringWidth(token);
				lines++;
				if (w > *width)
					*width = w;

				w = getStringWidth(token);
			}
		}

		w += getStringWidth(" ");
		text = Common::ltrim(text);
	}

	if (*width < w) *width = w;
	*width += 10;

	*height = lines * 10 + 20;

	return;
}


void Gfx::setFont(Font *font) {
	assert(font);
	_font = font;
	setFontShadow(false);
}

void Gfx::setFontShadow(bool enable) {
    _fontShadow = enable && (_vm->getPlatform() == Common::kPlatformAmiga);
}

void Gfx::restoreBackground(const Common::Rect& r) {

	int16 left = r.left;
	int16 top = r.top;
	int16 width = r.width();
	int16 height = r.height();

	if (left < 0) left = 0;
	if (top < 0) top = 0;

	if (left >= _backgroundWidth) return;
	if (top >= _backgroundHeight) return;

	if (left+width >= _backgroundWidth) width = _backgroundWidth - left;
	if (top+height >= _backgroundHeight) height = _backgroundHeight - top;

	Common::Rect q(width, height);
	q.moveTo(left, top);

	copyRect(kBitBack, q, (byte*)_buffers[kBit2]->getBasePtr(q.left, q.top), _backgroundWidth);

	return;
}



void Gfx::setBackground(Graphics::Surface *surface) {
	_buffers[kBit2] = surface;

	initBuffers(surface->w, surface->h);
	copyScreen(kBit2, kBitBack);
}

void Gfx::setMask(MaskBuffer *buffer) {
	_depthMask = buffer;
}

void Gfx::copyRect(uint width, uint height, byte *dst, uint dstPitch, byte *src, uint srcPitch) {

	for (uint16 _si = 0; _si < height; _si++) {
		memcpy(dst, src, width);

		src += srcPitch;
		dst += dstPitch;
	}

	return;
}

void Gfx::copyRect(Gfx::Buffers dstbuffer, const Common::Rect& r, byte *src, uint16 pitch) {
	byte *d = (byte*)_buffers[dstbuffer]->getBasePtr(r.left, r.top);
	copyRect(r.width(), r.height(), d, _backgroundWidth, src, pitch);
	return;
}


void Gfx::grabRect(byte *dst, const Common::Rect& r, Gfx::Buffers srcbuffer, uint16 pitch) {
	byte *s = (byte*)_buffers[srcbuffer]->getBasePtr(r.left, r.top);
	copyRect(r.width(), r.height(), dst, pitch, s, _backgroundWidth);
	return;
}


int16 Gfx::queryMask(int16 v) {

	for (uint16 _si = 0; _si < 3; _si++) {
		if (_bgLayers[_si+1] > v) return _si;
	}

	return 3;
}

Gfx::Gfx(Parallaction* vm) :
	_vm(vm) {

	g_system->beginGFXTransaction();
	g_system->initSize(_vm->_screenWidth, _vm->_screenHeight);
	_vm->initCommonGFX(_vm->getGameType() == GType_BRA);
	g_system->endGFXTransaction();

	_buffers[kBit2] = 0;
	_depthMask = 0;

	_buffers[kBitFront] = 0;
	_buffers[kBitBack] = 0;

	initBuffers(_vm->_screenWidth, _vm->_screenHeight);

	setPalette(_palette);

	_numBalloons = 0;
	_numItems = 0;
	_label = 0;

	_screenX = 0;
	_screenY = 0;

	_bgLayers[0] = _bgLayers[1] = _bgLayers[2] = _bgLayers[3] = 0;

	memset(_palettefx, 0, sizeof(_palettefx));

	_halfbrite = false;
    _hbCircleRadius = 0;

	_font = NULL;
	_fontShadow = false;

	return;
}

Gfx::~Gfx() {

	freeBuffers();

	return;
}

void Gfx::initBuffers(int w, int h) {

	_backgroundWidth = w;
	_backgroundHeight = h;

	if (!_buffers[kBitFront]) {
		_buffers[kBitFront] = new Graphics::Surface;
	}

	if (!_buffers[kBitBack]) {
		_buffers[kBitBack] = new Graphics::Surface;
	}

	if (_buffers[kBitFront]->w != w || _buffers[kBitFront]->h != h) {
		_buffers[kBitFront]->create(w, h, 1);
	}

	if (_buffers[kBitBack]->w != w || _buffers[kBitBack]->h != h) {
		_buffers[kBitBack]->create(w, h, 1);
	}

}

void Gfx::freeBuffers() {

	if (_buffers[kBitFront]) {
		_buffers[kBitFront]->free();
		delete _buffers[kBitFront];
	}

	if (_buffers[kBitBack]) {
		_buffers[kBitBack]->free();
		delete _buffers[kBitBack];
	}

	_buffers[kBitFront] = 0;
	_buffers[kBitBack] = 0;
}


void Gfx::setItem(Frames* frames, uint16 x, uint16 y) {
	_items[_numItems].data = frames;
	_items[_numItems].x = x;
	_items[_numItems].y = y;
	_numItems++;
}

void Gfx::setItemFrame(uint item, uint16 f) {
	assert(item < _numItems);
	_items[item].frame = f;
	_items[item].data->getRect(f, _items[item].rect);
	_items[item].rect.moveTo(_items[item].x, _items[item].y);
}

Gfx::Balloon *Gfx::createBalloon(char *text, uint16 maxwidth, uint16 winding) {
	assert(_numBalloons < 5);

	Gfx::Balloon *balloon = &_balloons[_numBalloons];
	_numBalloons++;

	int16 w, h;
	getStringExtent(text, maxwidth, &w, &h);

	balloon->surface.create(w + 5, h + 9, 1);
	balloon->surface.fillRect(Common::Rect(w + 5, h + 9), 2);

	Common::Rect r(w + 5, h);
	balloon->surface.fillRect(r, 0);
	r.grow(-1);
	balloon->surface.fillRect(r, 1);

	// draws tail
	// TODO: this bitmap tail should only be used for Dos games. Amiga should use a polygon fill.
	winding = (winding == 0 ? 1 : 0);
	Common::Rect s(BALLOON_TAIL_WIDTH, BALLOON_TAIL_HEIGHT);
	s.moveTo(r.width()/2 - 5, r.bottom - 1);
	flatBlit(s, _resBalloonTail[winding], &balloon->surface, 2);

	return balloon;
}


void Gfx::setDialogueBalloon(char *text, uint16 x, uint16 y, uint16 maxwidth, uint16 winding, byte textColor) {

	Common::Rect rect;

	setFont(_vm->_dialogueFont);
	Gfx::Balloon *balloon = createBalloon(text, maxwidth, winding);
	drawWrappedText(&balloon->surface, text, textColor, maxwidth);

	balloon->x = x;
	balloon->y = y;
}

void Gfx::freeBalloons() {
	for (uint i = 0; i < _numBalloons; i++) {
		_balloons[i].surface.free();
	}
	_numBalloons = 0;
}

void Gfx::freeItems() {
	_numItems = 0;
}

void Gfx::hideDialogueStuff() {
	freeItems();
	freeBalloons();
}

void Gfx::drawText(Graphics::Surface* surf, uint16 x, uint16 y, const char *text, byte color) {
	byte *dst = (byte*)surf->getBasePtr(x, y);
	_font->setColor(color);
	_font->drawString(dst, surf->w, text);
}

bool Gfx::drawWrappedText(Graphics::Surface* surf, char *text, byte color, int16 wrapwidth) {

	uint16 lines = 0;
	bool rv = false;
	uint16 linewidth = 0;

	uint16 rx = 10;
	uint16 ry = 4;

	char token[40];

	if (wrapwidth == -1)
		wrapwidth = _vm->_screenWidth;

	while (strlen(text) > 0) {

		text = parseNextToken(text, token, 40, "   ", true);

		if (!scumm_stricmp(token, "%p")) {
			lines++;
			rx = 10;
			ry = 4 + lines*10;	// y

			strcpy(token, "> .......");
			strncpy(token+2, _password, strlen(_password));
			rv = true;
		} else {

			linewidth += getStringWidth(token);

			if (linewidth > wrapwidth) {
				// wrap line
				lines++;
				rx = 10;			// x
				ry = 4 + lines*10;	// y
				linewidth = getStringWidth(token);
			}

			if (!scumm_stricmp(token, "%s")) {
				sprintf(token, "%d", _score);
			}

		}

		drawText(surf, rx, ry, token, color);

		rx += getStringWidth(token) + getStringWidth(" ");
		linewidth += getStringWidth(" ");

		text = Common::ltrim(text);
	}

	return rv;

}

} // namespace Parallaction
