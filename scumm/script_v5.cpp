/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "charset.h"
#include "intern.h"
#include "sound.h"
#include "verbs.h"

#define OPCODE(x)	{ &Scumm_v5::x, #x }

void Scumm_v5::setupOpcodes() {
	static const OpcodeEntryV5 opcodes[256] = {
		/* 00 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o5_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 04 */
		OPCODE(o5_isGreaterEqual),
		OPCODE(o5_drawObject),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_setState),
		/* 08 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* 0C */
		OPCODE(o5_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_getObjectState),
		/* 10 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o5_animateActor),
		OPCODE(o5_panCameraTo),
		OPCODE(o5_actorSet),
		/* 14 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o5_and),
		/* 18 */
		OPCODE(o5_jumpRelative),
		OPCODE(o5_doSentence),
		OPCODE(o5_move),
		OPCODE(o5_multiply),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o5_putActor),
		OPCODE(o5_getAnimCounter),
		OPCODE(o5_getActorY),
		/* 24 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_setVarRange),
		OPCODE(o5_stringOps),
		/* 28 */
		OPCODE(o5_equalZero),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_delayVariable),
		/* 2C */
		OPCODE(o5_cursorCommand),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_delay),
		OPCODE(o5_ifNotState),
		/* 30 */
		OPCODE(o5_matrixOps),
		OPCODE(o5_getInventoryCount),
		OPCODE(o5_setCameraAt),
		OPCODE(o5_roomOps),
		/* 34 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* 38 */
		OPCODE(o5_lessOrEqual),
		OPCODE(o5_doSentence),
		OPCODE(o5_subtract),
		OPCODE(o5_getActorScale),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* 40 */
		OPCODE(o5_cutscene),
		OPCODE(o5_putActor),
		OPCODE(o5_chainScript),
		OPCODE(o5_getActorX),
		/* 44 */
		OPCODE(o5_isLess),
		OPCODE(o5_drawObject),
		OPCODE(o5_increment),
		OPCODE(o5_setState),
		/* 48 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* 4C */
		OPCODE(o5_soundKludge),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_ifState),
		/* 50 */
		OPCODE(o5_pickupObjectOld),
		OPCODE(o5_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_actorSet),
		/* 54 */
		OPCODE(o5_setObjectName),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o5_or),
		/* 58 */
		OPCODE(o5_overRide),
		OPCODE(o5_doSentence),
		OPCODE(o5_add),
		OPCODE(o5_divide),
		/* 5C */
		OPCODE(o5_oldRoomEffect),
		OPCODE(o5_setClass),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* 60 */
		OPCODE(o5_freezeScripts),
		OPCODE(o5_putActor),
		OPCODE(o5_stopScript),
		OPCODE(o5_getActorFacing),
		/* 64 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o5_dummy),
		/* 68 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_debug),
		/* 6C */
		OPCODE(o5_getActorWidth),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_stopObjectScript),
		OPCODE(o5_ifNotState),
		/* 70 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o5_roomOps),
		/* 74 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* 78 */
		OPCODE(o5_isGreater),				/* less? */
		OPCODE(o5_doSentence),
		OPCODE(o5_verbOps),
		OPCODE(o5_getActorWalkBox),
		/* 7C */
		OPCODE(o5_isSoundRunning),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* 80 */
		OPCODE(o5_breakHere),
		OPCODE(o5_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 84 */
		OPCODE(o5_isGreaterEqual),	/* less equal? */
		OPCODE(o5_drawObject),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_setState),
		/* 88 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* 8C */
		OPCODE(o5_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_getObjectState),
		/* 90 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o5_animateActor),
		OPCODE(o5_panCameraTo),
		OPCODE(o5_actorSet),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o5_and),
		/* 98 */
		OPCODE(o5_quitPauseRestart),
		OPCODE(o5_doSentence),
		OPCODE(o5_move),
		OPCODE(o5_multiply),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o5_putActor),
		OPCODE(o5_getAnimCounter),
		OPCODE(o5_getActorY),
		/* A4 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_setVarRange),
		OPCODE(o5_dummy),
		/* A8 */
		OPCODE(o5_notEqualZero),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_saveRestoreVerbs),
		/* AC */
		OPCODE(o5_expression),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_wait),
		OPCODE(o5_ifNotState),
		/* B0 */
		OPCODE(o5_matrixOps),
		OPCODE(o5_getInventoryCount),
		OPCODE(o5_setCameraAt),
		OPCODE(o5_roomOps),
		/* B4 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* B8 */
		OPCODE(o5_lessOrEqual),
		OPCODE(o5_doSentence),
		OPCODE(o5_subtract),
		OPCODE(o5_getActorScale),
		/* BC */
		OPCODE(o5_stopSound),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* C0 */
		OPCODE(o5_endCutscene),
		OPCODE(o5_putActor),
		OPCODE(o5_chainScript),
		OPCODE(o5_getActorX),
		/* C4 */
		OPCODE(o5_isLess),
		OPCODE(o5_drawObject),
		OPCODE(o5_decrement),
		OPCODE(o5_setState),
		/* C8 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getVerbEntrypoint),
		/* CC */
		OPCODE(o5_pseudoRoom),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_ifState),
		/* D0 */
		OPCODE(o5_pickupObjectOld),
		OPCODE(o5_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_actorSet),
		/* D4 */
		OPCODE(o5_setObjectName),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o5_or),
		/* D8 */
		OPCODE(o5_printEgo),
		OPCODE(o5_doSentence),
		OPCODE(o5_add),
		OPCODE(o5_divide),
		/* DC */
		OPCODE(o5_oldRoomEffect),
		OPCODE(o5_setClass),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_isActorInBox),
		/* E0 */
		OPCODE(o5_freezeScripts),
		OPCODE(o5_putActor),
		OPCODE(o5_stopScript),
		OPCODE(o5_getActorFacing),
		/* E4 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o5_dummy),
		/* E8 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o5_setOwnerOf),
		OPCODE(o5_startScript),
		OPCODE(o5_debug),
		/* EC */
		OPCODE(o5_getActorWidth),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_stopObjectScript),
		OPCODE(o5_ifNotState),
		/* F0 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o5_roomOps),
		/* F4 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o5_startObject),
		/* F8 */
		OPCODE(o5_isGreater),
		OPCODE(o5_doSentence),
		OPCODE(o5_verbOps),
		OPCODE(o5_getActorWalkBox),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o5_findInventory),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox)
	};

	_opcodesV5 = opcodes;
}

void Scumm_v5::executeOpcode(int i) {
	OpcodeProcV5 op = _opcodesV5[i].proc;
	(this->*op) ();
}

const char *Scumm_v5::getOpcodeDesc(int i) {
	return _opcodesV5[i].desc;
}

