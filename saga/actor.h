/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Actor management module header file

#ifndef SAGA_ACTOR_H__
#define SAGA_ACTOR_H__

#include "saga/sprite.h"
#include "saga/itedata.h"
#include "saga/list.h"
#include "saga/saga.h"

namespace Saga {

class HitZone;


#define ACTOR_DEBUG

#define ACTOR_BARRIERS_MAX 16

#define ACTOR_MAX_STEPS_COUNT 32

#define ACTOR_DIALOGUE_HEIGHT 100

#define ACTOR_LMULT 4

#define ACTOR_COLLISION_WIDTH       32
#define ACTOR_COLLISION_HEIGHT       8

#define ACTOR_DIRECTIONS_COUNT	4	// for ActorFrameSequence
#define ACTOR_DIRECTION_RIGHT	0
#define ACTOR_DIRECTION_LEFT	1
#define ACTOR_DIRECTION_BACK	2
#define ACTOR_DIRECTION_FORWARD	3

#define ACTOR_SPEECH_STRING_MAX 16	// speech const
#define ACTOR_SPEECH_ACTORS_MAX 8

#define PATH_NODE_EMPTY -1

enum ActorActions {
	kActionWait = 0,
	kActionWalkToPoint = 1,
	kActionWalkToLink = 2,
	kActionWalkDir = 3,
	kActionSpeak = 4,
	kActionAccept = 5,
	kActionStoop = 6,
	kActionLook = 7,
	kActionCycleFrames = 8,
	kActionPongFrames = 9,
	kActionFreeze = 10,
	kActionFall = 11,
	kActionClimb = 12
};

enum SpeechFlags {
	kSpeakNoAnimate = 1,
	kSpeakAsync = 2,
	kSpeakSlow = 4
};

enum ActorFrameTypes {
	kFrameStand = 0,
	kFrameWalk = 1,
	kFrameSpeak = 2,
	kFrameGive = 3,
	kFrameGesture = 4,
	kFrameWait = 5,
	kFramePickUp = 6,
	kFrameLook = 7
//...some special
};

enum ActorFlagsEx {
	kActorNoCollide = (1 << 0),
	kActorNoFollow = (1 << 1),
	kActorCollided = (1 << 2),
	kActorBackwards = (1 << 3),
	kActorContinuous = (1 << 4),
	kActorFinalFace = (1 << 5),
	kActorFinishLeft = ((1 << 5) | (kDirLeft << 6)),
	kActorFinishRight = ((1 << 5) | (kDirRight << 6)),
	kActorFinishUp = ((1 << 5) | (kDirUp << 6)),
	kActorFinishDown = ((1 << 5) | (kDirDown << 6)),
	kActorFacingMask = (0xf << 5),
	kActorRandom = (1 << 10)
};

enum PathCellType {
	kPathCellEmpty = -1,
	//kDirUp = 0 .... kDirUpLeft = 7 
	kPathCellBarrier = 0x57
};

struct PathDirectionData {
	int8 direction;
	int16	x;
	int16 y;
};

struct ActorFrameRange {
	int frameIndex;
	int frameCount;
};

struct ActorFrameSequence {
	ActorFrameRange directions[ACTOR_DIRECTIONS_COUNT];
};

struct Location {
	int x;					// logical coordinates
	int y;					// 
	int z;					// 
	Location() {
		x = y = z = 0;
	}
	int distance(const Location &location) const {
		return MAX(ABS(x - location.x), ABS(y - location.y));
	}
	int &u() {
		return x;
	}
	int &v() {
		return y;
	}
	int u() const {
		return x;
	}
	int v() const {
		return y;
	}
	void delta(const Location &location, Location &result) const {
		result.x = x - location.x;
		result.y = y - location.y;
		result.z = z - location.z;
	}
	void add(const Location &location) {
		x += location.x;
		y += location.y;
		z += location.z;
	}
	void fromScreenPoint(const Point &screenPoint) {
		x = (screenPoint.x * ACTOR_LMULT);
		y = (screenPoint.y * ACTOR_LMULT);
		z = 0;
	}
	void toScreenPointXY(Point &screenPoint) const {
		screenPoint.x = x / ACTOR_LMULT;
		screenPoint.y = y / ACTOR_LMULT;
	}
	void toScreenPointXYZ(Point &screenPoint) const {
		screenPoint.x = x / ACTOR_LMULT;
		screenPoint.y = y / ACTOR_LMULT - z;
	}
};

class CommonObjectData {
public:
	int index;					// index in local array
	uint16 id;					// object id
	uint16 flags;				// initial flags
	int nameIndex;				// index in name string list
	int sceneNumber;			// scene
	int scriptEntrypointNumber;	// script entrypoint number

