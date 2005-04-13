/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/stream.h"


/**
 * A class which allows game engines to load game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 *
 * @todo Add error checking abilities.
 * @todo Change base class to SeekableReadStream; or alternatively,
 *       add a simple 'skip()' method which would allow skipping
 *       a number of bytes in the savefile.
 */
class InSaveFile : public Common::ReadStream {
public:
	virtual ~InSaveFile() {}

	virtual bool readingFailed() const { return false; }
	//bool eof() const;
};

/**
 * A class which allows game engines to save game state data.
 * That typically means "save games", but also includes things like the
 * IQ points in Indy3.
 *
 * @todo Add error checking abilities.
 */
class OutSaveFile : public Common::WriteStream {
public:
	virtual ~OutSaveFile() {}

	virtual bool writingFailed() const { return false; }
};

/**
 * Convenience intermediate class, to be removed.
 */
class SaveFile : public InSaveFile, public OutSaveFile {
public:
};

class SaveFileManager {

public:
	virtual ~SaveFileManager() {}

	/**
	 * Open the file with name filename in the given directory for saving.
	 * @param filename	the filename
	 * @return pointer to a SaveFile object, or NULL if an error occured.
	 */
	virtual OutSaveFile *openForSaving(const char *filename) = 0;

	/**
	 * Open the file with name filename in the given directory for loading.
	 * @param filename	the filename
	 * @return pointer to a SaveFile object, or NULL if an error occured.
	 */
	virtual InSaveFile *openForLoading(const char *filename) = 0;

	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num) = 0;

	/** Get the path to the save game directory. */
	virtual const char *getSavePath() const;
};

class DefaultSaveFileManager : public SaveFileManager {
public:
	virtual OutSaveFile *openForSaving(const char *filename);
	virtual InSaveFile *openForLoading(const char *filename);
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num);

protected:
	SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

#endif
