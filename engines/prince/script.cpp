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

#include "prince/script.h"
#include "prince/prince.h"
#include "prince/flags.h"
#include "prince/variatxt.h"
#include "prince/font.h"
#include "prince/hero.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/stream.h"
#include "common/archive.h"

#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

namespace Prince {

static const uint16 NUM_OPCODES = 144;

Script::Script() : _data(nullptr), _dataSize(0) {
}

Script::~Script() {
	delete[] _data;
	_dataSize = 0;
	_data = nullptr;
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
	_dataSize = stream.size();
	if (!_dataSize) 
		return false;

	_data = new byte[_dataSize];

	if (!_data)
		return false;

	stream.read(_data, _dataSize);
	
	return true;
}

InterpreterFlags::InterpreterFlags() {
	resetAllFlags();
}

void InterpreterFlags::resetAllFlags() {
	memset(_flags, 0, sizeof(_flags));
}

void InterpreterFlags::setFlagValue(Flags::Id flagId, uint16 value) {
	_flags[(uint16)flagId - FLAG_MASK] = value;
}

uint16 InterpreterFlags::getFlagValue(Flags::Id flagId) {
	return _flags[(uint16)flagId - FLAG_MASK];
}

Interpreter::Interpreter(PrinceEngine *vm, Script *script, InterpreterFlags *flags) : 
	_vm(vm), _script(script), _flags(flags),
	_stacktop(0), _opcodeNF(false),
	_waitFlag(0), _result(true) {

	// Initialize the script
	_mode = "fg";
	_fgOpcodePC = _script->getStartGameOffset();
	_bgOpcodePC = 0;
}

void Interpreter::debugInterpreter(const char *s, ...) {
	char buf[STRINGBUFLEN];
	 va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	Common::String str = Common::String::format("@0x%08X: ", _lastInstruction);
	str += Common::String::format("op %04d: ", _lastOpcode);
	//debugC(10, DebugChannel::kScript, "PrinceEngine::Script %s %s", str.c_str(), buf);

	debug("Prince::Script frame %08ld mode %s %s %s", _vm->_frameNr, _mode, str.c_str(), buf);
}

void Interpreter::step() {
	if (_bgOpcodePC) {
		_mode = "bg";
		_bgOpcodePC = step(_bgOpcodePC);
	}
	if (_fgOpcodePC) {
		_mode = "fg";
		_fgOpcodePC = step(_fgOpcodePC);
	}
}

uint32 Interpreter::step(uint32 opcodePC) {
	_currentInstruction = opcodePC;

	while (!_opcodeNF) {
		_lastInstruction = _currentInstruction;

		// Get the current opcode
		_lastOpcode = readScript<uint16>();

		if (_lastOpcode > NUM_OPCODES) 
			error(
				"Trying to execute unknown opcode @0x%04X: %02d", 
				_currentInstruction, 
				_lastOpcode);

		// Execute the current opcode
		OpcodeFunc op = _opcodes[_lastOpcode];
		(this->*op)();
		if (_opcodeNF) {
			_opcodeNF = 0;
			break;
		}
	}

	return _currentInstruction;
}

template <typename T>
T Interpreter::readScript() {
	T data = _script->read<T>(_currentInstruction);
	_currentInstruction += sizeof(data);
	return data;
}

uint16 Interpreter::readScriptFlagValue() {
	uint16 value = readScript<uint16>();
	if (value & InterpreterFlags::FLAG_MASK) {
		return _flags->getFlagValue((Flags::Id)value);
	}
	return value;
}

Flags::Id Interpreter::readScriptFlagId() { 
	return (Flags::Id)readScript<uint16>(); 
}

void Interpreter::O_WAITFOREVER() {
	debugInterpreter("O_WAITFOREVER");
	_opcodeNF = 1;
	_currentInstruction -= 2;
}

void Interpreter::O_BLACKPALETTE() {
	debugInterpreter("O_BLACKPALETTE");
}

void Interpreter::O_SETUPPALETTE() {
	debugInterpreter("O_SETUPPALETTE");
}

void Interpreter::O_INITROOM() {
	uint16 roomId = readScriptFlagValue();
	debugInterpreter("O_INITROOM %d", roomId);
	_vm->loadLocation(roomId);
	_opcodeNF = 1;
}

void Interpreter::O_SETSAMPLE() {
	uint16 sampleId = readScriptFlagValue();
	int32 sampleNameOffset = readScript<uint32>();
	const char * sampleName = _script->getString(_currentInstruction + sampleNameOffset - 4);
	debugInterpreter("O_SETSAMPLE %d %s", sampleId, sampleName);
	_vm->loadSample(sampleId, sampleName);
}

void Interpreter::O_FREESAMPLE() {
	uint16 sample = readScriptFlagValue();
	debugInterpreter("O_FREESAMPLE %d", sample);
}

void Interpreter::O_PLAYSAMPLE() {
	uint16 sampleId = readScriptFlagValue();
	uint16 loopType = readScript<uint16>();
	debugInterpreter("O_PLAYSAMPLE sampleId %d loopType %d", sampleId, loopType);
	_vm->playSample(sampleId, loopType);
}

void Interpreter::O_PUTOBJECT() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	uint16 objectId = readScriptFlagValue();
	debugInterpreter("O_PUTOBJECT roomId %d, slot %d, objectId %d", roomId, slot, objectId);
}

