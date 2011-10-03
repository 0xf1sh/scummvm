/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/system.h"
#include "video/qt_decoder.h"

#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/interface.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/caldoria/caldoria4dsystem.h"
#include "pegasus/neighborhood/caldoria/caldoriamessages.h"
#include "pegasus/neighborhood/caldoria/caldoriamirror.h"

namespace Pegasus {

const short kVidPhoneAngle = 30;
const short kReplicatorAngle = 50;
const short kDrawersAngle = -30;
const short kCaldoria53Angle = 45;
const short kCaldoria55Angle = -45;

const TimeValue kSinclairInterruptionTime1 = 2955;
const TimeValue kSinclairInterruptionTime2 = 6835;
const TimeValue kSinclairInterruptionTime3 = 9835;
const TimeValue kSinclairInterruptionTime4 = 12555;

const tInputBits kPullbackInterruptFilter = kFilterAllInput;
const tInputBits kRecalibrationInterruptFilter = kFilterAllInput;

const TimeValue kCaldoriaReplicatorIntroIn = 4933;
const TimeValue kCaldoriaReplicatorIntroOut = 6557;

const TimeValue kCaldoriaReplicatorWrongChoiceIn = 6557;
const TimeValue kCaldoriaReplicatorWrongChoiceOut = 8586;

const TimeValue kCaldoriaReplicatorOJChoiceIn = 8586;
const TimeValue kCaldoriaReplicatorOJChoiceOut = 11687;

const TimeValue kCaldoriaMessagesIntroIn = 11687;
const TimeValue kCaldoriaMessagesIntroOut = 13641;

const TimeValue kCaldoriaFirstMessageIn = 13641;
const TimeValue kCaldoriaFirstMessageOut = 14203;

const TimeValue kCaldoriaSecondMessageIn = 14203;
const TimeValue kCaldoriaSecondMessageOut = 14750;

const TimeValue kCaldoriaDoorCloseIn = 14750;
const TimeValue kCaldoriaDoorCloseOut = 15472;

const TimeValue kCaldoriaElevatorCloseIn = 15472;
const TimeValue kCaldoriaElevatorCloseOut = 16336;

const TimeValue kCaldoriaShowerCloseIn = 16336;
const TimeValue kCaldoriaShowerCloseOut = 17101;

const TimeValue kCaldoriaGTDoorCloseIn = 17101;
const TimeValue kCaldoriaGTDoorCloseOut = 18523;

const TimeValue kCaldoriaNobodyHomeIn = 18523;
const TimeValue kCaldoriaNobodyHomeOut = 21469;

const TimeValue kCaldoriaNoOtherFloorIn = 21469;
const TimeValue kCaldoriaNoOtherFloorOut = 28013;

const TimeValue kCaldoria4DInstructionsIn = 28013;
const TimeValue kCaldoria4DInstructionsOut = 29730;

const TimeValue kCaldoriaDrinkOJIn = 33910;
const TimeValue kCaldoriaDrinkOJOut = 35846;

const TimeValue kCaldoriaNoOtherDestinationIn = 35846;
const TimeValue kCaldoriaNoOtherDestinationOut = 37877;

const TimeValue kCaldoriaUhghIn = 37877;
const TimeValue kCaldoriaUhghOut = 38025;

const TimeValue kCaldoriaSinclairShootsOSIn = 38025;
const TimeValue kCaldoriaSinclairShootsOSOut = 40649;

const TimeValue kCaldoriaScreamingAfterIn = 40649;
const TimeValue kCaldoriaScreamingAfterOut = 47661;

const TimeValue k4FloorTime = 0;

const TimeValue k4To1Start = 40;
const TimeValue k4To1Stop = 7720;

const TimeValue k4To5Start = 7720;
const TimeValue k4To5Stop = 10280;

const TimeValue k4To2Time = 10280;

const TimeValue k4To3Time = 10320;

const TimeValue k1FloorTime = 10360;

const TimeValue k1To4Start = 10400;
const TimeValue k1To4Stop = 18080;

const TimeValue k1To5Start = 18080;
const TimeValue k1To5Stop = 28320;

const TimeValue k1To2Time = 28320;

const TimeValue k1To3Time = 28360;

const TimeValue k5FloorTime = 28400;

const TimeValue k5To1Start = 28440;
const TimeValue k5To1Stop = 38680;

const TimeValue k5To4Start = 38680;
const TimeValue k5To4Stop = 41240;

const TimeValue k5To2Time = 41240;

const TimeValue k5To3Time = 41280;

//	MMFuseFunction functions...

const tNotificationFlags kSinclairLoopDoneFlag = kLastNeighborhoodNotificationFlag << 1;

void doorBombTimerExpiredFunction(FunctionPtr *, void *caldoria) {
	((Caldoria *)caldoria)->doorBombTimerExpired();
}

void sinclairTimerExpiredFunction(FunctionPtr *, void *caldoria) {
	((Caldoria *)caldoria)->sinclairTimerExpired();
}

Caldoria::Caldoria(InputHandler* nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "Caldoria", kCaldoriaID) {
	setIsItemTaken(kKeyCard);
	setIsItemTaken(kOrangeJuiceGlassEmpty);
	GameState.setTakenItemID(kOrangeJuiceGlassFull, GameState.isTakenItemID(kOrangeJuiceGlassEmpty));
	_zoomOutSpot = 0;
	_gunSprite = 0;
}

Caldoria::~Caldoria() {
}

void Caldoria::init() {
	Neighborhood::init();

	//	We need this notification flag as well.
	_neighborhoodNotification.notifyMe(this, kSinclairLoopDoneFlag, kSinclairLoopDoneFlag);

	forceStridingStop(kCaldoria55, kSouth, kAltCaldoriaSinclairDown);
	forceStridingStop(kCaldoria50, kNorth, kAltCaldoriaSinclairDown);
}

void Caldoria::start() {	
	g_energyMonitor->stopEnergyDraining();

	if (!GameState.getCaldoriaSeenPullback()) {
		_vm->_gfx->doFadeOutSync(kOneSecond * kFifteenTicksPerSecond, kFifteenTicksPerSecond);

		g_system->delayMillis(2 * 1000);

		Video::VideoDecoder *pullbackMovie = new Video::QuickTimeDecoder();

		if (!pullbackMovie->loadFile("Images/Caldoria/Pullback.movie"))
			error("Could not load pullback movie");

		bool skipped = false;
		Input input;

		_vm->_gfx->doFadeInSync(kTwoSeconds * kFifteenTicksPerSecond, kFifteenTicksPerSecond);

		bool saveAllowed = _vm->swapSaveAllowed(false);
		bool openAllowed = _vm->swapLoadAllowed(false);

		while (!_vm->shouldQuit() && !pullbackMovie->endOfVideo()) {
			if (pullbackMovie->needsUpdate()) {
				const Graphics::Surface *frame = pullbackMovie->decodeNextFrame();

				if (frame) {
					g_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, 64, 112, frame->w, frame->h);
					g_system->updateScreen();
				}
			}

			InputHandler::getCurrentInputDevice()->getInput(input, kPullbackInterruptFilter);
			if (input.anyInput()) { // TODO: Save/Quit requests
				skipped = true;
				break;
			}

			g_system->delayMillis(10);
		}

		delete pullbackMovie;

		_vm->swapSaveAllowed(saveAllowed);
		_vm->swapLoadAllowed(openAllowed);

		ExtraTable::Entry entry;

		if (!skipped) {
			uint32 white = g_system->getScreenFormat().RGBToColor(0xff, 0xff, 0xff);

			_vm->_gfx->doFadeOutSync(kThreeSeconds * kFifteenTicksPerSecond, kFifteenTicksPerSecond, white);

			g_system->delayMillis(3 * 1000 / 2);

			getExtraEntry(kCaldoria00WakeUp1, entry);
			_navMovie.setTime(entry.movieStart);
			_navMovie.redrawMovieWorld();
			_navMovie.show();
			_vm->refreshDisplay();
			_vm->_gfx->doFadeInSync(kOneSecond * kFifteenTicksPerSecond, kFifteenTicksPerSecond, white);
		} else {
			getExtraEntry(kCaldoria00WakeUp1, entry);
			_navMovie.setTime(entry.movieStart);
			_navMovie.redrawMovieWorld();
			_navMovie.show();
		}

		GameState.setCaldoriaSeenPullback(true);
	}

	Neighborhood::start();
}

void Caldoria::flushGameState() {
	GameState.setCaldoriaFuseTimeLimit(_utilityFuse.getTimeRemaining());
}

void Caldoria::setUpAIRules() {
	Neighborhood::setUpAIRules();

	if (g_AIArea) {
		if (GameState.allTimeZonesFinished()) {
			AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Caldoria/X49NB1", false);
			AILocationCondition *locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kCaldoria49, kNorth));
			AIRule *rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);

#if 0
			// TODO
			messageAction = new AIPlayMessageAction("Images/AI/Caldoria/X56EH1", false);
			AIBombActiveCondition *activeCondition = new AIBombActiveCondition;
			rule = new AIRule(activeCondition, messageAction);
			g_AIArea->addAIRule(rule);
