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

#ifndef KYRA_KYRA_V1_H
#define KYRA_KYRA_V1_H

#include "kyra/kyra.h"
#include "kyra/script.h"
#include "kyra/screen_v1.h"
#include "kyra/gui_v1.h"

namespace Kyra {

class Movie;
class SoundDigital;
class SeqPlayer;
class Sprites;
class Debugger;
class ScreenAnimator;
class TextDisplayer;
class KyraEngine_v1;

struct Character {
	uint16 sceneId;
	uint8 height;
	uint8 facing;
	uint16 currentAnimFrame;
	uint8 inventoryItems[10];
	int16 x1, y1, x2, y2;
};

struct Shape {
	uint8 imageIndex;
	int8 xOffset, yOffset;
	uint8 x, y, w, h;
};

struct Room {
	uint8 nameIndex;
	uint16 northExit;
	uint16 eastExit;
	uint16 southExit;
	uint16 westExit;
	uint8 itemsTable[12];
	uint16 itemsXPos[12];
	uint8 itemsYPos[12];
	uint8 needInit[12];
};

struct Item {
	uint8 unk1;
	uint8 height;
	uint8 unk2;
	uint8 unk3;
};

struct SeqLoop {
	const uint8 *ptr;
	uint16 count;
};

struct SceneExits {
	uint16 northXPos;
	uint8  northYPos;
	uint16 eastXPos;
	uint8  eastYPos;
	uint16 southXPos;
	uint8  southYPos;
	uint16 westXPos;
	uint8  westYPos;
};

struct BeadState {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 dstX;
	int16 dstY;
	int16 width2;
	int16 unk8;
	int16 unk9;
	int16 tableIndex;
};

class KyraEngine_v1 : public KyraEngine {
	friend class MusicPlayer;
	friend class Debugger_v1;
	friend class ScreenAnimator;
	friend class GUI_v1;
public:
	KyraEngine_v1(OSystem *system, const GameFlags &flags);
	~KyraEngine_v1();

	Screen *screen() { return _screen; }
	ScreenAnimator *animator() { return _animator; }
	virtual Movie *createWSAMovie();

	uint8 **shapes() { return _shapes; }
	Character *currentCharacter() { return _currentCharacter; }
	Character *characterList() { return _characterList; }
	uint16 brandonStatus() { return _brandonStatusBit; }

	// TODO: remove me with workaround in animator.cpp l209
	uint16 getScene() { return _currentRoom; }

	int _paletteChanged;
	int16 _northExitHeight;

	typedef void (KyraEngine_v1::*IntroProc)();

	// static data access
	const char * const*seqWSATable() { return _seq_WSATable; }
	const char * const*seqCPSTable() { return _seq_CPSTable; }
	const char * const*seqCOLTable() { return _seq_COLTable; }
	const char * const*seqTextsTable() { return _seq_textsTable; }

	const uint8 * const*palTable1() { return &_specialPalettes[0]; }
	const uint8 * const*palTable2() { return &_specialPalettes[29]; }

protected:
	virtual int go();
	virtual int init();

public:
	// sequences
	// -> misc
	bool seq_skipSequence() const;
protected:
	// -> demo
	void seq_demo();

	// -> intro
	void seq_intro();
	void seq_introLogos();
	void seq_introStory();
	void seq_introMalcolmTree();
	void seq_introKallakWriting();
	void seq_introKallakMalcolm();

	// -> ingame animations
	void seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly);
	void seq_brandonHealing();
	void seq_brandonHealing2();
	void seq_poisonDeathNow(int now);
	void seq_poisonDeathNowAnim();
	void seq_playFluteAnimation();
	void seq_winterScroll1();
	void seq_winterScroll2();
	void seq_makeBrandonInv();
	void seq_makeBrandonNormal();
	void seq_makeBrandonNormal2();
	void seq_makeBrandonWisp();
	void seq_dispelMagicAnimation();
	void seq_fillFlaskWithWater(int item, int type);
	void seq_playDrinkPotionAnim(int item, int unk2, int flags);
	void seq_brandonToStone();

	// -> end fight
	int seq_playEnd();
	void seq_playEnding();

