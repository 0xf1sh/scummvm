/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SAGA_H
#define SAGA_H

#include "engines/engine.h"

#include "common/stream.h"

#include "saga/gfx.h"
#include "saga/list.h"

namespace Saga {

class SndRes;
class Sound;
class Music;
class Anim;
class Render;
class IsoMap;
class Gfx;
class Script;
class Actor;
class Font;
class Sprite;
class Scene;
class Interface;
class Console;
class Events;
class PalAnim;
class Puzzle;
class Resource;

struct ResourceContext;
struct StringList;

//#define MIN_IMG_RLECODE    3
//#define MODEX_SCANLINE_LIMIT 200 //TODO: remove

#define SAGA_IMAGE_DATA_OFFSET 776
#define SAGA_IMAGE_HEADER_LEN  8

#define MAXPATH 512 //TODO: remove

#define SAVE_TITLE_SIZE 28
#define MAX_SAVES 96
#define MAX_FILE_NAME 256

#define ID_NOTHING 0
#define ID_PROTAG 1
#define OBJECT_TYPE_SHIFT 13
#define OBJECT_TYPE_MASK ((1 << OBJECT_TYPE_SHIFT) - 1)

#define OBJ_SPRITE_BASE 9

#define memoryError(Place) error("%s Memory allocation error.", Place)

enum ERRORCODE {
	MEM = -2,//todo: remove
	FAILURE = -1,
	SUCCESS = 0
};

#include "sagagame.h"


enum GameObjectTypes {
	kGameObjectNone = 0,
	kGameObjectActor = 1,
	kGameObjectObject = 2,
	kGameObjectHitZone = 3,
	kGameObjectStepZone = 4
};

enum ScriptTimings {
	kScriptTimeTicksPerSecond = (728L/10L),
	kRepeatSpeedTicks = (728L/10L)/3,
	kNormalFadeDuration = 320, // 64 steps, 5 msec each
	kQuickFadeDuration = 64,  // 64 steps, 1 msec each
	kPuzzleHintTime = 30000000L  // 30 secs. used in timer
};

enum Directions {
	kDirUp = 0,
	kDirUpRight = 1,
	kDirRight = 2,
	kDirDownRight = 3,
	kDirDown = 4,
	kDirDownLeft = 5,
	kDirLeft = 6,
	kDirUpLeft = 7
};

enum HitZoneFlags {
	kHitZoneEnabled = (1 << 0),   // Zone is enabled
	kHitZoneExit = (1 << 1),      // Causes char to exit

	//	The following flag causes the zone to act differently.
	//	When the actor hits the zone, it will immediately begin walking
	//	in the specified direction, and the actual specified effect of
	//	the zone will be delayed until the actor leaves the zone.
	kHitZoneAutoWalk = (1 << 2),

	//      When set on a hit zone, this causes the character not to walk
	//      to the object (but they will look at it).
	kHitZoneNoWalk = (1 << 2),

	//	zone activates only when character stops walking
	kHitZoneTerminus = (1 << 3),

	//      Hit zones only - when the zone is clicked on it projects the
	//      click point downwards from the middle of the zone until it
	//      reaches the lowest point in the zone.
	kHitZoneProject = (1 << 3)
};

struct ImageHeader {
	int width;
	int height;
};

struct StringsTable {
	byte *stringsPointer;
	int stringsCount;
	const char **strings;

	const char *getString(int index) const {
		if ((stringsCount <= index) || (index < 0)) {
			error("StringList::getString wrong index 0x%X (%d)", index, stringsCount);
		}
		return strings[index];
	}

	void freeMem() {
		free(strings);
		free(stringsPointer);
		memset(this, 0, sizeof(*this));
	}

	StringsTable() {
		memset(this, 0, sizeof(*this));
	}
	~StringsTable() {
		freeMem();
	}
};



enum ColorId {
	kITEColorTransBlack = 0x00,
	kITEColorBrightWhite = 0x01,
	kITEColorWhite = 0x02,
	kITEColorLightGrey = 0x04,
	kITEColorGrey = 0x0a,
	kITEColorDarkGrey = 0x0b,
	kITEColorDarkGrey0C = 0x0C,
	kITEColorBlack = 0x0f,
	kITEColorRed = 0x65,
	kITEColorDarkBlue8a = 0x8a,
	kITEColorBlue89 = 0x89,
	kITEColorLightBlue92 = 0x92,
	kITEColorBlue = 0x93,
	kITEColorLightBlue94 = 0x94,
	kITEColorLightBlue96 = 0x96,
	kITEColorGreen = 0xba,

	kIHNMColorBlack = 0xfa,
	kIHNMColorPortrait = 0xfe
};

enum KnownColor {
	kKnownColorTransparent,
	kKnownColorBrightWhite,	
	kKnownColorBlack,

	kKnownColorSubtitleTextColor,
	kKnownColorVerbText,
	kKnownColorVerbTextShadow,
	kKnownColorVerbTextActive
};

struct SaveFileData {
	char name[SAVE_TITLE_SIZE];
	uint slotNumber;
};

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 version;
	char name[SAVE_TITLE_SIZE];
};

inline int ticksToMSec(int tick) {
	return tick * 1000 / kScriptTimeTicksPerSecond;
}

inline int clamp(int minValue, int value, int maxValue) {
	if (value <= minValue) {
		return minValue;
	} else {
		if (value >= maxValue) {
			return maxValue;
		} else {
			return value;
		}
	}
}

inline int integerCompare(int i1, int i2) {
	return ((i1) > (i2) ? 1 : ((i1) < (i2) ? -1 : 0));
}

inline int objectTypeId(uint16 objectId) {
	return objectId >> OBJECT_TYPE_SHIFT;
}

