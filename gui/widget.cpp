/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "widget.h"
#include "dialog.h"
#include "newgui.h"


#ifdef _MSC_VER
#	pragma warning( disable : 4068 ) // unknown pragma
#endif


Widget::Widget (Dialog *boss, int x, int y, int w, int h)
	: _type(0), _boss(boss), _x(x), _y(y), _w(w), _h(h),
	  _id(0), _flags(0), _hasFocus(false) {
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
}

void Widget::draw() {
	NewGui *gui = _boss->getGui();

	if (!isVisible() || !_boss->isVisible())
		return;

	// Account for our relative position in the dialog
	_x += _boss->_x;
	_y += _boss->_y;

	// Clear background (unless alpha blending is enabled)
	if (_flags & WIDGET_CLEARBG)
		gui->fillRect(_x, _y, _w, _h, gui->_bgcolor);

	// Draw border
	if (_flags & WIDGET_BORDER) {
		gui->box(_x, _y, _w, _h, (_flags & WIDGET_INV_BORDER) == WIDGET_INV_BORDER);
		_x += 4;
		_y += 4;
		_w -= 8;
	}

	// Now perform the actual widget draw
	drawWidget((_flags & WIDGET_HILITED) ? true : false);

	// Restore x/y
	if (_flags & WIDGET_BORDER) {
		_x -= 4;
		_y -= 4;
		_w += 8;
	}

	// Flag the draw area as dirty
	gui->addDirtyRect(_x, _y, _w, _h);

	_x -= _boss->_x;
	_y -= _boss->_y;
}

#pragma mark -

StaticTextWidget::StaticTextWidget(Dialog *boss, int x, int y, int w, int h, const String &text, int align)
	: Widget (boss, x, y, w, h), _align(align) {
	_type = kStaticTextWidget;
	setLabel(text);
}

void StaticTextWidget::setValue(int value) {
	char buf[256];
	sprintf(buf, "%d", value);
	_label = buf;
}

void StaticTextWidget::drawWidget(bool hilite) {
	NewGui *gui = _boss->getGui();
	gui->drawString(_label, _x, _y, _w, gui->_textcolor, _align);
}

#pragma mark -

ButtonWidget::ButtonWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, x, y, w, h, label, kTextAlignCenter), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey) {
	_flags = WIDGET_ENABLED | WIDGET_BORDER | WIDGET_CLEARBG;
	_type = kButtonWidget;
}

void ButtonWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h)
		sendCommand(_cmd, 0);
}

void ButtonWidget::drawWidget(bool hilite) {
	NewGui *gui = _boss->getGui();
	gui->drawString(_label, _x, _y, _w,
									!isEnabled() ? gui->_color :
									hilite ? gui->_textcolorhi : gui->_textcolor, _align);
}

#pragma mark -

PushButtonWidget::PushButtonWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey), _state(false) {
	_flags = WIDGET_ENABLED | WIDGET_BORDER | WIDGET_CLEARBG;
	_type = kButtonWidget;
}

void PushButtonWidget::setState(bool state) {
	if (_state != state) {
		_state = state;
		_flags ^= WIDGET_INV_BORDER;
		draw();
	}
}

#pragma mark -

/* 8x8 checkbox bitmap */
static uint32 checked_img[8] = {
	0x00000000,
	0x01000010,
	0x00100100,
	0x00011000,
	0x00011000,
	0x00100100,
	0x01000010,
	0x00000000,
};

CheckboxWidget::CheckboxWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey)
	: PushButtonWidget(boss, x, y, w, h, label, cmd, hotkey) {
	_flags = WIDGET_ENABLED;
	_type = kCheckboxWidget;
}

void CheckboxWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h) {
		toggleState();
		sendCommand(_cmd, 0);
	}
}

void CheckboxWidget::drawWidget(bool hilite) {
	NewGui *gui = _boss->getGui();

	// Draw the box
	gui->box(_x, _y, 14, 14);

	// If checked, draw cross inside the box
	if (_state)
		gui->drawBitmap(checked_img, _x + 3, _y + 3, !isEnabled() ? gui->_color : gui->_textcolor);
	else
		gui->fillRect(_x + 2, _y + 2, 10, 10, gui->_bgcolor);

	// Finally draw the label
	gui->drawString(_label, _x + 20, _y + 3, _w, !isEnabled() ? gui->_color : gui->_textcolor);
}

#pragma mark -

SliderWidget::SliderWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey),
	  _value(0), _valueMin(0), _valueMax(100) {
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kSliderWidget;
}

void SliderWidget::handleMouseMoved(int x, int y, int button) {
	// TODO: when the mouse is dragged outside the widget, the slider should
	// snap back to the old value.
	if (isEnabled() && _isDragging) {
		int newValue = posToValue(x);
		if (newValue < _valueMin)
			newValue = _valueMin;
		else if (newValue > _valueMax)
			newValue = _valueMax;

		if (newValue != _value) {
			_value = newValue; 
			draw();
			sendCommand(_cmd, _value);	// FIXME - hack to allow for "live update" in sound dialog
		}
	}
}

void SliderWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isEnabled()) {
		_isDragging = true;
		handleMouseMoved(x, y, button);
	}
}

void SliderWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && _isDragging) {
		sendCommand(_cmd, _value);
	}
	_isDragging = false;
}

void SliderWidget::drawWidget(bool hilite) {
	NewGui *gui = _boss->getGui();

	// Draw the box
	gui->box(_x, _y, _w, _h);

	// Draw the 'bar'
	gui->fillRect(_x + 2, _y + 2, valueToPos(_value), _h - 4, hilite ? gui->_textcolorhi : gui->_textcolor);
}

int SliderWidget::valueToPos(int value) {
	return ((_w - 4) * (value - _valueMin) / (_valueMax - _valueMin));
}

int SliderWidget::posToValue(int pos) {
	return (pos) * (_valueMax - _valueMin) / (_w - 4) + _valueMin;
}