void Scumm_v5::o5_actorFollowCamera() {
	actorFollowCamera(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_actorFromPos() {
	int x, y;
	getResultPos();
	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);
	setResult(getActorFromPos(x, y));
}

void Scumm_v5::o5_actorSet() {
	static const byte convertTable[20] =
		{ 1, 0, 0, 2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20 };
	int act = getVarOrDirectByte(0x80);
	Actor *a;
	int i, j;

	if (act == 0)
		act = 1;

	a = derefActorSafe(act, "actorSet");

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		if (_features & GF_SMALL_HEADER)
			_opcode = (_opcode & 0xE0) | convertTable[(_opcode & 0x1F) - 1];

		if (!a)
			return;

		switch (_opcode & 0x1F) {
		case 0:										/* dummy case */
			getVarOrDirectByte(0x80);
			break;
		case 1:										/* costume */
			a->setActorCostume(getVarOrDirectByte(0x80));
			break;
		case 2:										/* walkspeed */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			a->setActorWalkSpeed(i, j);
			break;
		case 3:										/* sound */
			a->sound[0] = getVarOrDirectByte(0x80);
			break;
		case 4:										/* walkanim */
			a->walkFrame = getVarOrDirectByte(0x80);
			break;
		case 5:										/* talkanim */
			a->talkFrame1 = getVarOrDirectByte(0x80);
			a->talkFrame2 = getVarOrDirectByte(0x40);
			break;
		case 6:										/* standanim */
			a->standFrame = getVarOrDirectByte(0x80);
			break;
		case 7:										/* ignore */
			getVarOrDirectByte(0x80);
			getVarOrDirectByte(0x40);
			getVarOrDirectByte(0x20);
			break;
		case 8:										/* init */
			a->initActor(0);
			break;
		case 9:										/* elevation */
			a->elevation = getVarOrDirectWord(0x80);
			a->needRedraw = true;
			a->needBgReset = true;
			break;
		case 10:										/* defaultanims */
			a->initFrame = 1;
			a->walkFrame = 2;
			a->standFrame = 3;
			a->talkFrame1 = 4;
			a->talkFrame2 = 5;
			break;
		case 11:										/* palette */
			i = getVarOrDirectByte(0x80);
			j = getVarOrDirectByte(0x40);
			checkRange(31, 0, i, "Illegal palet slot %d");
			a->palette[i] = j;
			a->needRedraw = true;
			break;
		case 12:										/* talk color */
			a->talkColor = getVarOrDirectByte(0x80);
			break;
		case 13:										/* name */
			loadPtrToResource(rtActorName, a->number, NULL);
			break;
		case 14:										/* initanim */
			a->initFrame = getVarOrDirectByte(0x80);
			break;
		case 15:										/* unk */
			error("o5_actorset:unk not implemented");
#if 0
			int args[16] =
				{
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				};
			getWordVararg(args);
			for (i = 0; i < 16; i++)
				if (args[i] != 0xFF)
					a->palette[i] = args[i];
#endif
			break;
		case 16:										/* width */
			a->width = getVarOrDirectByte(0x80);
			break;
		case 17:										/* scale */
			if (_gameId == GID_MONKEY_VGA) {
				a->scalex = a->scaley = getVarOrDirectByte(0x80);
			} else {
				a->scalex = getVarOrDirectByte(0x80);
				a->scaley = getVarOrDirectByte(0x40);
			}

			if (a->scalex > 255 || a->scaley > 255)
				error("Setting an bad actor scale!");
			a->needRedraw = true;
			a->needBgReset = true;
			break;
		case 18:										/* neverzclip */
			a->forceClip = 0;
			break;
		case 19:										/* setzclip */
			a->forceClip = getVarOrDirectByte(0x80);
			break;
		case 20:										/* ignoreboxes */
		case 21:										/* followboxes */
			a->ignoreBoxes = !(_opcode & 1);
			a->forceClip = 0;
			if (a->isInCurrentRoom())
				a->putActor(a->x, a->y, a->room);
			break;

		case 22:										/* animspeed */
			a->animSpeed = getVarOrDirectByte(0x80);
			a->animProgress = 0;
			break;
		case 23:										/* unk2 */
			a->shadow_mode = getVarOrDirectByte(0x80);	/* shadow mode */
			break;
		default:
			warning("o5_actorSet: default case");
		}
	}
}

void Scumm_v5::o5_setClass() {
	int obj = getVarOrDirectWord(0x80);
	int newClass;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		newClass = getVarOrDirectWord(0x80);
		if (newClass == 0) {
			_classData[obj] = 0;
			if ((_features & GF_SMALL_HEADER) && obj <= NUM_ACTORS) {
				Actor *a;
				a = derefActorSafe(obj, "setClass");
				a->ignoreBoxes = 0;
				a->forceClip = 0;
			}
			continue;
		}

		putClass(obj, newClass, (newClass & 0x80) ? true : false);
	}
}

void Scumm_v5::o5_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) + a);
}

void Scumm_v5::o5_and() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) & a);
}

void Scumm_v5::o5_animateActor() {
	int act = getVarOrDirectByte(0x80);
	int anim = getVarOrDirectByte(0x40);

	Actor *a = derefActorSafe(act, "o5_animateActor");
	if (!a)
		return;

	a->animateActor(anim);
}

void Scumm_v5::o5_badOpcode() {
	error("Scumm opcode %d illegal", _opcode);
}

void Scumm_v5::o5_breakHere() {
	updateScriptPtr();
	_currentScript = 0xFF;
}

void Scumm_v5::o5_chainScript() {
	int vars[16];
	int data;
	int cur;

	data = getVarOrDirectByte(0x80);

	getWordVararg(vars);

	cur = _currentScript;

	vm.slot[cur].number = 0;
	vm.slot[cur].status = 0;
	_currentScript = 0xFF;

	runScript(data, vm.slot[cur].unk1, vm.slot[cur].unk2, vars);
}

void Scumm_v5::o5_cursorCommand() {
	int i, j, k;
	int table[16];
	switch ((_opcode = fetchScriptByte()) & 0x1F) {
	case 1:											/* cursor show */
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 2:											/* cursor hide */
		_cursor.state = 0;
		verbMouseOver(0);
		break;
	case 3:											/* userput on */
		_userPut = 1;
		break;
	case 4:											/* userput off */
		_userPut = 0;
		break;
	case 5:											/* cursor soft on */
		_cursor.state++;
		verbMouseOver(0);
		break;
	case 6:											/* cursor soft off */
		_cursor.state--;
		verbMouseOver(0);
		break;
	case 7:											/* userput soft on */
		_userPut++;
		break;
	case 8:											/* userput soft off */
		_userPut--;
		break;
	case 10:											/* set cursor img */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		if (_gameId != GID_LOOM256)
			setCursorImg(i, j, 1);
		break;
	case 11:											/* set cursor hotspot */
		i = getVarOrDirectByte(0x80);
		j = getVarOrDirectByte(0x40);
		k = getVarOrDirectByte(0x20);
		setCursorHotspot2(j, k);
		break;
	case 12:											/* init cursor */
		setCursor(getVarOrDirectByte(0x80));
		break;
	case 13:											/* init charset */
		initCharset(getVarOrDirectByte(0x80));
		break;
	case 14:											/* unk */
		getWordVararg(table);
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1].t_charset][i] = (unsigned char)table[i];
		break;
	}

	_vars[VAR_CURSORSTATE] = _cursor.state;
	_vars[VAR_USERPUT] = _userPut;
}

void Scumm_v5::o5_cutscene() {
	int args[16];
	getWordVararg(args);
	cutscene(args);
}

void Scumm_v5::o5_endCutscene() {
	endCutscene();
}

void Scumm_v5::o5_debug() {
	getVarOrDirectWord(0x80);
}

void Scumm_v5::o5_decrement() {
	getResultPos();
	setResult(readVar(_resultVarNumber) - 1);
}

void Scumm_v5::o5_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm_v5::o5_delayVariable() {
	vm.slot[_currentScript].delay = readVar(fetchScriptWord());
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm_v5::o5_divide() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	if (a == 0) {
		error("Divide by zero");
		setResult(0);
	} else
		setResult(readVar(_resultVarNumber) / a);
}