	Location location;			// logical coordinates
	Point screenPosition;		// screen coordinates
	int screenDepth;			//
	int screenScale;			//

	SpriteList spriteList;		// sprite list data
	int spriteListResourceId;	// sprite list resource id

	int frameNumber;			// current frame number

	CommonObjectData() {
		screenDepth = screenScale = 0;
		flags = 0;
		frameNumber = 0;
	}
};

typedef CommonObjectData *CommonObjectDataPointer;

typedef SortedList<CommonObjectDataPointer> CommonObjectOrderList;

class ObjectData: public CommonObjectData {	
public:
	uint16 interactBits;
};

class ActorData: public CommonObjectData {
public:
	bool disabled;				// Actor disabled in init section
	byte speechColor;			// Actor dialogue color
	
	uint16 actorFlags;			// dynamic flags
	int currentAction;			// ActorActions type
	int facingDirection;		// orientation
	int actionDirection;
	int actionCycle;
	uint16 targetObject;
	const HitZone *lastZone;
	
	int cycleFrameSequence;
	uint8 cycleDelay;
	uint8 cycleTimeCount;
	uint8 cycleFlags;

	ActorFrameSequence *frames;	// Actor's frames
	int framesCount;			// Actor's frames count
	int frameListResourceId;	// Actor's frame list resource id
	
	//int walkPath[ACTOR_STEPS_MAX]; //todo: will gone

	int tileDirectionsAlloced;
	byte *tileDirections;

	int walkStepsAlloced;
	Point *walkStepsPoints;

	int walkStepsCount;
	int walkStepIndex;

	Location finalTarget;
	Location partialTarget;
	int walkFrameSequence;
	
	void cycleWrap(int cycleLimit) {
		if (actionCycle >= cycleLimit)
			actionCycle = 0;
	}

	void addWalkStepPoint(const Point &point) {
		if (walkStepsCount + 1 > walkStepsAlloced) {
			walkStepsAlloced += 100;
			walkStepsPoints = (Point*)realloc(walkStepsPoints, walkStepsAlloced * sizeof(*walkStepsPoints));
		}
		walkStepsPoints[walkStepsCount++] = point;
	}

	ActorData() {
		memset(this, 0xFE, sizeof(*this)); 
		walkStepsPoints = NULL;
		tileDirectionsAlloced = walkStepsAlloced = walkStepsCount = walkStepIndex = 0;
		tileDirections = NULL;
		memset(&spriteList, 0, sizeof(spriteList));
	}
	~ActorData() {
		free(frames);
		free(tileDirections);
		free(walkStepsPoints);
		spriteList.freeMem();
	}
};



struct SpeechData {
	int speechColor[ACTOR_SPEECH_ACTORS_MAX];
	int outlineColor[ACTOR_SPEECH_ACTORS_MAX];
	int speechFlags;
	const char *strings[ACTOR_SPEECH_STRING_MAX];
	Point speechCoords[ACTOR_SPEECH_ACTORS_MAX];
	int stringsCount;
	int slowModeCharIndex;
	uint16 actorIds[ACTOR_SPEECH_ACTORS_MAX];
	int actorsCount;
	int sampleResourceId;
	bool playing;
	int playingTime;

	SpeechData() { 
		memset(this, 0, sizeof(*this)); 
	}
};



class Actor {
public:
	ActorData *_centerActor;
	ActorData *_protagonist;
	StringsTable _actorsStrings;

	Actor(SagaEngine *vm);
	~Actor();

