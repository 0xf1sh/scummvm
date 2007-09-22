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

#include "common/savefile.h"

#include "gui/dialog.h"
#include "gui/widget.h"
#include "gui/ListWidget.h"
#include "gui/message.h"

#include "parallaction/parallaction.h"


/* Nippon Safes savefiles are called 'game.0' to 'game.9'. The game conventiently allows users to
 * give meanigful name to savegames, and it uses an extra file 'savegame' to keep track of these
 * names.
 *
 * This re-implementation disposes of the extra file to make moving of savefiles easier. Debugging
 * will benefit from this, too. The savegame name is written as the first line of the savefile
 * itself, thus breaking compatibility with the original version. Who cares anyway?
 */


namespace Parallaction {



class SaveLoadChooser : public GUI::Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	GUI::ListWidget		*_list;
	GUI::ButtonWidget	*_chooseButton;
	GUI::GraphicsWidget	*_gfxWidget;
	GUI::StaticTextWidget	*_date;
	GUI::StaticTextWidget	*_time;
	GUI::StaticTextWidget	*_playtime;
	GUI::ContainerWidget	*_container;
	Parallaction_ns			*_vm;

	uint8 _fillR, _fillG, _fillB;

public:
	SaveLoadChooser(const String &title, const String &buttonLabel, Parallaction_ns *engine);
	~SaveLoadChooser();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	const String &getResultString() const;
	void setList(const StringList& list);
	int runModal();

	virtual void reflowLayout();
};




void Parallaction_ns::doLoadGame(uint16 slot) {

	_introSarcData3 = 200;
	_introSarcData2 = 1;

	char filename[PATH_LEN];
	sprintf(filename, "game.%i", slot);

	Common::InSaveFile *f = _saveFileMan->openForLoading(filename);
	if (!f) return;

	char s[30];

	f->readLine(s, 29);

	f->readLine(s, 15);
	_char.setName(s);

	f->readLine(_location._name, 15);

	strcat(_location._name, ".");

	f->readLine(s, 15);
	_location._startPosition.x = atoi(s);

	f->readLine(s, 15);
	_location._startPosition.y = atoi(s);

	f->readLine(s, 15);
	_score = atoi(s);

	f->readLine(s, 15);
	_commandFlags = atoi(s);

	f->readLine(s, 15);

	// TODO (LIST): unify (and parametrize) calls to freeZones.
	// We aren't calling freeAnimations because it is not needed, since
	// kChangeLocation will trigger a complete deletion. Anyway, we still
	// need to invoke freeZones here with kEngineQuit set, because the
	// call in changeLocation preserve certain zones.
	_engineFlags |= kEngineQuit;
	freeZones();
	_engineFlags &= ~kEngineQuit;

	_numLocations = atoi(s);

	uint16 _si;
	for (_si = 0; _si < _numLocations; _si++) {
		f->readLine(s, 20);
		s[strlen(s)] = '\0';

		strcpy(_locationNames[_si], s);

		f->readLine(s, 15);
		_localFlags[_si] = atoi(s);
	}
	_locationNames[_si][0] = '\0';

	for (_si = 0; _si < 30; _si++) {
		f->readLine(s, 15);
		_inventory[_si]._id = atoi(s);

		f->readLine(s, 15);
		_inventory[_si]._index = atoi(s);
	}

	delete f;

	// force reload of character to solve inventory
	// bugs, but it's a good maneuver anyway
	strcpy(_characterName1, "null");

	strcat(_location._name, _char.getName());
	_engineFlags |= kEngineChangeLocation;

	return;
}


void Parallaction_ns::doSaveGame(uint16 slot, const char* name) {

	char path[PATH_LEN];
	sprintf(path, "game.%i", slot);

	Common::OutSaveFile *f = _saveFileMan->openForSaving(path);
	if (f == 0) {
		char buf[32];
		sprintf(buf, "Can't save game in slot %i\n\n(%s)", slot, path);
		GUI::MessageDialog dialog(buf);
		dialog.runModal();
		return;
	}

	char s[30];

	if (!name || name[0] == '\0') {
		sprintf(s, "default_%i", slot);
	} else {
		strncpy(s, name, 29);
	}

	f->writeString(s);
	f->writeString("\n");

	sprintf(s, "%s\n", _char.getFullName());
	f->writeString(s);

	sprintf(s, "%s\n", _saveData1);
	f->writeString(s);
	sprintf(s, "%d\n", _char._ani._left);
	f->writeString(s);
	sprintf(s, "%d\n", _char._ani._top);
	f->writeString(s);
	sprintf(s, "%d\n", _score);
	f->writeString(s);
	sprintf(s, "%u\n", _commandFlags);
	f->writeString(s);

	sprintf(s, "%d\n", _numLocations);
	f->writeString(s);
	for (uint16 _si = 0; _si < _numLocations; _si++) {
		sprintf(s, "%s\n%u\n", _locationNames[_si], _localFlags[_si]);
		f->writeString(s);
	}

	for (uint16 _si = 0; _si < 30; _si++) {
		sprintf(s, "%u\n%d\n", _inventory[_si]._id, _inventory[_si]._index);
		f->writeString(s);
	}

	delete f;

	return;
}

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT',
	kChooseCmd = 'CHOS'
};


