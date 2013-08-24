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

#include "common/scummsys.h"

#include "common/algorithm.h"
#include "common/hashmap.h"
#include "common/debug.h"

#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/cursor_manager.h"
#include "zvision/actions.h"
#include "zvision/action_node.h"
#include "zvision/utility.h"

namespace ZVision {

ScriptManager::ScriptManager(ZVision *engine)
	: _engine(engine),
	  _changeLocation(false) {
}

ScriptManager::~ScriptManager() {
	for (Common::List<Puzzle *>::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); iter++) {
		delete (*iter);
	}
	for (Common::List<Puzzle *>::iterator iter = _globalPuzzles.begin(); iter != _globalPuzzles.end(); iter++) {
		delete (*iter);
	}
}

void ScriptManager::initialize() {
	parseScrFile("universe.scr", true);
	changeLocation('g', 'a', 'r', 'y', 0);
}

void ScriptManager::update(uint deltaTimeMillis) {
	updateNodes(deltaTimeMillis);
	checkPuzzleCriteria();

	if (_changeLocation) {
		changeLocationIntern();
		_changeLocation = false;
	}
}

void ScriptManager::createReferenceTable() {
	// Iterate through each local Puzzle
	for (Common::List<Puzzle *>::iterator activePuzzleIter = _activePuzzles.begin(); activePuzzleIter != _activePuzzles.end(); activePuzzleIter++) {
		Puzzle *puzzlePtr = (*activePuzzleIter);

		// Iterate through each CriteriaEntry and add a reference from the criteria key to the Puzzle
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = (*activePuzzleIter)->criteriaList.begin(); criteriaIter != (*activePuzzleIter)->criteriaList.end(); criteriaIter++) {
			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = (*criteriaIter).begin(); entryIter != (*criteriaIter).end(); entryIter++) {
				_referenceTable[entryIter->key].push_back(puzzlePtr);

				// If the argument is a key, add a reference to it as well
				if (entryIter->argumentIsAKey) {
					_referenceTable[entryIter->argument].push_back(puzzlePtr);
				}
			}
		}
	}

	// Iterate through each global Puzzle
	for (Common::List<Puzzle *>::iterator globalPuzzleIter = _globalPuzzles.begin(); globalPuzzleIter != _globalPuzzles.end(); globalPuzzleIter++) {
		Puzzle *puzzlePtr = (*globalPuzzleIter);

		// Iterate through each CriteriaEntry and add a reference from the criteria key to the Puzzle
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = (*globalPuzzleIter)->criteriaList.begin(); criteriaIter != (*globalPuzzleIter)->criteriaList.end(); criteriaIter++) {
			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = (*criteriaIter).begin(); entryIter != (*criteriaIter).end(); entryIter++) {
				_referenceTable[entryIter->key].push_back(puzzlePtr);

				// If the argument is a key, add a reference to it as well
				if (entryIter->argumentIsAKey) {
					_referenceTable[entryIter->argument].push_back(puzzlePtr);
				}
			}
		}
	}

	// Remove duplicate entries
	for (Common::HashMap<uint32, Common::Array<Puzzle *> >::iterator referenceTableIter = _referenceTable.begin(); referenceTableIter != _referenceTable.end(); referenceTableIter++) {
		removeDuplicateEntries(referenceTableIter->_value);
	}
}

void ScriptManager::updateNodes(uint deltaTimeMillis) {
	// If process() returns true, it means the node can be deleted
	for (Common::List<ActionNode *>::iterator iter = _activeNodes.begin(); iter != _activeNodes.end();) {
		if ((*iter)->process(deltaTimeMillis)) {
			// Delete the node then remove the pointer
			delete (*iter);
			iter = _activeNodes.erase(iter);
		} else {
			iter++;
		}
	}
}

void ScriptManager::checkPuzzleCriteria() {
	while (!_puzzlesToCheck.empty()) {
		Puzzle *puzzle = _puzzlesToCheck.pop();

		// Check if the puzzle is already finished
		// Also check that the puzzle isn't disabled
		if (getStateValue(puzzle->key) == 1 &&
			(puzzle->flags & Puzzle::DISABLED) == 0) {
			continue;
		}

		// Check each Criteria

		bool criteriaMet = false;
		for (Common::List<Common::List<Puzzle::CriteriaEntry> >::iterator criteriaIter = puzzle->criteriaList.begin(); criteriaIter != puzzle->criteriaList.end(); criteriaIter++) {
			criteriaMet = false;

			for (Common::List<Puzzle::CriteriaEntry>::iterator entryIter = (*criteriaIter).begin(); entryIter != (*criteriaIter).end(); entryIter++) {
				// Get the value to compare against
				uint argumentValue;
				if ((*entryIter).argumentIsAKey)
					argumentValue = getStateValue(entryIter->argument);
				else
					argumentValue = entryIter->argument;

				// Do the comparison
				switch ((*entryIter).criteriaOperator) {
				case Puzzle::EQUAL_TO:
					criteriaMet = getStateValue(entryIter->key) == argumentValue;
					break;
				case Puzzle::NOT_EQUAL_TO:
					criteriaMet = getStateValue(entryIter->key) != argumentValue;
					break;
				case Puzzle::GREATER_THAN:
					criteriaMet = getStateValue(entryIter->key) > argumentValue;
					break;
				case Puzzle::LESS_THAN:
					criteriaMet = getStateValue(entryIter->key) < argumentValue;
					break;
				}

				// If one check returns false, don't keep checking
				if (!criteriaMet) {
					break;
				}
			}

			// If any of the Criteria are *fully* met, then execute the results
			if (criteriaMet) {
				break;
			}
		}

		// criteriaList can be empty. Aka, the puzzle should be executed immediately
		if (puzzle->criteriaList.empty() || criteriaMet) {
			debug("Puzzle %u criteria passed. Executing its ResultActions", puzzle->key);

			bool shouldContinue = true;
			for (Common::List<ResultAction *>::iterator resultIter = puzzle->resultActions.begin(); resultIter != puzzle->resultActions.end(); resultIter++) {
				shouldContinue = shouldContinue && (*resultIter)->execute(_engine);
			}

			// Set the puzzle as completed
			setStateValue(puzzle->key, 1);

			if (!shouldContinue) {
				break;
			}
		}
	}
}