	void cmdActorWalkTo(int argc, const char **argv);

	bool validActorId(uint16 id) { return (id == ID_PROTAG) || ((id >= objectIndexToId(kGameObjectActor, 0)) && (id < objectIndexToId(kGameObjectActor, _actorsCount))); }
	int actorIdToIndex(uint16 id) { return (id == ID_PROTAG ) ? 0 : objectIdToIndex(id); }
	uint16 actorIndexToId(int index) { return (index == 0 ) ? ID_PROTAG : objectIndexToId(kGameObjectActor, index); }
	ActorData *getActor(uint16 actorId);
	
// clarification: Obj - means game object, such Hat, Spoon etc,  Object - means Actor,Obj,HitZone,StepZone

	bool validObjId(uint16 id) { return (id >= objectIndexToId(kGameObjectObject, 0)) && (id < objectIndexToId(kGameObjectObject, _objsCount)); }
	int objIdToIndex(uint16 id) { return objectIdToIndex(id); }
	uint16 objIndexToId(int index) { return objectIndexToId(kGameObjectObject, index); }
	ObjectData *getObj(uint16 objId);
	
	int getObjectScriptEntrypointNumber(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectScriptEntrypointNumber wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->scriptEntrypointNumber : getActor(id)->scriptEntrypointNumber;
	}
	int getObjectFlags(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectFlags wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->flags : getActor(id)->flags;
	}

	int direct(int msec);
	int drawActors();
	void updateActorsScene(int actorsEntrance);			// calls from scene loading to update Actors info

	void drawPathTest();

	uint16 hitTest(const Point &testPoint);
	void takeExit(uint16 actorId, const HitZone *hitZone);
	bool actorEndWalk(uint16 actorId, bool recurse);
	bool actorWalkTo(uint16 actorId, const Location &toLocation);		
	ActorFrameRange *getActorFrameRange(uint16 actorId, int frameType);
	void actorFaceTowardsPoint(uint16 actorId, const Location &toLocation);
	void actorFaceTowardsObject(uint16 actorId, uint16 objectId);

	void realLocation(Location &location, uint16 objectId, uint16 walkFlags);

//	speech 
	void actorSpeech(uint16 actorId, const char **strings, int stringsCount, uint16 sampleResourceId, int speechFlags);
	void nonActorSpeech(const char **strings, int stringsCount, int speechFlags);
	void simulSpeech(const char *string, uint16 *actorIds, int actorIdsCount, int speechFlags);
	void setSpeechColor(int speechColor, int outlineColor) {
		_activeSpeech.speechColor[0] = speechColor;
		_activeSpeech.outlineColor[0] = outlineColor;
	}
	void abortAllSpeeches();
	void abortSpeech();
	bool isSpeaking() {
		return _activeSpeech.stringsCount > 0;
	}
	
private:
	bool loadActorResources(ActorData *actor);
	void stepZoneAction(ActorData *actor, const HitZone *hitZone, bool exit, bool stopped);

	void createDrawOrderList();
	void calcScreenPosition(CommonObjectData *commonObjectData);
	bool getSpriteParams(CommonObjectData *commonObjectData, int &frameNumber, SpriteList *&spriteList);

	bool followProtagonist(ActorData *actor);
	void findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint);
	void handleSpeech(int msec);
	void handleActions(int msec, bool setup);
	bool validPathCellPoint(const Point &testPoint) {
		return !((testPoint.x < 0) || (testPoint.x >= _xCellCount) ||
			(testPoint.y < 0) || (testPoint.y >= _yCellCount));
	}
	void setPathCell(const Point &testPoint, int8 value) {		
		if (!validPathCellPoint(testPoint)) {
			error("Actor::setPathCell wrong point");
		}
		_pathCell[testPoint.x + testPoint.y * _xCellCount] = value;
	}
	int8 getPathCell(const Point &testPoint) {
		if (!validPathCellPoint(testPoint)) {
			error("Actor::getPathCell wrong point");
		}
		return _pathCell[testPoint.x + testPoint.y * _xCellCount];
	}
	bool scanPathLine(const Point &point1, const Point &point2);
	int fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint);
	void setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint);
	void pathToNode();
	void condenseNodeList();
	void removeNodes();
	void nodeToPath();
	void removePathPoints();
	bool validFollowerLocation(const Location &location);
	
	int _lastTickMsec;
	SagaEngine *_vm;
	RSCFILE_CONTEXT *_actorContext;
	CommonObjectOrderList _drawOrderList;
	