void Interpreter::O_REMOBJECT() {
	uint16 roomId = readScriptFlagValue();
	uint16 objectId = readScriptFlagValue();

	debugInterpreter("O_REMOBJECT roomId %d objectId %d", roomId, objectId);
}

void Interpreter::O_SHOWANIM() {
	uint16 slot = readScriptFlagValue();
	uint16 animId = readScriptFlagValue();

	debugInterpreter("O_SHOWANIM slot %d, animId %d", slot, animId);
}

void Interpreter::O_CHECKANIMEND() {
	uint16 slot = readScriptFlagValue();
	uint16 frameId = readScriptFlagValue();

	debugInterpreter("O_CHECKANIMEND slot %d, frameId %d", slot, frameId);
	_opcodeNF = 1;
}

void Interpreter::O_FREEANIM() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_FREEANIM slot %d", slot);
}

void Interpreter::O_CHECKANIMFRAME() {
	uint16 slot = readScriptFlagValue();
	uint16 frameId = readScriptFlagValue();

	debugInterpreter("O_CHECKANIMFRAME slot %d, frameId %d", slot, frameId);
	_opcodeNF = 1;
}

void Interpreter::O_PUTBACKANIM() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	int32 animId = readScript<uint32>();
	debugInterpreter("O_PUTBACKANIM roomId %d, slot %d, animId %d", roomId, slot, animId);
}

void Interpreter::O_REMBACKANIM() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();

	debugInterpreter("O_REMBACKANIM roomId %d, slot %d", roomId, slot);
}

void Interpreter::O_CHECKBACKANIMFRAME() {
	uint16 slotId = readScriptFlagValue();
	uint16 frameId = readScriptFlagValue();

	debugInterpreter("O_CHECKBACKANIMFRAME slotId %d, frameId %d", slotId, frameId);
	_opcodeNF = 1;
}

void Interpreter::O_FREEALLSAMPLES() {
	debugInterpreter("O_FREEALLSAMPLES");
}

void Interpreter::O_SETMUSIC() {
	uint16 musicId = readScript<uint16>();

	debugInterpreter("O_SETMUSIC musicId %d", musicId);
}

void Interpreter::O_STOPMUSIC() {
	debugInterpreter("O_STOPMUSIC");
}

void Interpreter::O__WAIT() {
	uint16 pause = readScriptFlagValue();

	debugInterpreter("O__WAIT pause %d", pause);

	if (_waitFlag == 0) {
		// set new wait flag value and continue
		_waitFlag = pause;
		_opcodeNF = 1;
		_currentInstruction -= 4;
		return;
	}

	--_waitFlag;

	if (_waitFlag > 0) {
		_opcodeNF = 1;
		_currentInstruction -= 4;
		return;
	}
}

void Interpreter::O_UPDATEOFF() {
	debugInterpreter("O_UPDATEOFF");
	//_updateEnable = false;
}

void Interpreter::O_UPDATEON() {
	debugInterpreter("O_UPDATEON");
	//_updateEnable = true;
}

void Interpreter::O_UPDATE () {
	debugInterpreter("O_UPDATE");
	// Refresh screen
}

void Interpreter::O_CLS() {
	debugInterpreter("O_CLS");
	// do nothing
}

void Interpreter::O__CALL() {
	int32 address = readScript<uint32>();
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;
	_currentInstruction += address - 4;
	debugInterpreter("O__CALL 0x%04X", _currentInstruction);
}

void Interpreter::O_RETURN() {
	// Get the return address
	if (_stacktop > 0) {
		_stacktop--;
		_currentInstruction = _stack[_stacktop];
		debugInterpreter("O_RETURN 0x%04X", _currentInstruction);
	} else {
		error("Return: Stack is empty");
	}
}

void Interpreter::O_GO() {
	int32 opPC = readScript<uint32>();
	debugInterpreter("O_GO 0x%04X", opPC);
	_currentInstruction += opPC - 4;
}

void Interpreter::O_BACKANIMUPDATEOFF() {
	uint16 slotId = readScriptFlagValue();
	debugInterpreter("O_BACKANIMUPDATEOFF slotId %d", slotId);
}

void Interpreter::O_BACKANIMUPDATEON() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_BACKANIMUPDATEON %d", slot);
}

void Interpreter::O_CHANGECURSOR() {
	uint16 cursorId = readScriptFlagValue();
	debugInterpreter("O_CHANGECURSOR %x", cursorId);
	_vm->changeCursor(cursorId);
}

void Interpreter::O_CHANGEANIMTYPE() {
	// NOT IMPLEMENTED
}

void Interpreter::O__SETFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O__SETFLAG 0x%04X (%s) = %d", flagId, Flags::getFlagName(flagId), value);

	_flags->setFlagValue((Flags::Id)(flagId), value);
}

void Interpreter::O_COMPARE() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	_result = _flags->getFlagValue(flagId) != value;
	debugInterpreter("O_COMPARE flagId 0x%04X (%s), value %d == %d (%d)", flagId, Flags::getFlagName(flagId), value, _flags->getFlagValue(flagId), _result);
}