void Scumm_v5::o5_doSentence() {
	int a, b;
	SentenceTab *st;

	a = getVarOrDirectByte(0x80);
	if (a == 0xFE) {
		_sentenceNum = 0;
		stopScriptNr(_vars[VAR_SENTENCE_SCRIPT]);
		clearClickedStatus();
		return;
	}

	st = &_sentence[_sentenceNum++];

	st->unk5 = a;
	st->unk4 = getVarOrDirectWord(0x40);
	b = st->unk3 = getVarOrDirectWord(0x20);
	if (b == 0) {
		st->unk2 = 0;
	} else {
		st->unk2 = 1;
	}
	st->freezeCount = 0;
}

void Scumm_v5::o5_drawBox() {
	int x, y, x2, y2, color;

	x = getVarOrDirectWord(0x80);
	y = getVarOrDirectWord(0x40);

	_opcode = fetchScriptByte();
	x2 = getVarOrDirectWord(0x80);
	y2 = getVarOrDirectWord(0x40);
	color = getVarOrDirectByte(0x20);

	drawBox(x, y, x2, y2, color);
}

void Scumm_v5::o5_drawObject() {
	int state, obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	state = 1;
	xpos = ypos = 255;
	obj = getVarOrDirectWord(0x80);

	if (_features & GF_SMALL_HEADER) {
		xpos = getVarOrDirectWord(0x40);
		ypos = getVarOrDirectWord(0x20);
	} else {
		switch ((_opcode = fetchScriptByte()) & 0x1F) {
		case 1:										/* draw at */
			xpos = getVarOrDirectWord(0x80);
			ypos = getVarOrDirectWord(0x40);
			break;
		case 2:										/* set state */
			state = getVarOrDirectWord(0x80);
			break;
		case 0x1F:									/* neither */
			break;
		default:
			error("o5_drawObject: default case");
		}
	}

	idx = getObjectIndex(obj);
	if (idx == -1)
		return;

	od = &_objs[idx];
	if (xpos != 0xFF) {
		od->walk_x += (xpos << 3) - od->x_pos;
		od->x_pos = xpos << 3;
		od->walk_y += (ypos << 3) - od->y_pos;
		od->y_pos = ypos << 3;
	}
	addObjectToDrawQue(idx);

	x = od->x_pos;
	y = od->y_pos;
	w = od->width;
	h = od->height;

	i = _numLocalObjects;
	do {
		if (_objs[i].obj_nr && _objs[i].x_pos == x && _objs[i].y_pos == y && _objs[i].width == w && _objs[i].height == h)
			putState(_objs[i].obj_nr, 0);
	} while (--i);

	putState(obj, state);
}

void Scumm_v5::o5_dummy() {
	/* nothing */
}

void Scumm_v5::o5_expression() {
	int dst, i;

	_scummStackPos = 0;
	getResultPos();
	dst = _resultVarNumber;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:										/* varordirect */
			push(getVarOrDirectWord(0x80));
			break;
		case 2:										/* add */
			i = pop();
			push(i + pop());
			break;
		case 3:										/* sub */
			i = pop();
			push(pop() - i);
			break;
		case 4:										/* mul */
			i = pop();
			push(i * pop());
			break;
		case 5:										/* div */
			i = pop();
			if (i == 0)
				error("Divide by zero");
			push(pop() / i);
			break;
		case 6:										/* normal opcode */
			_opcode = fetchScriptByte();
			executeOpcode(_opcode);
			push(_vars[0]);
			break;
		}
	}

	_resultVarNumber = dst;
	setResult(pop());
}

void Scumm_v5::o5_faceActor() {
	int act, obj;
	Actor *a;
	act = getVarOrDirectByte(0x80);
	obj = getVarOrDirectWord(0x40);

	a = derefActorSafe(act, "o5_faceActor");
	assert(a);

	a->factToObject(obj);
}

void Scumm_v5::o5_findInventory() {
	int t;
	getResultPos();
	t = getVarOrDirectByte(0x80);
	setResult(findInventory(t, getVarOrDirectByte(0x40)));
}

void Scumm_v5::o5_findObject() {
	int t;
	getResultPos();
	t = getVarOrDirectWord(0x80);
	setResult(findObject(t, getVarOrDirectWord(0x40)));
}

void Scumm_v5::o5_freezeScripts() {
	int scr = getVarOrDirectByte(0x80);

	if (scr != 0)
		freezeScripts(scr);
	else
		unfreezeScripts();
}

void Scumm_v5::o5_getActorCostume() {
	int act;
	Actor *a;
	getResultPos();
	act = getVarOrDirectByte(0x80);

	a = derefActorSafe(act, "o5_getActorCostume");
	if (!a) {
		warning("Invalid actor %d in o5_getActorCostume", act);
		return;
	}

	setResult(a->costume);
}

void Scumm_v5::o5_getActorElevation() {
	int act;
	Actor *a;
	getResultPos();
	act = getVarOrDirectByte(0x80);

	a = derefActorSafe(act, "o5_getActorElevation");
	if (!a) {
		warning("Invalid actor %d in o5_getActorElevation", act);
		return;
	}

	setResult(a->elevation);
}

void Scumm_v5::o5_getActorFacing() {
	int act;
	Actor *a;
	getResultPos();
	act = getVarOrDirectByte(0x80);

	a = derefActorSafe(act, "o5_getActorFacing");
	if (!a) {
		warning("Invalid actor %d in o5_getActorFacing", act);
		return;
	}

	setResult(newDirToOldDir (a->facing));
}

void Scumm_v5::o5_getActorMoving() {
	int act;
	Actor *a;
	getResultPos();
	act = getVarOrDirectByte(0x80);

	a = derefActorSafe(act, "o5_getActorMoving");
	if (!a) {
		warning("Invalid actor %d in o5_getActorMoving", act);
		return;
	}

	setResult(a->moving);
}

void Scumm_v5::o5_getActorRoom() {
	int act;
	Actor *a;
	getResultPos();
	act = getVarOrDirectByte(0x80);

	a = derefActorSafe(act, "o5_getActorRoom");
	if (!a) {
		warning("Invalid actor %d in o5_getActorRoom", act);
		return;
	}

	setResult(a->room);
}

void Scumm_v5::o5_getActorScale() {
	// INDY3 uses this opcode as a wait_for_actor();
	if (_gameId == GID_INDY3_256) {
		byte *oldaddr = _scriptPointer - 1;
		if (derefActorSafe(getVarOrDirectByte(0x80), "o5_wait")->moving) {
			_scriptPointer = oldaddr;
			o5_breakHere();
		}
		return;
	}

	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorScale")->scalex);
}

void Scumm_v5::o5_getActorWalkBox() {
	Actor *a;
	getResultPos();
	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorWalkbox");
	if (a)												// FIXME - bug 572977 workaround
		setResult(a->walkbox);
	else
		setResult(0);
}

