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

// TODO - how about we #define WINDOWS for all our windows targets?
#if defined(_MSC_VER) || defined(__MINGW32__)

#include "../fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <TCHAR.h>

/*
 * Implementation of the ScummVM file system API based on Windows API.
 */

class WindowsFilesystemNode : public FilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;
	String _path;
	WindowsFilesystemNode *_parentNode;
	
public:
	WindowsFilesystemNode();
	WindowsFilesystemNode(const String &path);
	WindowsFilesystemNode(const WindowsFilesystemNode *node);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual FSList *listDir() const;
	virtual FilesystemNode *parent() const;
	virtual FilesystemNode *clone() const { return new WindowsFilesystemNode(this); }

private:
	static char *toAscii(TCHAR *x);
	static TCHAR* toUnicode(char *x);
	static void addFile (FSList* list, const WindowsFilesystemNode *parentNode, const char *base, WIN32_FIND_DATA* find_data);
};


char* WindowsFilesystemNode::toAscii(TCHAR *x) {
static char asciiString[MAX_PATH];

#ifndef UNICODE
	return (char*)x;
#else
	WideCharToMultiByte(CP_ACP, 0, x, _tcslen(x) + 1, asciiString, sizeof(asciiString), NULL, NULL);
	return asciiString;
#endif
}

TCHAR* WindowsFilesystemNode::toUnicode(char *x) {
static TCHAR unicodeString[MAX_PATH];

#ifndef UNICODE
	return (TCHAR*)x;
#else
	MultiByteToWideChar(CP_ACP, 0, x, strlen(x) + 1, unicodeString, sizeof(unicodeString));
	return unicodeString;
#endif
}

void WindowsFilesystemNode::addFile (FSList* list, const WindowsFilesystemNode *parentNode, const char *base, WIN32_FIND_DATA* find_data) {
	WindowsFilesystemNode entry;
	char *asciiName = toAscii(find_data->cFileName);
	bool isDirectory;

	// Skip local directory (.) and parent (..)
	if (!strcmp(asciiName, ".") || !strcmp(asciiName, ".."))
		return;
	
	isDirectory = (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false);

	// Only keep the directories for the moment (files already supported)
	if (!isDirectory)
		return;

	entry._isDirectory = isDirectory;
	entry._parentNode = (entry._isDirectory ? (WindowsFilesystemNode*)parentNode : NULL);
	entry._displayName = asciiName;
	entry._path = base;
	entry._path += asciiName;
	if (entry._isDirectory)
		entry._path += "\\";
	entry._isValid = true;	
	entry._isPseudoRoot = false;

	list->push_back(entry);
}

FilesystemNode *FilesystemNode::getRoot() {
	return new WindowsFilesystemNode();
}

WindowsFilesystemNode::WindowsFilesystemNode() {
	_displayName = "Root";
	_isDirectory = true;
#ifndef _WIN32_WCE
	// Create a virtual root directory for standard Windows system
	_isValid = false;
	_path = "";
	_isPseudoRoot = true;
#else
	// No need to create a pseudo root directory on Windows CE
	_isValid = true;
	_path = "\\";
	_isPseudoRoot = false;
#endif
	_parentNode = this;
}

WindowsFilesystemNode::WindowsFilesystemNode(const WindowsFilesystemNode *node) {
	_displayName = node->_displayName;
	_isDirectory = node->_isDirectory;
	_isValid = node->_isValid;
	_isPseudoRoot = node->_isPseudoRoot;
	_path = node->_path;
	_parentNode = node->_parentNode;
}

FSList *WindowsFilesystemNode::listDir() const {
	assert(_isDirectory);

	FSList *myList = new FSList();

	if (_isPseudoRoot) {
#ifndef _WIN32_WCE
		// Drives enumeration
		TCHAR drive_buffer[100];
		GetLogicalDriveStrings(sizeof(drive_buffer) / sizeof(TCHAR), drive_buffer);

		for (TCHAR *current_drive = drive_buffer; *current_drive; 
			current_drive += _tcslen(current_drive) + 1) {
				WindowsFilesystemNode entry;		
				char drive_name[2];

				drive_name[0] = toAscii(current_drive)[0];
				drive_name[1] = '\0';
				entry._displayName = drive_name;
				entry._isDirectory = true;
				entry._isValid = true;
				entry._isPseudoRoot = false;
				entry._path = toAscii(current_drive);
				myList->push_back(entry);
#endif
		}
	}
	else {
		// Files enumeration
		WIN32_FIND_DATA desc;
		HANDLE			handle;
		char			searchPath[MAX_PATH + 10];

		sprintf(searchPath, "%s*", _path.c_str());

		handle = FindFirstFile(toUnicode(searchPath), &desc);
		if (handle == INVALID_HANDLE_VALUE)
			return myList;
		addFile(myList, this, _path.c_str(), &desc);
		while (FindNextFile(handle, &desc))
			addFile(myList, this, _path.c_str(), &desc);

		FindClose(handle);
	}

	return myList;
}

FilesystemNode *WindowsFilesystemNode::parent() const {
	assert(_isDirectory);
	return _parentNode;
}

#endif // defined(_MSC_VER) || defined(__MINGW32__)
