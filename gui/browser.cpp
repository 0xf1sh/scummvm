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

#include "browser.h"
#include "newgui.h"
#include "ListWidget.h"

#include "backends/fs/fs.h"

/* We want to use this as a general directory selector at some point... possible uses
 * - to select the data dir for a game
 * - to select the place where save games are stored
 * - others???
 */

enum {
	kChooseCmd = 'Chos',
	kGoUpCmd = 'GoUp'
};

BrowserDialog::BrowserDialog(NewGui *gui, const char *title)
	: Dialog(gui, 40, 10, 320 -2 * 40, 200 - 2 * 10),
	  _node(0), _nodeContent(0) {
	// Headline - TODO: should be customizable during creation time
	new StaticTextWidget(this, 10, 8, _w-2 * 10, kLineHeight, title, kTextAlignCenter);

	// Current path - TODO: handle long paths ?
	_currentPath = new StaticTextWidget(this, 10, 20, _w - 2 * 10, kLineHeight,
								"DUMMY", kTextAlignLeft);

	// Add file list
	_fileList = new ListWidget(this, 10, 34, _w - 2 * 10, _h - 34 - 24 - 10);
	_fileList->setNumberingMode(kListNumberingOff);

	// Buttons
	addButton(10, _h - 24, "Go up", kGoUpCmd, 0);
	addButton(_w-2 * (kButtonWidth+10), _h - 24, "Cancel", kCloseCmd, 0);
	addButton(_w-(kButtonWidth+10), _h-24, "Choose", kChooseCmd, 0);
}

BrowserDialog::~BrowserDialog() {
	delete _node;
	delete _nodeContent;
	delete _choice;
}

void BrowserDialog::open() {
	// If no node has been set, or the last used one is now invalid,
	// go back to the root/default dir.
	if (_node == NULL || !_node->isValid()) {
		delete _node;
		_node = FilesystemNode::getRoot();
		assert(_node != NULL);
	}

	// Alway refresh file list
	updateListing();

	// Nothing chosen by default
	delete _choice;
	_choice = 0;
	
	// Call super implementation
	Dialog::open();
}

void BrowserDialog::close() {
	delete _nodeContent;
	_nodeContent = 0;

	// Call super implementation
	Dialog::close();
}

void BrowserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	FilesystemNode *tmp;
	
	switch (cmd) {
	case kChooseCmd: {
			// If nothing is selected in the list widget, choose the current dir.
			// Else, choose the dir that is selected.
			int selection = _fileList->getSelected();
			if (selection >= 0) {
				_choice = (*_nodeContent)[selection].clone();
			} else {
				_choice = _node->clone();
			}
			setResult(1);
			close();
		}
		break;
	case kGoUpCmd:
		tmp = _node->parent();
		delete _node;
		_node = tmp;
		updateListing();
		break;
	case kListItemDoubleClickedCmd:
		tmp = (*_nodeContent)[data].clone();
		delete _node;
		_node = tmp;
		updateListing();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void BrowserDialog::updateListing() {
	assert(_node != NULL);

	// Update the path display
	_currentPath->setLabel(_node->path());

	// Read in the data from the file system
	delete _nodeContent;
	_nodeContent = _node->listDir();
	assert(_nodeContent != NULL);

	// Populate the ListWidget
	ScummVM::StringList list;
	int size = _nodeContent->size();
	for (int i = 0; i < size; i++) {
		list.push_back((*_nodeContent)[i].displayName());
	}
	_fileList->setList(list);
	_fileList->scrollTo(0);
	
	// Finally, redraw
	draw();
}