void Scumm_v5::o5_getActorWidth() {
	getResultPos();
	setResult(derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorWidth")->width);
}

void Scumm_v5::o5_getActorX() {
	int a;
	getResultPos();

	if (_gameId == GID_INDY3_256)
		a = getVarOrDirectByte(0x80);
	else
		a = getVarOrDirectWord(0x80);

	setResult(getObjX(a));
}

void Scumm_v5::o5_getActorY() {
	int a;
	getResultPos();

	if (_gameId == GID_INDY3_256) {
		a = getVarOrDirectByte(0x80);

		// Indy3 hack to cheat the 'Leap of Faith' grail test
		// This test is so damn annoying, I'm leaving this in.
		if (_roomResource == 85) {
			setResult(94);
			return;
		}
		setResult(getObjY(a) - 1);	// FIXME: Is this right, or can it be less specific?
						// It's here to fix bug 636433 in specific, the actors
						// are one pixel off what the script waits for.
		return;
	} else
		a = getVarOrDirectWord(0x80);

	setResult(getObjY(a));
}

void Scumm_v5::o5_getAnimCounter() {
	Actor *a;
	getResultPos();

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_getActorAnimCounter");

	if (a)												// FIXME
		setResult(a->cost.animCounter1);
	else
		setResult(0);
}

void Scumm_v5::o5_getClosestObjActor() {
	int obj;
	int act;
	int closest_obj = 0xFF, closest_dist = 0xFF;
	int dist;

	getResultPos();

	act = getVarOrDirectWord(0x80);
	obj = _vars[VAR_V5_OBJECT_HI];

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj >= _vars[VAR_V5_OBJECT_LO]);

	setResult(closest_dist);
}

void Scumm_v5::o5_getDist() {
	int o1, o2;
	int r;
	getResultPos();
	o1 = getVarOrDirectWord(0x80);
	o2 = getVarOrDirectWord(0x40);
	r = getObjActToObjActDist(o1, o2);

	/* FIXME: MI2 race workaround, see bug 597022 */
	if (_gameId == GID_MONKEY2 && vm.slot[_currentScript].number == 40 && r < 60) 
		r = 60; 

	setResult(r);
}

void Scumm_v5::o5_getInventoryCount() {
	getResultPos();
	setResult(getInventoryCount(getVarOrDirectByte(0x80)));
}

void Scumm_v5::o5_getObjectOwner() {
	getResultPos();
	setResult(getOwner(getVarOrDirectWord(0x80)));
}

void Scumm_v5::o5_getObjectState() {
	if (_features & GF_SMALL_HEADER) {
		o5_ifState();
	} else {
		getResultPos();
		setResult(getState(getVarOrDirectWord(0x80)));
	}
}

void Scumm_v5::o5_ifState() {
	int a = getVarOrDirectWord(0x80);
	int b = getVarOrDirectByte(0x40);

	if (getState(a) != b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v5::o5_ifNotState() {
	int a = getVarOrDirectWord(0x80);
	int b = getVarOrDirectByte(0x40);

	if (getState(a) == b)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v5::o5_getRandomNr() {
	getResultPos();
	setResult(_rnd.getRandomNumber(getVarOrDirectByte(0x80)));
}

void Scumm_v5::o5_isScriptRunning() {
	getResultPos();
	setResult(isScriptRunning(getVarOrDirectByte(0x80)));
}

void Scumm_v5::o5_getVerbEntrypoint() {
	int a, b;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	b = getVarOrDirectWord(0x40);

	setResult(getVerbEntrypoint(a, b));
}

void Scumm_v5::o5_ifClassOfIs() {
	int act, cls, b = 0;
	bool cond = true;

	act = getVarOrDirectWord(0x80);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		cls = getVarOrDirectWord(0x80);

		if (!cls) // FIXME: Ender can't remember why this is here,
			b=false;  // but it fixes an oddball zak256 crash
		else
			b = getClass(act, cls);

		if (cls & 0x80 && !b || !(cls & 0x80) && b)
			cond = false;
	}
	if (cond)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_increment() {
	getResultPos();
	setResult(readVar(_resultVarNumber) + 1);
}

void Scumm_v5::o5_isActorInBox() {
	int box;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_isActorInBox");
	box = getVarOrDirectByte(0x40);

	if (!checkXYInBoxBounds(box, a->x, a->y))
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v5::o5_isEqual() {
	int16 a, b;
	int var;

	var = fetchScriptWord();
	a = readVar(var);
	b = getVarOrDirectWord(0x80);

	// HACK: See bug report #602348. The sound effects for Largo's screams
	// are only played on type 5 soundcards. However, there is at least one
	// other sound effect (the bartender spitting) which is only played on
	// type 3 soundcards.

	if (_gameId == GID_MONKEY2 && var == VAR_SOUNDCARD && b == 5)
		b = a;

	if (b == a)
		ignoreScriptWord();
	else
		o5_jumpRelative();

}

void Scumm_v5::o5_isGreater() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isGreaterEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isLess() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);

	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_lessOrEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isNotEqual() {
	int16 a = readVar(fetchScriptWord());
	int16 b = getVarOrDirectWord(0x80);
	if (b != a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_notEqualZero() {
	int a = readVar(fetchScriptWord());
	if (a != 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_equalZero() {
	int a = readVar(fetchScriptWord());
	if (a == 0)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v5::o5_isSoundRunning() {
	int snd;
	getResultPos();
	snd = getVarOrDirectByte(0x80);
	if (snd)
		snd = _sound->isSoundRunning(snd);
	setResult(snd);
}

void Scumm_v5::o5_jumpRelative() {
	_scriptPointer += (int16)fetchScriptWord();
}

void Scumm_v5::o5_lights() {
	int a, b, c;

	a = getVarOrDirectByte(0x80);
	b = fetchScriptByte();
	c = fetchScriptByte();

	if (c == 0)
		_vars[VAR_CURRENT_LIGHTS] = a;
	else if (c == 1) {
		_flashlightXStrips = a;
		_flashlightYStrips = b;
	}
	_fullRedraw = 1;
}

void Scumm_v5::o5_loadRoom() {
	int room;

	room = getVarOrDirectByte(0x80);
	
	// For small header games, we only call startScene if the room
	// actually changed. This avoid unwanted (wrong) fades in Zak256
	// and others. OTOH, it seems to cause a problem in newer games.
	if (!(_features & GF_SMALL_HEADER) || room != _currentRoom)
		startScene(room, 0, 0);
	_fullRedraw = 1;
}

void Scumm_v5::o5_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);

	a = derefActorSafe(_vars[VAR_EGO], "o5_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	x = (int16)fetchScriptWord();
	y = (int16)fetchScriptWord();

	_vars[VAR_WALKTO_OBJ] = obj;
	startScene(a->room, a, obj);
	_vars[VAR_WALKTO_OBJ] = 0;

	camera._dest.x = camera._cur.x = a->x;
	setCameraAt(a->x, a->y);
	setCameraFollows(a);

	_fullRedraw = 1;

	if (x != -1) {
		a->startWalkActor(x, y, -1);
	}
}

void Scumm_v5::o5_matrixOps() {
	int a, b;

	if (_features & GF_OLD256) {
		a = getVarOrDirectByte(0x80);
		b = fetchScriptByte();
		setBoxFlags(a, b);
		return;
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxFlags(a, b);
		break;
	case 2:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxScale(a, b);
		break;
	case 3:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		setBoxScale(a, (b - 1) | 0x8000);
		break;
	case 4:
		createBoxMatrix();
		break;
	}
}

void Scumm_v5::o5_move() {
	getResultPos();
	setResult(getVarOrDirectWord(0x80));
}

void Scumm_v5::o5_multiply() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) * a);
}

void Scumm_v5::o5_or() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) | a);
}

void Scumm_v5::o5_overRide() {
	if (fetchScriptByte() != 0)
		beginOverride();
	else
		endOverride();
}

void Scumm_v5::o5_panCameraTo() {
	panCameraTo(getVarOrDirectWord(0x80), 0);
}

void Scumm_v5::o5_pickupObject() {
	int obj, room;
	if (_features & GF_OLD256) {
		o5_drawObject();
		return;
	}

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);
	if (room == 0)
		room = _roomResource;
	addObjectToInventory(obj, room);
	putOwner(obj, _vars[VAR_EGO]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
	runHook(1);
}

void Scumm_v5::o5_print() {
	_actorToPrintStrFor = getVarOrDirectByte(0x80);
	decodeParseString();
}

void Scumm_v5::o5_printEgo() {
	_actorToPrintStrFor = (unsigned char)_vars[VAR_EGO];
	decodeParseString();
}

void Scumm_v5::o5_pseudoRoom() {
	int i = fetchScriptByte(), j;
	while ((j = fetchScriptByte()) != 0) {
		if (j >= 0x80) {
			_resourceMapper[j & 0x7F] = i;
		}
	}
}

void Scumm_v5::o5_putActor() {
	int x, y;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_putActor");
	if (!a)
		return;
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);

	a->putActor(x, y, a->room);
}

