/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "gui/about.h"
#include "gui/dialog.h"
#include "gui/options.h"
#include "gui/widget.h"

#ifndef DISABLE_HELP
#include "scumm/help.h"
#endif

namespace GUI {
	class ListWidget;
}


namespace Scumm {

class ScummEngine;

class ScummDialog : public GUI::Dialog {
public:
	ScummDialog(ScummEngine *scumm, int x, int y, int w, int h)
		: GUI::Dialog(x, y, w, h), _vm(scumm) {}
	
protected:
	typedef Common::String String;

	ScummEngine *_vm;

	// Query a string from the resources
	const String queryResString(int stringno);
};

class SaveLoadChooser;

class MainMenuDialog : public ScummDialog {
public:
	MainMenuDialog(ScummEngine *scumm);
	~MainMenuDialog();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

protected:
	GUI::Dialog		*_aboutDialog;
	GUI::Dialog		*_optionsDialog;
#ifndef DISABLE_HELP
	GUI::Dialog		*_helpDialog;
#endif
	SaveLoadChooser	*_saveDialog;
	SaveLoadChooser	*_loadDialog;

	void save();
	void load();
};

#ifndef DISABLE_HELP

class HelpDialog : public ScummDialog {
public:
	HelpDialog(ScummEngine *scumm);
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

protected:
	typedef Common::String String;

	GUI::ButtonWidget *_nextButton;
	GUI::ButtonWidget *_prevButton;

	GUI::StaticTextWidget *_title;
	GUI::StaticTextWidget *_key[HELP_NUM_LINES];
	GUI::StaticTextWidget *_dsc[HELP_NUM_LINES];

	int _page;
	int _numPages;

	void displayKeyBindings();
};

#endif

class ConfigDialog : public GUI::OptionsDialog {
protected:
	ScummEngine *_vm;
#ifdef _WIN32_WCE
	GUI::Dialog		*_keysDialog;
#endif

public:
	ConfigDialog(ScummEngine *scumm);
	~ConfigDialog();

	virtual void open();
	virtual void close();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

protected:
	GUI::CheckboxWidget *subtitlesCheckbox;
};

/**
 * A dialog which displays an arbitrary message to the user and returns
 * ther users reply as its result value. More specifically, it returns
 * the ASCII code of the key used to close the dialog (0 if a mouse
 * click closed the dialog).
 */
class InfoDialog : public ScummDialog {
public:
	// arbitrary message
	InfoDialog(ScummEngine *scumm, const String& message);
	// from resources
	InfoDialog(ScummEngine *scumm, int res);

	virtual void handleMouseDown(int x, int y, int button, int clickCount) { 
		setResult(0);
		close();
	}
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers) {
		setResult(ascii);
		close();
	}

protected:
	void setInfoText (const String& message);
};

/**
 * The pause dialog, visible whenever the user activates pause mode. Goes
 * away uon any key or mouse button press.
 */
class PauseDialog : public InfoDialog {
public:
	PauseDialog(ScummEngine *scumm, int res);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
};

/**
 * A simple yes/no dialog, used to ask the user whether to really
 * quit/restart ScummVM.
 */
class ConfirmDialog : public InfoDialog {
public:
	ConfirmDialog(ScummEngine *scumm, const String& message);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
};

/**
 * A dialog used to display the music volume / text speed.
 * Given a label string, and a float value in the range of 0.0 to 1.0,
 * it will display a corresponding graphic.
 * Automatically closes after a brief time passed.
 */
class ValueDisplayDialog : public GUI::Dialog {
public:
	ValueDisplayDialog(const Common::String& label, int minVal, int maxVal, int val, uint16 incKey, uint16 decKey);

	void drawDialog();
	void handleTickle();

	virtual void handleMouseDown(int x, int y, int button, int clickCount) { 
		close();
	}
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:
	enum {
		kPercentBarWidth = 50,
		kDisplayDelay = 1500
	};
	Common::String _label;
	const int _min, _max;
	const uint16 _incKey, _decKey;
	int _value;
	uint32 _timer;
};

} // End of namespace Scumm

#endif