inline int objectIdToIndex(uint16 objectId) {
	return OBJECT_TYPE_MASK & objectId;
}

inline uint16 objectIndexToId(int type, int index) {
	return (type << OBJECT_TYPE_SHIFT) | (OBJECT_TYPE_MASK & index);
}


class SagaEngine : public Engine {
	friend class Scene;

protected:
	int go();
	int init();
public:
	SagaEngine(OSystem *syst);
	virtual ~SagaEngine();
	void shutDown() { _quit = true; }

	void save(const char *fileName, const char *saveName);
	void load(const char *fileName);
	uint32 getCurrentLoadVersion() {
		return _saveHeader.version;
	}
	void fillSaveList();
	char *calcSaveFileName(uint slotNumber);

	SaveFileData *getSaveFile(uint idx);
	uint getSaveSlotNumber(uint idx);
	uint getNewSaveSlotNumber();
	bool locateSaveFile(char *saveName, uint &titleNumber);
	bool isSaveListFull() const {
		return _saveFilesMaxCount == _saveFilesCount;
	}
	uint getSaveFilesCount() const {
		return isSaveListFull() ? _saveFilesCount : _saveFilesCount + 1;
	}

	int16 _framesEsc;

	uint32 _globalFlags;
	int16 _ethicsPoints[8];
	int _spiritualBarometer;

	int _soundVolume;
	int _musicVolume;
	bool _subtitlesEnabled;
	int _readingSpeed;

	bool _copyProtection;

	SndRes *_sndRes;
	Sound *_sound;
	Music *_music;
	Anim *_anim;
	Render *_render;
	IsoMap *_isoMap;
	Gfx *_gfx;
	Script *_script;
	Actor *_actor;
	Font *_font;
	Sprite *_sprite;
	Scene *_scene;
	Interface *_interface;
	Console *_console;
	Events *_events;
	PalAnim *_palanim;
	Puzzle *_puzzle;
	Resource *_resource;


	/** Random number generator */
	Common::RandomSource _rnd;

private:
	int decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len);
	int flipImage(byte *img_buf, int columns, int scanlines);
	int unbankBGImage(byte *dest_buf, const byte *src_buf, int columns, int scanlines);
	uint32 _previousTicks;

public:
	int decodeBGImage(const byte *image_data, size_t image_size,
			  byte **output_buf, size_t *output_buf_len, int *w, int *h, bool flip = false);
	const byte *getImagePal(const byte *image_data, size_t image_size);
	void loadStrings(StringsTable &stringsTable, const byte *stringsPointer, size_t stringsLength);

	const char *getObjectName(uint16 objectId);
public:
	int processInput(void);
	const Point &mousePos() const {
		return _mousePos;
	}

	const bool leftMouseButtonPressed() const {
		return _leftMouseButtonPressed;
	}

	const bool rightMouseButtonPressed() const {
		return _rightMouseButtonPressed;
	}

	const bool mouseButtonPressed() const {
		return _leftMouseButtonPressed || _rightMouseButtonPressed;
	}

 private:
	uint _saveFilesMaxCount;
	uint _saveFilesCount;
	SaveFileData _saveFiles[MAX_SAVES];
	bool _saveMarks[MAX_SAVES];
	SaveGameHeader _saveHeader;

	Point _mousePos;
	bool _leftMouseButtonPressed;
	bool _rightMouseButtonPressed;

	bool _quit;

//current game description
	int _gameNumber;
	GameDescription *_gameDescription;
	Common::String _gameTitle;
	Common::Rect _displayClip;

protected:
	GameDisplayInfo _gameDisplayInfo;

public:
	int32 _frameCount;

public:
	bool initGame(void);
public:
	const GameDescription *getGameDescription() const { return _gameDescription; }
	const bool isBigEndian() const { return (_gameDescription->features & GF_BIG_ENDIAN_DATA) != 0; }
	const bool isMacResources() const { return (getPlatform() == Common::kPlatformMacintosh); }
	const GameResourceDescription *getResourceDescription() { return _gameDescription->resourceDescription; }
	const GameSoundInfo *getVoiceInfo() const { return _gameDescription->voiceInfo; }
	const GameSoundInfo *getSfxInfo() const { return _gameDescription->sfxInfo; }
	const GameSoundInfo *getMusicInfo() const { return _gameDescription->musicInfo; }

	const GameFontDescription *getFontDescription(int index) {
		assert(index < _gameDescription->fontsCount);
		return &_gameDescription->fontDescriptions[index];
	}
	int getFontsCount() const { return _gameDescription->fontsCount; }

	int getGameId() const { return _gameDescription->gameId; }
	int getGameType() const { return _gameDescription->gameType; }
	uint32 getFeatures() const { return _gameDescription->features; }
	Common::Language getLanguage() const { return _gameDescription->language; }
	Common::Platform getPlatform() const { return _gameDescription->platform; }
	int getGameNumber() const { return _gameNumber; }
	int getStartSceneNumber() const { return _gameDescription->startSceneNumber; }


	const Common::Rect &getDisplayClip() const { return _displayClip;}
	int getDisplayWidth() const { return _gameDisplayInfo.logicalWidth; }
	int getDisplayHeight() const { return _gameDisplayInfo.logicalHeight;}
	const GameDisplayInfo & getDisplayInfo() { return _gameDisplayInfo; }

	const char *getTextString(int textStringId);
	void getExcuseInfo(int verb, const char *&textString, int &soundResourceId);

private:

public:
	ColorId KnownColor2ColorId(KnownColor knownColor);
	void setTalkspeed(int talkspeed); 
	int getTalkspeed(); 
};

} // End of namespace Saga

#endif