#endif
		} else {
			AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Caldoria/XAB2", false);
			AITimerCondition *timerCondition = new AITimerCondition(kLateWarning3TimeLimit, 1, true);
			AILocationCondition *locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kCaldoria44, kEast));
			AINotCondition *notCondition = new AINotCondition(locCondition);
			AIAndCondition *andCondition = new AIAndCondition(timerCondition, notCondition);
			AIRule *rule = new AIRule(andCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Caldoria/XAB1", false);
			timerCondition = new AITimerCondition(kLateWarning2TimeLimit, 1, true);
			locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kCaldoria44, kEast));
			notCondition = new AINotCondition(locCondition);
			andCondition = new AIAndCondition(timerCondition, notCondition);
			rule = new AIRule(andCondition, messageAction);
			g_AIArea->addAIRule(rule);

			messageAction = new AIPlayMessageAction("Images/AI/Caldoria/XA44EB", false);
			locCondition = new AILocationCondition(3);
			locCondition->addLocation(MakeRoomView(kCaldoria01, kNorth));
			locCondition->addLocation(MakeRoomView(kCaldoria01, kEast));
			locCondition->addLocation(MakeRoomView(kCaldoria01, kSouth));
			rule = new AIRule(locCondition, messageAction);
			g_AIArea->addAIRule(rule);
			
			messageAction = new AIPlayMessageAction("Images/AI/Caldoria/X42WH1", false);
			AICondition *condition = makeLocationAndDoesntHaveItemCondition(kCaldoria44, kEast, kKeyCard);
			rule = new AIRule(condition, messageAction);
			g_AIArea->addAIRule(rule);
			
			AIActivateRuleAction *ruleAction = new AIActivateRuleAction(rule);
			locCondition = new AILocationCondition(1);
			locCondition->addLocation(MakeRoomView(kCaldoria42, kEast));
			rule = new AIRule(locCondition, ruleAction);
			g_AIArea->addAIRule(rule);
		}
 	}
}

uint16 Caldoria::getDateResID() const {
	return kDate2318ID;
}

TimeValue Caldoria::getViewTime(const tRoomID room, const tDirectionConstant direction) {
	ExtraTable::Entry extra;	
	uint32 extraID = 0xffffffff;

	switch (room) {
	case kCaldoria00:
		if (direction == kEast && _privateFlags.getFlag(kCaldoriaPrivate4DSystemOpenFlag))
			extraID = k4DEnvironOpenView;
		break;
	case kCaldoriaDrawers:
		if (direction == kNorth && _privateFlags.getFlag(kCaldoriaPrivateRightDrawerOpenFlag)) {
			if (GameState.isTakenItemID(kKeyCard))
				extraID = kRightDrawerOpenViewNoKeys;
			else
				extraID = kRightDrawerOpenViewWithKeys;
		}
		break;
	case kCaldoria16:
		if (direction == kSouth && GameState.getCaldoriaSeenSinclairInElevator())
			extraID = kCaldoria16SouthViewWithElevator;
		break;
	case kCaldoriaReplicator:
		if (GameState.getCaldoriaMadeOJ() && !(GameState.isTakenItemID(kOrangeJuiceGlassEmpty) || GameState.isTakenItemID(kOrangeJuiceGlassFull)))
			extraID = kReplicatorNorthViewWithOJ;
		break;
	case kCaldoriaKiosk:
	case kCaldoriaBinoculars:
		return 0xffffffff;
	case kCaldoria48:
		if (direction == kNorth && GameState.getCaldoriaRoofDoorOpen())
			extraID = kCa48NorthExplosion;
		break;
	}

	if (extraID == 0xffffffff)
		return Neighborhood::getViewTime(room, direction);

	getExtraEntry(extraID, extra);
	return extra.movieEnd - 1;
}

void Caldoria::startSpotOnceOnly(TimeValue startTime, TimeValue stopTime) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kCaldoria13, kEast):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen13CarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen13CarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria14, kEast):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen14CarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen14CarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria18, kWest):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen18CarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen18CarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria23, kSouth):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen23CarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen23CarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria33, kSouth):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen33CarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen33CarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria36, kNorth):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen36CarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen36CarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria41, kNorth):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen41NorthCarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen41NorthCarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria41, kEast):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen41EastCarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen41EastCarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	case MakeRoomView(kCaldoria41, kWest):
		if (!_privateFlags.getFlag(kCaldoriaPrivateSeen41WestCarFlag) && _vm->getRandomBit() == 0) {
			_privateFlags.setFlag(kCaldoriaPrivateSeen41WestCarFlag, true);
			Neighborhood::startSpotOnceOnly(startTime, stopTime);
		}
		break;
	default:
		Neighborhood::startSpotOnceOnly(startTime, stopTime);
		break;
	}
}

void Caldoria::findSpotEntry(const tRoomID room, const tDirectionConstant direction, tSpotFlags flags, SpotTable::Entry &entry) {	
	Neighborhood::findSpotEntry(room, direction, flags, entry);

	switch (room) {
	case kCaldoria00:
		if (direction == kEast && (!GameState.getCaldoriaINNAnnouncing() || GameState.getCaldoriaSeenINN()))
			entry.clear();
		break;
	case kCaldoriaVidPhone:
		if (direction == kNorth && GameState.getCaldoriaSeenMessages())
			entry.clear();
		break;
	case kCaldoria44:
		if (direction == kEast && GameState.getLastRoom() != kCaldoria42)
			entry.clear();
		break;
	}
}

void Caldoria::startExitMovie(const ExitTable::Entry &exitEntry) {
	switch (GameState.getCurrentRoom()) {
	case kCaldoria05:
	case kCaldoria07:
		if (GameState.getCurrentDirection() == kWest)
			closeCroppedMovie();
		// fall through
	case kCaldoria11:
		if (GameState.getCurrentDirection() == kEast)
			closeCroppedMovie();
		break;
	case kCaldoria13:
	case kCaldoria14:
		if (GameState.getCurrentDirection() == kNorth)
			closeCroppedMovie();
		break;
	}

	Neighborhood::startExitMovie(exitEntry);
}

void Caldoria::startZoomMovie(const ZoomTable::Entry &zoomEntry) {
	switch (GameState.getCurrentRoom()) {
	case kCaldoria12:
		if (GameState.getCurrentDirection() == kNorth)
			closeCroppedMovie();
		break;
	}

	Neighborhood::startZoomMovie(zoomEntry);
}

void Caldoria::startDoorOpenMovie(const TimeValue startTime, const TimeValue stopTime) {
	if (GameState.getCurrentRoom() == kCaldoria27 || GameState.getCurrentRoom() == kCaldoria28 || GameState.getCurrentRoom() == kCaldoria45)
		// Must be opening elevator door.
		closeCroppedMovie();

	if (GameState.getCurrentRoom() == kCaldoria44 && GameState.getLastRoom() != kCaldoria42)
		startExtraSequence(kArriveAtCaldoriaFromTSA, kDoorOpenCompletedFlag, false);
	else
		Neighborhood::startDoorOpenMovie(startTime, stopTime);
}

void Caldoria::startTurnPush(const tTurnDirection turnDirection, const TimeValue newViewTime, const tDirectionConstant destDirection) {
	switch (GameState.getCurrentRoom()) {
	case kCaldoria05:
	case kCaldoria07:
		if (GameState.getCurrentDirection() == kWest)
			closeCroppedMovie();
		break;
	case kCaldoria11:
		if (GameState.getCurrentDirection() == kEast)
			closeCroppedMovie();
		break;
	case kCaldoria12:
	case kCaldoria13:
	case kCaldoria14:
	case kCaldoria27:
	case kCaldoria28:
	case kCaldoria45:
		if (GameState.getCurrentDirection() == kNorth)
			closeCroppedMovie();
		break;
	case kCaldoria48:
		if (_croppedMovie.isSurfaceValid())
			closeCroppedMovie();
		break;
	}

	Neighborhood::startTurnPush(turnDirection, newViewTime, destDirection);
}

void Caldoria::bumpIntoWall() {
	requestSpotSound(kCaldoriaUhghIn, kCaldoriaUhghOut, kFilterNoInput, 0);
	Neighborhood::bumpIntoWall();
}

void Caldoria::closeDoorOffScreen(const tRoomID room, const tDirectionConstant direction) {
	switch (room) {
	case kCaldoria08:
		if (direction == kNorth)
			playSpotSoundSync(kCaldoriaShowerCloseIn, kCaldoriaShowerCloseOut);
		else
			playSpotSoundSync(kCaldoriaDoorCloseIn, kCaldoriaDoorCloseOut);
		break;
	case kCaldoria09:
		playSpotSoundSync(kCaldoriaShowerCloseIn, kCaldoriaShowerCloseOut);
		break;
	case kCaldoria16:
	case kCaldoria38:
	case kCaldoria46:
	case kCaldoria27:
	case kCaldoria28:
	case kCaldoria45:
		playSpotSoundSync(kCaldoriaElevatorCloseIn, kCaldoriaElevatorCloseOut);
		break;
	case kCaldoria44:
	case kCaldoria42:
		if (GameState.getCurrentRoom() == kCaldoria42)
			playSpotSoundSync(kCaldoriaGTDoorCloseIn, kCaldoriaGTDoorCloseOut);
		break;
	default:
		playSpotSoundSync(kCaldoriaDoorCloseIn, kCaldoriaDoorCloseOut);
		break;
	}
}