void Scumm_v5::o5_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_putActorAtObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;
	}
	a->putActor(x, y, a->room);
}

void Scumm_v5::o5_putActorInRoom() {
	int room;
	Actor *a;

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_putActorInRoom");
	room = getVarOrDirectByte(0x40);

	if (a == NULL) return; // FIXME - yet another null dref hack, see bug 639201
	if (a->visible && _currentRoom != room && _vars[VAR_TALK_ACTOR] == a->number) {
		clearMsgQueue();
	}
	a->room = room;
	if (!room)
		a->putActor(0, 0, 0);
}

void Scumm_v5::o5_quitPauseRestart() {
	switch (fetchScriptByte()) {
	case 1:
		pauseGame(false);
		break;
	case 3:
		shutDown(0);
		break;
	}
}

void Scumm_v5::o5_resourceRoutines() {
	const ResTypes resType[4] = { rtScript, rtSound, rtCostume, rtRoom };
	int resid = 0;
	int foo, bar;

	_opcode = fetchScriptByte();
	if (_opcode != 17)
		resid = getVarOrDirectByte(0x80);
	if (_gameId == GID_ZAK256)
		_opcode &= 0x3F;
	else
		_opcode &= 0x1F;

	switch (_opcode) {
	case 1:											// load script
	case 2:											// load sound
	case 3:											// load costume
		ensureResourceLoaded(resType[_opcode-1], resid);
		break;
	case 4:											// load room 
		if (_gameId == GID_ZAK256) {
			ensureResourceLoaded(rtRoom, resid);
			if (resid > 0x7F)
				resid = _resourceMapper[resid & 0x7F];

			if (_currentRoom != resid) {
				res.flags[rtRoom][resid] |= 1;
			}
		} else
			ensureResourceLoaded(rtRoom, resid);
		break;

	case 5:											// nuke script
	case 6:											// nuke sound
	case 7:											// nuke costume
	case 8:											// nuke room
		if (_gameId == GID_ZAK256)
			warning("o5_resourceRoutines %d should not occur in Zak256", _opcode);
		else
			setResourceCounter(resType[_opcode-5], resid, 0x7F);
		break;

	case 9:											// lock script
		if (resid >= _numGlobalScripts)
			break;
		lock(rtScript, resid);
		break;
	case 10:											// lock sound
		lock(rtSound, resid);
		break;
	case 11:											// lock costume
		lock(rtCostume, resid);
		break;
	case 12:											// lock room
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		lock(rtRoom, resid);
		break;

	case 13:											// unlock script
		if (resid >= _numGlobalScripts)
			break;
		unlock(rtScript, resid);
		break;
	case 14:											// unlock sound
		unlock(rtSound, resid);
		break;
	case 15:											// unlock costume
		unlock(rtCostume, resid);
		break;
	case 16:											// unlock room
		if (resid > 0x7F)
			resid = _resourceMapper[resid & 0x7F];
		unlock(rtRoom, resid);
		break;

	case 17:											// clear heap
		heapClear(0);
		unkHeapProc2(0, 0);
		break;
	case 18:											// load charset
		loadCharset(resid);
		break;
	case 19:											// nuke charset
		nukeCharset(resid);
		break;
	case 20:											// load fl object
		loadFlObject(getVarOrDirectWord(0x40), resid);
		break;

	case 0x1F + 1:
		// TODO
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode,  vm.slot[_currentScript].number);
		break;
	case 0x20 + 1:
		// TODO
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode,  vm.slot[_currentScript].number);
		break;
	case 0x22 + 1:
		// TODO
		foo = getVarOrDirectByte(0x40);
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode,  vm.slot[_currentScript].number);
		break;
	case 0x23 + 1:
		// TODO
		foo = getVarOrDirectByte(0x40);
		bar = fetchScriptByte();
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode,  vm.slot[_currentScript].number);
		break;
	case 0x24 + 1:
		// TODO
		foo = getVarOrDirectByte(0x40);
		warning("o5_resourceRoutines %d not yet handled (script %d)", _opcode,  vm.slot[_currentScript].number);
		break;

	default:
		warning("Unknown o5_resourceRoutines: %d", _opcode);
		break;
	}
}

