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
 */

#ifndef VOYEUR_FILES_H
#define VOYEUR_FILES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "voyeur/graphics.h"

namespace Voyeur {

class VoyeurEngine;
class BoltFile;
class BoltGroup;
class BoltEntry;
class PictureResource;
class ViewPortResource;
class ViewPortListResource;
class FontResource;
class CMapResource;
class VInitCyclResource;

#define DECOMPRESS_SIZE 0x7000

typedef void (BoltFile::*BoltMethodPtr)();

class ResolveEntry {
public:
	uint32 _id;
	byte **_p;

	ResolveEntry(uint32 id, byte **p) { _id = id; _p = p; }
};

class BoltFilesState {
public:
	VoyeurEngine *_vm;
	BoltFile *_curLibPtr;
	BoltGroup *_curGroupPtr;
	BoltEntry *_curMemberPtr;
	byte *_curMemInfoPtr;
	int _fromGroupFlag;
	byte _xorMask;
	bool _encrypt;
	int _curFilePosition;
	int _bufferEnd;
	int _bufferBegin;
	int _bytesLeft;
	int _bufSize;
	byte *_bufStart;
	byte *_bufPos;
	byte _decompressBuf[DECOMPRESS_SIZE];
	int _historyIndex;
	byte _historyBuffer[0x200];
	int _runLength;
	int _decompState;
	int _runType;
	int _runValue;
	int _runOffset;
	Common::File *_curFd;
	Common::Array<ResolveEntry> _resolves;

	byte *_boltPageFrame;
	int _sImageShift;
	bool _SVGAReset;
public:
	BoltFilesState();

	byte *decompress(byte *buf, int size, int mode);
	void nextBlock();

	void EMSGetFrameAddr(byte **pageFrame) {} // TODO: Maybe?
	bool EMSAllocatePages(uint *planeSize) { return false; } // TODO: Maybe?
	void EMSMapPageHandle(int planeSize, int idx1, int idx2) {} // TODO: Maybe?
};

class BoltFile {
private:
	static const BoltMethodPtr _fnInitType[25];
private:
	BoltFilesState &_state;
	Common::Array<BoltGroup> _groups;
	Common::File _file;

	// initType method table
	void initDefault();
	void sInitPic();
	void vInitCMap();
	void vInitCycl();
	void initViewPort();
	void initViewPortList();
	void initFontInfo();
	void initFont();
	void initSoundMap();
private:
	void resolveAll();
	byte *getBoltMember(uint32 id);

	void termType() {}	// TODO
	void initMem(int id) {}	// TODO
	void termMem() {}	// TODO
	void initGro() {}	// TODO
	void termGro() {}	// TODO
public:
	BoltFile(BoltFilesState &state);
	~BoltFile();

	bool getBoltGroup(uint32 id);
	void freeBoltGroup(uint32 id);
	byte *memberAddr(uint32 id);
	byte *memberAddrOffset(uint32 id);
	void resolveIt(uint32 id, byte **p);
	void resolveFunction(uint32 id, GraphicMethodPtr *fn);

	BoltEntry &boltEntry(uint16 id); 
	BoltEntry &getBoltEntryFromLong(uint32 id);
	PictureResource *getPictureResource(uint32 id);
	CMapResource *getCMapResource(uint32 id);
};

class BoltGroup {
private:
	Common::SeekableReadStream *_file;
public:
	byte _loaded;
	bool _processed;
	bool _callInitGro;
	int _termGroIndex;
	int _count;
	int _fileOffset;
	Common::Array<BoltEntry> _entries;
public:
	BoltGroup(Common::SeekableReadStream *f); 
	virtual ~BoltGroup();

	void load();
	void unload();
};


class BoltEntry {
private:
	Common::SeekableReadStream *_file;
public:
	byte _mode;
	byte _field1;
	byte _initMethod;
	int _fileOffset;
	byte _xorMask;
	int _size;
	byte *_data;

	PictureResource *_picResource;
	ViewPortResource *_viewPortResource;
	ViewPortListResource *_viewPortListResource;
	FontResource *_fontResource;
	FontInfoResource *_fontInfoResource;
	CMapResource *_cMapResource;
	VInitCyclResource *_vInitCyclResource;
public:
	BoltEntry(Common::SeekableReadStream *f);
	virtual ~BoltEntry();

	void load();
	bool hasResource() const;
};

class FilesManager {
private:
	int _decompressSize;
public:
	BoltFilesState _boltFilesState;
	BoltFile *_curLibPtr;
public:
	FilesManager();
	void setVm(VoyeurEngine *vm) { _boltFilesState._vm = vm; }

