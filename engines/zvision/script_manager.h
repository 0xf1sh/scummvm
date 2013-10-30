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

#ifndef ZVISION_SCRIPT_MANAGER_H
#define ZVISION_SCRIPT_MANAGER_H

#include "zvision/puzzle.h"
#include "zvision/control.h"
#include "zvision/sidefx.h"

#include "common/hashmap.h"
#include "common/queue.h"


namespace Common {
class String;
class SeekableReadStream;
}

namespace ZVision {

class ZVision;

enum StateKey {
	StateKey_World = 3,
	StateKey_Room  = 4,
	StateKey_Node  = 5,
	StateKey_View  = 6,
	StateKey_ViewPos = 7,
	StateKey_KeyPress = 8,
	StateKey_InventoryItem = 9,
	StateKey_LMouse = 10,
	StateKey_Rounds = 12,
	StateKey_Venus = 13,
	StateKey_RMouse = 18,
	StateKey_MenuState = 19,
	StateKey_RestoreFlag = 20,
	StateKey_Quitting = 39,
	StateKey_LastWorld = 40,
	StateKey_LastRoom = 41,
	StateKey_LastNode = 42,
	StateKey_LastView = 43,
	StateKey_LastViewPos = 44,
	StateKey_Menu_LastWorld = 45,
	StateKey_Menu_LastRoom = 46,
	StateKey_Menu_LastNode = 47,
	StateKey_Menu_LastView = 48,
	StateKey_Menu_LastViewPos = 49,
	StateKey_KbdRotateSpeed = 50,
	StateKey_Subtitles = 51,
	StateKey_StreamSkipKey = 52,
	StateKey_RotateSpeed = 53,
	StateKey_Volume = 56,
	StateKey_Qsound = 57,
	StateKey_VenusEnable = 58,
	StateKey_HighQuality = 59,
	StateKey_VideoLineSkip = 65,
	StateKey_Platform = 66,
	StateKey_InstallLevel = 67,
	StateKey_CountryCode = 68,
	StateKey_CPU = 69,
	StateKey_MovieCursor = 70,
	StateKey_NoTurnAnim = 71,
	StateKey_WIN958 = 72,
	StateKey_ShowErrorDlg = 73,
	StateKey_DebugCheats = 74,
	StateKey_JapanFonts = 75,
	StateKey_Brightness = 77,
	StateKey_EF9_B = 91,
	StateKey_EF9_G = 92,
	StateKey_EF9_R = 93,
	StateKey_EF9_Speed = 94,
	StateKey_Inv_0_Slot = 100,
	StateKey_Inv_1_Slot = 101,
	StateKey_Inv_49_Slot = 149,
	StateKey_Inv_TotalSlots = 150
};

struct Location {
	Location() : world('g'), room('a'), node('r'), view('y'), offset(0) {}

	char world;
	char room;
	char node;
	char view;
	uint32 offset;
};

typedef Common::List<Puzzle *> PuzzleList;
typedef Common::Queue<Puzzle *> PuzzleQueue;
typedef Common::List<Control *> ControlList;
typedef Common::HashMap<uint32, uint32> StateMap;
typedef Common::List<SideFX *> SideFXList;

class ScriptManager {
public:
	ScriptManager(ZVision *engine);
	~ScriptManager();

private:
	ZVision *_engine;

	struct script_scope {
		uint32 proc_count;

		PuzzleList *scope_queue; // For adding puzzles to queue
		PuzzleList *exec_queue;  // Switch to it when execute
		PuzzleList _priv_queue_one;
		PuzzleList _priv_queue_two;

		PuzzleList  _puzzles;
		ControlList _controls;
	};

	struct puzzle_ref {
		Puzzle *puz;
		script_scope *scope;
	};

	typedef Common::HashMap<uint32, Common::Array<puzzle_ref> > PuzzleMap;

	/**
	 * Holds the global state variable. Do NOT directly modify this. Use the accessors and
	 * mutators getStateValue() and setStateValue(). This ensures that Puzzles that reference a
	 * particular state key are checked after the key is modified.
	 */
	StateMap _globalState;
	/** Holds execute flags */
	StateMap _globalStateFlags;
	/** References _globalState keys to Puzzles */
	PuzzleMap _referenceTable;
	/** Holds the currently active controls */
	ControlList *_activeControls;

