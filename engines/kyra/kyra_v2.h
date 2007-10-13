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

#ifndef KYRA_KYRA_V2_H
#define KYRA_KYRA_V2_H

#include "kyra/kyra.h"
#include "kyra/script.h"
#include "kyra/screen_v2.h"

#include "common/list.h"

namespace Kyra {

enum kSequences {
	kSequenceVirgin = 0,
	kSequenceWestwood = 1,
	kSequenceTitle = 2,
	kSequenceOverview = 3,
	kSequenceLibrary = 4,
	kSequenceHand = 5,
	kSequencePoint = 6,
	kSequenceZanFaun = 7
};

class WSAMovieV2;
class KyraEngine_v2;
class TextDisplayer_v2;
class Debugger_v2;

struct SequenceControl {
	int8 frameIndex;
	int8 frameDelay;
};

struct ActiveWSA {
	WSAMovieV2 *movie;
	uint16 currentFrame;
	uint16 endFrame;
	uint16 frameDelay;
	uint32 nextFrame;
	void (KyraEngine_v2::*callback)(int);
	const SequenceControl *control;
};

struct ActiveChat {
	uint16 strIndex;
	uint16 x;
	uint16 y;
	int duration;
	uint16 field_8;
	uint16 startTime;
	uint16 field_E;
};

struct Sequence {
	uint8 type;
	const char *filename;
	int (KyraEngine_v2::*callback)(int);
	uint8 frameDelay;
	uint16 duration;
	uint8 numFrames;
	bool timeOut;
	bool fadeOut;
};

class KyraEngine_v2 : public KyraEngine {
friend class Debugger_v2;
friend class TextDisplayer_v2;
public:
	KyraEngine_v2(OSystem *system, const GameFlags &flags);
	~KyraEngine_v2();
	
	virtual Screen *screen() { return _screen; }
	Screen_v2 *screen_v2() { return _screen; }
	int language() const { return _lang; }
	
	virtual Movie *createWSAMovie();
protected:
	// Main menu code, also used for Kyra 3
	static const char *_mainMenuStrings[];

	virtual void gui_initMainMenu() {}
	int gui_handleMainMenu();
	virtual void gui_updateMainMenuAnimation();
	void gui_drawMainMenu(const char * const *strings, int select);
	void gui_drawMainBox(int x, int y, int w, int h, int fill);
	bool gui_mainMenuGetInput();
	
	void gui_printString(const char *string, int x, int y, int col1, int col2, int flags, ...);

	// intro
	void seq_playSequences(int startSeq, int endSeq = -1);
	int seq_introWestwood(int seqNum);
	int seq_introTitle(int seqNum);
	int seq_introOverview(int seqNum);
	int seq_introLibrary(int seqNum);	
	int seq_introHand(int seqNum);
	int seq_introPoint(int seqNum);
	int seq_introZanFaun(int seqNum);

	void seq_introOverviewOver1(int currentFrame);
	void seq_introOverviewForest(int currentFrame);	
	void seq_introOverviewDragon(int currentFrame);
	void seq_loadWSA(int wsaNum, const char *filename, int frameDelay, void (KyraEngine_v2::*callback)(int) = 0, 
					 const SequenceControl *control = 0 );
	void seq_unloadWSA(int wsaNum);
	void seq_playWSAs();
	void seq_showChats();
	void seq_playIntroChat(uint8 chatNum);
	void seq_resetAllChatEntries();
	void seq_waitForChatsToFinish();
	void seq_setChatEntry(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 unk1);

	void mainMenu();

	int init();
	int go();
	
	Screen_v2 *_screen;
	TextDisplayer_v2 *_text;
	Debugger_v2 *_debugger;
	
	ActiveWSA *_activeWSA;
	ActiveChat *_activeChat;
	uint8 *_mouseSHPBuf;

	static const char *_dosSoundFileList[];
	static const int _dosSoundFileListSize;
	static const int8 _dosTrackMap[];
	static const int _dosTrackMapSize;

	static const char *_introSoundList[];
	static const int _introSoundListSize;
	static const char *_introStrings[];
	static const int _introStringsSize;
	