uint ScriptManager::getStateValue(uint32 key) {
	if (_globalState.contains(key))
		return _globalState[key];
	else
		return 0;
}

void ScriptManager::setStateValue(uint32 key, uint value) {
	_globalState[key] = value;

	if (_referenceTable.contains(key)) {
		for (Common::Array<Puzzle *>::iterator iter = _referenceTable[key].begin(); iter != _referenceTable[key].end(); iter++) {
			_puzzlesToCheck.push((*iter));
		}
	}
}

void ScriptManager::addToStateValue(uint32 key, uint valueToAdd) {
	_globalState[key] += valueToAdd;
}

bool ScriptManager::enableControl(uint32 key) {
	if (!_activeControls.contains(key)) {
		return false;
	} else {
		return _activeControls[key]->enable();
	}
}

bool ScriptManager::disableControl(uint32 key) {
	if (!_activeControls.contains(key)) {
		return false;
	} else {
		return _activeControls[key]->disable();
	}
}

void ScriptManager::addActionNode(ActionNode *node) {
	_activeNodes.push_back(node);
}

void ScriptManager::changeLocation(char world, char room, char node, char view, uint32 offset) {
	_nextLocation.world = world;
	_nextLocation.room = room;
	_nextLocation.node = node;
	_nextLocation.view = view;
	_nextLocation.offset = offset;

	_changeLocation = true;
}

void ScriptManager::changeLocationIntern() {
	assert(_nextLocation.world != 0);
	debug("Changing location to: %c %c %c %c %u", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view, _nextLocation.offset);

	// Clear all the containers
	_referenceTable.clear();
	_puzzlesToCheck.clear();
	for (Common::List<Puzzle *>::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); iter++) {
		delete (*iter);
	}
	_activePuzzles.clear();
	for (Common::HashMap<uint32, Control *>::iterator iter = _activeControls.begin(); iter != _activeControls.end(); iter++) {
		delete (*iter)._value;
	}
	_activeControls.clear();
	_engine->clearAllMouseEvents();
	// TODO: See if we need to clear _activeNodes as well. And if so, remember to delete the nodes before clearing the list

	// Revert to the idle cursor
	_engine->getCursorManager()->revertToIdle();

	// Reset the background velocity
	_engine->getRenderManager()->setBackgroundVelocity(0);

	// Parse into puzzles and controls
	Common::String fileName = Common::String::format("%c%c%c%c.scr", _nextLocation.world, _nextLocation.room, _nextLocation.node, _nextLocation.view);
	parseScrFile(fileName);

	// Change the background position
	_engine->getRenderManager()->setBackgroundPosition(_nextLocation.offset);

	// Enable all the controls
	for (Common::HashMap<uint32, Control *>::iterator iter = _activeControls.begin(); iter != _activeControls.end(); iter++) {
		(*iter)._value->enable();
	}

	// Add all the local puzzles to the queue to be checked
	for (Common::List<Puzzle *>::iterator iter = _activePuzzles.begin(); iter != _activePuzzles.end(); iter++) {
		// Reset any Puzzles that have the flag ONCE_PER_INST
		if (((*iter)->flags & Puzzle::ONCE_PER_INST) == Puzzle::ONCE_PER_INST) {
			setStateValue((*iter)->key, 0);
		}

		_puzzlesToCheck.push((*iter));
	}

	// Add all the global puzzles to the queue to be checked
	for (Common::List<Puzzle *>::iterator iter = _globalPuzzles.begin(); iter != _globalPuzzles.end(); iter++) {
		// Reset any Puzzles that have the flag ONCE_PER_INST
		if (((*iter)->flags & Puzzle::ONCE_PER_INST) == Puzzle::ONCE_PER_INST) {
			setStateValue((*iter)->key, 0);
		}

		_puzzlesToCheck.push((*iter));
	}

	// Create the puzzle reference table
	createReferenceTable();
}

} // End of namespace ZVision