protected:
	friend class IsoMap;
	int _actorsCount;
	ActorData **_actors;

private:
	int _objsCount;
	ObjectData **_objs;

	SpeechData _activeSpeech;

//path stuff
	struct PathNode {
		Point point;
		int link;
	};

	Rect _barrierList[ACTOR_BARRIERS_MAX];
	int _barrierCount;
	int8 *_pathCell;

	int _xCellCount;
	int _yCellCount;
	Rect _pathRect;

	PathDirectionData *_pathDirectionList;
	int _pathDirectionListCount;
	int _pathDirectionListAlloced;
	PathDirectionData * addPathDirectionListData() {
		if (_pathDirectionListCount + 1 >= _pathDirectionListAlloced) {
			_pathDirectionListAlloced += 100;
			_pathDirectionList = (PathDirectionData*) realloc(_pathDirectionList, _pathDirectionListAlloced * sizeof(*_pathDirectionList));
		}
		return &_pathDirectionList[_pathDirectionListCount++];
	}

	Point *_pathList;
	int _pathListIndex;
	int _pathListAlloced;
	void addPathListPoint(const Point &point) {
		++_pathListIndex;
		if (_pathListIndex >= _pathListAlloced) {
			_pathListAlloced += 100;
			_pathList = (Point*) realloc(_pathList, _pathListAlloced * sizeof(*_pathList));
			
		}
		_pathList[_pathListIndex] = point;
	}

	int _pathNodeListIndex;
	int _pathNodeListAlloced;
	PathNode *_pathNodeList;
	void addPathNodeListPoint(const Point &point) {
		++_pathNodeListIndex;
		if (_pathNodeListIndex >= _pathNodeListAlloced) {
			_pathNodeListAlloced += 100;
			_pathNodeList = (PathNode*) realloc(_pathNodeList, _pathNodeListAlloced * sizeof(*_pathNodeList));

		}
		_pathNodeList[_pathNodeListIndex].point = point;
	}

	int _newPathNodeListIndex;
	int _newPathNodeListAlloced;
	PathNode *_newPathNodeList;
	void incrementNewPathNodeListIndex() {
		++_newPathNodeListIndex;
		if (_newPathNodeListIndex >= _newPathNodeListAlloced) {
			_newPathNodeListAlloced += 100;
			_newPathNodeList = (PathNode*) realloc(_newPathNodeList, _newPathNodeListAlloced * sizeof(*_newPathNodeList));

		}
	}
	void addNewPathNodeListPoint(const PathNode &pathNode) {
		incrementNewPathNodeListIndex();
		_newPathNodeList[_newPathNodeListIndex] = pathNode;
	}

public:
#ifdef ACTOR_DEBUG
//path debug - use with care
	struct DebugPoint {
		Point point;
		byte color;
	};
	DebugPoint *_debugPoints;
	int _debugPointsCount;
	int _debugPointsAlloced;
	void addDebugPoint(const Point &point, byte color) {
		if (_debugPointsCount + 1 > _debugPointsAlloced) {
			_debugPointsAlloced += 1000;
			_debugPoints = (DebugPoint*) realloc(_debugPoints, _debugPointsAlloced * sizeof(*_debugPoints));
		}
		_debugPoints[_debugPointsCount].color = color;
		_debugPoints[_debugPointsCount++].point = point;
	}
#endif
};

inline int16 quickDistance(const Point &point1, const Point &point2) {
	Point delta;
	delta.x = ABS(point1.x - point2.x);
	delta.y = ABS(point1.y - point2.y);
	return ((delta.x < delta.y) ? (delta.y + delta.x / 2) : (delta.x + delta.y / 2));
}
} // End of namespace Saga

#endif