	int _introStringsDuration[21];
	
protected:
	// game initialization
	void startup();
	void runLoop();
	void cleanup();
	
	void setupTimers();
	void setupOpcodeTable();
	
	void loadMouseShapes();
	void loadItemShapes();
	
	// run
	void update();
	void updateWithText();

	Functor0Mem<void, KyraEngine_v2> _updateFunctor;

	void updateMouse();
	
	int checkInput(void *p);
	void removeInputTop();
	void handleInput(int x, int y);
	bool handleInputUnkSub(int x, int y);

	int inputSceneChange(int x, int y, int unk1, int unk2);

	// - Input
	void updateInput();

	int _mouseX, _mouseY;
	Common::List<Common::Event> _eventList;
	
	// gfx/animation specific
	uint8 *_gamePlayBuffer;
	void restorePage3();

	uint8 *_screenBuffer;
	uint8 *_maskPage;
	uint8 *_gfxBackUpRect;

	void backUpGfxRect24x24(int x, int y);
	void restoreGfxRect24x24(int x, int y);
	
	uint8 *getShapePtr(int index) { return _defaultShapeTable[index]; }
	uint8 *_defaultShapeTable[250];
	uint8 *_sceneShapeTable[50];
	
	WSAMovieV2 *_wsaSlots[10];
	
	void freeSceneShapePtrs();
	
	struct ShapeDesc {
		uint8 unk0, unk1, unk2, unk3, unk4;
		uint16 width, height;
		int16 xAdd, yAdd;
	};
	
	ShapeDesc *_shapeDescTable;
	
	struct SceneAnim {
		uint16 flags;
		int16 x, y;
		int16 x2, y2;
		int16 width, height;
		uint16 unkE;
		uint16 specialSize;
		uint16 unk12;
		int16 shapeIndex;
		uint16 wsaFlag;
		char filename[14];
	};
	
	SceneAnim _sceneAnims[10];
	WSAMovieV2 *_sceneAnimMovie[10];
	bool _specialSceneScriptState[10];
	ScriptState _sceneSpecialScripts[10];
	uint32 _sceneSpecialScriptsTimer[10];
	int _lastProcessedSceneScript;
	bool _specialSceneScriptRunFlag;
	
	void updateSpecialSceneScripts();	
	void freeSceneAnims();
	
	int _loadedZTable;
	void loadZShapes(int shapes);
	void loadInventoryShapes();
	
	void resetScaleTable();
	void setScaleTableItem(int item, int data);
	int getScale(int x, int y);
	uint16 _scaleTable[15];
	
	void setDrawLayerTableEntry(int entry, int data);
	int getDrawLayer(int x, int y);
	int _drawLayerTable[15];

	int _layerFlagTable[16]; // seems to indicate layers where items get destroyed when dropped to (TODO: check this!)

	char _newShapeFilename[13];
	int _newShapeLastEntry;
	int _newShapeWidth, _newShapeHeight;
	int _newShapeXAdd, _newShapeYAdd;
	int _newShapeFlag;
	uint8 *_newShapeFiledata;
	int _newShapeCount;
	int _newShapeAnimFrame;
	int _newShapeDelay;

	int initNewShapes(uint8 *filedata);
	void processNewShapes(int unk1, int unk2);
	void resetNewShapes(int count, uint8 *filedata);
	
	// animator
	struct AnimObj {
		uint16 index;
		uint16 type;
		uint16 enabled;
		uint16 needRefresh;
		uint16 unk8;
		uint16 animFlags;
		uint16 flags;
		int16 xPos1, yPos1;
		uint8 *shapePtr;
		uint16 shapeIndex1;
		uint16 animNum;
		uint16 shapeIndex3;
		uint16 shapeIndex2;
		uint16 unk1E;
		uint8 unk20;
		uint8 unk21;
		uint8 unk22;
		uint8 unk23;
		int16 xPos2, yPos2;
		int16 xPos3, yPos3;
		int16 width, height;
		int16 width2, height2;
		AnimObj *nextObject;
	};
	
	AnimObj _animObjects[42];
	void clearAnimObjects();
	