	script_scope universe;
	script_scope world;
	script_scope room;
	script_scope nodeview;

	/** Holds the currently active timers, musics, other */
	SideFXList _activeSideFx;

	Location _currentLocation;
	Location _nextLocation;

	uint32 _currentlyFocusedControl;

public:
	void initialize();
	void update(uint deltaTimeMillis);
	void queuePuzzles(uint32 key);

	uint getStateValue(uint32 key);
	void setStateValue(uint32 key, uint value);
	void addToStateValue(uint32 key, uint valueToAdd);

	uint getStateFlag(uint32 key);
	void setStateFlag(uint32 key, uint value);
	void unsetStateFlag(uint32 key, uint value);

	void addControl(Control *control);
	Control *getControl(uint32 key);

	void enableControl(uint32 key);
	void disableControl(uint32 key);

	void focusControl(uint32 key);

	void addSideFX(SideFX *fx);
	SideFX *getSideFX(uint32 key);
	void deleteSideFx(uint32 key);
	void stopSideFx(uint32 key);
	void killSideFx(uint32 key);
	void killSideFxType(SideFX::SideFXType type);

	/**
	 * Called when LeftMouse is pushed.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	void onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	/**
	 * Called when LeftMouse is lifted.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	void onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	/**
	 * Called on every MouseMove.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 * @return                           Was the cursor changed?
	 */
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	/**
	 * Called when a key is pressed.
	 *
	 * @param keycode    The key that was pressed
	 */
	void onKeyDown(Common::KeyState keyState);
	/**
	 * Called when a key is released.
	 *
	 * @param keycode    The key that was pressed
	 */
	void onKeyUp(Common::KeyState keyState);

	/** Mark next location */
	void changeLocation(char world, char room, char node, char view, uint32 offset);

	void serializeStateTable(Common::WriteStream *stream);
	void deserializeStateTable(Common::SeekableReadStream *stream);

	Location getCurrentLocation() const;

private:
	void addPuzzlesToReferenceTable(script_scope &scope);
	void updateNodes(uint deltaTimeMillis);
	void updateControls(uint deltaTimeMillis);
	void checkPuzzleCriteria(Puzzle *puzzle, uint counter);
	void cleanStateTable();
	void cleanScriptScope(script_scope &scope);
	void execScope(script_scope &scope);

	/** Perform change location */
	void do_changeLocation();

// TODO: Make this private. It was only made public so Console::cmdParseAllScrFiles() could use it
public:
	/**
	 * Parses a script file into triggers and events
	 *
	 * @param fileName    Name of the .scr file
	 * @param isGlobal    Are the puzzles included in the file global (true). AKA, the won't be purged during location changes
	 */
	void parseScrFile(const Common::String &fileName, script_scope &scope);

private:
	/**
	 * Parses the stream into a Puzzle object
	 * Helper method for parseScrFile.
	 *
	 * @param puzzle    The object to store what is parsed
	 * @param stream    Scr file stream
	 */
	void parsePuzzle(Puzzle *puzzle, Common::SeekableReadStream &stream);

	/**
	 * Parses the stream into a Criteria object
	 * Helper method for parsePuzzle.
	 *
	 * @param criteria    Pointer to the Criteria object to fill
	 * @param stream      Scr file stream
	 * @return            Whether any criteria were read
	 */
	bool parseCriteria(Common::SeekableReadStream &stream, Common::List<Common::List<Puzzle::CriteriaEntry> > &criteriaList) const;

	/**
	 * Parses the stream into a ResultAction objects
	 * Helper method for parsePuzzle.
	 *
	 * @param stream        Scr file stream
	 * @param actionList    The list where the results will be added
	 * @return              Created Results object
	 */
	void parseResults(Common::SeekableReadStream &stream, Common::List<ResultAction *> &actionList) const;

	/**
	 * Helper method for parsePuzzle. Parses the stream into a bitwise or of the StateFlags enum
	 *
	 * @param stream    Scr file stream
	 * @return          Bitwise OR of all the flags set within the puzzle
	 */
	uint parseFlags(Common::SeekableReadStream &stream) const;

	/**
	 * Helper method for parseScrFile. Parses the stream into a Control object
	 *
	 * @param line      The line initially read
	 * @param stream    Scr file stream
	 */
	Control *parseControl(Common::String &line, Common::SeekableReadStream &stream);
};


} // End of namespace ZVision

#endif