	int handleMalcolmFlag();
	int handleBeadState();
	void initBeadState(int x, int y, int x2, int y2, int unk1, BeadState *ptr);
	int processBead(int x, int y, int &x2, int &y2, BeadState *ptr);

	// -> credits
	void seq_playCredits();

public:
	// delay
	void delayUntil(uint32 timestamp, bool updateGameTimers = false, bool update = false, bool isMainLoop = false);
	void delay(uint32 millis, bool update = false, bool isMainLoop = false);
	void delayWithTicks(int ticks);
	void waitForEvent();

	// TODO
	void registerDefaultSettings();
	void readSettings();
	void writeSettings();

	void snd_playSoundEffect(int track, int volume=0xFF);
	void snd_playWanderScoreViaMap(int command, int restart);
	virtual void snd_playVoiceFile(int id);
	void snd_voiceWaitForFinish(bool ingame = true);

protected:
	void saveGame(const char *fileName, const char *saveName);
	void loadGame(const char *fileName);

protected:
	// input
	void processInput();
	int processInputHelper(int xpos, int ypos);
	int clickEventHandler(int xpos, int ypos);
	void clickEventHandler2();
	void updateMousePointer(bool forceUpdate = false);
	bool hasClickedOnExit(int xpos, int ypos);

	bool _skipFlag;
	bool skipFlag() const { return _skipFlag; }
	void resetSkipFlag(bool removeEvent = true) { _skipFlag = false; }

	// scene
	// -> init
	void loadSceneMsc();
	void startSceneScript(int brandonAlive);
	void setupSceneItems();
	void initSceneData(int facing, int unk1, int brandonAlive);
	void initSceneObjectList(int brandonAlive);
	void initSceneScreen(int brandonAlive);
	void setupSceneResource(int sceneId);

	// -> process
	void enterNewScene(int sceneId, int facing, int unk1, int unk2, int brandonAlive);
	int handleSceneChange(int xpos, int ypos, int unk1, int frameReset);
	int processSceneChange(int *table, int unk1, int frameReset);
	int changeScene(int facing);

	// -> modification
	void transcendScenes(int roomIndex, int roomName);
	void setSceneFile(int roomIndex, int roomName);

	// -> pathfinder
	int findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize);
	bool lineIsPassable(int x, int y);

	// -> item handling
	// --> misc
	void addItemToRoom(uint16 sceneId, uint8 item, int itemIndex, int x, int y);

	// --> drop handling
	void itemDropDown(int x, int y, int destX, int destY, byte freeItem, int item);
	int processItemDrop(uint16 sceneId, uint8 item, int x, int y, int unk1, int unk2);
	void dropItem(int unk1, int item, int x, int y, int unk2);

	// --> dropped item handling
	int countItemsInScene(uint16 sceneId);
	void exchangeItemWithMouseItem(uint16 sceneId, int itemIndex);
	byte findFreeItemInScene(int scene);
	byte findItemAtPos(int x, int y);

	// --> drop area handling
	void addToNoDropRects(int x, int y, int w, int h);
	void clearNoDropRects();
	int isDropable(int x, int y);
	int checkNoDropRects(int x, int y);

	// --> player items handling
	void updatePlayerItemsForScene();

	// --> item GFX handling
	void backUpItemRect0(int xpos, int ypos);
	void restoreItemRect0(int xpos, int ypos);
	void backUpItemRect1(int xpos, int ypos);
	void restoreItemRect1(int xpos, int ypos);

	// items
	// -> misc
	void placeItemInGenericMapScene(int item, int index);

	// -> mouse item
	void createMouseItem(int item);
	void destroyMouseItem();
	void setMouseItem(int item);

	// -> graphics effects
	void wipeDownMouseItem(int xpos, int ypos);
	void itemSpecialFX(int x, int y, int item);
	void itemSpecialFX1(int x, int y, int item);
	void itemSpecialFX2(int x, int y, int item);
	void magicOutMouseItem(int animIndex, int itemPos);
	void magicInMouseItem(int animIndex, int item, int itemPos);
	void specialMouseItemFX(int shape, int x, int y, int animIndex, int tableIndex, int loopStart, int maxLoops);
	void processSpecialMouseItemFX(int shape, int x, int y, int tableValue, int loopStart, int maxLoops);

	// character
	// -> movement
	void moveCharacterToPos(int character, int facing, int xpos, int ypos);
	void setCharacterPositionWithUpdate(int character);
	int setCharacterPosition(int character, int *facingTable);
	void setCharacterPositionHelper(int character, int *facingTable);
	void setCharactersPositions(int character);

	// -> brandon
	void setBrandonPoisonFlags(int reset);
	void resetBrandonPoisonFlags();

	// chat
	// -> process
	void characterSays(int vocFile, const char *chatStr, int8 charNum, int8 chatDuration);
	void waitForChatToFinish(int vocFile, int16 chatDuration, const char *str, uint8 charNum);

	// -> initialization
	int initCharacterChat(int8 charNum);
	void backupChatPartnerAnimFrame(int8 charNum);
	void restoreChatPartnerAnimFrame(int8 charNum);
	int8 getChatPartnerNum();

	// -> deinitialization
	void endCharacterChat(int8 charNum, int16 arg_4);

	// graphics
	// -> misc
	int findDuplicateItemShape(int shape);
	void updateKyragemFading();

	// -> interface
	void loadMainScreen(int page = 3);
	void redrawInventory(int page);