	AnimObj *_animList;
	bool _drawNoShapeFlag;
	AnimObj *initAnimList(AnimObj *list, AnimObj *entry);
	AnimObj *addToAnimListSorted(AnimObj *list, AnimObj *entry);
	AnimObj *deleteAnimListEntry(AnimObj *list, AnimObj *entry);
	
	void drawAnimObjects();
	void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	
	void refreshAnimObjects(int force);
	void refreshAnimObjectsIfNeed();

	void flagAnimObjsForRefresh();
	
	void updateCharFacing();
	void updateCharacterAnim(int);
	void updateSceneAnim(int anim, int newFrame);

	void addItemToAnimList(int item);
	void deleteItemAnimEntry(int item);

	int _animObj0Width, _animObj0Height;
	void setCharacterAnimDim(int w, int h);
	void resetCharacterAnimDim();
	
	// scene
	struct SceneDesc {
		char filename[10];
		uint16 exit1, exit2, exit3, exit4;
		uint8 flags;
		uint8 sound;
	};
	
	SceneDesc *_sceneList;
	int _sceneListSize;
	uint16 _currentScene;

	const char *_sceneCommentString;
	uint16 _sceneExit1, _sceneExit2, _sceneExit3, _sceneExit4;
	int _sceneEnterX1, _sceneEnterY1, _sceneEnterX2, _sceneEnterY2,
		_sceneEnterX3, _sceneEnterY3, _sceneEnterX4, _sceneEnterY4;
	int _specialExitCount;
	uint16 _specialExitTable[25];
	bool checkSpecialSceneExit(int num, int x, int y);
	uint8 _scenePal[688];
	bool _overwriteSceneFacing;
	
	void enterNewScene(uint16 newScene, int facing, int unk1, int unk2, int unk3);
	void enterNewSceneUnk1(int facing, int unk1, int unk2);
	void enterNewSceneUnk2(int unk1);
	void unloadScene();

	void loadScenePal();
	void loadSceneMsc();

	void fadeScenePal(int srcIndex, int delay);
	
	void startSceneScript(int unk1);
	void runSceneScript2();
	void runSceneScript4(int unk1);
	void runSceneScript6();
	void runSceneScript7();
	
	void initSceneAnims(int unk1);
	void initSceneScreen(int unk1);
	
	int trySceneChange(int *moveTable, int unk1, int updateChar);
	int checkSceneChange();
	
	// pathfinder
	int _movFacingTable[600];
	int findWay(int curX, int curY, int dstX, int dstY, int *moveTable, int moveTableSize);
	bool lineIsPassable(int x, int y);
	bool directLinePassable(int x, int y, int toX, int toY);
	
	int pathfinderUnk1(int *moveTable);
	int pathfinderUnk2(int index, int v1, int v2);
	int pathfinderUnk3(int tableLen, int x, int y);
	int pathfinderUnk4(int index, int v);
	void pathfinderUnk5(int *moveTable, int unk1, int x, int y, int moveTableSize);
	
	int _pathfinderUnkTable1[400];
	int _pathfinderUnkTable2[200];
	
	// item
	uint8 _itemHtDat[176];
	
	struct Item {
		uint16 id;
		uint16 sceneId;
		int16 x;
		uint8 y;
		uint16 unk7;
	};
	Item *_itemList;
	
	int findFreeItem();
	int countAllItems();
	int findItem(uint16 sceneId, uint16 id);
	int checkItemCollision(int x, int y);
	void resetItemList();
	
	int _itemInHand;
	int _handItemSet;

	bool dropItem(int unk1, uint16 item, int x, int y, int unk2);
	bool processItemDrop(uint16 sceneId, uint16 item, int x, int y, int unk1, int unk2);
	void itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, uint16 item);
	void exchangeMouseItem(int itemPos);
	bool pickUpItem(int x, int y);

	bool isDropable(int x, int y);

	static const byte _itemStringMap[];
	static const int _itemStringMapSize;

	// Just used in French version
	int getItemCommandStringDrop(uint16 item);
	int getItemCommandStringPickUp(uint16 item);

	void setMouseCursor(uint16 item);
	void setHandItem(uint16 item);
	void removeHandItem();
	
	// inventroy
	static const int _inventoryX[];
	static const int _inventoryY[];
	
