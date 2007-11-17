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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#if defined(__MORPHOS__)

#include <proto/dos.h>

#include <stdio.h>
#include <sys/stat.h>

#include "common/util.h"
#include "base/engine.h"
#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API based on the MorphOS A-Box API.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemNode.
 */
class ABoxFilesystemNode : public AbstractFilesystemNode {
protected:
	BPTR _lock;
	String _displayName;
	String _path;
	bool _isDirectory;
	bool _isValid;

public:
	/**
	 * Creates a ABoxFilesystemNode with the root node as path.
	 */
	ABoxFilesystemNode();
	
	/**
	 * Creates a ABoxFilesystemNode for a given path.
	 * 
	 * @param path String with the path the new node should point to.
	 */
	ABoxFilesystemNode(const String &p);
	
	/**
	 * FIXME: document this constructor.
	 */
	ABoxFilesystemNode(BPTR lock, CONST_STRPTR display_name = NULL);
	
	/**
	 * Copy constructor.
	 */
	ABoxFilesystemNode(const ABoxFilesystemNode &node);

	/**
	 * Destructor.
	 */
	~ABoxFilesystemNode();

	virtual bool exists() const { return true; }		//FIXME: this is just a stub
	virtual String getDisplayName() const { return _displayName; }
	virtual String getName() const { return _displayName; };
	virtual String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return true; }	//FIXME: this is just a stub
	virtual bool isWritable() const { return true; }	//FIXME: this is just a stub
	
	virtual AbstractFilesystemNode *getChild(const String &name) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFilesystemNode *getParent() const;
	
	/**
	 * Return the list of child nodes for the root node.
	 */
	static AbstractFSList getRootChildren();
};

/**
 * Returns the last component of a given path.
 * 
 * @param str String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
const char *lastPathComponent(const Common::String &str) {
	if (str.empty())
		return "";
	
	const char *str = _path.c_str();
	while (offset > 0 && (str[offset-1] == '/' || str[offset-1] == ':') )
		offset--;
	while (offset > 0 && (str[offset-1] != '/' && str[offset-1] != ':')) {
		len++;
		offset--;
	}
	
	return str + offset;
}

ABoxFilesystemNode::ABoxFilesystemNode()
{
	_path = "";
	_displayName = "Mounted Volumes";
	_isValid = true;
	_isDirectory = true;
	_lock = NULL;
}

ABoxFilesystemNode::ABoxFilesystemNode(const String &p) {
	int len = 0, offset = p.size();

	assert(offset > 0);

	_path = p;
	_displayName = lastPathComponent(_path);
	_lock = NULL;
	_isDirectory = false;

	struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
	if (!fib)
	{
		debug(6, "FileInfoBlock is NULL");
		return;
	}

	// Check whether the node exists and if it is a directory
	BPTR pLock = Lock((STRPTR)_path.c_str(), SHARED_LOCK);
	if (pLock)
	{
		if (Examine(pLock, fib) != DOSFALSE) {
			if (fib->fib_EntryType > 0)
			{
				_isDirectory = true;
				_lock = DupLock(pLock);
				_isValid = (_lock != 0);

				// Add a trailing slash if it is needed
				const char c = _path.lastChar();
				if (c != '/' && c != ':')
					_path += '/';

			}
			else
			{
				_isDirectory = false;
				_isValid = true;
			}
		}
	
		UnLock(pLock);
	}

	FreeDosObject(DOS_FIB, fib);
}

ABoxFilesystemNode::ABoxFilesystemNode(BPTR lock, CONST_STRPTR display_name)
{
	int bufsize = 256;

	_lock = NULL;
	for (;;)
	{
		char name[bufsize];
		if (NameFromLock(lock, name, bufsize) != DOSFALSE)
		{
			_path = name;
			_displayName = display_name ? display_name : FilePart(name);
			break;
		}
		if (IoErr() != ERROR_LINE_TOO_LONG)
		{
			_isValid = false;
			debug(6, "Error while retrieving path name: %ld", IoErr());
			return;
		}
		bufsize *= 2;
	}

	_isDirectory = false;
	_isValid = false;

	FileInfoBlock *fib = (FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);
	if (fib == NULL)
	{
		debug(6, "Failed to allocate memory for FileInfoBlock");
		return;
	}

	if (Examine(lock, fib) != DOSFALSE)
	{
		_isDirectory = fib->fib_EntryType > 0;
		if (_isDirectory)
		{
			if (fib->fib_EntryType != ST_ROOT)
				_path += "/";
			_lock = DupLock(lock);
			_isValid = (_lock != NULL);
		}
		else
		{
			_isValid = true;
		}
	}
	
	FreeDosObject(DOS_FIB, fib);
}

ABoxFilesystemNode::ABoxFilesystemNode(const ABoxFilesystemNode& node)
{
	_path = node._path;
	_displayName = node._displayName;
	_isValid = node._isValid;
	_isDirectory = node._isDirectory;
	_lock = DupLock(node._lock);
}

ABoxFilesystemNode::~ABoxFilesystemNode()
{
	if (_lock)
	{
		UnLock(_lock);
		_lock = NULL;
	}
}

AbstractFilesystemNode *ABoxFilesystemNode::getChild(const String &name) const {
	assert(_isDirectory);
	String newPath(_path);

	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += name;

	BPTR lock = Lock(newPath.c_str(), SHARED_LOCK);

	if (!lock)
	{
		return 0;
    }

	UnLock(lock);

	return new ABoxFilesystemNode(newPath);
}

bool ABoxFilesystemNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const
{
	//TODO: honor the hidden flag
	
	if (!_isValid)
	{
		debug(6, "listDir() called on invalid node");
		return false;
	}
	if (!_isDirectory)
	{
		debug(6, "listDir() called on file node");
		return false;
	}

	if (_lock == NULL)
	{
		/* This is the root node */
		list = getRootChildren();
		return true;
	}

	/* "Normal" file system directory */
	FileInfoBlock *fib = (FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);

	if (fib == NULL)
	{
		debug(6, "Failed to allocate memory for FileInfoBlock");
		return false;
	}

	if (Examine(_lock, fib) != DOSFALSE)
	{
		while (ExNext(_lock, fib) != DOSFALSE)
		{
			ABoxFilesystemNode *entry;
			String full_path;
			BPTR lock;

			if ((mode == FilesystemNode::kListAll) ||
			     (fib->fib_EntryType > 0 && (mode & FilesystemNode::kListDirectoriesOnly)) ||
				 (fib->fib_EntryType < 0 && (mode & FilesystemNode::kListFilesOnly)))
			{
				full_path = _path;
				full_path += fib->fib_FileName;
				lock = Lock(full_path.c_str(), SHARED_LOCK);
				if (lock)
				{
					entry = new ABoxFilesystemNode(lock, fib->fib_FileName);
					if (entry)
					{
						//FIXME: since the isValid() function is no longer part of the AbstractFilesystemNode
						//       specification, the following call had to be changed:
						//       	if (entry->isValid())
						//		 Please verify that the logic of the code remains coherent. Also, remember
						//		 that the isReadable() and isWritable() methods are available.
						if (entry->exists())
							list.push_back(entry);
						else
							delete entry;
					}
					UnLock(lock);
				}
			}
		}

		if (IoErr() != ERROR_NO_MORE_ENTRIES)
			debug(6, "Error while reading directory: %ld", IoErr());
	}

	FreeDosObject(DOS_FIB, fib);

	return true;
}