void Scumm_v5::o5_roomOps() {
	int a = 0, b = 0, c, d, e;

	if (_features & GF_OLD256) {
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
	}

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* room scroll */
		if (!(_features & GF_OLD256)) {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
		}
		if (a < (_realWidth / 2))
			a = (_realWidth / 2);
		if (b < (_realWidth / 2))
			b = (_realWidth / 2);
		if (a > _scrWidth - (_realWidth / 2))
			a = _scrWidth - (_realWidth / 2);
		if (b > _scrWidth - (_realWidth / 2))
			b = _scrWidth - (_realWidth / 2);
		_vars[VAR_CAMERA_MIN_X] = a;
		_vars[VAR_CAMERA_MAX_X] = b;
		break;
	case 2:											/* room color */
		if (_features & GF_SMALL_HEADER) {
			if (!(_features & GF_OLD256)) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			checkRange(256, 0, a, "o5_roomOps: 2: Illegal room color slot (%d)");
			// FIXME - fingolfin thinks our whole _shadowPalette usage is weird.
			// It seems very suspicious that subopcode 2 is identical to subopcode 4
			// for GF_SMALL_HEADER games. Needs investigation.
//			printf("copyPalColor(%d, %d)\n", a, b);
//			copyPalColor(a, b);
			_shadowPalette[b] = a;
			setDirtyColors(b, b);
		} else {
			error("room-color is no longer a valid command");
		}
		break;

	case 3:											/* set screen */
		if (!(_features & GF_OLD256)) {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
		}
		initScreens(0, a, _realWidth, b);
		break;
	case 4:											/* set palette color */
		if (_features & GF_SMALL_HEADER) {
			if (!(_features & GF_OLD256)) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			checkRange(256, 0, a, "o5_roomOps: 2: Illegal room color slot (%d)");
			_shadowPalette[b] = a;
			setDirtyColors(b, b);
		} else {
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectWord(0x40);
			c = getVarOrDirectWord(0x20);
			_opcode = fetchScriptByte();
			d = getVarOrDirectByte(0x80);
			setPalColor(d, a, b, c);	/* index, r, g, b */
		}
		break;
	case 5:											/* shake on */
		setShake(1);
		break;
	case 6:											/* shake off */
		setShake(0);
		break;
	case 7:											/* room scale for old games */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		c = getVarOrDirectByte(0x80);
		d = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		e = getVarOrDirectByte(0x40);
		setScaleItem(e - 1, b, a, d, c);
		break;
	case 8:											/* room scale? */
		if (_features & GF_SMALL_HEADER) {
			if (!(_features & GF_OLD256)) {
				a = getVarOrDirectWord(0x80);
				b = getVarOrDirectWord(0x40);
			}
			c = getVarOrDirectWord(0x20);
		} else {
			a = getVarOrDirectByte(0x80);
			b = getVarOrDirectByte(0x40);
			c = getVarOrDirectByte(0x20);
		}
		darkenPalette(a, a, a, b, c);
		break;
	case 9:											/* ? */
		_saveLoadFlag = getVarOrDirectByte(0x80);
		_saveLoadSlot = getVarOrDirectByte(0x40);
		_saveLoadSlot = 99;					/* use this slot */
		_saveLoadCompatible = true;
		break;
	case 10:											/* ? */
		a = getVarOrDirectWord(0x80);
		if (a) {
			_switchRoomEffect = (byte)a;
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
		break;
	case 11:											/* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		darkenPalette(a, b, c, d, e);
		break;
	case 12:											/* ? */
		a = getVarOrDirectWord(0x80);
		b = getVarOrDirectWord(0x40);
		c = getVarOrDirectWord(0x20);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		e = getVarOrDirectByte(0x40);
		setupShadowPalette(a, b, c, d, e);
		break;

	case 13:{										/* save-string */
			// TODO - use class File instead of fopen/fwrite/fclose
			char buf[256], *s;
			FILE *out;

			a = getVarOrDirectByte(0x80);

			// FIXME - check for buffer overflow
			strcpy(buf, getSavePath());
			s = buf + strlen(buf);
			while ((*s++ = fetchScriptByte()));

			// Use buf as filename
			out = fopen(buf, "wb");
			if (out) {
				byte *ptr;
				ptr = getResourceAddress(rtString, a);
				fwrite(ptr, resStrLen(ptr) + 1, 1, out);
				fclose(out);
			}
			break;
		}
	case 14:{										/* load-string */
			// TODO - use class File instead of fopen/fread/fclose
			char buf[256], *s;
			FILE *in;

			a = getVarOrDirectByte(0x80);

			// FIXME - check for buffer overflow
			strcpy(buf, getSavePath());
			s = buf + strlen(buf);
			while ((*s++ = fetchScriptByte()));

			// Use buf as filename
			in = fopen(buf, "rb");
			if (in) {
				byte *ptr;
				int len;
				fseek(in, 0, SEEK_END);
				len = ftell(in);				// Determine file size
				ptr = (byte *)calloc(len + 1, 1);	// Create a zero terminated buffer
				fseek(in, 0, SEEK_SET);
				fread(ptr, len, 1, in);	// Read in the data
				fclose(in);
				loadPtrToResource(rtString, a, ptr);
				free(ptr);
			}
			break;
		}
	case 15:											/* palmanip */
		a = getVarOrDirectByte(0x80);
		_opcode = fetchScriptByte();
		b = getVarOrDirectByte(0x80);
		c = getVarOrDirectByte(0x40);
		_opcode = fetchScriptByte();
		d = getVarOrDirectByte(0x80);
		palManipulateInit(b, c, a, d);
		break;

	case 16:
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		if (a < 1)
			a = 1;										/* FIXME: ZAK256 */
		checkRange(16, 1, a, "o5_roomOps: 16: color cycle out of range (%d)");
		_colorCycle[a - 1].delay = (b != 0) ? 0x4000 / (b * 0x4C) : 0;
		break;
	}
}

void Scumm_v5::o5_saveRestoreVerbs() {
	int a, b, c, slot, slot2;

	_opcode = fetchScriptByte();

	a = getVarOrDirectByte(0x80);
	b = getVarOrDirectByte(0x40);
	c = getVarOrDirectByte(0x20);

	switch (_opcode) {
	case 1:											/* hide verbs */
		while (a <= b) {
			slot = getVerbSlot(a, 0);
			if (slot && _verbs[slot].saveid == 0) {
				_verbs[slot].saveid = c;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 2:											/* show verbs */
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot) {
				slot2 = getVerbSlot(a, 0);
				if (slot2)
					killVerb(slot2);
				slot = getVerbSlot(a, c);
				_verbs[slot].saveid = 0;
				drawVerb(slot, 0);
				verbMouseOver(0);
			}
			a++;
		}
		break;
	case 3:											/* kill verbs */
		while (a <= b) {
			slot = getVerbSlot(a, c);
			if (slot)
				killVerb(slot);
			a++;
		}
		break;
	default:
		error("o5_saveRestoreVerbs: invalid opcode");
	}
}

void Scumm_v5::o5_setCameraAt() {
	setCameraAtEx(getVarOrDirectWord(0x80));
}

void Scumm_v5::o5_setObjectName() {
	int obj = getVarOrDirectWord(0x80);
	int size;
	int a;
	int i = 0;
	byte *name = NULL;
	unsigned char work[256];
	
	// Read in new name
	while ((a = fetchScriptByte()) != 0) {
		work[i++] = a;
		if (a == 0xFF) {
			work[i++] = fetchScriptByte();
			work[i++] = fetchScriptByte();
			work[i++] = fetchScriptByte();
		}
	}
	work[i] = 0;

	if (obj < NUM_ACTORS)
		error("Can't set actor %d name with new-name-of", obj);

	if (!getOBCDFromObject(obj)) {
		// FIXME: Bug 587553. This is an odd one and looks more like
		// an actual bug in the original script. Usually we would error
		warning("Can't find OBCD to rename object %d to %s", obj, work);
		return;
	}

	name = getObjOrActorName(obj);

	if (_features & GF_SMALL_HEADER) {
		// FIXME this is hack to make MonkeyVGA work. needed at least for the german
		// version but possibly for others as well. There is no apparent other
		// way to determine the available space that works in all cases...
		byte *objptr;
		byte offset = 0;

		objptr = getOBCDFromObject(obj);
		offset = READ_LE_UINT16(objptr + 18);
		size = READ_LE_UINT16(objptr) - offset;
	} else {
		size = getResourceDataSize(name);
	}

	if (i >= size) {
		warning("New name of object %d too long (old *%s* new *%s*)", obj, name, work);
		i = size - 1;
	}

	memcpy(name, work, i+1);
	runHook(0);
}

void Scumm_v5::o5_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(0x80);
	owner = getVarOrDirectByte(0x40);

	setOwnerOf(obj, owner);
}

void Scumm_v5::o5_setState() {
	int obj, state;
	obj = getVarOrDirectWord(0x80);
	state = getVarOrDirectByte(0x40);
	putState(obj, state);
	removeObjectFromRoom(obj);
	if (_BgNeedsRedraw)
		clearDrawObjectQueue();
}

void Scumm_v5::o5_setVarRange() {
	int a, b;

	getResultPos();
	a = fetchScriptByte();
	do {
		if (_opcode & 0x80)
			b = fetchScriptWord();
		else
			b = fetchScriptByte();

		setResult(b);
		_resultVarNumber++;
	} while (--a);
}

void Scumm_v5::o5_soundKludge() {
	int items[15];
	int i;

	if (_features & GF_SMALL_HEADER) {	// Is WaitForSentence in SCUMM V3
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
				return;
		} else if (!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
			return;

		_scriptPointer--;
		o5_breakHere();
		return;
	}

	for (i = 0; i < 15; i++)
		items[i] = 0;

	int num = getWordVararg(items);

	_sound->soundKludge(items, num);
}