	// localization
	void loadCCodeBuffer(const char *file);
	void loadOptionsBuffer(const char *file);
	void loadChapterBuffer(int chapter);
	uint8 *_optionsBuffer;
	uint8 *_cCodeBuffer;

	uint8 *_chapterBuffer;
	int _currentChapter;
	int _newChapterFile;
	
	const uint8 *getTableEntry(const uint8 *buffer, int id);
	const char *getTableString(int id, const uint8 *buffer, int decode);
	const char *getChapterString(int id);
	int decodeString1(const char *src, char *dst);
	void decodeString2(const char *src, char *dst);

	void changeFileExtension(char *buffer);
	
	char _internStringBuf[200];
	static const char *_languageExtension[];
	static const char *_scriptLangExt[];
	
	// character
	struct Character {
		uint16 sceneId;
		uint16 unk2;
		uint8 height;
		uint8 facing;
		uint16 animFrame;
		uint8 unk8;
		uint8 unk9;
		uint8 unkA;
		uint16 inventory[20];
		int16 x1, y1;
		int16 x2, y2;
	};
	
	Character _mainCharacter;
	bool _useCharPal;
	int _charPalEntry;
	uint8 _charPalTable[16];
	void updateCharPal(int unk1);
	void setCharPalEntry(int entry, int value);
	
	void moveCharacter(int facing, int x, int y);
	int updateCharPos(int *table);
	void updateCharPosWithUpdate();
	void updateCharAnimFrame(int num, int *table);
	
	int checkCharCollision(int x, int y);

	int _mainCharX, _mainCharY;
	int _charScaleX, _charScaleY;

	static const int _characterFrameTable[];
	
	// text
	void showMessageFromCCode(int id, int16 palIndex, int);
	void showMessage(const char *string, int16 palIndex);
	void showChapterMessage(int id, int16 palIndex);

	void updateCommandLineEx(int str1, int str2, int16 palIndex);
	
	const char *_shownMessage;

	byte _messagePal[3];
	int _msgUnk1;

	// chat
	int _vocHigh;

	const char *_chatText;
	int _chatObject;
	bool _chatIsNote;
	uint32 _chatEndTime;
	int _chatVocHigh, _chatVocLow;

	ScriptData _chatScriptData;
	ScriptState _chatScriptState;

	int chatGetType(const char *text);
	int chatCalcDuration(const char *text);

	void objectChat(const char *text, int object, int vocHigh, int vocLow);
	void objectChatInit(const char *text, int object, int vocHigh, int vocLow);
	void objectChatPrintText(const char *text, int object);
	void objectChatProcess(const char *script);
	void objectChatWaitToFinish();

	// sound
	int _oldTalkFile;
	int _currentTalkFile;
	void openTalkFile(int newFile);

	virtual void snd_playVoiceFile(int id);
	void snd_loadSoundFile(int id);

	void playVoice(int high, int low);
	
	// timer
	void timerFunc2(int);
	void timerFunc3(int);
	void timerFunc4(int);
	void timerFunc5(int);
	void timerFunc6(int);
	
	void setTimer1DelaySecs(int secs);
	
	// delay
	void delay(uint32 millis, bool updateGame = false, bool isMainLoop = false);