public:
	void drawSentenceCommand(const char *sentence, int unk1);
	void updateSentenceCommand(const char *str1, const char *str2, int unk1);
	void updateTextFade();

protected:
	// -> amulet
	void drawJewelPress(int jewel, int drawSpecial);
	void drawJewelsFadeOutStart();
	void drawJewelsFadeOutEnd(int jewel);

	// -> shape handling
	void setupShapes123(const Shape *shapeTable, int endShape, int flags);
	void freeShapes123();

	// misc (TODO)
	void startup();
	void mainLoop();

	int checkForNPCScriptRun(int xpos, int ypos);
	void runNpcScript(int func);

	void loadMouseShapes();
	void loadCharacterShapes();
	void loadSpecialEffectShapes();
	void loadItems();
	void loadButtonShapes();
	void initMainButtonList();
	void setCharactersInDefaultScene();
	void setupPanPages();
	void freePanPages();
	void closeFinalWsa();

	//void setTimer19();
	void setupTimers();
	void timerUpdateHeadAnims(int timerNum);
	void timerSetFlags1(int timerNum);
	void timerSetFlags2(int timerNum);
	void timerSetFlags3(int timerNum);
	void timerCheckAnimFlag1(int timerNum);
	void timerCheckAnimFlag2(int timerNum);
	void checkAmuletAnimFlags();
	void timerRedrawAmulet(int timerNum);
	void timerFadeText(int timerNum);
	void updateAnimFlag1(int timerNum);
	void updateAnimFlag2(int timerNum);
	void drawAmulet();
	void setTextFadeTimerCountdown(int16 countdown);
	void setWalkspeed(uint8 newSpeed);

	int buttonInventoryCallback(Button *caller);
	int buttonAmuletCallback(Button *caller);

	bool _skipIntroFlag;
	bool _abortIntroFlag;
	bool _menuDirectlyToLoad;
	bool _abortWalkFlag;
	bool _abortWalkFlag2;
	bool _mousePressFlag;
	uint8 *_itemBkgBackUp[2];
	uint8 *_shapes[373];
	int8 _itemInHand;
	int _mouseState;
	bool _handleInput;
	bool _changedScene;
	int _unkScreenVar1, _unkScreenVar2, _unkScreenVar3;
	int _beadStateVar;
	int _unkAmuletVar;

	int _malcolmFlag;
	int _endSequenceSkipFlag;
	int _endSequenceNeedLoading;
	int _unkEndSeqVar2;
	uint8 *_endSequenceBackUpRect;
	int _unkEndSeqVar4;
	int _unkEndSeqVar5;
	int _lastDisplayedPanPage;
	uint8 *_panPagesTable[20];
	Movie *_finalA, *_finalB, *_finalC;

	Movie *_movieObjects[10];

	uint16 _entranceMouseCursorTracks[8];
	uint16 _walkBlockNorth;
	uint16 _walkBlockEast;
	uint16 _walkBlockSouth;
	uint16 _walkBlockWest;

	int32 _scaleMode;
	int16 _scaleTable[145];

	Rect _noDropRects[11];

	int8 _birthstoneGemTable[4];
	int8 _idolGemsTable[3];

	int8 _marbleVaseItem;
	int8 _foyerItemTable[3];

	int8 _cauldronState;
	int8 _crystalState[2];

	uint16 _brandonStatusBit;
	uint8 _brandonStatusBit0x02Flag;
	uint8 _brandonStatusBit0x20Flag;
	uint8 _brandonPoisonFlagsGFX[256];
	uint8 _deathHandler;
	int16 _brandonInvFlag;
	uint8 _poisonDeathCounter;
	int _brandonPosX;
	int _brandonPosY;

	uint16 _currentChatPartnerBackupFrame;
	uint16 _currentCharAnimFrame;

	int8 *_sceneAnimTable[50];

	Item _itemTable[145];
	int _lastProcessedItem;
	int _lastProcessedItemHeight;

	int16 *_exitListPtr;
	int16 _exitList[11];
	SceneExits _sceneExits;
	uint16 _currentRoom;
	int _scenePhasingFlag;

	int _sceneChangeState;
	int _loopFlag2;

	int _pathfinderFlag;
	int _pathfinderFlag2;
	int _lastFindWayRet;
	int *_movFacingTable;

	int8 _talkingCharNum;
	int8 _charSayUnk2;
	int8 _charSayUnk3;
	int8 _currHeadShape;
	uint8 _currSentenceColor[3];
	int8 _startSentencePalIndex;
	bool _fadeText;

	uint8 _configTextspeed;

	ScreenAnimator *_animator;
	SeqPlayer *_seq;
	Sprites *_sprites;
	Screen_v1 *_screen;
	Debugger *_debugger;

	ScriptState *_scriptMain;

	ScriptState *_npcScript;
	ScriptData *_npcScriptData;

	ScriptState *_scriptClick;
	ScriptData *_scriptClickData;

	Character *_characterList;
	Character *_currentCharacter;

	Button *_buttonList;
	GUI_v1 *_gui;

	struct KyragemState {
		uint16 nextOperation;
		uint16 rOffset;
		uint16 gOffset;
		uint16 bOffset;
		uint32 timerCount;
	} _kyragemFadingState;

	static const int8 _dosTrackMap[];
	static const int _dosTrackMapSize;

	// TODO: get rid of all variables having pointers to the static resources if possible
	// i.e. let them directly use the _staticres functions
	void initStaticResource();

	const uint8 *_seq_Forest;
	const uint8 *_seq_KallakWriting;
	const uint8 *_seq_KyrandiaLogo;
	const uint8 *_seq_KallakMalcolm;
	const uint8 *_seq_MalcolmTree;
	const uint8 *_seq_WestwoodLogo;
	const uint8 *_seq_Demo1;
	const uint8 *_seq_Demo2;
	const uint8 *_seq_Demo3;
	const uint8 *_seq_Demo4;
	const uint8 *_seq_Reunion;

	const char * const*_seq_WSATable;
	const char * const*_seq_CPSTable;
	const char * const*_seq_COLTable;
	const char * const*_seq_textsTable;

	int _seq_WSATable_Size;
	int _seq_CPSTable_Size;
	int _seq_COLTable_Size;
	int _seq_textsTable_Size;

	const char * const*_itemList;
	const char * const*_takenList;
	const char * const*_placedList;
	const char * const*_droppedList;
	const char * const*_noDropList;
	const char * const*_putDownFirst;
	const char * const*_waitForAmulet;
	const char * const*_blackJewel;
	const char * const*_poisonGone;
	const char * const*_healingTip;
	const char * const*_thePoison;
	const char * const*_fluteString;
	const char * const*_wispJewelStrings;
	const char * const*_magicJewelString;
	const char * const*_flaskFull;
	const char * const*_fullFlask;
	const char * const*_veryClever;
	const char * const*_homeString;
	const char * const*_newGameString;

	int _itemList_Size;
	int _takenList_Size;
	int _placedList_Size;
	int _droppedList_Size;
	int _noDropList_Size;
	int _putDownFirst_Size;
	int _waitForAmulet_Size;
	int _blackJewel_Size;
	int _poisonGone_Size;
	int _healingTip_Size;
	int _thePoison_Size;
	int _fluteString_Size;
	int _wispJewelStrings_Size;
	int _magicJewelString_Size;
	int _flaskFull_Size;
	int _fullFlask_Size;
	int _veryClever_Size;
	int _homeString_Size;
	int _newGameString_Size;

	const char * const*_characterImageTable;
	int _characterImageTableSize;

	const char * const*_guiStrings;
	int _guiStringsSize;

	const char * const*_configStrings;
	int _configStringsSize;

	Shape *_defaultShapeTable;
	int _defaultShapeTableSize;

	const Shape *_healingShapeTable;
	int  _healingShapeTableSize;
	const Shape *_healingShape2Table;
	int  _healingShape2TableSize;

	const Shape *_posionDeathShapeTable;
	int _posionDeathShapeTableSize;

	const Shape *_fluteAnimShapeTable;
	int _fluteAnimShapeTableSize;

	const Shape *_winterScrollTable;
	int _winterScrollTableSize;
	const Shape *_winterScroll1Table;
	int _winterScroll1TableSize;
	const Shape *_winterScroll2Table;
	int _winterScroll2TableSize;

	const Shape *_drinkAnimationTable;
	int _drinkAnimationTableSize;

	const Shape *_brandonToWispTable;
	int _brandonToWispTableSize;

	const Shape *_magicAnimationTable;
	int _magicAnimationTableSize;

	const Shape *_brandonStoneTable;
	int _brandonStoneTableSize;

	Room *_roomTable;
	int _roomTableSize;
	const char * const*_roomFilenameTable;
	int _roomFilenameTableSize;

	const uint8 *_amuleteAnim;

	const uint8 * const*_specialPalettes;

	const char *const *_soundFiles;
	int _soundFilesSize;
	const char *const *_soundFilesIntro;
	int _soundFilesIntroSize;
	const int32 *_cdaTrackTable;
	int _cdaTrackTableSize;
	const AudioDataStruct * _soundData;

	static const int8 _charXPosTable[];
	static const int8 _charYPosTable[];

	// positions of the inventory
	static const uint16 _itemPosX[];
	static const uint8 _itemPosY[];

	void setupButtonData();
	Button *_buttonData;
	Button **_buttonDataListPtr;

	static const uint8 _magicMouseItemStartFrame[];
	static const uint8 _magicMouseItemEndFrame[];
	static const uint8 _magicMouseItemStartFrame2[];
	static const uint8 _magicMouseItemEndFrame2[];

	static const uint16 _amuletX[];
	static const uint16 _amuletY[];
	static const uint16 _amuletX2[];
	static const uint16 _amuletY2[];