int16 Caldoria::getStaticCompassAngle(const tRoomID room, const tDirectionConstant dir) {
	int16 result = Neighborhood::getStaticCompassAngle(room, dir);

	switch (room) {
	case kCaldoriaVidPhone:
		result += kVidPhoneAngle;
		break;
	case kCaldoriaReplicator:
		result += kReplicatorAngle;
		break;
	case kCaldoriaDrawers:
		result += kDrawersAngle;
		break;
	case kCaldoria53:
		result += kCaldoria53Angle;
		break;
	case kCaldoria55:
		result += kCaldoria55Angle;
		break;
	}

	return result;
}

void Caldoria::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	Neighborhood::getExitCompassMove(exitEntry, compassMove);

	switch (MakeRoomView(exitEntry.room, exitEntry.direction)) {
	case MakeRoomView(kCaldoria08, kNorth):
	case MakeRoomView(kCaldoria09, kSouth):
		compassMove.insertFaderKnot((exitEntry.movieStart + exitEntry.movieEnd) >> 1, compassMove.getNthKnotValue(0) + 30);
		break;
	case MakeRoomView(kCaldoria10, kEast):
		compassMove.insertFaderKnot(exitEntry.movieStart + 4 * kCaldoriaFrameDuration, 90);
		compassMove.insertFaderKnot(exitEntry.movieStart + 19 * kCaldoriaFrameDuration, -90);
		break;
	case MakeRoomView(kCaldoria42, kWest):
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), exitEntry.movieStart, -90, exitEntry.movieEnd, 90);
		compassMove.insertFaderKnot(exitEntry.movieStart + 3 * kCaldoriaFrameDuration, -90);
		compassMove.insertFaderKnot(exitEntry.movieStart + 33 * kCaldoriaFrameDuration, 90);
		break;
	case MakeRoomView(kCaldoria54, kEast):
		if (getCurrentAlternate() != kAltCaldoriaSinclairDown) {
			compassMove.insertFaderKnot(exitEntry.movieStart + 16 * kCaldoriaFrameDuration, 135);
			compassMove.insertFaderKnot(exitEntry.movieEnd, 135);
		}
		break;
	case MakeRoomView(kCaldoria55, kNorth):
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), exitEntry.movieStart, 315, exitEntry.movieEnd, 270);
		break;
	}
}

void Caldoria::getZoomCompassMove(const ZoomTable::Entry &zoomEntry, FaderMoveSpec &compassMove) {
	Neighborhood::getZoomCompassMove(zoomEntry, compassMove);

	switch (zoomEntry.hotspot) {
	case kCaBathroomToiletSpotID:
		compassMove.insertFaderKnot(zoomEntry.movieStart + 4 * kCaldoriaFrameDuration, 90);
		compassMove.insertFaderKnot(zoomEntry.movieStart + 19 * kCaldoriaFrameDuration, -90);
		compassMove.insertFaderKnot(zoomEntry.movieEnd, -90);
		break;
	}
}

void Caldoria::getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) {
	switch (entry.extra) {
	case kCaldoria00WakeUp1:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 90, entry.movieEnd, 180);
		compassMove.insertFaderKnot(entry.movieStart + 1000, 90);
		compassMove.insertFaderKnot(entry.movieStart + 1640, 120);
		compassMove.insertFaderKnot(entry.movieStart + 2240, 135);
		compassMove.insertFaderKnot(entry.movieStart + 2640, 180);
		break;
	case kCaldoria00WakeUp2:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 180, entry.movieEnd, 90);
		compassMove.insertFaderKnot(entry.movieStart + 560, 90);
		break;
	case kCaldoria56BombStage1:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 90, entry.movieEnd, 10);
		compassMove.insertFaderKnot(entry.movieStart + 31 * kCaldoriaFrameDuration, 60);
		compassMove.insertFaderKnot(entry.movieStart + 49 * kCaldoriaFrameDuration, 60);
		compassMove.insertFaderKnot(entry.movieStart + 66 * kCaldoriaFrameDuration, 10);
		break;
	case kCaldoria56BombStage7:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 10, entry.movieEnd, 90);
		compassMove.insertFaderKnot(entry.movieStart + 131 * kCaldoriaFrameDuration, 10);
		compassMove.insertFaderKnot(entry.movieStart + 148 * kCaldoriaFrameDuration, 60);
		compassMove.insertFaderKnot(entry.movieStart + 165 * kCaldoriaFrameDuration, 60);
		compassMove.insertFaderKnot(entry.movieEnd - 5 * kCaldoriaFrameDuration, 90);
		break;
	default:
		Neighborhood::getExtraCompassMove(entry, compassMove);
		break;
	}
}

void Caldoria::loadAmbientLoops() {
	tRoomID room = GameState.getCurrentRoom();

	if (room == kCaldoria00 && GameState.getCaldoriaWokenUp())
		loadLoopSound1("Sounds/Caldoria/Apartment Music.AIFF", 0x100 / 4);
	else if (room >= kCaldoria01 && room <= kCaldoria14)
		loadLoopSound1("Sounds/Caldoria/Apartment Music.AIFF", 0x100 / 4);
	else if (room == kCaldoria27 || room == kCaldoria28 || room == kCaldoria45)
		loadLoopSound1("Sounds/Caldoria/Elevator Loop.AIFF", 0x100 / 5);
	else if (room == kCaldoria44)
		loadLoopSound1("Sounds/Caldoria/TSA Hum Loop.AIFF");
	else if (room >= kCaldoria15 && room <= kCaldoria48)
		loadLoopSound1("Sounds/Caldoria/Industrial Nuage.aiff", 2 * 0x100 / 3);
	else if (room >= kCaldoria49 && room <= kCaldoria56)
		loadLoopSound1("Sounds/Caldoria/A50NLB00.22K.AIFF", 0x100 / 4);
}

void Caldoria::checkContinuePoint(const tRoomID room, const tDirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kCaldoria06, kSouth):
	case MakeRoomView(kCaldoria13, kNorth):
	case MakeRoomView(kCaldoria16, kSouth):
	case MakeRoomView(kCaldoria38, kEast):
	case MakeRoomView(kCaldoria38, kWest):
	case MakeRoomView(kCaldoria40, kNorth):
	case MakeRoomView(kCaldoria44, kEast):
	case MakeRoomView(kCaldoria48, kNorth):
	case MakeRoomView(kCaldoria49, kNorth):
		makeContinuePoint();
		break;
	}
}

void Caldoria::spotCompleted() {
	Neighborhood::spotCompleted();
	if (GameState.getCurrentRoom() == kCaldoriaBinoculars)
		startExtraSequence(kBinocularsZoomInOnShip, kExtraCompletedFlag, kFilterNoInput);
}