	// opcodes
	int o2_setCharacterFacingRefresh(ScriptState *script);
	int o2_setCharacterPos(ScriptState *script);
	int o2_defineObject(ScriptState *script);
	int o2_refreshCharacter(ScriptState *script);
	int o2_getCharacterX(ScriptState *script);
	int o2_getCharacterY(ScriptState *script);
	int o2_getCharacterFacing(ScriptState *script);
	int o2_setSceneComment(ScriptState *script);
	int o2_setCharacterAnimFrame(ScriptState *script);
	int o2_trySceneChange(ScriptState *script);
	int o2_showChapterMessage(ScriptState *script);
	int o2_wsaClose(ScriptState *script);
	int o2_displayWsaFrame(ScriptState *script);
	int o2_displayWsaSequentialFrames(ScriptState *script);
	int o2_wsaOpen(ScriptState *script);
	int o2_checkForItem(ScriptState *script);
	int o2_defineItem(ScriptState *script);
	int o2_countItemInInventory(ScriptState *script);
	int o2_queryGameFlag(ScriptState *script);
	int o2_resetGameFlag(ScriptState *script);
	int o2_setGameFlag(ScriptState *script);
	int o2_setHandItem(ScriptState *script);
	int o2_handItemSet(ScriptState *script);
	int o2_hideMouse(ScriptState *script);
	int o2_addSpecialExit(ScriptState *script);
	int o2_setMousePos(ScriptState *script);
	int o2_showMouse(ScriptState *script);
	//int o2_playSoundEffect(ScriptState *script);
	int o2_delay(ScriptState *script);
	int o2_setScaleTableItem(ScriptState *script);
	int o2_setDrawLayerTableItem(ScriptState *script);
	int o2_setCharPalEntry(ScriptState *script);
	int o2_drawSceneShape(ScriptState *script);
	int o2_drawSceneShapeOnPage(ScriptState *script);
	int o2_restoreBackBuffer(ScriptState *script);
	int o2_update(ScriptState *script);
	int o2_fadeScenePal(ScriptState *script);
	int o2_enterNewSceneEx(ScriptState *script);
	int o2_switchScene(ScriptState *script);
	int o2_getShapeFlag1(ScriptState *script);
	int o2_setLayerFlag(ScriptState *script);
	int o2_setZanthiaPos(ScriptState *script);
	int o2_loadMusicTrack(ScriptState *script);
	int o2_playWanderScoreViaMap(ScriptState *script);
	int o2_playSoundEffect(ScriptState *script);
	int o2_getRand(ScriptState *script);
	int o2_encodeShape(ScriptState *script);
	int o2_defineRoomEntrance(ScriptState *script);
	int o2_runTemporaryScript(ScriptState *script);
	int o2_setSpecialSceneScriptRunTime(ScriptState *script);
	int o2_defineSceneAnim(ScriptState *script);
	int o2_updateSceneAnim(ScriptState *script);
	int o2_defineRoom(ScriptState *script);
	int o2_countItemInstances(ScriptState *script);
	int o2_setSpecialSceneScriptState(ScriptState *script);
	int o2_clearSpecialSceneScriptState(ScriptState *script);
	int o2_querySpecialSceneScriptState(ScriptState *script);
	int o2_customChat(ScriptState *script);
	int o2_customChatFinish(ScriptState *script);
	int o2_setVocHigh(ScriptState *script);
	int o2_getVocHigh(ScriptState *script);
	int o2_zanthiaChat(ScriptState *script);
	int o2_isVoiceEnabled(ScriptState *script);
	int o2_isVoicePlaying(ScriptState *script);
	int o2_stopVoicePlaying(ScriptState *script);
	int o2_getGameLanguage(ScriptState *script);
	int o2_dummy(ScriptState *script);

	// opcodes temporary
	// TODO: rename it from temporary to something more appropriate
	int o2t_defineNewShapes(ScriptState *script);
	int o2t_setCurrentFrame(ScriptState *script);
	int o2t_playSoundEffect(ScriptState *script);
	int o2t_setShapeFlag(ScriptState *script);
	
	// script
	void runStartScript(int script, int unk1);
	void loadNPCScript();
	
	bool _noScriptEnter;

	ScriptData _npcScriptData;
	
	ScriptData _sceneScriptData;
	ScriptState _sceneScriptState;
	
	ScriptData _temporaryScriptData;
	ScriptState _temporaryScriptState;
	bool _temporaryScriptExecBit;
	Common::Array<const Opcode*> _opcodesTemporary;

	void runTemporaryScript(const char *filename, int unk1, int unk2, int newShapes, int shapeUnload);
	
	// pathfinder
	int _pathfinderFlag;
	
	// unk
	struct Object {
		char filename[13];
		uint8 scriptId;
		int16 x, y;
		int8 color;
	};
	Object *_objectList;
	
	uint8 *_unkBuf500Bytes;
	uint8 *_unkBuf200kByte;
	bool _unkFlag1;
	int _unk3, _unk4, _unk5;
	bool _unkSceneScreenFlag1;
	bool _unkHandleSceneChangeFlag;
};

} // end of namespace Kyra

#endif