void Interpreter::O_JUMPZ() {
	int32 offset = readScript<uint32>();
	if (! _result) {
		_currentInstruction += offset - 4;
	}

	debugInterpreter("O_JUMPZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Interpreter::O_JUMPNZ() {
	int32 offset = readScript<uint32>();
	if (_result) {
		_currentInstruction += offset - 4;
	}

	debugInterpreter("O_JUMPNZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Interpreter::O_EXIT() {
	uint16 exitCode = readScriptFlagValue();
	debugInterpreter("O_EXIT exitCode %d", exitCode);
	// Set exit code and shows credits 
	// if exit code == 0x02EAD
}

void Interpreter::O_ADDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) + value);
	if (_flags->getFlagValue(flagId))
		_result = 1;
	else
		_result = 0;

	debugInterpreter("O_ADDFLAG flagId %04x (%s), value %d", flagId, Flags::getFlagName(flagId), value);
}

void Interpreter::O_TALKANIM() {
	uint16 animSlot = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();

	debugInterpreter("O_TALKANIM animSlot %d, slot %d", animSlot, slot);
}

void Interpreter::O_SUBFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) - value);
	if (_flags->getFlagValue(flagId))
		_result = 1;
	else
		_result = 0;

	debugInterpreter("O_SUBFLAG flagId %d, value %d", flagId, value);
}

void Interpreter::O_SETSTRING() {
	int32 offset = readScript<uint32>();
	_currentString = offset;

	// FIXME: Make it better ;)
	if (offset >= 80000) {
		debugInterpreter("GetVaria %s", _vm->_variaTxt->getString(offset - 80000));
	}
	else if (offset < 2000) {
		uint32 of = READ_LE_UINT32(_vm->_talkTxt+offset*4);
		const char * txt = (const char *)&_vm->_talkTxt[of];
		_string = &_vm->_talkTxt[of];
		debugInterpreter("TalkTxt %d %s", of, txt);
	}

	debugInterpreter("O_SETSTRING %04d", offset);
}

void Interpreter::O_ANDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_ANDFLAG flagId %d, value %d", flagId, value);

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) & value);

	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_GETMOBDATA() {
	Flags::Id flagId = readScriptFlagId();
	uint16 mobId = readScript<uint16>();
	uint16 mobOffset = readScript<uint16>();

	debugInterpreter("O_GETMOBDATA flagId %d, modId %d, mobOffset %d", flagId, mobId, mobOffset);
}

void Interpreter::O_ORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();
	
	debugInterpreter("O_ORFLAG flagId %d, value %d", flagId, value);
	
	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) | value);

	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_SETMOBDATA() {
	uint16 mobId = readScriptFlagValue();
	uint16 mobOffset = readScriptFlagValue();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_SETMOBDATA mobId %d, mobOffset %d, value %d", mobId, mobOffset, value);
}

void Interpreter::O_XORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_XORFLAG flagId %d, value %d", flagId, value);

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) ^ value);

	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_GETMOBTEXT() {
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_GETMOBTEXT value %d", value);
	// Use Mob::ExamText as current string 
}

void Interpreter::O_MOVEHERO() {
	uint16 heroId = readScriptFlagValue();
	uint16 x = readScriptFlagValue();
	uint16 y = readScriptFlagValue();
	uint16 dir = readScriptFlagValue();
	
	debugInterpreter("O_MOVEHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Interpreter::O_WALKHERO() {
	uint16 heroId = readScriptFlagValue();

	debugInterpreter("O_WALKHERO %d", heroId);
	_opcodeNF = 1;
}

void Interpreter::O_SETHERO() {
	uint16 hero = readScriptFlagValue();
	int16 x = readScriptFlagValue();
	int16 y = readScriptFlagValue();
	uint16 dir = readScriptFlagValue();
	debugInterpreter("O_SETHERO hero %d, x %d, y %d, dir %d", hero, x, y, dir);
	_vm->_mainHero->setPos(x, y);
}

void Interpreter::O_HEROOFF() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_HEROOFF %d", heroId);
	_vm->_mainHero->setVisible(false);
}

void Interpreter::O_HEROON() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_HEROON %d", heroId);
	_vm->_mainHero->setVisible(true);
}

void Interpreter::O_CLSTEXT() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_CLSTEXT slot %d", slot);
	// Sets text line to null
	// Sets text timeout to zero
}

void Interpreter::O_CALLTABLE() {
	uint16 flag = readScript<uint16>();
	int32 table = readScript<uint32>();

	debugInterpreter("O_CALLTABLE flag %d, table %d", flag, table);
	// makes a call from script function table
	// must read table pointer from _code and
	// use table entry as next opcode
}

void Interpreter::O_CHANGEMOB() {
	uint16 mob = readScriptFlagValue();
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_CHANGEMOB mob %d, value %d", mob, value);
	// Probably sets mobs visible flag to value
}

void Interpreter::O_ADDINV() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	debugInterpreter("O_ADDINV hero %d, item %d", hero, item);
}

void Interpreter::O_REMINV() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	debugInterpreter("O_REMINV hero %d, item %d", hero, item);
}

void Interpreter::O_REPINV() {
	uint16 hero = readScript<uint16>();
	uint16 item1 = readScript<uint16>();
	uint16 item2 = readScript<uint16>();
	// shouldn't be uses
	error("O_REPINV hero %d, item1 %d, item2 %d", hero, item1, item2);
}

