/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#ifndef WIDGET_H
#define WIDGET_H

#include "scummsys.h"
#include "common/str.h"

class Dialog;

enum {
	WIDGET_ENABLED		= 1 << 0,
	WIDGET_INVISIBLE	= 1 << 1,
	WIDGET_HILITED		= 1 << 2,
	WIDGET_BORDER		= 1 << 3,
	WIDGET_CLEARBG		= 1 << 4,
	WIDGET_WANT_TICKLE	= 1 << 5,
	WIDGET_TRACK_MOUSE	= 1 << 6,
	WIDGET_RETAIN_FOCUS	= 1 << 7		// Retain focus on mouse up. By default widgets lose focus on mouseup, but some widgets might want to retain it - widgets where you enter text, for instance

};

enum {
	kStaticTextWidget	= 'TEXT',
	kButtonWidget		= 'BTTN',
	kCheckboxWidget		= 'CHKB',
	kSliderWidget		= 'SLDE',
	kListWidget			= 'LIST',
	kScrollBarWidget	= 'SCRB'
};

enum {
	kButtonWidth = 54,
	kButtonHeight = 16,
};



class CommandReceiver;
class CommandSender;

class CommandReceiver
{
friend class CommandSender;
protected:
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) = 0;
};

class CommandSender
{
protected:
	CommandReceiver	*_target;
public:
	CommandSender(CommandReceiver *target) : _target(target) {}

	void setTarget(CommandReceiver *target)	{ _target = target; }
	CommandReceiver *getTarget() const		{ return _target; }

	virtual void sendCommand(uint32 cmd, uint32 data)
	{
		if (_target && cmd)
			_target->handleCommand(this, cmd, data);
	}
};

/* Widget */
class Widget {
friend class Dialog;
protected:
	uint32		_type;
	Dialog		*_boss;
	Widget		*_next;
	int16		_x, _y;
	uint16		_w, _h;
	uint16		_id;
	uint16		_flags;
	bool		_hasFocus;

public:
	Widget(Dialog *boss, int x, int y, int w, int h);
	virtual ~Widget() {}

	virtual void handleMouseDown(int x, int y, int button, int clickCount) {}
	virtual void handleMouseUp(int x, int y, int button, int clickCount) {}
	virtual void handleMouseEntered(int button) {}
	virtual void handleMouseLeft(int button) {}
	virtual void handleMouseMoved(int x, int y, int button) {}
	virtual void handleMouseWheel(int x, int y, int direction) {}
	virtual bool handleKeyDown(char key, int modifiers) { return false; }	// Return true if the event was handled
	virtual bool handleKeyUp(char key, int modifiers) { return false; }	// Return true if the event was handled
	virtual void handleTickle() {}
	void draw();
	void receivedFocus() { _hasFocus = true; receivedFocusWidget(); }
	void lostFocus() { _hasFocus = false; lostFocusWidget(); }
	virtual bool wantsFocus() { return false; };

	void setFlags(int flags)	{ _flags |= flags; }
	void clearFlags(int flags)	{ _flags &= ~flags; }
	int getFlags() const		{ return _flags; }
	
	void setEnabled(bool e)		{ if (e) setFlags(WIDGET_ENABLED); else clearFlags(WIDGET_ENABLED); }
	bool isEnabled() const		{ return _flags & WIDGET_ENABLED; }
	bool isVisible() const		{ return !(_flags & WIDGET_INVISIBLE); }

protected:
	virtual void drawWidget(bool hilite) {}

	virtual void receivedFocusWidget() {}
	virtual void lostFocusWidget() {}
};


/* StaticTextWidget */
class StaticTextWidget : public Widget {
protected:
	typedef ScummVM::String String;

	String _label;
	int		_align;
public:
	StaticTextWidget(Dialog *boss, int x, int y, int w, int h, const String &text, int align);
	void setValue(int value);
	void setLabel(const String &label) { _label = label; }
	const String &getLabel() const     { return _label; }
	void setAlign(int align)           { _align = align; }
	int getAlign() const               { return _align; }

protected:
	void drawWidget(bool hilite);
};


/* ButtonWidget */
class ButtonWidget : public StaticTextWidget, public CommandSender {
	friend class Dialog;	// Needed for the hotkey handling
protected:
	uint32			_cmd;
	uint8			_hotkey;
public:
	ButtonWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd = 0, uint8 hotkey = 0);

	void setCmd(uint32 cmd)					{ _cmd = cmd; }
	uint32 getCmd() const					{ return _cmd; }

	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); draw(); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); draw(); }

protected:
	void drawWidget(bool hilite);
};

/* CheckboxWidget */
class CheckboxWidget : public ButtonWidget {
protected:
	bool	_state;
public:
	CheckboxWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd = 0, uint8 hotkey = 0);
	void setState(bool state)	{ _state = state; }
	bool getState() const		{ return _state; }

	void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseEntered(int button)	{}
	virtual void handleMouseLeft(int button)	{}

protected:
	void drawWidget(bool hilite);
};

/* SliderWidget */
class SliderWidget : public ButtonWidget {
protected:
	int		_value, _oldValue;
	int		_valueMin, _valueMax;
	bool	_isDragging;
public:
	SliderWidget(Dialog *boss, int x, int y, int w, int h, const String &label, uint32 cmd = 0, uint8 hotkey = 0);
	void setValue(int value)	{ _value = value; }
	int getValue() const		{ return _value; }

	void setMinValue(int value)	{ _valueMin = value; }
	int getMinValue() const		{ return _valueMin; }
	void setMaxValue(int value)	{ _valueMax = value; }
	int getMaxValue() const		{ return _valueMax; }

	void handleMouseMoved(int x, int y, int button);
	void handleMouseDown(int x, int y, int button, int clickCount);
	void handleMouseUp(int x, int y, int button, int clickCount);

protected:
	void drawWidget(bool hilite);
	
	int valueToPos(int value);
	int posToValue(int pos);
};


#endif