protected:
	void setupOpcodeTable();

	// Opcodes
	int o1_magicInMouseItem(ScriptState *script);
	int o1_characterSays(ScriptState *script);
	int o1_pauseTicks(ScriptState *script);
	int o1_drawSceneAnimShape(ScriptState *script);
	int o1_queryGameFlag(ScriptState *script);
	int o1_setGameFlag(ScriptState *script);
	int o1_resetGameFlag(ScriptState *script);
	int o1_runNPCScript(ScriptState *script);
	int o1_setSpecialExitList(ScriptState *script);
	int o1_blockInWalkableRegion(ScriptState *script);
	int o1_blockOutWalkableRegion(ScriptState *script);
	int o1_walkPlayerToPoint(ScriptState *script);
	int o1_dropItemInScene(ScriptState *script);
	int o1_drawAnimShapeIntoScene(ScriptState *script);
	int o1_createMouseItem(ScriptState *script);
	int o1_savePageToDisk(ScriptState *script);
	int o1_sceneAnimOn(ScriptState *script);
	int o1_sceneAnimOff(ScriptState *script);
	int o1_getElapsedSeconds(ScriptState *script);
	int o1_mouseIsPointer(ScriptState *script);
	int o1_destroyMouseItem(ScriptState *script);
	int o1_runSceneAnimUntilDone(ScriptState *script);
	int o1_fadeSpecialPalette(ScriptState *script);
	int o1_playAdlibSound(ScriptState *script);
	int o1_playAdlibScore(ScriptState *script);
	int o1_phaseInSameScene(ScriptState *script);
	int o1_setScenePhasingFlag(ScriptState *script);
	int o1_resetScenePhasingFlag(ScriptState *script);
	int o1_queryScenePhasingFlag(ScriptState *script);
	int o1_sceneToDirection(ScriptState *script);
	int o1_setBirthstoneGem(ScriptState *script);
	int o1_placeItemInGenericMapScene(ScriptState *script);
	int o1_setBrandonStatusBit(ScriptState *script);
	int o1_pauseSeconds(ScriptState *script);
	int o1_getCharactersLocation(ScriptState *script);
	int o1_runNPCSubscript(ScriptState *script);
	int o1_magicOutMouseItem(ScriptState *script);
	int o1_internalAnimOn(ScriptState *script);
	int o1_forceBrandonToNormal(ScriptState *script);
	int o1_poisonDeathNow(ScriptState *script);
	int o1_setScaleMode(ScriptState *script);
	int o1_openWSAFile(ScriptState *script);
	int o1_closeWSAFile(ScriptState *script);
	int o1_runWSAFromBeginningToEnd(ScriptState *script);
	int o1_displayWSAFrame(ScriptState *script);
	int o1_enterNewScene(ScriptState *script);
	int o1_setSpecialEnterXAndY(ScriptState *script);
	int o1_runWSAFrames(ScriptState *script);
	int o1_popBrandonIntoScene(ScriptState *script);
	int o1_restoreAllObjectBackgrounds(ScriptState *script);
	int o1_setCustomPaletteRange(ScriptState *script);
	int o1_loadPageFromDisk(ScriptState *script);
	int o1_customPrintTalkString(ScriptState *script);
	int o1_restoreCustomPrintBackground(ScriptState *script);
	int o1_hideMouse(ScriptState *script);
	int o1_showMouse(ScriptState *script);
	int o1_getCharacterX(ScriptState *script);
	int o1_getCharacterY(ScriptState *script);
	int o1_changeCharactersFacing(ScriptState *script);
	int o1_copyWSARegion(ScriptState *script);
	int o1_printText(ScriptState *script);
	int o1_random(ScriptState *script);
	int o1_loadSoundFile(ScriptState *script);
	int o1_displayWSAFrameOnHidPage(ScriptState *script);
	int o1_displayWSASequentialFrames(ScriptState *script);
	int o1_drawCharacterStanding(ScriptState *script);
	int o1_internalAnimOff(ScriptState *script);
	int o1_changeCharactersXAndY(ScriptState *script);
	int o1_clearSceneAnimatorBeacon(ScriptState *script);
	int o1_querySceneAnimatorBeacon(ScriptState *script);
	int o1_refreshSceneAnimator(ScriptState *script);
	int o1_placeItemInOffScene(ScriptState *script);
	int o1_wipeDownMouseItem(ScriptState *script);
	int o1_placeCharacterInOtherScene(ScriptState *script);
	int o1_getKey(ScriptState *script);
	int o1_specificItemInInventory(ScriptState *script);
	int o1_popMobileNPCIntoScene(ScriptState *script);
	int o1_mobileCharacterInScene(ScriptState *script);
	int o1_hideMobileCharacter(ScriptState *script);
	int o1_unhideMobileCharacter(ScriptState *script);
	int o1_setCharactersLocation(ScriptState *script);
	int o1_walkCharacterToPoint(ScriptState *script);
	int o1_specialEventDisplayBrynnsNote(ScriptState *script);
	int o1_specialEventRemoveBrynnsNote(ScriptState *script);
	int o1_setLogicPage(ScriptState *script);
	int o1_fatPrint(ScriptState *script);
	int o1_preserveAllObjectBackgrounds(ScriptState *script);
	int o1_updateSceneAnimations(ScriptState *script);
	int o1_sceneAnimationActive(ScriptState *script);
	int o1_setCharactersMovementDelay(ScriptState *script);
	int o1_getCharactersFacing(ScriptState *script);
	int o1_bkgdScrollSceneAndMasksRight(ScriptState *script);
	int o1_dispelMagicAnimation(ScriptState *script);
	int o1_findBrightestFireberry(ScriptState *script);
	int o1_setFireberryGlowPalette(ScriptState *script);
	int o1_setDeathHandlerFlag(ScriptState *script);
	int o1_drinkPotionAnimation(ScriptState *script);
	int o1_makeAmuletAppear(ScriptState *script);
	int o1_drawItemShapeIntoScene(ScriptState *script);
	int o1_setCharactersCurrentFrame(ScriptState *script);
	int o1_waitForConfirmationMouseClick(ScriptState *script);
	int o1_pageFlip(ScriptState *script);
	int o1_setSceneFile(ScriptState *script);
	int o1_getItemInMarbleVase(ScriptState *script);
	int o1_setItemInMarbleVase(ScriptState *script);
	int o1_addItemToInventory(ScriptState *script);
	int o1_intPrint(ScriptState *script);
	int o1_shakeScreen(ScriptState *script);
	int o1_createAmuletJewel(ScriptState *script);
	int o1_setSceneAnimCurrXY(ScriptState *script);
	int o1_poisonBrandonAndRemaps(ScriptState *script);
	int o1_fillFlaskWithWater(ScriptState *script);
	int o1_getCharactersMovementDelay(ScriptState *script);
	int o1_getBirthstoneGem(ScriptState *script);
	int o1_queryBrandonStatusBit(ScriptState *script);
	int o1_playFluteAnimation(ScriptState *script);
	int o1_playWinterScrollSequence(ScriptState *script);
	int o1_getIdolGem(ScriptState *script);
	int o1_setIdolGem(ScriptState *script);
	int o1_totalItemsInScene(ScriptState *script);
	int o1_restoreBrandonsMovementDelay(ScriptState *script);
	int o1_setMousePos(ScriptState *script);
	int o1_getMouseState(ScriptState *script);
	int o1_setEntranceMouseCursorTrack(ScriptState *script);
	int o1_itemAppearsOnGround(ScriptState *script);
	int o1_setNoDrawShapesFlag(ScriptState *script);
	int o1_fadeEntirePalette(ScriptState *script);
	int o1_itemOnGroundHere(ScriptState *script);
	int o1_queryCauldronState(ScriptState *script);
	int o1_setCauldronState(ScriptState *script);
	int o1_queryCrystalState(ScriptState *script);
	int o1_setCrystalState(ScriptState *script);
	int o1_setPaletteRange(ScriptState *script);
	int o1_shrinkBrandonDown(ScriptState *script);
	int o1_growBrandonUp(ScriptState *script);
	int o1_setBrandonScaleXAndY(ScriptState *script);
	int o1_resetScaleMode(ScriptState *script);
	int o1_getScaleDepthTableValue(ScriptState *script);
	int o1_setScaleDepthTableValue(ScriptState *script);
	int o1_message(ScriptState *script);
	int o1_checkClickOnNPC(ScriptState *script);
	int o1_getFoyerItem(ScriptState *script);
	int o1_setFoyerItem(ScriptState *script);
	int o1_setNoItemDropRegion(ScriptState *script);
	int o1_walkMalcolmOn(ScriptState *script);
	int o1_passiveProtection(ScriptState *script);
	int o1_setPlayingLoop(ScriptState *script);
	int o1_brandonToStoneSequence(ScriptState *script);
	int o1_brandonHealingSequence(ScriptState *script);
	int o1_protectCommandLine(ScriptState *script);
	int o1_pauseMusicSeconds(ScriptState *script);
	int o1_resetMaskRegion(ScriptState *script);
	int o1_setPaletteChangeFlag(ScriptState *script);
	int o1_fillRect(ScriptState *script);
	int o1_dummy(ScriptState *script);
	int o1_vocUnload(ScriptState *script);
	int o1_vocLoad(ScriptState *script);
};

} // end of namespace Kyra

#endif