void Interpreter::O_OBSOLETE_GETACTION() {
	// shouldn't be uses
	error("O_OBSOLETE_GETACTION");
}

void Interpreter::O_ADDWALKAREA() {
	uint16 x1 = readScript<uint16>();
	uint16 y1 = readScript<uint16>();
	uint16 x2 = readScript<uint16>();
	uint16 y2 = readScript<uint16>();
	// shouldn't be uses
	error("O_ADDWALKAREA x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
}

void Interpreter::O_REMWALKAREA() {
	uint16 x1 = readScript<uint16>();
	uint16 y1 = readScript<uint16>();
	uint16 x2 = readScript<uint16>();
	uint16 y2 = readScript<uint16>();

	// shouldn't be uses
	error("O_REMWALKAREA x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
}
 
void Interpreter::O_RESTOREWALKAREA() {
	debugInterpreter("O_RESTOREWALKAREA");
}

void Interpreter::O_WAITFRAME() {
	debugInterpreter("O_WAITFRAME");
	_opcodeNF = true;
}

void Interpreter::O_SETFRAME() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	debugInterpreter("O_SETFRAME anim %d, frame %d", anim, frame);
}

void Interpreter::O_RUNACTION() {
	// It's empty in original and never used in script
	// it's better to report error
	error("O_RUNACTION");
}

void Interpreter::O_COMPAREHI() {
	Flags::Id flag = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_COMPAREHI flag %d, value %d", flag, value);
	_result = value < _flags->getFlagValue(flag);
}

void Interpreter::O_COMPARELO() {
	Flags::Id flag = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_COMPARELO flag %d, value %d", flag, value);
	_result = value > _flags->getFlagValue(flag);
}

void Interpreter::O_PRELOADSET() {
	// not used in script
	int32 offset = readScript<uint32>();
	debugInterpreter("O_PRELOADSET offset %04x", offset);
}

void Interpreter::O_FREEPRELOAD() {
	// not used in script
	debugInterpreter("O_FREEPRELOAD");
}

void Interpreter::O_CHECKINV() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	debugInterpreter("O_CHECKINV hero %d, item %d", hero, item);
	// checks if item is in heros inventory	
}

void Interpreter::O_TALKHERO() {
	uint16 hero = readScriptFlagValue();
	debugInterpreter("O_TALKHERO hero %d", hero);
}

void Interpreter::O_WAITTEXT() {
	uint16 slot = readScriptFlagValue();
	Text &text = _vm->_textSlots[slot];
	if (text._time) {
		_opcodeNF = 1;
		_currentInstruction -= 4;
	}
}

void Interpreter::O_SETHEROANIM() {
	uint16 hero = readScriptFlagValue();
	int32 offset = readScript<uint32>();
	debugInterpreter("O_SETHEROANIM hero %d, offset %d", hero, offset);
}

void Interpreter::O_WAITHEROANIM() {
	uint16 hero = readScriptFlagValue();

	debugInterpreter("O_WAITHEROANIM hero %d", hero);
}

void Interpreter::O_GETHERODATA() {
	uint16 flag = readScript<uint16>();
	uint16 hero = readScriptFlagValue();
	uint16 heroOffset = readScriptFlagValue();
	debugInterpreter("O_GETHERODATA flag %d, hero %d, heroOffset %d", flag, hero, heroOffset);
}

void Interpreter::O_GETMOUSEBUTTON() {
	debugInterpreter("O_GETMOUSEBUTTON");
}

void Interpreter::O_CHANGEFRAMES() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	uint16 lastFrame = readScriptFlagValue();
	uint16 loopFrame = readScriptFlagValue();

	debugInterpreter(
		"O_CHANGFRAMES anim %d, fr1 %d, fr2 %d, fr3 %d", 
		anim, 
		frame, 
		lastFrame, 
		loopFrame);

}

void Interpreter::O_CHANGEBACKFRAMES() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	uint16 lastFrame = readScriptFlagValue();
	uint16 loopFrame = readScriptFlagValue();

	debugInterpreter(
		"O_CHANGEBACKFRAMES anim %d, fr1 %d, fr2 %d, fr3 %d", 
		anim, 
		frame, 
		lastFrame, 
		loopFrame);
}

void Interpreter::O_GETBACKANIMDATA() {
	uint16 flag = readScript<uint16>();
	uint16 anim = readScript<uint16>();
	uint16 animOffset = readScript<uint16>();
	debugInterpreter("O_GETBACKANIMDATA flag %d, anim %d, animOffset %d", flag, anim, animOffset);
}

void Interpreter::O_GETANIMDATA() {
	uint16 flag = readScript<uint16>();
	uint16 anim = readScriptFlagValue();
	uint16 animOffset = readScriptFlagValue();
	debugInterpreter("O_GETANIMDATA flag %d, anim %d, animOffset %d", flag, anim, animOffset);
	// Gets value of anim data 
	// use anim offset as attribute id not an offset
}

void Interpreter::O_SETBGCODE() {
	int32 offset = readScript<uint32>();
	_bgOpcodePC = _currentInstruction + offset - 4;
	debugInterpreter("O_SETBGCODE next %08x, offset %08x", _bgOpcodePC, offset);
}