void Scumm_v5::o5_startMusic() {
	_sound->addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_startObject() {
	int obj, script;
	int data[16];

	obj = getVarOrDirectWord(0x80);
	script = getVarOrDirectByte(0x40);

	getWordVararg(data);
	runVerbCode(obj, script, 0, 0, data);
}

void Scumm_v5::o5_startScript() {
	int op, script;
	int data[16];
	int a, b;

	op = _opcode;
	script = getVarOrDirectByte(0x80);

	getWordVararg(data);

	a = b = 0;
	if (op & 0x40)
		b = 1;
	if (op & 0x20)
		a = 1;

	runScript(script, a, b, data);
}

void Scumm_v5::o5_startSound() {
	_vars[VAR_MUSIC_FLAG] = 0;
	_sound->addSoundToQueue(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_stopMusic() {
	_sound->stopAllSounds();
}

void Scumm_v5::o5_stopObjectCode() {
	stopObjectCode();
}

void Scumm_v5::o5_stopObjectScript() {
	stopObjectScript(getVarOrDirectWord(0x80));
}

void Scumm_v5::o5_stopScript() {
	int script;

	script = getVarOrDirectByte(0x80);
	if (!script)
		stopObjectCode();
	else
		stopScriptNr(script);
}

void Scumm_v5::o5_stopSound() {
	_sound->stopSound(getVarOrDirectByte(0x80));
}

void Scumm_v5::o5_stringOps() {
	int a, b, c, i;
	byte *ptr;

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* loadstring */
		loadPtrToResource(rtString, getVarOrDirectByte(0x80), NULL);
		break;
	case 2:											/* copystring */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		nukeResource(rtString, a);
		ptr = getResourceAddress(rtString, b);
		if (ptr)
			loadPtrToResource(rtString, a, ptr);
		break;
	case 3:											/* set string char */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		c = getVarOrDirectByte(0x20);
		ptr = getResourceAddress(rtString, a);
		if (_gameId != GID_LOOM256) {	/* FIXME - LOOM256 */
			if (ptr == NULL)
				error("String %d does not exist", a);
			ptr[b] = c;
		}

		break;

	case 4:											/* get string char */
		getResultPos();
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		ptr = getResourceAddress(rtString, a);
		if (ptr == NULL)
			error("String %d does not exist", a);
		setResult(ptr[b]);
		break;

	case 5:											/* create empty string */
		a = getVarOrDirectByte(0x80);
		b = getVarOrDirectByte(0x40);
		nukeResource(rtString, a);
		if (b) {
			ptr = createResource(rtString, a, b);
			if (ptr) {
				for (i = 0; i < b; i++)
					ptr[i] = 0;
			}
		}
		break;
	}
}

void Scumm_v5::o5_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) - a);
}