	bool openBoltLib(const Common::String &filename, BoltFile *&boltFile);
	byte *fload(const Common::String &filename, int *size = NULL);
};

enum DisplayFlag { DISPFLAG_1 = 1, DISPFLAG_2 = 2, DISPFLAG_4 = 4, DISPFLAG_8 = 8, 
	DISPFLAG_10 = 0x10, DISPFLAG_20 = 0x20, DISPFLAG_40 = 0x40, DISPFLAG_80 = 0x80,
	DISPFLAG_100 = 0x100, DISPFLAG_200 = 0x200, DISPFLAG_400 = 0x400, 
	DISPFLAG_800 = 0x800, DISPFLAG_1000 = 0x1000, DISPFLAG_2000 = 0x2000,
	DISPFLAG_4000 = 0x4000, DISPFLAG_VIEWPORT = 0x8000, DISPFLAG_CURSOR = 0x10000 };

class DisplayResource {
public:
	uint32 _flags;
};

class PictureResource: public DisplayResource {
public:
	byte _select;
	byte _pick;
	byte _onOff;
	byte _depth;
	Common::Rect _bounds;
	uint32 _maskData;
	uint _planeSize;

	byte *_imgData;
public:
	PictureResource(BoltFilesState &state, const byte *src);
	PictureResource(int flags, int select, int pick, int onOff, int depth, 
		const Common::Rect &bounds, int maskData, byte *imgData, int planeSize);
	PictureResource();
	virtual ~PictureResource();
};

typedef void (ViewPortResource::*ViewPortMethodPtr)();

class ViewPortResource: public DisplayResource {
private:
	BoltFilesState &_state;
private:
	void setupViewPort(PictureResource *page, Common::Rect *clipRect, ViewPortSetupPtr setupFn,
		ViewPortAddPtr addFn, ViewPortRestorePtr restoreFn);
public:
	ViewPortResource *_parent;
	int _pageCount;
	int _pageIndex;
	int _lastPage;
	Common::Rect _bounds;
	int _field18;
	PictureResource *_currentPic;
	PictureResource *_activePage;
	PictureResource *_pages[2];
	byte *_field30;

	// Rect lists and counts. Note that _rectListCount values of '-1' seem to have
	// special significance, which is why I'm not making them redundant in favour 
	// of the arrays' .size() method
	Common::Array<Common::Rect> *_rectListPtr[3];
	int _rectListCount[3];

	Common::Rect _clipRect;
	byte *_field7A;
	GraphicMethodPtr _fn1;
	ViewPortSetupPtr _setupFn;
	ViewPortAddPtr _addFn;
	ViewPortRestorePtr _restoreFn;
	PictureResource _fontChar;
	Common::Rect _fontRect;
public:
	ViewPortResource(BoltFilesState &state, const byte *src);
	virtual ~ViewPortResource();

	void setupViewPort();
	int drawText(const Common::String &msg);
	int textWidth(const Common::String &msg);
	void addSaveRect(int pageIndex, const Common::Rect &r);
	void sFillBox(int width);
};

class ViewPortPalEntry  {
public:
	uint16 _rEntry, _gEntry, _bEntry;
	uint16 _rChange, _gChange, _bChange;
	uint16 _palIndex;
public:
	ViewPortPalEntry(const byte *src);
};

class ViewPortListResource {
public:
	Common::Array<ViewPortPalEntry> _palette;
	Common::Array<ViewPortResource *> _entries;
	int _palIndex;

	ViewPortListResource(BoltFilesState &state, const byte *src);
	virtual ~ViewPortListResource() {}
};

class FontResource {
public:
	int _minChar, _maxChar;
	int field2;
	int _padding;
	int _fontHeight;
	int _topPadding;
	int *_charWidth;
	byte *_data1;
	byte *_data2;

	FontResource(BoltFilesState &state, byte *src);
	virtual ~FontResource();
};

enum FontJustify { ALIGN_LEFT = 0, ALIGN_CENTRE = 1, ALIGN_RIGHT = 2 };

class FontInfoResource {
public:
	FontResource *_curFont;
	byte _picFlags;
	byte _picSelect;
	byte _picPick;
	byte _picOnOff;
	byte _fontFlags;
	FontJustify _justify;
	int _fontSaveBack;
	Common::Point _pos;
	int _justifyWidth;
	int _justifyHeight;
	Common::Point _shadow;
	int _foreColor;
	int _backColor;
	int _shadowColor;
public:
	FontInfoResource(BoltFilesState &state, const byte *src);
	FontInfoResource();
	FontInfoResource(byte picFlags, byte picSelect, byte picPick, byte picOnOff, byte fontFlags, 
		FontJustify justify, int fontSaveBack, const Common::Point &pos, int justifyWidth, 
		int justifyHeight, const Common::Point &shadow, int foreColor, int backColor, 
		int shadowColor);
};

class CMapResource {
private:
	VoyeurEngine *_vm;
public:
	int _steps;
	int _fadeStatus;
	int _start;
	int _end;
	byte *_entries;
public:
	CMapResource(BoltFilesState &state, const byte *src);
	virtual ~CMapResource();

	void startFade();
};

class VInitCyclResource {
public:
	byte *_ptr[4];
public:
	VInitCyclResource(BoltFilesState &state, const byte *src);
	virtual ~VInitCyclResource() {}
};

} // End of namespace Voyeur

#endif /* VOYEUR_FILES_H */