void Interpreter::O_SETBACKFRAME() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();

	debugInterpreter("O_SETBACKFRAME anim %d, frame %d", anim, frame);
}

void Interpreter::O_GETRND() {
	Flags::Id flag = readScriptFlagId();
	uint16 rndSeed = readScript<uint16>();
	debugInterpreter("O_GETRND flag %d, rndSeed %d", flag, rndSeed);
	// puts and random value as flag value
	// fairly random value ;)
	// _flags->setFlagValue(flag, 4);
}

void Interpreter::O_TALKBACKANIM() {
	uint16 animSlot = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_TALKBACKANIM animSlot %d, slot %d", animSlot, slot);
}

void Interpreter::O_LOADPATH() {
	int32 offset = readScript<uint32>();
	debugInterpreter("O_LOADPATH offset %d", offset);
	// _currentInstruction + offset path file name ptr
	// free path bitmap
	// load packet path bitmap and puts in Sala
}

void Interpreter::O_GETCHAR() {
	Flags::Id flagId = readScriptFlagId();

	_flags->setFlagValue(flagId, *_string);

	debugInterpreter(
		"O_GETCHAR %04X (%s) %02x", 
		flagId, 
		Flags::getFlagName(flagId), 
		_flags->getFlagValue(flagId));

	++_string;
}

void Interpreter::O_SETDFLAG() {
	uint16 flag = readScript<uint16>();
	int32 offset = readScript<uint32>();
	debugInterpreter("O_SETDFLAG flag %d, offset %04x", flag, offset);
	// run this through debugger looks strange
	// it looks like this one store two 16 bit value in one go
}

void Interpreter::O_CALLDFLAG() {
	uint16 flag = readScript<uint16>();
	debugInterpreter("O_CALLDFLAG flag %d", flag);
	// it seems that some flags are 32 bit long
}

void Interpreter::O_PRINTAT() {
	uint16 slot = readScriptFlagValue();
	uint16 fr1 = readScriptFlagValue();
	uint16 fr2 = readScriptFlagValue();

	debugInterpreter("O_PRINTAT slot %d, fr1 %d, fr2 %d", slot, fr1, fr2);

	uint8 color = _flags->getFlagValue(Flags::KOLOR);

	_vm->printAt(slot, color, (const char *)_string, fr1, fr2);

	while (*_string) {
		++_string;
	}
	++_string;
}

void Interpreter::O_ZOOMIN() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_ZOOMIN slot %04d", slot);
}

void Interpreter::O_ZOOMOUT() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_ZOOMOUT slot %d", slot);
}

void Interpreter::O_SETSTRINGOFFSET() {
	int32 offset = readScript<uint32>();
	debugInterpreter("O_SETSTRINGOFFSET offset %04x", offset);
	// _currentString = ""
	// _string = (const char *)_currentInstruction + offset 
}

void Interpreter::O_GETOBJDATA() {
	Flags::Id flag = readScriptFlagId();
	uint16 obj = readScriptFlagValue();
	int16 objOffset = readScriptFlagValue();
	debugInterpreter("O_GETOBJDATA flag %d, obj %d, objOffset %d", flag, obj, objOffset);
}

void Interpreter::O_SETOBJDATA() {
	uint16 obj = readScriptFlagValue();
	int16 objOffset = readScriptFlagValue();
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_SETOBJDATA obj %d, objOffset %d, value %d", obj, objOffset, value);
}

void Interpreter::O_SWAPOBJECTS() {
	uint16 obj1 = readScript<uint16>();
	uint16 obj2 = readScript<uint16>();
	debugInterpreter("O_SWAPOBJECTS obj1 %d, obj2 %d", obj1, obj2);
}

void Interpreter::O_CHANGEHEROSET() {
	uint16 hero = readScriptFlagValue();
	uint16 heroSet = readScriptFlagValue();
	debugInterpreter("O_CHANGEHEROSET hero %d, heroSet %d", hero, heroSet);
}

void Interpreter::O_ADDSTRING() {
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_ADDSTRING value %d", value);
	// _string += value
}

void Interpreter::O_SUBSTRING() {
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_SUBSTRING value %d", value);
	// _string -= value
}

void Interpreter::O_INITDIALOG() {
	debugInterpreter("O_INITDIALOG");
}

void Interpreter::O_ENABLEDIALOGOPT() {
	uint16 opt = readScriptFlagValue();
	debugInterpreter("O_ENABLEDIALOGOPT opt %d", opt);
}

void Interpreter::O_DISABLEDIALOGOPT() {
	uint16 opt = readScriptFlagValue();
	debugInterpreter("O_DISABLEDIALOGOPT opt %d", opt);
}

void Interpreter::O_SHOWDIALOGBOX() {
	uint16 box = readScriptFlagValue();
	debugInterpreter("O_SHOWDIALOGBOX box %d", box);
}

void Interpreter::O_STOPSAMPLE() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_STOPSAMPLE slot %d", slot);
	_vm->stopSample(slot);
}

