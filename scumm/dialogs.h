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

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "gui/dialog.h"

class ListWidget;
class Scumm;

class ScummDialog : public Dialog {
public:
	ScummDialog(NewGui *gui, Scumm *scumm, int x, int y, int w, int h)
		: Dialog(gui, x, y, w, h), _scumm(scumm) {}
	
protected:
	typedef ScummVM::String String;

	Scumm *_scumm;

	void addResText(int x, int y, int w, int h, int resID);

	// Query a string from the resources
	const String queryResString(int stringno);
	
	// Query a custom string. This is in a seperate method so that we
	// can easily localize the messages in the future if we want to.
	const char *queryCustomString(int stringno);
};

class SaveLoadDialog : public ScummDialog {
public:
	SaveLoadDialog(NewGui *gui, Scumm *scumm);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void open();	
	virtual void close();

protected:
	ListWidget       *_savegameList;
	
	PushButtonWidget *_saveButton;
	PushButtonWidget *_loadButton;
	
	bool			_saveMode;

	void fillList();
	void save();
	void load();
	void switchToSaveMode();
	void switchToLoadMode();
};


class AboutDialog : public ScummDialog {
public:
	AboutDialog(NewGui *gui, Scumm *scumm);
};


class OptionsDialog : public ScummDialog {
protected:
	Dialog		*_aboutDialog;
#ifdef _WIN32_WCE
	Dialog		*_keysDialog;
#endif

public:
	OptionsDialog(NewGui *gui, Scumm *scumm);
	~OptionsDialog();

	virtual void open();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	
	int _soundVolumeMaster;
	int _soundVolumeMusic;
	int _soundVolumeSfx;

	SliderWidget *masterVolumeSlider;
	SliderWidget *musicVolumeSlider;
	SliderWidget *sfxVolumeSlider;
	
	StaticTextWidget *masterVolumeLabel;
	StaticTextWidget *musicVolumeLabel;
	StaticTextWidget *sfxVolumeLabel;
	
	CheckboxWidget *subtitlesCheckbox;
	CheckboxWidget *amigaPalCheckbox;
};

class InfoDialog : public ScummDialog {
public:
	// arbitrary message
	InfoDialog(NewGui *gui, Scumm *scumm, const String& message);
	// from resources
	InfoDialog(NewGui *gui, Scumm *scumm, int res);

	virtual void handleMouseDown(int x, int y, int button, int clickCount)
		{ close(); }
	virtual void handleKeyDown(char key, int modifiers)
		{
			if (key == ' ')  // Close pause dialog if space key is pressed
				close();
			else
				ScummDialog::handleKeyDown(key, modifiers);
		}
protected:
	void setInfoText (const String& message);
};

class PauseDialog : public InfoDialog {
public:
	PauseDialog(NewGui *gui, Scumm *scumm);
};

#ifdef _WIN32_WCE

class KeysDialog : public ScummDialog {
public:
	KeysDialog(NewGui *gui, Scumm *scumm);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(char key, int modifiers);

protected:

	ListWidget		 *_actionsList;
	StaticTextWidget *_actionTitle;
	StaticTextWidget *_keyMapping;
	int				 _actionSelected;
};

#endif

#endif