SaveLoadChooser::SaveLoadChooser(const String &title, const String &buttonLabel, Parallaction_ns *engine)
	: Dialog("scummsaveload"), _list(0), _chooseButton(0), _gfxWidget(0), _vm(engine) {

//	_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;

	new GUI::StaticTextWidget(this, "scummsaveload_title", title);

	// Add choice list
	_list = new GUI::ListWidget(this, "scummsaveload_list");
	_list->setEditable(true);
	_list->setNumberingMode(GUI::kListNumberingOne);

	_container = new GUI::ContainerWidget(this, 0, 0, 10, 10);
	_container->setHints(GUI::THEME_HINT_USE_SHADOW);

	_gfxWidget = new GUI::GraphicsWidget(this, 0, 0, 10, 10);

	_date = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No date saved", GUI::kTextAlignCenter);
	_time = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No time saved", GUI::kTextAlignCenter);
	_playtime = new GUI::StaticTextWidget(this, 0, 0, 10, 10, "No playtime saved", GUI::kTextAlignCenter);

	// Buttons
	new GUI::ButtonWidget(this, "scummsaveload_cancel", "Cancel", GUI::kCloseCmd, 0);
	_chooseButton = new GUI::ButtonWidget(this, "scummsaveload_choose", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);
}

SaveLoadChooser::~SaveLoadChooser() {
}

const Common::String &SaveLoadChooser::getResultString() const {
	return _list->getSelectedString();
}

void SaveLoadChooser::setList(const StringList& list) {
	_list->setList(list);
}

int SaveLoadChooser::runModal() {
	if (_gfxWidget)
		_gfxWidget->setGfx(0);
	int ret = GUI::Dialog::runModal();
	return ret;
}

void SaveLoadChooser::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();
	switch (cmd) {
	case GUI::kListItemActivatedCmd:
	case GUI::kListItemDoubleClickedCmd:
		if (selItem >= 0) {
			if (!getResultString().empty()) {
				_list->endEditMode();
				setResult(selItem);
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		setResult(selItem);
		close();
		break;
	case GUI::kListSelectionChangedCmd: {
		_list->startEditMode();
		// Disable button if nothing is selected, or (in load mode) if an empty
		// list item is selected. We allow choosing an empty item in save mode
		// because we then just assign a default name.
		_chooseButton->setEnabled(selItem >= 0 && (!getResultString().empty()));
		_chooseButton->draw();
	} break;
	case GUI::kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooser::reflowLayout() {
	_container->setFlags(GUI::WIDGET_INVISIBLE);
	_gfxWidget->setFlags(GUI::WIDGET_INVISIBLE);
	_date->setFlags(GUI::WIDGET_INVISIBLE);
	_time->setFlags(GUI::WIDGET_INVISIBLE);
	_playtime->setFlags(GUI::WIDGET_INVISIBLE);

	Dialog::reflowLayout();
}

int Parallaction_ns::buildSaveFileList(Common::StringList& l) {

	char name[16];
	char buf[30];

	int count = 0;

	for (int i = 0; i < 10; i++) {
		sprintf(name, "game.%i", i);

		buf[0] = '\0';
		Common::InSaveFile *f = _saveFileMan->openForLoading(name);

		if (f) {
			f->readLine(buf, 29);
			delete f;

			count++;
		}

		l.push_back(buf);
	}

	return count;
}


int Parallaction_ns::selectSaveFile(uint16 arg_0, const char* caption, const char* button) {

	SaveLoadChooser* slc = new SaveLoadChooser(caption, button, this);

	Common::StringList l;

	/*int count = */ buildSaveFileList(l);
	slc->setList(l);

	int idx = slc->runModal();
	if (idx >= 0) {
		_saveFileName = slc->getResultString();
	}

	delete slc;

	return idx;
}



void Parallaction_ns::loadGame() {

	int _di = selectSaveFile( 0, "Load file", "Load" );
	if (_di == -1) {
		return;
	}

	doLoadGame(_di);

	GUI::TimedMessageDialog dialog("Loading game...", 1500);
	dialog.runModal();

	changeCursor(kCursorArrow);

	return;
}


void Parallaction_ns::saveGame() {

	if (!scumm_stricmp(_location._name, "caveau"))
		return;

	int slot = selectSaveFile( 1, "Save file", "Save" );
	if (slot == -1) {
		return;
	}

	doSaveGame(slot, _saveFileName.c_str());

	GUI::TimedMessageDialog dialog("Saving game...", 1500);
	dialog.runModal();

	return;


}





} // namespace Parallaction