AbstractFilesystemNode *ABoxFilesystemNode::getParent() const
{
	AbstractFilesystemNode *node = NULL;

	if (!_isDirectory)
	{
		debug(6, "parent() called on file node");
		return NULL;
	}

	if (_lock == NULL) {
		/* Parent of the root is the root itself */
		return new ABoxFilesystemNode(*this);
	} else {
		BPTR parent_lock = ParentDir(_lock);
		if (parent_lock) {
			node = new ABoxFilesystemNode(parent_lock);
			UnLock(parent_lock);
		} else
			node = new ABoxFilesystemNode();
	}

	return node;
}

AbstractFSList ABoxFilesystemNode::getRootChildren()
{
	AbstractFSList list;
	DosList *dosList;
	CONST ULONG lockDosListFlags = LDF_READ | LDF_VOLUMES;
	char name[256];

	dosList = LockDosList(lockDosListFlags);
	if (dosList == NULL)
	{
		return list;
	}

	dosList = NextDosEntry(dosList, LDF_VOLUMES);
	while (dosList)
	{
		if (dosList->dol_Type == DLT_VOLUME &&	// Should always be true, but ...
			 dosList->dol_Name &&				// Same here
			 dosList->dol_Task					// Will be NULL if volume is removed from drive but still in use by some program
			)
		{
			ABoxFilesystemNode *entry;
			CONST_STRPTR volume_name = (CONST_STRPTR)BADDR(dosList->dol_Name)+1;
			CONST_STRPTR device_name = (CONST_STRPTR)((struct Task *)dosList->dol_Task->mp_SigTask)->tc_Node.ln_Name;
			BPTR volume_lock;

			strcpy(name, volume_name);
			strcat(name, ":");
			volume_lock = Lock(name, SHARED_LOCK);
			if (volume_lock)
			{
				sprintf(name, "%s (%s)", volume_name, device_name);
				entry = new ABoxFilesystemNode(volume_lock, name);
				if (entry)
				{
					//FIXME: since the isValid() function is no longer part of the AbstractFilesystemNode
					//       specification, the following call had to be changed:
					//       	if (entry->isValid())
					//		 Please verify that the logic of the code remains coherent. Also, remember
					//		 that the isReadable() and isWritable() methods are available.
					if (entry->exists())
						list.push_back(entry);
					else
						delete entry;
				}
				UnLock(volume_lock);
			}
		}
		dosList = NextDosEntry(dosList, LDF_VOLUMES);
	}

	UnLockDosList(lockDosListFlags);

	return list;
}

#endif // defined(__MORPHOS__)