void Interpreter::O_BACKANIMRANGE() {
	uint16 slotId = readScriptFlagValue();
	uint16 animId = readScript<uint16>();
	uint16 low = readScriptFlagValue();
	uint16 high = readScriptFlagValue();

	if (animId != 0xFFFF) {
		if (animId & InterpreterFlags::FLAG_MASK) {
			animId = _flags->getFlagValue((Flags::Id)animId);
		}
	}

	debugInterpreter("O_BACKANIMRANGE slotId %d, animId %d, low %d, high %d", slotId, animId, low, high);
	_result = 1;
}

void Interpreter::O_CLEARPATH() {
	debugInterpreter("O_CLEARPATH");
	// Fill Sala with 255
}

void Interpreter::O_SETPATH() {
	debugInterpreter("O_SETPATH");
	// CopyPath
}

void Interpreter::O_GETHEROX() {
	uint16 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();

	debugInterpreter("O_GETHEROX heroId %d, flagId %d", heroId, flagId);
}

void Interpreter::O_GETHEROY() {
	uint16 heroId = readScript<uint16>();
	Flags::Id flagId = readScriptFlagId();

	debugInterpreter("O_GETHEROY heroId %d, flagId %d", heroId, flagId);
}

void Interpreter::O_GETHEROD() {
	uint16 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();

	debugInterpreter("O_GETHEROD heroId %d, flagId %d", heroId, flagId);
}

void Interpreter::O_PUSHSTRING() {
	debugInterpreter("O_PUSHSTRING");
	// push on the stack
	// _currentString
	// _dialogData
	// _string
}

void Interpreter::O_POPSTRING() {
	debugInterpreter("O_POPSTRING");
	// pop from the stack
	// _currentString
	// _dialogData
	// _string
}

void Interpreter::O_SETFGCODE() {
	int32 offset = readScript<uint32>();
	_fgOpcodePC = _currentInstruction + offset - 4;	

	debugInterpreter("O_SETFGCODE next %08x, offset %08x", _fgOpcodePC, offset);
}

void Interpreter::O_STOPHERO() {
	uint16 heroId = readScriptFlagValue();

	debugInterpreter("O_STOPHERO heroId %d", heroId);

	// clear move steps for hero
}

void Interpreter::O_ANIMUPDATEOFF() {
	uint16 slotId = readScript<uint16>();
	debugInterpreter("O_ANIMUPDATEOFF slotId %d", slotId);
}

void Interpreter::O_ANIMUPDATEON() {
	uint16 slotId = readScriptFlagValue();
	debugInterpreter("O_ANIMUPDATEON slotId %d", slotId);
}

void Interpreter::O_FREECURSOR() {
	debugInterpreter("O_FREECURSOR");
	// Change cursor to 0
	// free inv cursor 1
}

void Interpreter::O_ADDINVQUIET() {
	uint16 heroId = readScriptFlagValue();
	uint16 itemId = readScriptFlagValue();

	debugInterpreter("O_ADDINVQUIET heorId %d, itemId %d", heroId, itemId);
}