void Scumm_v5::o5_verbOps() {
	int verb, slot;
	VerbSlot *vs;
	int a, b;
	byte *ptr;

	verb = getVarOrDirectByte(0x80);

	slot = getVerbSlot(verb, 0);
	checkRange(_maxVerbs - 1, 0, slot, "Illegal new verb slot %d");

	vs = &_verbs[slot];
	vs->verbid = verb;

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0x1F) {
		case 1:										/* load image */
			a = getVarOrDirectWord(0x80);
			if (slot) {
				setVerbObject(_roomResource, a, slot);
				vs->type = kImageVerbType;
			}
			break;
		case 2:										/* load from code */
			loadPtrToResource(rtVerb, slot, NULL);
			if (slot == 0)
				nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 3:										/* color */
			vs->color = getVarOrDirectByte(0x80);
			break;
		case 4:										/* set hi color */
			vs->hicolor = getVarOrDirectByte(0x80);
			break;
		case 5:										/* set xy */
			vs->x = getVarOrDirectWord(0x80);
			vs->y = getVarOrDirectWord(0x40);
			// FIXME: hack loom notes into right spot
			if (_gameId == GID_LOOM256) {
				if ((verb >= 90) && (verb <= 97)) {	// Notes
					switch (verb) {
					case 90:
					case 91:
						vs->y -= 7;
						break;
					case 92:
						vs->y -= 6;
						break;
					case 93:
						vs->y -= 4;
						break;
					case 94:
						vs->y -= 3;
						break;
					case 95:
						vs->y -= 1;
						break;
					case 97:
						vs->y -= 5;
					}
				}
			}
			break;
		case 6:										/* set on */
			vs->curmode = 1;
			break;
		case 7:										/* set off */
			vs->curmode = 0;
			break;
		case 8:										/* delete */
			killVerb(slot);
			break;
		case 9:										/* new */
			slot = getVerbSlot(verb, 0);
			if (slot == 0) {
				for (slot = 1; slot < _maxVerbs; slot++) {
					if (_verbs[slot].verbid == 0)
						break;
				}
				if (slot == _maxVerbs)
					error("Too many verbs");
			}
			vs = &_verbs[slot];
			vs->verbid = verb;
			vs->color = 2;
			vs->hicolor = 0;
			vs->dimcolor = 8;
			vs->type = kTextVerbType;
			vs->charset_nr = _string[0].t_charset;
			vs->curmode = 0;
			vs->saveid = 0;
			vs->key = 0;
			vs->center = 0;
			vs->imgindex = 0;
			break;

		case 16:										/* set dim color */
			vs->dimcolor = getVarOrDirectByte(0x80);
			break;
		case 17:										/* dim */
			vs->curmode = 2;
			break;
		case 18:										/* set key */
			vs->key = getVarOrDirectByte(0x80);
			break;
		case 19:										/* set center */
			vs->center = 1;
			break;
		case 20:										/* set to string */
			ptr = getResourceAddress(rtString, getVarOrDirectWord(0x80));
			if (!ptr)
				nukeResource(rtVerb, slot);
			else {
				loadPtrToResource(rtVerb, slot, ptr);
			}
			if (slot == 0)
				nukeResource(rtVerb, slot);
			vs->type = kTextVerbType;
			vs->imgindex = 0;
			break;
		case 22:										/* assign object */
			a = getVarOrDirectWord(0x80);
			b = getVarOrDirectByte(0x40);
			if (slot && vs->imgindex != a) {
				setVerbObject(b, a, slot);
				vs->type = kImageVerbType;
				vs->imgindex = a;
			}
			break;
		case 23:										/* set back color */
			vs->bkcolor = getVarOrDirectByte(0x80);
			break;
		}
	}
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void Scumm_v5::o5_wait() {
	byte *oldaddr;

	oldaddr = _scriptPointer - 1;

	if (_gameId == GID_INDY3_256) {
		_opcode = 2;
	} else
		_opcode = fetchScriptByte();

	switch (_opcode & 0x1F) {
	case 1:	{										/* wait for actor */
			Actor *a = derefActorSafe(getVarOrDirectByte(0x80), "o5_wait");
			if (a && a->isInCurrentRoom() && a->moving)
				break;
			return;
		}
	case 2:											/* wait for message */
		if (_vars[VAR_HAVE_MSG])
			break;
		return;
	case 3:											/* wait for camera */
		if (camera._cur.x >> 3 != camera._dest.x >> 3)
			break;
		return;
	case 4:											/* wait for sentence */
		if (_sentenceNum) {
			if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
				return;
			break;
		}
		if (!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
			return;
		break;
	default:
		error("o5_wait: default case");
		return;
	}

	_scriptPointer = oldaddr;
	o5_breakHere();
}

void Scumm_v5::o5_walkActorTo() {
	int x, y;
	Actor *a;
	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_walkActorTo");
	x = getVarOrDirectWord(0x40);
	y = getVarOrDirectWord(0x20);
	a->startWalkActor(x, y, -1);
}

void Scumm_v5::o5_walkActorToActor() {
	int b, x, y;
	Actor *a, *a2;
	int nr;
	int nr2 = getVarOrDirectByte(0x80);
	a = derefActorSafe(nr2, "o5_walkActorToActor");
	if (!a)
		return;

	if (!a->isInCurrentRoom()) {
		getVarOrDirectByte(0x40);
		fetchScriptByte();
		return;
	}

	nr = getVarOrDirectByte(0x40);
	if (nr == 106 && _gameId == GID_INDY4) {
		warning("Bypassing Indy4 bug");
		fetchScriptByte();
		return;
	}
	// warning("walk actor %d to actor %d", nr, nr2);
	a2 = derefActorSafe(nr, "o5_walkActorToActor(2)");
	if (!a2)
		return;

	if (!a2->isInCurrentRoom()) {
		fetchScriptByte();
		return;
	}
	b = fetchScriptByte();				/* distance from actor */
	if (b == 0xFF) {
		b = a2->scalex * a->width / 0xFF;
		b = b + b / 2;
	}
	x = a2->x;
	y = a2->y;
	if (x < a->x)
		x += b;
	else
		x -= b;

	a->startWalkActor(x, y, -1);
}

void Scumm_v5::o5_walkActorToObject() {
	int obj;
	Actor *a;

	// warning("walk object to object");

	a = derefActorSafe(getVarOrDirectByte(0x80), "o5_walkActorToObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

int Scumm_v5::getWordVararg(int *ptr) {
	int i;

	for (i = 0; i < 15; i++)
		ptr[i] = 0;

	i = 0;
	while ((_opcode = fetchScriptByte()) != 0xFF) {
		ptr[i++] = getVarOrDirectWord(0x80);
	}
	return i;
}

int Scumm_v5::getVarOrDirectWord(byte mask) {
	if (_opcode & mask)
		return readVar(fetchScriptWord());
	return (int16)fetchScriptWord();
}

int Scumm_v5::getVarOrDirectByte(byte mask) {
	if (_opcode & mask)
		return readVar(fetchScriptWord());
	return fetchScriptByte();
}

void Scumm_v5::decodeParseString() {
	int textSlot;

	switch (_actorToPrintStrFor) {
	case 252:
		textSlot = 3;
		break;
	case 253:
		textSlot = 2;
		break;
	case 254:
		textSlot = 1;
		break;
	default:
		textSlot = 0;
	}

	setStringVars(textSlot);

	while ((_opcode = fetchScriptByte()) != 0xFF) {
		switch (_opcode & 0xF) {
		case 0:										/* set string xy */
			_string[textSlot].xpos = getVarOrDirectWord(0x80);
			_string[textSlot].ypos = getVarOrDirectWord(0x40);
			_string[textSlot].overhead = false;
			break;
		case 1:										/* color */
			_string[textSlot].color = getVarOrDirectByte(0x80);
			break;
		case 2:										/* right */
			_string[textSlot].right = getVarOrDirectWord(0x80);
			break;
		case 4:										/* center */
			_string[textSlot].center = true;
			_string[textSlot].overhead = false;
			break;
		case 6:										/* left */
			_string[textSlot].center = false;
			_string[textSlot].overhead = false;
			break;
		case 7:										/* overhead */
			_string[textSlot].overhead = true;
			break;
		case 8:{									/* play loom talkie sound - used in other games ? */
				int offset = (uint16)getVarOrDirectWord(0x80);
				int delay = (uint16)getVarOrDirectWord(0x40);

				if (_gameId == GID_LOOM256) {
					_vars[VAR_MI1_TIMER] = 0;
					if (offset == 0 && delay == 0) {
						_sound->stopCD();
					} else {
						// Loom specified the offset from the start of the CD;
						// thus we have to subtract the lenght of the first track
						// (22500 frames) plus the 2 second = 150 frame leadin.
						// I.e. in total 22650 frames.
						offset = (int)(offset * 7.5 - 22650);

						// Slightly increase the delay (5 frames = 1/25 of a second).
						// This noticably improves the experience in Loom CD.
						delay = (int)(delay * 7.5 + 5);
						
						_sound->playCDTrack(1, 0, offset, delay);
					}
				} else {
					warning("parseString: 8");
				}
			}
			break;
		case 15:
			_messagePtr = _scriptPointer;
			switch (textSlot) {
			case 0:
				actorTalk();
				break;
			case 1:
				drawString(1);
				break;
			case 2:
				unkMessage1();
				break;
			case 3:
				unkMessage2();
				break;
			}

			// FIXME: Store positions, this is needed for Indy3 (Grail Diary)..
			// I don't believe this is the correct fix, may cause other problems
			// later in the game.
			if (_gameId == GID_INDY3_256) {
				_string[textSlot].t_xpos = _string[textSlot].xpos;
				_string[textSlot].t_ypos = _string[textSlot].ypos;
			}

			_scriptPointer = _messagePtr;
			return;
		default:
			return;
		}
	}

	_string[textSlot].t_xpos = _string[textSlot].xpos;
	_string[textSlot].t_ypos = _string[textSlot].ypos;
	_string[textSlot].t_center = _string[textSlot].center;
	_string[textSlot].t_overhead = _string[textSlot].overhead;
	_string[textSlot].t_right = _string[textSlot].right;
	_string[textSlot].t_color = _string[textSlot].color;
	_string[textSlot].t_charset = _string[textSlot].charset;
}

void Scumm_v5::o5_oldRoomEffect() {
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(0x80);

#if 1
		if (_gameId == GID_ZAK256) {
			// FIXME / TODO: OK the first thing to note is: at least in Zak256,
			// maybe also in other games, this opcode does a bit more. I added
			// some stubs here, but somebody with a full IDA or more knowledge
			// about this will have to fill in the gaps. At least now we know
			// that something is missing here :-)
		
			if (a == 4) {
				// No idea what byte_2FCCF is, but it's a globale boolean flag.
				// I only add it here as a temporary hack to make the pseudo code compile.
				int byte_2FCCF = 0;

				if (byte_2FCCF) {
					// Here now "sub_1C44" is called, which sets byte_2FCCF to 0 then
					// calls yet another sub (which also reads byte_2FCCF):

					byte_2FCCF = 0;
					//call    sub_0BB3
					
					
					// Now sub_085C is called. This is quite simply: it sets 
					// 0xF000 bytes. starting at 0x40000 to 0. No idea what that
					// buffer is, maybe a screen buffer, though. Note that
					// 0xF000 = 320*192
					
					// call sub_085C

					
					// And then sub_1C54 is called, which is almost identical to
					// the above sub_1C44, only it sets byte_2FCCF to 1:
					
					byte_2FCCF = 1;
					// call    sub_0BB3

				} else {
					// Here only sub_085C is called (see comment above) 

					// call    sub_085C
				}
			return;
			}
#endif

		}
		if (a) {
			_switchRoomEffect = (byte)a;
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
	}
}

void Scumm_v5::o5_pickupObjectOld() {
	int obj = getVarOrDirectWord(0x80);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;											/* object twice */

	// warning("adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	removeObjectFromRoom(obj);
	putOwner(obj, _vars[VAR_EGO]);
	putClass(obj, 32, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runHook(1);
}