void Caldoria::arriveAt(const tRoomID room, const tDirectionConstant direction) {	
	switch (room) {
	case kCaldoria56:
		if (!GameState.getCaldoriaGunAimed())
			// Fall through...
	case kCaldoria49:
	case kCaldoria50:
	case kCaldoria51:
	case kCaldoria52:
	case kCaldoria53:
	case kCaldoria54:
	case kCaldoria55:
		if (GameState.getCaldoriaSinclairShot())
			setCurrentAlternate(kAltCaldoriaSinclairDown);
		break;
	}

	Neighborhood::arriveAt(room, direction);
	Input dummy;

	switch (room) {
	case kCaldoria00:
		arriveAtCaldoria00();
		break;
	case kCaldoria05:
		if (direction == kWest && GameState.getCaldoriaINNAnnouncing())
			loopCroppedMovie("Images/Caldoria/A05 Light Loop", kCaldoriaA05LightLoopLeft, kCaldoriaA05LightLoopTop);
		break;
	case kCaldoria07:
		if (direction == kWest && GameState.getCaldoriaINNAnnouncing())
			loopCroppedMovie("Images/Caldoria/A07 Light Loop", kCaldoriaA07LightLoopLeft, kCaldoriaA07LightLoopTop);
		break;
	case kCaldoria09:
		_lastExtra = 0xffffffff;
		break;
	case kCaldoriaToilet:
		GameState.setScoringReadPaper(true);
		break;
	case kCaldoriaReplicator:
		setCurrentActivation(kActivateReplicatorReady);
		requestSpotSound(kCaldoriaReplicatorIntroIn, kCaldoriaReplicatorIntroOut, kFilterNoInput, 0);
		break;
	case kCaldoria11:
		setCurrentAlternate(kAltCaldoriaNormal);
		if (direction == kEast && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A11 Message Machine Loop", kCaldoria11MessageLoopLeft, kCaldoria11MessageLoopTop);
		break;
	case kCaldoria12:
		if (direction == kNorth && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A12 Message Machine Loop", kCaldoria12MessageLoopLeft, kCaldoria12MessageLoopTop);
		break;
	case kCaldoriaDrawers:
		setCurrentActivation(kActivateDrawersClosed);
		break;
	case kCaldoria13:
		GameState.setCaldoriaINNAnnouncing(true);
		if (direction == kNorth && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A13 Message Machine Loop", kCaldoria13MessageLoopLeft, kCaldoria13MessageLoopTop);
		break;
	case kCaldoria14:
		if (direction == kNorth && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A14 Message Machine Loop", kCaldoria14MessageLoopLeft, kCaldoria14MessageLoopTop);
		break;
	case kCaldoria08:
		if (direction == kWest)
			setCurrentActivation(kActivateMirrorReady);
		// Fall through...
	case kCaldoria15:
		GameState.setCaldoriaINNAnnouncing(true);
		break;
	case kCaldoria27:
	case kCaldoria28:
	case kCaldoria45:
		if (GameState.getCurrentDirection() == kNorth)
			openDoor();
		break;
	case kCaldoriaBinoculars:
		GameState.setScoringLookThroughTelescope(true);
		break;
	case kCaldoriaKiosk:
		GameState.setScoringSawCaldoriaKiosk(true);
		startExtraSequenceSync(kCaldoriaKioskVideo, kFilterAllInput);
		downButton(dummy);
		break;
	case kCaldoria44:
		arriveAtCaldoria44();
		break;
	case kCaldoria49:
		arriveAtCaldoria49();
		break;
	case kCaldoria53:
		if (direction == kEast && !GameState.getCaldoriaSinclairShot())
			zoomToSinclair();
		break;
	case kCaldoria50:
		if (direction == kNorth && !GameState.getCaldoriaSinclairShot())
			setUpSinclairLoops();
		break;
	case kCaldoria54:
		if (direction == kSouth && !GameState.getCaldoriaSinclairShot())
			setUpSinclairLoops();
		break;
	case kCaldoria56:
		arriveAtCaldoria56();
		break;
	case kCaldoriaDeathRoom:
		arriveAtCaldoriaDeath();
		break;
	}

	checkSinclairShootsOS();
	setUpRoofTop();
}

void Caldoria::doAIRecalibration() {
	GameState.setCaldoriaDidRecalibration(true);
	if (!g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Caldoria/XA01EB1", true, kRecalibrationInterruptFilter))
		return;

	g_interface->calibrateEnergyBar();
	if (!g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Caldoria/XA01EB4", true, kRecalibrationInterruptFilter))
		return;

	g_interface->raiseInventoryDrawerSync();
	if (!g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Caldoria/XA01EB6", true, kRecalibrationInterruptFilter)) {
		g_interface->lowerInventoryDrawerSync();
		return;
	}

	g_interface->lowerInventoryDrawerSync();
	g_interface->raiseBiochipDrawerSync();

	if (!g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Caldoria/XA01EB5", true, kRecalibrationInterruptFilter)) {
		g_interface->lowerBiochipDrawerSync();
		return;
	}

	g_interface->lowerBiochipDrawerSync();

	g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Caldoria/XA01EB8", false, kRecalibrationInterruptFilter);
}

void Caldoria::arriveAtCaldoria00() {
	if (GameState.getCurrentDirection() == kEast) {
		if (GameState.getCaldoriaWokenUp()) {
			if (!GameState.getCaldoriaDidRecalibration())
				doAIRecalibration();
			setCurrentActivation(kActivate4DClosed);
		} else {
			// Good morning, sleeping beauty
			ExtraTable::Entry extra;
			getExtraEntry(kCaldoria00WakeUp1, extra);

			if (_navMovie.getTime() != extra.movieStart) {
				_navMovie.setTime(extra.movieStart);
				_navMovie.redrawMovieWorld();
			}

			startExtraSequenceSync(kCaldoria00WakeUp1, kFilterNoInput);
			GameState.setCaldoriaWokenUp(true);
			playCroppedMovieOnce("Images/Caldoria/VidPhone.movie", kCaldoriaVidPhoneLeft, kCaldoriaVidPhoneTop, kFilterAllInput);
			startExtraSequence(kCaldoria00WakeUp2, kExtraCompletedFlag, kFilterNoInput);
		}
	}
}

bool Caldoria::wantsCursor() {
	return GameState.getCaldoriaDidRecalibration();
}

void Caldoria::arriveAtCaldoria44() {
	if (GameState.getLastNeighborhood() != kCaldoriaID) {
		openDoor();
	} else {
		setCurrentActivation(kActivateReadyForCard);
		loopExtraSequence(kCaldoriaTransporterArrowLoop, 0);
	}
}

void Caldoria::arriveAtCaldoria49() {
	if (GameState.getLastRoom() == kCaldoria48)
		setCurrentAlternate(kAltCaldoriaNormal);

	// Need to force the loop to play.
	if (GameState.getCurrentDirection() == kNorth) {
		GameState.setCaldoriaFuseTimeLimit(kSinclairShootsTimeLimit);
		startExtraSequence(kCa49NorthVoiceAnalysis, kExtraCompletedFlag, kFilterNoInput);
	}
}

void Caldoria::arriveAtCaldoria56() {
	if (!GameState.getCaldoriaBombDisarmed()) {
		_privateFlags.setFlag(kCaldoriaPrivateZoomingToBombFlag, true);

		if (GameState.getCurrentDirection() == kNorth) {
			turnRight();
		} else if (GameState.getCurrentDirection() == kSouth) {
			turnLeft();
		} else if (GameState.getCurrentDirection() == kEast) {
			_privateFlags.setFlag(kCaldoriaPrivateZoomingToBombFlag, false);
			newInteraction(kCaldoriaBombInteractionID);
		}
	}
}

void Caldoria::arriveAtCaldoriaDeath() {
	if (GameState.getLastRoom() == kCaldoria49) {
		if (GameState.getCaldoriaSinclairShot()) {
			die(kDeathNuclearExplosion);
		} else {
			playSpotSoundSync(kCaldoriaSinclairShootsOSIn, kCaldoriaSinclairShootsOSOut);
			playSpotSoundSync(kCaldoriaScreamingAfterIn, kCaldoriaScreamingAfterOut);
			die(kDeathSinclairShotDelegate);
		}
	} else {
		die(kDeathShotBySinclair);
	}
}

void Caldoria::setUpRoofTop() {
	switch (GameState.getCurrentRoom()) {
	case kCaldoria48:
		if (GameState.getCurrentDirection() == kNorth) {
			if (GameState.getCaldoriaRoofDoorOpen()) {
				setCurrentAlternate(kAltCaldoriaRoofDoorBlown);
			} else if (GameState.getCaldoriaDoorBombed()) {
				// Long enough for AI hints...?
				_utilityFuse.primeFuse(kCardBombCountDownTime);
				_utilityFuse.setFunctionPtr(&doorBombTimerExpiredFunction, (void *)this);
				_utilityFuse.lightFuse();

				loopCroppedMovie("Images/Caldoria/A48 Bomb Loop", kCaldoria48CardBombLoopLeft, kCaldoria48CardBombLoopTop);
			} else {
				setCurrentActivation(kActivateRoofSlotEmpty);
			}
		}
		break;
	case kCaldoria56:
		if (GameState.getCurrentDirection() == kEast && GameState.getCaldoriaGunAimed())
			startExtraSequence(kCa53EastShootSinclair, kExtraCompletedFlag, false);
		else
			//	Fall through...
	case kCaldoria49:
	case kCaldoria50:
	case kCaldoria51:
	case kCaldoria52:
	case kCaldoria53:
	case kCaldoria54:
	case kCaldoria55:
		if (!GameState.getCaldoriaSinclairShot()) {
			if (GameState.getCaldoriaSawVoiceAnalysis() && !_utilityFuse.isFuseLit()) {
				_utilityFuse.primeFuse(GameState.getCaldoriaFuseTimeLimit());
				_utilityFuse.setFunctionPtr(&sinclairTimerExpiredFunction, (void *)this);
				_utilityFuse.lightFuse();
			}
		} else {
			setCurrentAlternate(kAltCaldoriaSinclairDown);
		}
		break;
	}
}

void Caldoria::downButton(const Input &input) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kCaldoria01, kEast):
		GameState.setCaldoriaWokenUp(true);
		startExtraSequence(kCaldoria00SitDown, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		Neighborhood::downButton(input);
		break;
	}
}

void Caldoria::turnTo(const tDirectionConstant direction) {
	Neighborhood::turnTo(direction);

	switch (GameState.getCurrentRoom()) {
	case kCaldoria00:
		if (direction == kEast)
			setCurrentActivation(kActivate4DClosed);
		break;
	case kCaldoria01:
		if (direction == kEast) {
			GameState.setCaldoriaWokenUp(true);
			startExtraSequence(kCaldoria00SitDown, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kCaldoria05:
		if (direction == kWest && GameState.getCaldoriaINNAnnouncing())
			loopCroppedMovie("Images/Caldoria/A05 Light Loop", kCaldoriaA05LightLoopLeft, kCaldoriaA05LightLoopTop);
		break;
	case kCaldoria07:
		if (direction == kWest && GameState.getCaldoriaINNAnnouncing())
			loopCroppedMovie("Images/Caldoria/A07 Light Loop", kCaldoriaA07LightLoopLeft, kCaldoriaA07LightLoopTop);
		break;
	case kCaldoria08:
		if (direction == kWest)
			setCurrentActivation(kActivateMirrorReady);
		break;
	case kCaldoria09:
		_lastExtra = 0xffffffff;
		break;
	case kCaldoria11:
		if (direction == kEast && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A11 Message Machine Loop", kCaldoria11MessageLoopLeft, kCaldoria11MessageLoopTop);
		break;
	case kCaldoria12:
		if (direction == kNorth && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A12 Message Machine Loop", kCaldoria12MessageLoopLeft, kCaldoria12MessageLoopTop);
		break;
	case kCaldoria13:
		if (direction == kNorth && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A13 Message Machine Loop", kCaldoria13MessageLoopLeft, kCaldoria13MessageLoopTop);
		break;
	case kCaldoria14:
		if (direction == kNorth && !GameState.getCaldoriaSeenMessages())
			loopCroppedMovie("Images/Caldoria/A14 Message Machine Loop", kCaldoria14MessageLoopLeft, kCaldoria14MessageLoopTop);
		break;
	case kCaldoria27:
	case kCaldoria28:
	case kCaldoria45:
		if (direction == kNorth)
			openElevatorMovie();
		else
			closeCroppedMovie();
		break;
	case kCaldoria48:
		if (direction == kNorth && !GameState.getCaldoriaDoorBombed())
			setCurrentActivation(kActivateRoofSlotEmpty);
		break;
	case kCaldoria53:
		if (GameState.getCurrentDirection() == kEast && !GameState.getCaldoriaSinclairShot())
			zoomToSinclair();
		break;
	case kCaldoria50:
		if (direction == kNorth && !GameState.getCaldoriaSinclairShot())
			setUpSinclairLoops();
		break;
	case kCaldoria54:
		if (direction == kSouth && !GameState.getCaldoriaSinclairShot())
			setUpSinclairLoops();
		break;
	case kCaldoria56:
		if (_privateFlags.getFlag(kCaldoriaPrivateZoomingToBombFlag)) {
			_privateFlags.setFlag(kCaldoriaPrivateZoomingToBombFlag, false);
			newInteraction(kCaldoriaBombInteractionID);
		} else if (GameState.getCaldoriaBombDisarmed()) {
			_vm->playEndMessage();
		}
		break;
	}

	checkSinclairShootsOS();
}

void Caldoria::zoomTo(const Hotspot *zoomOutSpot) {
	// Need to set _zoomOutSpot here because we may come through
	// this function another way, say by pressing the down arrow,
	// that doesn't involve the ClickInHotSpot function.
	_zoomOutSpot = zoomOutSpot;

	if (zoomOutSpot->getObjectID() == kCaldoriaDrawersOutSpotID) {
		if (_privateFlags.getFlag(kCaloriaPrivateLeftDrawerOpenFlag)) {
			_privateFlags.setFlag(kCaloriaPrivateLeftDrawerOpenFlag, false);
			startExtraSequence(kLeftDrawerClose, kExtraCompletedFlag, kFilterNoInput);
		} else if (_privateFlags.getFlag(kCaldoriaPrivateRightDrawerOpenFlag)) {
			_privateFlags.setFlag(kCaldoriaPrivateRightDrawerOpenFlag, false);
			if (GameState.isTakenItemID(kKeyCard))
				startExtraSequence(kRightDrawerCloseNoKeys, kExtraCompletedFlag, false);
			else
				startExtraSequence(kRightDrawerCloseWithKeys, kExtraCompletedFlag, false);
		} else {
			Neighborhood::zoomTo(zoomOutSpot);
		}
	} else {
		Neighborhood::zoomTo(zoomOutSpot);
	}
}

void Caldoria::setUpSinclairLoops() {
	_navMovie.stop();
	scheduleNavCallBack(kSinclairLoopDoneFlag);
	_sinclairLoopCount = 0;
	_numSinclairLoops = 2;
	_navMovie.start();
}

void Caldoria::zoomToSinclair() {
	// TODO
}

void Caldoria::receiveNotification(Notification *notification, const tNotificationFlags flags) {	
	Neighborhood::receiveNotification(notification, flags);

	if ((flags & kExtraCompletedFlag) != 0) {
		InventoryItem *item;
		_interruptionFilter = kFilterAllInput;

		switch (_lastExtra) {
		case kCaldoria00WakeUp2:
			makeContinuePoint();
			// Force ArriveAt to do its thing...
			GameState.setCurrentRoom(kNoRoomID);
			arriveAt(kCaldoria00, kEast);
			break;
		case k4DEnvironOpenToINN:
			GameState.setCaldoriaSeenINN(true);
			GameState.setScoringSawINN(true);
			// Fall through to k4DEnvironOpen...
		case k4DEnvironOpen:
			_privateFlags.setFlag(kCaldoriaPrivate4DSystemOpenFlag, true);
			setCurrentActivation(kActivate4DOpen);
			newInteraction(kCaldoria4DInteractionID);
			break;
		case kCaldoriaShowerUp:
			GameState.setScoringTookShower(true);
			GameState.setCaldoriaDoneHygiene(true);
			break;
		case kLeftDrawerClose:
		case kRightDrawerCloseNoKeys:
		case kRightDrawerCloseWithKeys:
			if (_zoomOutSpot && _zoomOutSpot->getObjectID() == kCaldoriaDrawersOutSpotID) {
				Input input;
				clickInHotspot(input, _zoomOutSpot);
			}
			break;
		case kCreateOrangeJuice:
			setCurrentActivation(kActivateOJOnThePad);
			requestSpotSound(kCaldoriaReplicatorOJChoiceIn, kCaldoriaReplicatorOJChoiceOut, kFilterNoInput, 0);
			break;
		case kCaldoria00SitDown:
			arriveAt(kCaldoria00, kEast);
			break;
		case kCaldoria16ElevatorUp:
			startExtraSequence(kCaldoria16ElevatorDown, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kCaldoria16ElevatorDown:
			GameState.setCaldoriaSeenSinclairInElevator(true);
			_privateFlags.setFlag(kCaldoriaPrivateCanOpenElevatorDoorFlag, true);
			openDoor();
			break;
		case kCaldoriaFourthToGround:
		case kCaldoriaRoofToGround:
			arriveAt(kCaldoria28, GameState.getCurrentDirection());
			break;
		case kCaldoriaFourthToRoof:
		case kCaldoriaGroundToRoof:
			arriveAt(kCaldoria45, GameState.getCurrentDirection());
			break;
		case kCaldoriaGroundToFourth:
		case kCaldoriaRoofToFourth:
			arriveAt(kCaldoria27, GameState.getCurrentDirection());
			break;
		case kCaGTCardSwipe:
			item = (InventoryItem *)g_allItems.findItemByID(kKeyCard);
			_vm->addItemToInventory(item);
			setCurrentActivation(kActivateReadyToTransport);
			break;
		case kCaGTFryTheFly:
		case kCaGTGoToTSA:
			_vm->jumpToNewEnvironment(kFullTSAID, kTSA00, kNorth);
			break;
		case kCaGTGoToTokyo:
			playDeathExtra(kCaGTArriveAtTokyo, kDeathUncreatedInCaldoria);
			break;
		case kCaGTGoToBeach:
			playDeathExtra(kCaGTArriveAtBeach, kDeathUncreatedInCaldoria);
			break;
		case kCa48NorthExplosion:
			// Current biochip must be the shield if we got here.
			_vm->getCurrentBiochip()->setItemState(kShieldNormal);
			break;
		case kBinocularsZoomInOnShip:
			setCurrentActivation(kActivateFocusedOnShip);
			break;
		case kCa49NorthVoiceAnalysis:
			_utilityFuse.primeFuse(kSinclairShootsTimeLimit);
			_utilityFuse.setFunctionPtr(&sinclairTimerExpiredFunction, (void*) this);
			_utilityFuse.lightFuse();
			GameState.setCaldoriaSawVoiceAnalysis(true);
			break;
		case kCa53EastZoomToSinclair:
			if (GameState.getCaldoriaSinclairShot()) {
				delete _gunSprite;
				_gunSprite = 0;
				startExtraSequence(kCa53EastShootSinclair, kExtraCompletedFlag, false);
			} else {
				playDeathExtra(kCa53EastDeath2, kDeathSinclairShotDelegate);
			}
			break;
		case kCa53EastShootSinclair:
			_vm->addItemToInventory((InventoryItem *)g_allItems.findItemByID(kStunGun));
			startExtraSequence(kCa53EastZoomOutFromSinclair, kExtraCompletedFlag, false);
			GameState.setScoringStunnedSinclair(true);
			break;
		case kCa53EastZoomOutFromSinclair:
			setCurrentAlternate(kAltCaldoriaSinclairDown);
			updateViewFrame();
			makeContinuePoint();
			break;
		}
	} else if ((flags & kSpotSoundCompletedFlag) != 0) {
		switch (GameState.getCurrentRoom()) {
		case kCaldoria20:
		case kCaldoria21:
		case kCaldoria26:
		case kCaldoria29:
		case kCaldoria34:
		case kCaldoria35:
			updateViewFrame();
			break;
		case kCaldoria27:
		case kCaldoria28:
		case kCaldoria45:
			updateElevatorMovie();
			break;
		case kCaldoriaReplicator:
			emptyOJGlass();
			break;
		}
	} else if ((flags & kSinclairLoopDoneFlag) != 0) {
		if (++_sinclairLoopCount == _numSinclairLoops) {
			switch (GameState.getCurrentRoom()) {
			case kCaldoria50:
				playDeathExtra(kCa50SinclairShoots, kDeathShotBySinclair);
				break;
			case kCaldoria54:
				playDeathExtra(kCa54SouthDeath, kDeathShotBySinclair);
				break;
			}
		} else {
			_navMovie.stop();
			scheduleNavCallBack(kSinclairLoopDoneFlag);
			_navMovie.start();
		}
	}

	g_AIArea->checkMiddleArea();
}

tInputBits Caldoria::getInputFilter() {
	tInputBits result = Neighborhood::getInputFilter();

	switch (GameState.getCurrentRoom()) {
	case kCaldoria00:
		if (_privateFlags.getFlag(kCaldoriaPrivate4DSystemOpenFlag))
			result &= ~kFilterAllDirections;
		break;
	case kCaldoriaBinoculars:
		if (getCurrentActivation() == kActivateNotFocusedOnShip)
			result &= ~(kFilterDownButton | kFilterDownAuto);
		break;
	case kCaldoria53:
		if (_privateFlags.getFlag(kCaldoriaPrivateReadyToShootFlag) && !GameState.getCaldoriaSinclairShot())
			result &= ~kFilterAllDirections;
		break;
	case kCaldoria48:
		if (GameState.getCaldoriaDoorBombed())
			result &= ~kFilterAllDirections;
	}

	return result;
}

void Caldoria::activateHotspots() {
	Neighborhood::activateHotspots();

	switch (GameState.getCurrentRoom()) {
	case kCaldoriaDrawers:
		if (getCurrentActivation() == kActivateRightOpen)
			if (GameState.isTakenItemID(kKeyCard)) {
				g_allHotspots.activateOneHotspot(kCaldoriaRightDrawerNoKeysCloseSpotID);
				g_allHotspots.deactivateOneHotspot(kCaldoriaRightDrawerWithKeysCloseSpotID);
			} else {
				g_allHotspots.activateOneHotspot(kCaldoriaRightDrawerWithKeysCloseSpotID);
				g_allHotspots.deactivateOneHotspot(kCaldoriaRightDrawerNoKeysCloseSpotID);
			}
	case kCaldoriaReplicator:
		if (GameState.getCaldoriaMadeOJ())
			g_allHotspots.deactivateOneHotspot(kCaldoriaMakeOJSpotID);
		break;
	case kCaldoria27:
		if (GameState.isCurrentDoorOpen()) {
			g_allHotspots.deactivateOneHotspot(kCaldoriaFourthFloorElevator1);
			g_allHotspots.deactivateOneHotspot(kCaldoriaFourthFloorElevator2);
			g_allHotspots.deactivateOneHotspot(kCaldoriaFourthFloorElevator3);
			g_allHotspots.deactivateOneHotspot(kCaldoriaFourthFloorElevator4);
			g_allHotspots.deactivateOneHotspot(kCaldoriaFourthFloorElevator5);
		}
		break;
	case kCaldoria28:
		if (GameState.isCurrentDoorOpen()) {
			g_allHotspots.deactivateOneHotspot(kCaldoriaGroundElevator1);
			g_allHotspots.deactivateOneHotspot(kCaldoriaGroundElevator2);
			g_allHotspots.deactivateOneHotspot(kCaldoriaGroundElevator3);
			g_allHotspots.deactivateOneHotspot(kCaldoriaGroundElevator4);
			g_allHotspots.deactivateOneHotspot(kCaldoriaGroundElevator5);
		}
		break;
	case kCaldoria45:
		if (GameState.isCurrentDoorOpen()) {
			g_allHotspots.deactivateOneHotspot(kCaldoriaRoofElevator1);
			g_allHotspots.deactivateOneHotspot(kCaldoriaRoofElevator2);
			g_allHotspots.deactivateOneHotspot(kCaldoriaRoofElevator3);
			g_allHotspots.deactivateOneHotspot(kCaldoriaRoofElevator4);
			g_allHotspots.deactivateOneHotspot(kCaldoriaRoofElevator5);
		}
		break;
	}
}

void Caldoria::clickInHotspot(const Input &input, const Hotspot *spot) {
	switch (spot->getObjectID()) {
	case kCa4DEnvironOpenSpotID:
		if (!GameState.getCaldoriaINNAnnouncing() || GameState.getCaldoriaSeenINN()) {
			startExtraSequence(k4DEnvironOpen, kExtraCompletedFlag, kFilterNoInput);
		} else {
			// This trick depends on the following sequences being in order in the
			// world movie:
			// k4DEnvironOpenToINN
			// k4DINNInterruption
			// k4DINNIntro
			// k4DINNMarkJohnson
			// k4DINNMeganLove
			// k4DINNFadeOut
			// k4DEnvironOpenFromINN
			loadLoopSound1("");
			loadLoopSound2("");
			startExtraLongSequence(k4DEnvironOpenToINN, k4DEnvironOpenFromINN, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kCa4DEnvironCloseSpotID:
		((Caldoria4DSystem *)_currentInteraction)->shutDown4DSystem();
		break;
	case kCaBathroomMirrorSpotID:
		newInteraction(kCaldoriaMirrorInteractionID);
		break;
	case kCaShowerSpotID:
		requestExtraSequence(kCaldoriaShowerTitle, 0, kFilterNoInput);
		requestExtraSequence(kCaldoriaShowerButton, 0, kFilterNoInput);
		requestExtraSequence(kCaldoriaShowerDown, 0, kFilterNoInput);
		requestExtraSequence(kCaldoriaShowerUp, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaLeftDrawerOpenSpotID:
		_privateFlags.setFlag(kCaloriaPrivateLeftDrawerOpenFlag, true);
		setCurrentActivation(kActivateLeftOpen);
		startExtraSequence(kLeftDrawerOpen, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaLeftDrawerCloseSpotID:
		_privateFlags.setFlag(kCaloriaPrivateLeftDrawerOpenFlag, false);
		setCurrentActivation(kActivateDrawersClosed);
		startExtraSequence(kLeftDrawerClose, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaRightDrawerOpenSpotID:
		_privateFlags.setFlag(kCaldoriaPrivateRightDrawerOpenFlag, true);
		setCurrentActivation(kActivateRightOpen);
		if (GameState.isTakenItemID(kKeyCard))
			startExtraSequence(kRightDrawerOpenNoKeys, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kRightDrawerOpenWithKeys, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaRightDrawerWithKeysCloseSpotID:
		_privateFlags.setFlag(kCaldoriaPrivateRightDrawerOpenFlag, false);
		setCurrentActivation(kActivateDrawersClosed);
		startExtraSequence(kRightDrawerCloseWithKeys, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaRightDrawerNoKeysCloseSpotID:
		_privateFlags.setFlag(kCaldoriaPrivateRightDrawerOpenFlag, false);
		setCurrentActivation(kActivateDrawersClosed);
		startExtraSequence(kRightDrawerCloseNoKeys, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaMakeStickyBunsSpotID:
		requestSpotSound(kCaldoriaReplicatorWrongChoiceIn, kCaldoriaReplicatorWrongChoiceOut, kFilterNoInput, 0);
		break;
	case kCaldoriaMakeOJSpotID:
		GameState.setCaldoriaMadeOJ(true);
		startExtraSequence(kCreateOrangeJuice, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaBedroomVidPhoneActivationSpotID:
		newInteraction(kCaldoriaMessagesInteractionID);
		break;
	case kCaldoriaFourthFloorElevatorSpotID:
		if (!GameState.getCaldoriaSeenSinclairInElevator()) {
			startExtraSequence(kCaldoria16ElevatorUp, kExtraCompletedFlag, kFilterNoInput);
		} else {
			_privateFlags.setFlag(kCaldoriaPrivateCanOpenElevatorDoorFlag, true);
			openDoor();
		}
		break;
	case kCaldoriaGroundElevatorSpotID:
		_privateFlags.setFlag(kCaldoriaPrivateCanOpenElevatorDoorFlag, true);
		openDoor();
		break;
	case kCaldoriaRoofElevatorSpotID:
		_privateFlags.setFlag(kCaldoriaPrivateCanOpenElevatorDoorFlag, true);
		openDoor();
		break;
	case kCaldoriaFourthFloorElevator1:
	case kCaldoriaFourthFloorElevator2:
	case kCaldoriaFourthFloorElevator3:
	case kCaldoriaFourthFloorElevator4:
	case kCaldoriaFourthFloorElevator5:
		// Assumes that elevator hot spots are consecutive.
		takeElevator(4, spot->getObjectID() - kCaldoriaFourthFloorElevator1 + 1);
		break;
	case kCaldoriaGroundElevator1:
	case kCaldoriaGroundElevator2:
	case kCaldoriaGroundElevator3:
	case kCaldoriaGroundElevator4:
	case kCaldoriaGroundElevator5:
		// Assumes that elevator hot spots are consecutive.
		takeElevator(1, spot->getObjectID() - kCaldoriaGroundElevator1 + 1);
		break;
	case kCaldoriaRoofElevator1:
	case kCaldoriaRoofElevator2:
	case kCaldoriaRoofElevator3:
	case kCaldoriaRoofElevator4:
	case kCaldoriaRoofElevator5:
		// Assumes that elevator hot spots are consecutive.
		takeElevator(5, spot->getObjectID() - kCaldoriaRoofElevator1 + 1);
		break;
	case kCaldoriaGTTokyoSpotID:
		startExtraSequence(kCaGTGoToTokyo, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaGTTSASpotID:
		GameState.setScoringGoToTSA(true);
		startExtraLongSequence(kCaGTFryTheFly, kCaGTGoToTSA, kExtraCompletedFlag, false);
		break;
	case kCaldoriaGTBeachSpotID:
		startExtraSequence(kCaGTGoToBeach, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaGTOtherSpotID:
		showExtraView(kCaGTOtherChoice);
		playSpotSoundSync(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut);
		showExtraView(kCaGTCardSwipe);
		break;
	case kCaldoriaZoomInOnShipSpotID:
		startExtraSequence(kBinocularsZoomInOnShip, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoriaRoofDoorSpotID:
		startExtraSequence(kCa48NorthRooftopClosed, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaldoria20DoorbellSpotID:
	case kCaldoria21DoorbellSpotID:
	case kCaldoria26DoorbellSpotID:
	case kCaldoria29DoorbellSpotID:
	case kCaldoria34DoorbellSpotID:
	case kCaldoria35DoorbellSpotID:
		clickOnDoorbell(spot->getObjectID());
		break;
	default:
		Neighborhood::clickInHotspot(input, spot);
		break;
	}
}

void Caldoria::clickOnDoorbell(const tHotSpotID doorBellSpotID) {
	uint32 extra;
	ExtraTable::Entry entry;

	switch (doorBellSpotID) {
	case kCaldoria20DoorbellSpotID:
		extra = kCaldoria20Doorbell;
		break;
	case kCaldoria21DoorbellSpotID:
		extra = kCaldoria21Doorbell;
		break;
	case kCaldoria26DoorbellSpotID:
		extra = kCaldoria26Doorbell;
		break;
	case kCaldoria29DoorbellSpotID:
		extra = kCaldoria29Doorbell;
		break;
	case kCaldoria34DoorbellSpotID:
		extra = kCaldoria34Doorbell;
		break;
	case kCaldoria35DoorbellSpotID:
		extra = kCaldoria35Doorbell;
		break;
	}

	getExtraEntry(extra, entry);
	showViewFrame(entry.movieStart);
	requestSpotSound(kCaldoriaNobodyHomeIn, kCaldoriaNobodyHomeOut, kFilterNoInput, kSpotSoundCompletedFlag);
}

tCanOpenDoorReason Caldoria::canOpenDoor(DoorTable::Entry &entry) {
	switch (GameState.getCurrentRoom()) {
	case kCaldoria16:
	case kCaldoria38:
	case kCaldoria46:
		if (GameState.getCurrentDirection() == kSouth && !_privateFlags.getFlag(kCaldoriaPrivateCanOpenElevatorDoorFlag))
			return kCantOpenLocked;
		break;
	}

	return Neighborhood::canOpenDoor(entry);
}

void Caldoria::doorOpened() {
	Neighborhood::doorOpened();
	_privateFlags.setFlag(kCaldoriaPrivateCanOpenElevatorDoorFlag, false);
}

GameInteraction *Caldoria::makeInteraction(const tInteractionID interactionID) {
	switch (interactionID) {
	case kCaldoria4DInteractionID:
		return new Caldoria4DSystem(this);
		break;
	case kCaldoriaBombInteractionID:
		// TODO
		error("STUB: Bomb game");
		break;
	case kCaldoriaMessagesInteractionID:
		return new CaldoriaMessages(this, kCaldoriaMessagesNotificationID, _vm);
		break;
	case kCaldoriaMirrorInteractionID:
		return new CaldoriaMirror(this);
		break;
	}

	return 0;
}

void Caldoria::newInteraction(const tInteractionID interactionID) {
	Neighborhood::newInteraction(interactionID);

	if (!_currentInteraction) {
		if (_privateFlags.getFlag(kCaldoriaPrivate4DSystemOpenFlag)) {
			_privateFlags.setFlag(kCaldoriaPrivate4DSystemOpenFlag, false);
			setCurrentActivation(kActivate4DClosed);
			startExtraSequence(k4DEnvironClose, kExtraCompletedFlag, kFilterNoInput);
		} else if (GameState.getCaldoriaBombDisarmed()) {
			turnLeft();
		}
	}
}

//	Only called when trying to pick up an item and the player can't (because
//	the inventory is too full or because the player lets go of the item before
//	dropping it into the inventory).
Hotspot *Caldoria::getItemScreenSpot(Item *item, DisplayElement *element) {
	tHotSpotID destSpotID = kNoHotSpotID;
	
	switch (item->getObjectID()) {
	case kKeyCard:
		destSpotID = kCaldoriaKeyCardSpotID;
		break;
	case kOrangeJuiceGlassEmpty:
	case kOrangeJuiceGlassFull:
		destSpotID = kCaldoriaOrangeJuiceSpotID;
		break;
	}

	if (destSpotID == kNoHotSpotID)
		return Neighborhood::getItemScreenSpot(item, element);

	return g_allHotspots.findHotspotByID(destSpotID);
}

void Caldoria::pickedUpItem(Item *item) {
	switch (item->getObjectID()) {
	case kKeyCard:
		GameState.setScoringGotKeyCard(true);
		break;
	case kOrangeJuiceGlassFull:
		setCurrentActivation(kActivateReplicatorReady);
		requestSpotSound(kCaldoriaDrinkOJIn, kCaldoriaDrinkOJOut, kFilterNoInput, kSpotSoundCompletedFlag);
		break;
	case kStunGun:
		GameState.setCaldoriaGunAimed(false);
		break;
	}
}

void Caldoria::dropItemIntoRoom(Item *item, Hotspot *dropSpot) {	
	switch (item->getObjectID()) {
	case kKeyCard:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		if (dropSpot->getObjectID() == kCaldoriaGTCardDropSpotID)
			startExtraSequence(kCaGTCardSwipe, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kOrangeJuiceGlassEmpty:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		if (dropSpot->getObjectID() == kCaldoriaOrangeJuiceDropSpotID) {
			GameState.setCaldoriaMadeOJ(false);
			startExtraSequence(kDisposeOrangeJuice, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kCardBomb:
		GameState.setCaldoriaDoorBombed(true);
		setCurrentActivation(kActivateHotSpotAlways);
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		// Long enough for AI hints...?
		_utilityFuse.primeFuse(kCardBombCountDownTime);
		_utilityFuse.setFunctionPtr(&doorBombTimerExpiredFunction, (void *)this);
		_utilityFuse.lightFuse();
		GameState.setCaldoriaFuseTimeLimit(kCardBombCountDownTime);
		loopCroppedMovie("Images/Caldoria/A48 Bomb Loop", kCaldoria48CardBombLoopLeft, kCaldoria48CardBombLoopTop);
		GameState.setScoringUsedCardBomb(true);
		break;
	case kStunGun:
		GameState.setCaldoriaGunAimed(true);
		GameState.setCaldoriaSinclairShot(true);
		_gunSprite = item->getDragSprite(0);
		_gunSprite->setCurrentFrameIndex(1);
		_gunSprite->setDisplayOrder(kDragSpriteOrder);
		_gunSprite->moveElementTo(kCaldoriaGunSpriteLeft, kCaldoriaGunSpriteTop);
		_gunSprite->startDisplaying();
		_gunSprite->show();
		break;
	default:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	}
}

void Caldoria::takeElevator(uint startFloor, uint endFloor) {
	_croppedMovie.stop();
	_croppedMovie.setSegment(0, _croppedMovie.getDuration());

	switch (startFloor) {
	case 1:
		switch (endFloor) {
		case 1:
			// Do nothing.
			break;
		case 2:
			_croppedMovie.setTime(k1To2Time);
			requestSpotSound(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut, kFilterNoInput, kSpotSoundCompletedFlag);
			break;
		case 3:
			_croppedMovie.setTime(k1To3Time);
			requestSpotSound(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut, kFilterNoInput, kSpotSoundCompletedFlag);
			break;
		case 4:
			_croppedMovie.setSegment(k1To4Start, k1To4Stop);
			_croppedMovie.setTime(k1To4Start);
			startExtraSequence(kCaldoriaGroundToFourth, kExtraCompletedFlag, false);
			_croppedMovie.start();
			break;
		case 5:
			_croppedMovie.setSegment(k1To5Start, k1To5Stop);
			_croppedMovie.setTime(k1To5Start);
			startExtraSequence(kCaldoriaGroundToRoof, kExtraCompletedFlag, false);
			_croppedMovie.start();
			break;
		}
		break;
	case 4:
		switch (endFloor) {
		case 1:
			_croppedMovie.setSegment(k4To1Start, k4To1Stop);
			_croppedMovie.setTime(k4To1Start);
			startExtraSequence(kCaldoriaFourthToGround, kExtraCompletedFlag, false);
			_croppedMovie.start();
			break;
		case 2:
			_croppedMovie.setTime(k4To2Time);
			requestSpotSound(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut, kFilterNoInput, kSpotSoundCompletedFlag);
			break;
		case 3:
			_croppedMovie.setTime(k4To3Time);
			requestSpotSound(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut, kFilterNoInput, kSpotSoundCompletedFlag);
			break;
		case 4:
			// Do nothing.
			break;
		case 5:
			_croppedMovie.setSegment(k4To5Start, k4To5Stop);
			_croppedMovie.setTime(k4To5Start);
			startExtraSequence(kCaldoriaFourthToRoof, kExtraCompletedFlag, false);
			_croppedMovie.start();
			break;
		}
		break;
	case 5:
		switch (endFloor) {
		case 1:
			_croppedMovie.setSegment(k5To1Start, k5To1Stop);
			_croppedMovie.setTime(k5To1Start);
			startExtraSequence(kCaldoriaRoofToGround, kExtraCompletedFlag, false);
			_croppedMovie.start();
			break;
		case 2:
			_croppedMovie.setTime(k5To2Time);
			requestSpotSound(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut, kFilterNoInput, kSpotSoundCompletedFlag);
			break;
		case 3:
			_croppedMovie.setTime(k5To3Time);
			requestSpotSound(kCaldoriaNoOtherDestinationIn, kCaldoriaNoOtherDestinationOut, kFilterNoInput, kSpotSoundCompletedFlag);
			break;
		case 4:
			_croppedMovie.setSegment(k5To4Start, k5To4Stop);
			_croppedMovie.setTime(k5To4Start);
			startExtraSequence(kCaldoriaRoofToFourth, kExtraCompletedFlag, false);
			_croppedMovie.start();
			break;
		case 5:
			// Do nothing.
			break;
		}
		break;
	};
}

void Caldoria::updateElevatorMovie() {
	TimeValue time = 0xffffffff;

	if (GameState.getCurrentDirection() == kNorth) {
		switch (GameState.getCurrentRoom()) {
		case kCaldoria27:
			time = k4FloorTime;
			break;
		case kCaldoria28:
			time = k1FloorTime;
			break;
		case kCaldoria45:
			time = k5FloorTime;
			break;
		}
	}

	_croppedMovie.stop();

	if (time == 0xffffffff) {
		_croppedMovie.hide();
	} else {
		_croppedMovie.stop();
		_croppedMovie.setSegment(0, _croppedMovie.getDuration());
		_croppedMovie.setTime(time);
		_croppedMovie.redrawMovieWorld();
		_croppedMovie.show();
		
		// *** Why do I need this?
		// clone2727: "don't ask me!"
		_navMovie.redrawMovieWorld();
	}
}

void Caldoria::openElevatorMovie() {
	if (!_croppedMovie.isSurfaceValid())
		openCroppedMovie("Images/Caldoria/Caldoria Elevator.movie", kCaldoriaElevatorLeft, kCaldoriaElevatorTop);

	updateElevatorMovie();
}

void Caldoria::emptyOJGlass() {
	GameState.setTakenItemID(kOrangeJuiceGlassFull, false);
	GameState.setTakenItemID(kOrangeJuiceGlassEmpty, true);
	_vm->removeItemFromInventory((InventoryItem *)g_allItems.findItemByID(kOrangeJuiceGlassFull));
	_vm->addItemToInventory((InventoryItem *)g_allItems.findItemByID(kOrangeJuiceGlassEmpty));
}

void Caldoria::doorBombTimerExpired() {
	closeCroppedMovie();

	if (GameState.getShieldOn()) {
		_vm->getCurrentBiochip()->setItemState(kShieldCardBomb);
		setCurrentAlternate(kAltCaldoriaRoofDoorBlown);
		startExtraSequence(kCa48NorthExplosion, kExtraCompletedFlag, kFilterNoInput);
		GameState.setScoringShieldedCardBomb(true);
		GameState.setCaldoriaDoorBombed(false);
		GameState.setCaldoriaRoofDoorOpen(true);
	} else {
		playDeathExtra(kCa48NorthExplosionDeath, kDeathCardBomb);
	}
}

void Caldoria::sinclairTimerExpired() {
	_privateFlags.setFlag(kCaldoriaPrivateSinclairTimerExpiredFlag, true);
	checkSinclairShootsOS();
}

void Caldoria::checkSinclairShootsOS() {
	if (_privateFlags.getFlag(kCaldoriaPrivateSinclairTimerExpiredFlag))
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kCaldoria49, kNorth):
		case MakeRoomView(kCaldoria49, kSouth):
		case MakeRoomView(kCaldoria49, kEast):
		case MakeRoomView(kCaldoria49, kWest):
		case MakeRoomView(kCaldoria50, kSouth):
		case MakeRoomView(kCaldoria50, kEast):
		case MakeRoomView(kCaldoria50, kWest):
		case MakeRoomView(kCaldoria51, kNorth):
		case MakeRoomView(kCaldoria51, kSouth):
		case MakeRoomView(kCaldoria51, kWest):
		case MakeRoomView(kCaldoria52, kNorth):
		case MakeRoomView(kCaldoria52, kSouth):
		case MakeRoomView(kCaldoria52, kWest):
		case MakeRoomView(kCaldoria53, kNorth):
		case MakeRoomView(kCaldoria53, kSouth):
		case MakeRoomView(kCaldoria53, kWest):
		case MakeRoomView(kCaldoria54, kNorth):
		case MakeRoomView(kCaldoria54, kEast):
		case MakeRoomView(kCaldoria54, kWest):
			playSpotSoundSync(kCaldoriaSinclairShootsOSIn, kCaldoriaSinclairShootsOSOut);
			playSpotSoundSync(kCaldoriaScreamingAfterIn, kCaldoriaScreamingAfterOut);
			die(kDeathSinclairShotDelegate);
			break;
		}
}

void Caldoria::checkInterruptSinclair() {
	if (GameState.getCaldoriaSinclairShot()) {
		_navMovie.stop();
		_neighborhoodNotification.setNotificationFlags(kExtraCompletedFlag, kExtraCompletedFlag);
		g_AIArea->unlockAI();
	} else {
		// TODO
	}
}

Common::String Caldoria::getBriefingMovie() {
	Common::String movieName = Neighborhood::getBriefingMovie();

	if (movieName.empty()) {
		if (GameState.allTimeZonesFinished())
			return "Images/AI/Caldoria/XA02";

		return "Images/AI/Caldoria/XA01";
	}

	return movieName;
}

Common::String Caldoria::getEnvScanMovie() {	
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		tRoomID room = GameState.getCurrentRoom();

		if (room >= kCaldoria00 && room <= kCaldoria14) {
			// Inside apartment.
			if (GameState.getCaldoriaDoneHygiene())
				return "Images/AI/Caldoria/XAE2";

			return "Images/AI/Caldoria/XAE1";
		} else if (room >= kCaldoria15 && room <= kCaldoria48) {
			// Wandering the halls...
			return "Images/AI/Caldoria/XAE3";
		} else {
			// Must be the roof.
			return "Images/AI/Caldoria/XAEH2";
		}
	}

	return movieName;
}

uint Caldoria::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kCaldoria44, kEast):
			if (!GameState.isTakenItemID(kKeyCard) && GameState.getOpenDoorRoom() == kNoRoomID)
				numHints = 1;
			break;
		case MakeRoomView(kCaldoria48, kNorth):
			if (!GameState.getCaldoriaRoofDoorOpen()) {
				if (_croppedMovie.isRunning()) // Bomb must be looping.
					numHints = 3;
				else if (GameState.isTakenItemID(kCardBomb))
					numHints = 1;
			}
			break;
		case MakeRoomView(kCaldoria49, kEast):
		case MakeRoomView(kCaldoria54, kEast):
			numHints = 1;
			break;
		case MakeRoomView(kCaldoria49, kNorth):
			numHints = 1;
			break;
		}
	}
	
	return numHints;
}

Common::String Caldoria::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty()) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kCaldoria44, kEast):
			return "Images/AI/Caldoria/X42WH2";
		case MakeRoomView(kCaldoria48, kNorth):
			if (_croppedMovie.isRunning()) { // Bomb must be looping.
				if (hintNum == 1)
					return "Images/AI/Caldoria/X48ND1";
				else if (hintNum == 2)
					return "Images/AI/Caldoria/X48ND2";
				else if (GameState.isTakenItemID(kShieldBiochip))
					return "Images/AI/Caldoria/X48ND3";

				//	*** Doesn't work yet, need global movies.
				break;
			} 

			return "Images/AI/Globals/XGLOB1A";
		case MakeRoomView(kCaldoria49, kEast):
		case MakeRoomView(kCaldoria54, kEast):
			return "Images/AI/Caldoria/X49E";
		case MakeRoomView(kCaldoria49, kNorth):
			return "Images/AI/Caldoria/X49NB2";
		}
	}

	return movieName;
}

void Caldoria::updateCursor(const Common::Point where, const Hotspot *cursorSpot) {
	switch (cursorSpot->getObjectID()) {
	case kCa4DEnvironCloseSpotID:
		_vm->_cursor->setCurrentFrameIndex(2);
		break;
	case kCaldoriaKioskSpotID:
		_vm->_cursor->setCurrentFrameIndex(3);
		break;
	default:
		Neighborhood::updateCursor(where, cursorSpot);
		break;
	}
}

Common::String Caldoria::getNavMovieName() {
	return "Images/Caldoria/Caldoria.movie";
}

Common::String Caldoria::getSoundSpotsName() {
	return "Sounds/Caldoria/Caldoria Spots";
}

} // End of namespace Pegasus