void Interpreter::O_RUNHERO() {
	uint16 heroId = readScriptFlagValue();
	uint16 x = readScriptFlagValue();
	uint16 y = readScriptFlagValue();
	uint16 dir = readScriptFlagValue();

	debugInterpreter("O_RUNHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Interpreter::O_SETBACKANIMDATA() {
	uint16 animId = readScriptFlagValue();
	uint16 animOffset = readScriptFlagValue();
	uint16 wart = readScriptFlagValue();

	debugInterpreter("O_SETBACKANIMDATA animId %d, animOffset %d, wart %d", animId, animOffset, wart);
}

void Interpreter::O_VIEWFLC() {
	uint16 animNr = readScriptFlagValue();
	debugInterpreter("O_VIEWFLC animNr %d", animNr);
	_vm->loadAnim(animNr, false);
}

void Interpreter::O_CHECKFLCFRAME() {
	uint16 frameNr = readScriptFlagValue();

	debugInterpreter("O_CHECKFLCFRAME frame number %d", frameNr);

	if (_vm->_flicPlayer.getCurFrame() != frameNr) {
		// Move instruction pointer before current instruciton
		// must do this check once again till it's false 
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Interpreter::O_CHECKFLCEND() {

	//debugInterpreter("O_CHECKFLCEND");

	const Video::FlicDecoder &flicPlayer = _vm->_flicPlayer;

	//debug("frameCount %d, currentFrame %d", flicPlayer.getFrameCount(), flicPlayer.getCurFrame());

	if (flicPlayer.getFrameCount() - flicPlayer.getCurFrame() > 1) {
		// Move instruction pointer before current instruciton
		// must do this check once again till it's false 
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Interpreter::O_FREEFLC() {
	debugInterpreter("O_FREEFLC");
}

void Interpreter::O_TALKHEROSTOP() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_TALKHEROSTOP %d", heroId);
}

void Interpreter::O_HEROCOLOR() {
	uint16 heroId = readScriptFlagValue();
	uint16 kolorr = readScriptFlagValue();
	debugInterpreter("O_HEROCOLOR heroId %d, kolorr %d", heroId, kolorr);
}

void Interpreter::O_GRABMAPA() {
	debugInterpreter("O_GRABMAPA");
}

void Interpreter::O_ENABLENAK() {
	uint16 nakId = readScriptFlagValue();
	debugInterpreter("O_ENABLENAK nakId %d", nakId);
}

void Interpreter::O_DISABLENAK() {
	uint16 nakId = readScriptFlagValue();
	debugInterpreter("O_DISABLENAK nakId %d", nakId);
}

void Interpreter::O_GETMOBNAME() {
	uint16 war = readScriptFlagValue();
	debugInterpreter("O_GETMOBNAME war %d", war);
}

void Interpreter::O_SWAPINVENTORY() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_SWAPINVENTORY heroId %d", heroId);
}

void Interpreter::O_CLEARINVENTORY() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_CLEARINVENTORY heroId %d", heroId);
}

void Interpreter::O_SKIPTEXT() {
	debugInterpreter("O_SKIPTEXT");
}

void Interpreter::SetVoice(uint32 sampleSlot) {
	uint16 slot = readScriptFlagValue();
	_vm->loadVoice(
		slot,
		sampleSlot,
		Common::String::format(
			"%03d-%02d.WAV", 
			_currentString, 
			_flags->getFlagValue(Flags::VOICE_H_LINE)
		)
	);
}

void Interpreter::O_SETVOICEH() {
	static const uint32 VOICE_H_SLOT = 28;
	SetVoice(VOICE_H_SLOT);
}

void Interpreter::O_SETVOICEA() {
	static const uint32 VOICE_A_SLOT = 29;
	SetVoice(VOICE_A_SLOT);
}

void Interpreter::O_SETVOICEB() {
	static const uint32 VOICE_B_SLOT = 30;
	SetVoice(VOICE_B_SLOT);
}

void Interpreter::O_SETVOICEC() {
	static const uint32 VOICE_C_SLOT = 31;
	SetVoice(VOICE_C_SLOT);
}

void Interpreter::O_SETVOICED() {
	static const uint32 VOICE_D_SLOT = 32;
	SetVoice(VOICE_D_SLOT);
}

void Interpreter::O_VIEWFLCLOOP() {
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_VIEWFLCLOOP animId %d", value);

	_vm->loadAnim(value, true);
}

void Interpreter::O_FLCSPEED() {
	uint16 speed = readScriptFlagValue();
	debugInterpreter("O_FLCSPEED speed %d", speed);
}

void Interpreter::O_OPENINVENTORY() {
	debugInterpreter("O_OPENINVENTORY");
	_opcodeNF = 1;
	// _showInventoryFlag = true
}

void Interpreter::O_KRZYWA() {
	debugInterpreter("O_KRZYWA");
}

void Interpreter::O_GETKRZYWA() {
	debugInterpreter("O_GETKRZYWA");
	// _flags->setFlagValue(Flags::TORX1, krzywa[_krzywaIndex++])
	// _flags->setFlagValue(Flags::TORY1, krzywa[_krzywaIndex++])
	// Check _krzywaIndex 
}

void Interpreter::O_GETMOB() {
	Flags::Id flagId = readScriptFlagId();
	uint16 mx = readScriptFlagValue();
	uint16 my = readScriptFlagValue();
	debugInterpreter("O_GETMOB flagId %d, mx %d, my %d", flagId, mx, my);
	// check if current mob pos = (mx, my)
}

void Interpreter::O_INPUTLINE() {
	debugInterpreter("O_INPUTLINE");
}


void Interpreter::O_BREAK_POINT() {
	debugInterpreter("O_BREAK_POINT");
}

Interpreter::OpcodeFunc Interpreter::_opcodes[NUM_OPCODES] = {
	&Interpreter::O_WAITFOREVER,
	&Interpreter::O_BLACKPALETTE,
	&Interpreter::O_SETUPPALETTE,
	&Interpreter::O_INITROOM,
	&Interpreter::O_SETSAMPLE,
	&Interpreter::O_FREESAMPLE,
	&Interpreter::O_PLAYSAMPLE,
	&Interpreter::O_PUTOBJECT,
	&Interpreter::O_REMOBJECT,
	&Interpreter::O_SHOWANIM,
	&Interpreter::O_CHECKANIMEND,
	&Interpreter::O_FREEANIM,
	&Interpreter::O_CHECKANIMFRAME,
	&Interpreter::O_PUTBACKANIM,
	&Interpreter::O_REMBACKANIM,
	&Interpreter::O_CHECKBACKANIMFRAME,
	&Interpreter::O_FREEALLSAMPLES,
	&Interpreter::O_SETMUSIC,
	&Interpreter::O_STOPMUSIC,
	&Interpreter::O__WAIT,
	&Interpreter::O_UPDATEOFF,
	&Interpreter::O_UPDATEON,
	&Interpreter::O_UPDATE ,
	&Interpreter::O_CLS,
	&Interpreter::O__CALL,
	&Interpreter::O_RETURN,
	&Interpreter::O_GO,
	&Interpreter::O_BACKANIMUPDATEOFF,
	&Interpreter::O_BACKANIMUPDATEON,
	&Interpreter::O_CHANGECURSOR,
	&Interpreter::O_CHANGEANIMTYPE,
	&Interpreter::O__SETFLAG,
	&Interpreter::O_COMPARE,
	&Interpreter::O_JUMPZ,
	&Interpreter::O_JUMPNZ,
	&Interpreter::O_EXIT,
	&Interpreter::O_ADDFLAG,
	&Interpreter::O_TALKANIM,
	&Interpreter::O_SUBFLAG,
	&Interpreter::O_SETSTRING,
	&Interpreter::O_ANDFLAG,
	&Interpreter::O_GETMOBDATA,
	&Interpreter::O_ORFLAG,
	&Interpreter::O_SETMOBDATA,
	&Interpreter::O_XORFLAG,
	&Interpreter::O_GETMOBTEXT,
	&Interpreter::O_MOVEHERO,
	&Interpreter::O_WALKHERO,
	&Interpreter::O_SETHERO,
	&Interpreter::O_HEROOFF,
	&Interpreter::O_HEROON,
	&Interpreter::O_CLSTEXT,
	&Interpreter::O_CALLTABLE,
	&Interpreter::O_CHANGEMOB,
	&Interpreter::O_ADDINV,
	&Interpreter::O_REMINV,
	&Interpreter::O_REPINV,
	&Interpreter::O_OBSOLETE_GETACTION,
	&Interpreter::O_ADDWALKAREA,
	&Interpreter::O_REMWALKAREA,
	&Interpreter::O_RESTOREWALKAREA,
	&Interpreter::O_WAITFRAME,
	&Interpreter::O_SETFRAME,
	&Interpreter::O_RUNACTION,
	&Interpreter::O_COMPAREHI,
	&Interpreter::O_COMPARELO,
	&Interpreter::O_PRELOADSET,
	&Interpreter::O_FREEPRELOAD,
	&Interpreter::O_CHECKINV,
	&Interpreter::O_TALKHERO,
	&Interpreter::O_WAITTEXT,
	&Interpreter::O_SETHEROANIM,
	&Interpreter::O_WAITHEROANIM,
	&Interpreter::O_GETHERODATA,
	&Interpreter::O_GETMOUSEBUTTON,
	&Interpreter::O_CHANGEFRAMES,
	&Interpreter::O_CHANGEBACKFRAMES,
	&Interpreter::O_GETBACKANIMDATA,
	&Interpreter::O_GETANIMDATA,
	&Interpreter::O_SETBGCODE,
	&Interpreter::O_SETBACKFRAME,
	&Interpreter::O_GETRND,
	&Interpreter::O_TALKBACKANIM,
	&Interpreter::O_LOADPATH,
	&Interpreter::O_GETCHAR,
	&Interpreter::O_SETDFLAG,
	&Interpreter::O_CALLDFLAG,
	&Interpreter::O_PRINTAT,
	&Interpreter::O_ZOOMIN,
	&Interpreter::O_ZOOMOUT,
	&Interpreter::O_SETSTRINGOFFSET,
	&Interpreter::O_GETOBJDATA,
	&Interpreter::O_SETOBJDATA,
	&Interpreter::O_SWAPOBJECTS,
	&Interpreter::O_CHANGEHEROSET,
	&Interpreter::O_ADDSTRING,
	&Interpreter::O_SUBSTRING,
	&Interpreter::O_INITDIALOG,
	&Interpreter::O_ENABLEDIALOGOPT,
	&Interpreter::O_DISABLEDIALOGOPT,
	&Interpreter::O_SHOWDIALOGBOX,
	&Interpreter::O_STOPSAMPLE,
	&Interpreter::O_BACKANIMRANGE,
	&Interpreter::O_CLEARPATH,
	&Interpreter::O_SETPATH,
	&Interpreter::O_GETHEROX,
	&Interpreter::O_GETHEROY,
	&Interpreter::O_GETHEROD,
	&Interpreter::O_PUSHSTRING,
	&Interpreter::O_POPSTRING,
	&Interpreter::O_SETFGCODE,
	&Interpreter::O_STOPHERO,
	&Interpreter::O_ANIMUPDATEOFF,
	&Interpreter::O_ANIMUPDATEON,
	&Interpreter::O_FREECURSOR,
	&Interpreter::O_ADDINVQUIET,
	&Interpreter::O_RUNHERO,
	&Interpreter::O_SETBACKANIMDATA,
	&Interpreter::O_VIEWFLC,
	&Interpreter::O_CHECKFLCFRAME,
	&Interpreter::O_CHECKFLCEND,
	&Interpreter::O_FREEFLC,
	&Interpreter::O_TALKHEROSTOP,
	&Interpreter::O_HEROCOLOR,
	&Interpreter::O_GRABMAPA,
	&Interpreter::O_ENABLENAK,
	&Interpreter::O_DISABLENAK,
	&Interpreter::O_GETMOBNAME,
	&Interpreter::O_SWAPINVENTORY,
	&Interpreter::O_CLEARINVENTORY,
	&Interpreter::O_SKIPTEXT,
	&Interpreter::O_SETVOICEH,
	&Interpreter::O_SETVOICEA,
	&Interpreter::O_SETVOICEB,
	&Interpreter::O_SETVOICEC,
	&Interpreter::O_VIEWFLCLOOP,
	&Interpreter::O_FLCSPEED,
	&Interpreter::O_OPENINVENTORY,
	&Interpreter::O_KRZYWA,
	&Interpreter::O_GETKRZYWA,
	&Interpreter::O_GETMOB,
	&Interpreter::O_INPUTLINE,
	&Interpreter::O_SETVOICED,
	&Interpreter::O_BREAK_POINT,
};

}

/* vim: set tabstop=4 noexpandtab: */
