/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/dataio.h"
#include "gob/global.h"
#include "gob/init.h"
#include "gob/video.h"
#include "gob/sound.h"
#include "gob/timer.h"
#include "gob/sound.h"
#include "gob/game.h"
#include "gob/draw.h"
#include "gob/util.h"
#include "gob/cdrom.h"

namespace Gob {

void game_start(void);

const char *Init::_fontNames[] = { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

Init::Init(GobEngine *vm) : _vm(vm) {
	_palDesc = 0;
}

void Init::findBestCfg(void) {
	_vm->_global->_videoMode = VIDMODE_VGA;
	_vm->_global->_useMouse = _vm->_global->_mousePresent;
	_vm->_global->_soundFlags = MIDI_FLAG | SPEAKER_FLAG | BLASTER_FLAG | ADLIB_FLAG;
}

void Init::cleanup(void) {
	if (_vm->_global->_debugFlag == 0)
		_vm->_gtimer->disableTimer();

	_vm->_video->freeDriver();
	_vm->_video->freeSurfDesc(_vm->_global->_pPrimarySurfDesc);
	_vm->_global->_pPrimarySurfDesc = 0;

	_vm->_snd->speakerOff();

	_vm->_dataio->closeDataFile();

	if (_vm->_global->_sprAllocated != 0)
		warning("cleanup: Allocated sprites left: %d", _vm->_global->_sprAllocated);

	_vm->_snd->stopSound(0);
}

void Init::initGame(char *totName) {
	int16 handle2;
	int16 handle;
	int32 i;
	char *infBuf;
	char *infPtr;
	char *infEnd;
	int16 j;
	char buffer[20];
	int32 varsCount;
/*
src		= byte ptr -2Eh
var_1A		= word ptr -1Ah
var_18		= word ptr -18h
var_16		= dword	ptr -16h
var_12		= word ptr -12h
var_10		= word ptr -10h
handle2		= word ptr -0Eh
fileHandle	= word ptr -0Ch
numFromTot	= word ptr -0Ah
memAvail	= dword	ptr -6
memBlocks	= word ptr -2*/

	_vm->_global->_disableVideoCfg = 0x11;
	_vm->_global->_disableMouseCfg = 0x15;

	soundVideo(1000, 1);

	handle2 = _vm->_dataio->openData("intro.stk");
	if (handle2 >= 0) {
		_vm->_dataio->closeData(handle2);
		_vm->_dataio->openDataFile("intro.stk");
	}

	_vm->_util->initInput();

	_vm->_video->setHandlers();
	_vm->_video->initPrimary(_vm->_global->_videoMode);
	_vm->_global->_mouseXShift = 1;
	_vm->_global->_mouseYShift = 1;

	_vm->_game->_totTextData = 0;
	_vm->_game->_totFileData = 0;
	_vm->_game->_totResourceTable = 0;
	_vm->_global->_inter_variables = 0;
	_vm->_global->_inter_variablesSizes = 0;
	_palDesc = new Video::PalDesc;

	if ((_vm->_global->_videoMode != 0x13) && (_vm->_global->_videoMode != 0x14))
		error("initGame: Only 0x13 or 0x14 video mode is supported!");

	_palDesc->vgaPal = _vm->_draw->_vgaPalette;
	_palDesc->unused1 = _vm->_draw->_unusedPalette1;
	_palDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_video->setFullPalette(_palDesc);

	for (i = 0; i < 8; i++)
		_vm->_draw->_fonts[i] = 0;

	handle = _vm->_dataio->openData("intro.inf");

	if (handle < 0) {
		for (i = 0; i < 4; i++) {
			handle2 = _vm->_dataio->openData(_fontNames[i]);
			if (handle2 >= 0) {
				_vm->_dataio->closeData(handle2);
				_vm->_draw->_fonts[i] =
				    _vm->_util->loadFont(_fontNames[i]);
			}
		}
	} else {
		_vm->_dataio->closeData(handle);

		infPtr = _vm->_dataio->getData("intro.inf");
		infBuf = infPtr;

		infEnd = infBuf + _vm->_dataio->getDataSize("intro.inf");

		for (i = 0; i < 4; i++, infPtr++) {
			for (j = 0; infPtr < infEnd && *infPtr >= ' ';
			    j++, infPtr++)
				buffer[j] = *infPtr;

			buffer[j] = 0;
			strcat(buffer, ".let");
			handle2 = _vm->_dataio->openData(buffer);
			if (handle2 >= 0) {
				_vm->_dataio->closeData(handle2);
				_vm->_draw->_fonts[i] = _vm->_util->loadFont(buffer);
			}

			if (infPtr == infEnd)
				break;

			infPtr++;
			if (infPtr == infEnd)
				break;
		}
		delete[] infBuf;
	}

	if (totName != 0) {
		strcpy(buffer, totName);
		strcat(buffer, ".tot");
	} else {
		strcpy(buffer, _vm->_startTot);
	}

	handle = _vm->_dataio->openData(buffer);

	if (handle >= 0) {
		// Get variables count
		_vm->_dataio->seekData(handle, 0x2c, SEEK_SET);
		_vm->_dataio->readData(handle, (char *)&varsCount, 2);
		varsCount = FROM_LE_16(varsCount);
		_vm->_dataio->closeData(handle);

		_vm->_global->_inter_variables = new char[varsCount * 4];
		_vm->_global->_inter_variablesSizes = new byte[varsCount * 4];
		_vm->_global->clearVars(varsCount);
		WRITE_VAR(58, 1);

		strcpy(_vm->_game->_curTotFile, buffer);

		_vm->_cdrom->testCD(1, "GOB");
		_vm->_cdrom->readLIC("gob.lic");
		_vm->_game->start();

		_vm->_cdrom->stopPlaying();
		_vm->_cdrom->freeLICbuffer();

		delete[] _vm->_global->_inter_variables;
		delete[] _vm->_global->_inter_variablesSizes;
		delete[] _vm->_game->_totFileData;
		if (_vm->_game->_totTextData) {
			if (_vm->_game->_totTextData->items)
				delete[] _vm->_game->_totTextData->items;
			delete _vm->_game->_totTextData;
		}
		if (_vm->_game->_totResourceTable) {
			delete[] _vm->_game->_totResourceTable->items;
			delete _vm->_game->_totResourceTable;
		}
	}

	for (i = 0; i < 4; i++) {
		if (_vm->_draw->_fonts[i] != 0)
			_vm->_util->freeFont(_vm->_draw->_fonts[i]);
	}

	delete _palDesc;
	_vm->_dataio->closeDataFile();
	_vm->_video->initPrimary(-1);
	cleanup();
}

}				// End of namespace Gob
