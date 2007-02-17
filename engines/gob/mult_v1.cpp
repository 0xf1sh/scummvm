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
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/anim.h"
#include "gob/draw.h"
#include "gob/palanim.h"

namespace Gob {

Mult_v1::Mult_v1(GobEngine *vm) : Mult(vm) {
}

void Mult_v1::loadMult(int16 resId) {
	int16 palIndex;
	int16 i, j;
	char *extData;

	_multData = new Mult_Data;
	memset(_multData, 0, sizeof(Mult_Data));

	_multData->sndSlotsCount = 0;
	_multData->frameStart = 0;
	extData = _vm->_game->loadExtData(resId, 0, 0);
	Common::MemoryReadStream data((byte *) extData, 4294967295U);

	_multData->staticCount = data.readSByte() + 1;
	_multData->animCount = data.readSByte() + 1;

	for (i = 0; i < _multData->staticCount; i++, data.seek(14, SEEK_CUR)) {
		_multData->staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_multData->staticIndices[i] >= 100) {
			_multData->staticIndices[i] -= 100;
			_multData->staticLoaded[i] = 1;
		} else {
			_multData->staticLoaded[i] = 0;
		}
	}

	for (i = 0; i < _multData->animCount; i++, data.seek(14, SEEK_CUR)) {
		_multData->animIndices[i] = _vm->_scenery->loadAnim(1);

		if (_multData->animIndices[i] >= 100) {
			_multData->animIndices[i] -= 100;
			_multData->animLoaded[i] = 1;
		} else {
			_multData->animLoaded[i] = 0;
		}
	}

	_multData->frameRate = data.readSint16LE();
	_multData->staticKeysCount = data.readSint16LE();
	_multData->staticKeys = new Mult_StaticKey[_multData->staticKeysCount];
	for (i = 0; i < _multData->staticKeysCount; i++) {
		_multData->staticKeys[i].frame = data.readSint16LE();
		_multData->staticKeys[i].layer = data.readSint16LE();
	}

	for (j = 0; j < 4; j++) {
		_multData->animKeysCount[j] = data.readSint16LE();
		_multData->animKeys[j] = new Mult_AnimKey[_multData->animKeysCount[j]];
		for (i = 0; i < _multData->animKeysCount[j]; i++) {
			_multData->animKeys[j][i].frame = data.readSint16LE();
			_multData->animKeys[j][i].layer = data.readSint16LE();
			_multData->animKeys[j][i].posX = data.readSint16LE();
			_multData->animKeys[j][i].posY = data.readSint16LE();
			_multData->animKeys[j][i].order = data.readSint16LE();
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			_fadePal[palIndex][i].red = data.readByte();
			_fadePal[palIndex][i].green = data.readByte();
			_fadePal[palIndex][i].blue = data.readByte();
		}
	}

	_multData->palFadeKeysCount = data.readSint16LE();
	_multData->palFadeKeys = new Mult_PalFadeKey[_multData->palFadeKeysCount];
	for (i = 0; i < _multData->palFadeKeysCount; i++) {
		_multData->palFadeKeys[i].frame = data.readSint16LE();
		_multData->palFadeKeys[i].fade = data.readSint16LE();
		_multData->palFadeKeys[i].palIndex = data.readSint16LE();
		_multData->palFadeKeys[i].flag = data.readSByte();
	}

	_multData->palKeysCount = data.readSint16LE();
	_multData->palKeys = new Mult_PalKey[_multData->palKeysCount];
	for (i = 0; i < _multData->palKeysCount; i++) {
		_multData->palKeys[i].frame = data.readSint16LE();
		_multData->palKeys[i].cmd = data.readSint16LE();
		_multData->palKeys[i].rates[0] = data.readSint16LE();
		_multData->palKeys[i].rates[1] = data.readSint16LE();
		_multData->palKeys[i].rates[2] = data.readSint16LE();
		_multData->palKeys[i].rates[3] = data.readSint16LE();
		_multData->palKeys[i].unknown0 = data.readSint16LE();
		_multData->palKeys[i].unknown1 = data.readSint16LE();
		data.read(_multData->palKeys[i].subst, 64);
	}

	_multData->textKeysCount = data.readSint16LE();
	_multData->textKeys = new Mult_TextKey[_multData->textKeysCount];
	for (i = 0; i < _multData->textKeysCount; i++) {
		_multData->textKeys[i].frame = data.readSint16LE();
		_multData->textKeys[i].cmd = data.readSint16LE();
		for (int k = 0; k < 9; ++k)
			_multData->textKeys[i].unknown0[k] = data.readSint16LE();
		_multData->textKeys[i].index = data.readSint16LE();
		_multData->textKeys[i].unknown1[0] = data.readSint16LE();
		_multData->textKeys[i].unknown1[1] = data.readSint16LE();
	}

	_multData->sndKeysCount = data.readSint16LE();
	_multData->sndKeys = new Mult_SndKey[_multData->sndKeysCount];
	for (i = 0; i < _multData->sndKeysCount; i++) {
		_multData->sndKeys[i].frame = data.readSint16LE();
		_multData->sndKeys[i].cmd = data.readSint16LE();
		_multData->sndKeys[i].freq = data.readSint16LE();
		_multData->sndKeys[i].fadeLength = data.readSint16LE();
		_multData->sndKeys[i].repCount = data.readSint16LE();
		_multData->sndKeys[i].soundIndex = -1;
		_multData->sndKeys[i].resId = -1;
		data.seek(26, SEEK_CUR);
		switch (_multData->sndKeys[i].cmd) {
		case 1:
		case 4:
			_multData->sndKeys[i].resId = READ_LE_UINT16(_vm->_global->_inter_execPtr);

			for (j = 0; j < i; j++) {
				if (_multData->sndKeys[i].resId ==
				    _multData->sndKeys[j].resId) {
					_multData->sndKeys[i].soundIndex =
					    _multData->sndKeys[j].soundIndex;
					_vm->_global->_inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				_vm->_inter->loadSound(19 - _multData->sndSlotsCount);
				_multData->sndKeys[i].soundIndex =
				    19 - _multData->sndSlotsCount;
				_multData->sndSlotsCount++;
			}
			break;

		case 3:
			_vm->_global->_inter_execPtr += 6;
			break;

		case 5:
			_vm->_global->_inter_execPtr += _multData->sndKeys[i].freq * 2;
			break;
		}
	}

	delete[] extData;
}

void Mult_v1::setMultData(uint16 multindex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::multSub(uint16 multindex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::playMult(int16 startFrame, int16 endFrame, char checkEscape,
	    char handleMouse) {
	char stopNoClear;
	char stop;
	Mult_Object *multObj;
	Mult_AnimData *animData;

	if (_multData == 0)
		return;

	stopNoClear = 0;
	_frame = startFrame;
	if (endFrame == -1)
		endFrame = 32767;

	if (_frame == -1) {
		_doPalSubst = 0;
		_palFadingRed = 0;
		_palFadingGreen = 0;
		_palFadingBlue = 0;

		_oldPalette = _vm->_global->_pPaletteDesc->vgaPal;
		memcpy((char *)_palAnimPalette,
				(char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		if (_vm->_anim->_animSurf == 0) {
			_vm->_util->setFrameRate(_multData->frameRate);
			_vm->_anim->_areaTop = 0;
			_vm->_anim->_areaLeft = 0;
			_vm->_anim->_areaWidth = 320;
			_vm->_anim->_areaHeight = 200;
			_objCount = 4;

			_objects = new Mult_Object[_objCount];
			memset(_objects, 0, _objCount * sizeof(Mult_Object));
			_renderData = new int16[9 * _objCount];
			memset(_renderData, 0, _objCount * 9 * sizeof(int16));

			_animArrayX = new int32[_objCount];
			_animArrayY = new int32[_objCount];

			_animArrayData = new Mult_AnimData[_objCount];
			memset(_animArrayData, 0, _objCount * sizeof(Mult_AnimData));

			for (_counter = 0; _counter < _objCount; _counter++) {
				multObj = &_objects[_counter];

				multObj->pPosX = (int32 *)&_animArrayX[_counter];
				multObj->pPosY = (int32 *)&_animArrayY[_counter];

				multObj->pAnimData = &_animArrayData[_counter];

				animData = multObj->pAnimData;
				animData->isStatic = 1;

				multObj->tick = 0;
				multObj->lastLeft = -1;
				multObj->lastTop = -1;
				multObj->lastRight = -1;
				multObj->lastBottom = -1;
			}

			_vm->_anim->_animSurf =
			    _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);
			_vm->_draw->_spritesArray[22] = _vm->_anim->_animSurf;

			_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
			    0, 0, 319, 199, 0, 0, 0);

			_animDataAllocated = 1;
		} else
			_animDataAllocated = 0;
		_frame = 0;
	}

	do {
		stop = 1;

		if (VAR(58) == 0) {
			stop = drawStatics(stop);
			stop = drawAnims(stop);
		}

		animate();
		if (handleMouse) {
			_vm->_draw->animateCursor(-1);
		} else {
			_vm->_draw->blitInvalidated();
		}

		if (VAR(58) == 0) {
			drawText(&stop, &stopNoClear);
		}

		stop = prepPalAnim(stop);
		doPalAnim();

		stop = doFadeAnim(stop);
		stop = doSoundAnim(stop, _frame);

		if (_frame >= endFrame)
			stopNoClear = 1;

		if (_vm->_snd->_playingSound)
			stop = 0;

		_vm->_util->processInput();
		if (checkEscape && _vm->_util->checkKey() == 0x11b)	// Esc
			stop = 1;

		_frame++;
		_vm->_util->waitEndFrame();
	} while (stop == 0 && stopNoClear == 0 && !_vm->_quitRequested);

	if (stopNoClear == 0) {
		if (_animDataAllocated) {
			delete[] _objects;
			_objects = 0;

			delete[] _renderData;
			_renderData = 0;

			delete[] _animArrayX;
			_animArrayX = 0;

			delete[] _animArrayY;
			_animArrayY = 0;

			delete[] _animArrayData;
			_animArrayData = 0;

			_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
			_vm->_anim->_animSurf = 0;
			_vm->_draw->_spritesArray[22] = 0;

			_animDataAllocated = 0;
		}

		if (_vm->_snd->_playingSound != 0)
			_vm->_snd->stopSound(10);

		WRITE_VAR(57, (uint32)-1);
	} else
		WRITE_VAR(57, _frame - 1 - _multData->frameStart);
}

char Mult_v1::drawStatics(char stop) {
	if (_multData->staticKeys[_multData->staticKeysCount - 1].frame > _frame)
		stop = 0;

	for (_counter = 0; _counter < _multData->staticKeysCount;
	    _counter++) {
		if (_multData->staticKeys[_counter].frame != _frame
		    || _multData->staticKeys[_counter].layer == -1)
			continue;

		_vm->_scenery->_curStaticLayer = _multData->staticKeys[_counter].layer;
		for (_vm->_scenery->_curStatic = 0;
				_vm->_scenery->_curStaticLayer >=
					_vm->_scenery->_statics[_multData->staticIndices[_vm->_scenery->_curStatic]].layersCount;
				_vm->_scenery->_curStatic++) {
			_vm->_scenery->_curStaticLayer -=
			    _vm->_scenery->_statics[_multData->staticIndices[_vm->_scenery->_curStatic]].layersCount;
		}

		_vm->_scenery->_curStatic = _multData->staticIndices[_vm->_scenery->_curStatic];
		_vm->_scenery->renderStatic(_vm->_scenery->_curStatic, _vm->_scenery->_curStaticLayer);
		_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
		    0, 0, 319, 199, 0, 0, 0);
	}
	return stop;
}

char Mult_v1::drawAnims(char stop) {
	Mult_AnimKey *key;
	Mult_Object *animObj;
	int16 i;
	int16 count;

	for (_index = 0; _index < 4; _index++) {
		for (_counter = 0; _counter < _multData->animKeysCount[_index]; _counter++) {
			key = &_multData->animKeys[_index][_counter];
			animObj = &_objects[_index];
			if (key->frame != _frame)
				continue;

			if (key->layer != -1) {
				(*animObj->pPosX) = key->posX;
				(*animObj->pPosY) = key->posY;

				animObj->pAnimData->frame = 0;
				animObj->pAnimData->order = key->order;
				animObj->pAnimData->animType = 1;

				animObj->pAnimData->isPaused = 0;
				animObj->pAnimData->isStatic = 0;
				animObj->pAnimData->maxTick = 0;
				animObj->tick = 0;
				animObj->pAnimData->layer = key->layer;

				count =
					_vm->_scenery->_animations[_multData->animIndices[0]].layersCount;
				i = 0;
				while (animObj->pAnimData->layer >= count) {
					animObj->pAnimData->layer -= count;
					i++;

					count =
						_vm->_scenery->_animations[_multData->animIndices[i]].layersCount;
				}
				animObj->pAnimData->animation = _multData->animIndices[i];
			} else {
				animObj->pAnimData->isStatic = 1;
			}
		}
	}
	return stop;
}

void Mult_v1::drawText(char *pStop, char *pStopNoClear) {
	char *savedIP;

	int16 cmd;
	for (_index = 0; _index < _multData->textKeysCount; _index++) {
		if (_multData->textKeys[_index].frame != _frame)
			continue;

		cmd = _multData->textKeys[_index].cmd;
		if (cmd == 0) {
			*pStop = 0;
		} else if (cmd == 1) {
			*pStopNoClear = 1;
			_multData->frameStart = 0;
		} else if (cmd == 3) {
			*pStop = 0;
			savedIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr =
				(char *)(&_multData->textKeys[_index].index);
			_vm->_global->_inter_execPtr = savedIP;
		}
	}
}

char Mult_v1::prepPalAnim(char stop) {
	_palKeyIndex = -1;
	do {
		_palKeyIndex++;
		if (_palKeyIndex >= _multData->palKeysCount)
			return stop;
	} while (_multData->palKeys[_palKeyIndex].frame != _frame);

	if (_multData->palKeys[_palKeyIndex].cmd == -1) {
		stop = 0;
		_doPalSubst = 0;
		_vm->_global->_pPaletteDesc->vgaPal = _oldPalette;

		memcpy((char *)_palAnimPalette,
				(char *)_vm->_global->_pPaletteDesc->vgaPal, 768);

		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	} else {
		stop = 0;
		_doPalSubst = 1;
		_palAnimKey = _palKeyIndex;

		_palAnimIndices[0] = 0;
		_palAnimIndices[1] = 0;
		_palAnimIndices[2] = 0;
		_palAnimIndices[3] = 0;

		_vm->_global->_pPaletteDesc->vgaPal = _palAnimPalette;
	}
	return stop;
}

void Mult_v1::doPalAnim(void) {
	int16 off;
	int16 off2;
	Video::Color *palPtr;
	Mult_PalKey *palKey;

	if (_doPalSubst == 0)
		return;

	for (_index = 0; _index < 4; _index++) {
		palKey = &_multData->palKeys[_palAnimKey];

		if ((_frame % palKey->rates[_index]) != 0)
			continue;

		_palAnimRed[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].red;
		_palAnimGreen[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].green;
		_palAnimBlue[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].blue;

		while (1) {
			off = palKey->subst[(_palAnimIndices[_index] + 1) % 16][_index];
			if (off == 0) {
				off = palKey->subst[_palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off].red = _palAnimRed[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].green = _palAnimGreen[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].blue = _palAnimBlue[_index];
			} else {
				off = palKey->subst[(_palAnimIndices[_index] + 1) % 16][_index] - 1;
				off2 = palKey->subst[_palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off2].red =
					_vm->_global->_pPaletteDesc->vgaPal[off].red;
				_vm->_global->_pPaletteDesc->vgaPal[off2].green =
					_vm->_global->_pPaletteDesc->vgaPal[off].green;
				_vm->_global->_pPaletteDesc->vgaPal[off2].blue =
					_vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}

			_palAnimIndices[_index] = (_palAnimIndices[_index] + 1) % 16;

			off = palKey->subst[_palAnimIndices[_index]][_index];

			if (off == 0) {
				_palAnimIndices[_index] = 0;
				off = palKey->subst[0][_index] - 1;

				_palAnimRed[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				_palAnimGreen[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				_palAnimBlue[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}
			if (_palAnimIndices[_index] == 0)
				break;
		}
	}

	if (_vm->_global->_colorCount == 256) {
		_vm->_video->waitRetrace(_vm->_global->_videoMode);

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (_counter = 0; _counter < 16; _counter++) {
			_vm->_video->setPalElem(_counter, palPtr->red, palPtr->green,
					palPtr->blue, 0, 0x13);
			palPtr++;
		}

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (_counter = 0; _counter < 16; _counter++) {
			_vm->_global->_redPalette[_counter] = palPtr->red;
			_vm->_global->_greenPalette[_counter] = palPtr->green;
			_vm->_global->_bluePalette[_counter] = palPtr->blue;
			palPtr++;
		}
	} else {
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}
}

char Mult_v1::doFadeAnim(char stop) {
	Mult_PalFadeKey *fadeKey;

	for (_index = 0; _index < _multData->palFadeKeysCount; _index++) {
		fadeKey = &_multData->palFadeKeys[_index];

		if (fadeKey->frame != _frame)
			continue;

		stop = 0;
		if ((fadeKey->flag & 1) == 0) {
			if (fadeKey->fade == 0) {
				_vm->_global->_pPaletteDesc->vgaPal = _fadePal[fadeKey->palIndex];
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			} else {
				_vm->_global->_pPaletteDesc->vgaPal = _fadePal[fadeKey->palIndex];
				_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, 0);
			}
		} else {
			_vm->_global->_pPaletteDesc->vgaPal = _fadePal[fadeKey->palIndex];
			_vm->_palanim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, -1);

			_palFadingRed = (fadeKey->flag >> 1) & 1;
			_palFadingGreen = (fadeKey->flag >> 2) & 1;
			_palFadingBlue = (fadeKey->flag >> 3) & 1;
		}
	}

	if (_palFadingRed) {
		_palFadingRed = !_vm->_palanim->fadeStep(1);
		stop = 0;
	}
	if (_palFadingGreen) {
		_palFadingGreen = !_vm->_palanim->fadeStep(2);
		stop = 0;
	}
	if (_palFadingBlue) {
		_palFadingBlue = !_vm->_palanim->fadeStep(3);
		stop = 0;
	}
	return stop;
}

char Mult_v1::doSoundAnim(char stop, int16 frame) {
	Mult_SndKey *sndKey;
	for (_index = 0; _index < _multData->sndKeysCount; _index++) {
		sndKey = &_multData->sndKeys[_index];
		if (sndKey->frame != frame)
			continue;

		if (sndKey->cmd != -1) {
			if (sndKey->cmd == 1) {
				_vm->_snd->stopSound(0);
				stop = 0;
				playSound(_vm->_game->_soundSamples[sndKey->soundIndex],
						sndKey->repCount, sndKey->freq, sndKey->fadeLength);

			} else if (sndKey->cmd == 4) {
				_vm->_snd->stopSound(0);
				stop = 0;
				playSound(_vm->_game->_soundSamples[sndKey->soundIndex],
						sndKey->repCount, sndKey->freq, sndKey->fadeLength);
			}
		} else {
			if (_vm->_snd->_playingSound)
				_vm->_snd->stopSound(sndKey->fadeLength);
		}
	}
	return stop;
}

void Mult_v1::animate(void) {
	int16 minOrder;
	int16 maxOrder;
	int16 *pCurLefts;
	int16 *pCurRights;
	int16 *pCurTops;
	int16 *pCurBottoms;
	int16 *pDirtyLefts;
	int16 *pDirtyRights;
	int16 *pDirtyTops;
	int16 *pDirtyBottoms;
	int16 *pNeedRedraw;
	Mult_AnimData *pAnimData;
	int16 i, j;
	int16 order;

	if (_renderData == 0)
		return;

	pDirtyLefts = _renderData;
	pDirtyRights = pDirtyLefts + _objCount;
	pDirtyTops = pDirtyRights + _objCount;
	pDirtyBottoms = pDirtyTops + _objCount;
	pNeedRedraw = pDirtyBottoms + _objCount;
	pCurLefts = pNeedRedraw + _objCount;
	pCurRights = pCurLefts + _objCount;
	pCurTops = pCurRights + _objCount;
	pCurBottoms = pCurTops + _objCount;
	minOrder = 100;
	maxOrder = 0;

	//Find dirty areas
	for (i = 0; i < _objCount; i++) {
		pNeedRedraw[i] = 0;
		pDirtyTops[i] = 1000;
		pDirtyLefts[i] = 1000;
		pDirtyBottoms[i] = 1000;
		pDirtyRights[i] = 1000;
		pAnimData = _objects[i].pAnimData;

		if (pAnimData->isStatic == 0 && pAnimData->isPaused == 0 &&
		    _objects[i].tick == pAnimData->maxTick) {
			if (pAnimData->order < minOrder)
				minOrder = pAnimData->order;

			if (pAnimData->order > maxOrder)
				maxOrder = pAnimData->order;

			pNeedRedraw[i] = 1;
			_vm->_scenery->updateAnim(pAnimData->layer, pAnimData->frame,
			    pAnimData->animation, 0,
			    *(_objects[i].pPosX), *(_objects[i].pPosY),
			    0);

			if (_objects[i].lastLeft != -1) {
				pDirtyLefts[i] =
				    MIN(_objects[i].lastLeft,
				    _vm->_scenery->_toRedrawLeft);
				pDirtyTops[i] =
				    MIN(_objects[i].lastTop,
				    _vm->_scenery->_toRedrawTop);
				pDirtyRights[i] =
				    MAX(_objects[i].lastRight,
				    _vm->_scenery->_toRedrawRight);
				pDirtyBottoms[i] =
				    MAX(_objects[i].lastBottom,
				    _vm->_scenery->_toRedrawBottom);
			} else {
				pDirtyLefts[i] = _vm->_scenery->_toRedrawLeft;
				pDirtyTops[i] = _vm->_scenery->_toRedrawTop;
				pDirtyRights[i] = _vm->_scenery->_toRedrawRight;
				pDirtyBottoms[i] = _vm->_scenery->_toRedrawBottom;
			}
			pCurLefts[i] = _vm->_scenery->_toRedrawLeft;
			pCurRights[i] = _vm->_scenery->_toRedrawRight;
			pCurTops[i] = _vm->_scenery->_toRedrawTop;
			pCurBottoms[i] = _vm->_scenery->_toRedrawBottom;
		} else {
			if (_objects[i].lastLeft != -1) {
				if (pAnimData->order < minOrder)
					minOrder = pAnimData->order;

				if (pAnimData->order > maxOrder)
					maxOrder = pAnimData->order;

				if (pAnimData->isStatic)
					*pNeedRedraw = 1;

				pCurLefts[i] = _objects[i].lastLeft;
				pDirtyLefts[i] = _objects[i].lastLeft;

				pCurTops[i] = _objects[i].lastTop;
				pDirtyTops[i] = _objects[i].lastTop;

				pCurRights[i] = _objects[i].lastRight;
				pDirtyRights[i] = _objects[i].lastRight;

				pCurBottoms[i] = _objects[i].lastBottom;
				pDirtyBottoms[i] = _objects[i].lastBottom;
			}
		}
	}

	// Find intersections
	for (i = 0; i < _objCount; i++) {
		pAnimData = _objects[i].pAnimData;
		pAnimData->intersected = 200;

		if (pAnimData->isStatic)
			continue;

		for (j = 0; j < _objCount; j++) {
			if (i == j)
				continue;

			if (_objects[j].pAnimData->isStatic)
				continue;

			if (pCurRights[i] < pCurLefts[j])
				continue;

			if (pCurRights[j] < pCurLefts[i])
				continue;

			if (pCurBottoms[i] < pCurTops[j])
				continue;

			if (pCurBottoms[j] < pCurTops[i])
				continue;

			pAnimData->intersected = j;
			break;
		}
	}

	// Restore dirty areas
	for (i = 0; i < _objCount; i++) {

		if (pNeedRedraw[i] == 0 || _objects[i].lastLeft == -1)
			continue;

		_vm->_draw->_sourceSurface = 22;
		_vm->_draw->_destSurface = 21;
		_vm->_draw->_spriteLeft = pDirtyLefts[i] - _vm->_anim->_areaLeft;
		_vm->_draw->_spriteTop = pDirtyTops[i] - _vm->_anim->_areaTop;
		_vm->_draw->_spriteRight = pDirtyRights[i] - pDirtyLefts[i] + 1;
		_vm->_draw->_spriteBottom = pDirtyBottoms[i] - pDirtyTops[i] + 1;
		_vm->_draw->_destSpriteX = pDirtyLefts[i];
		_vm->_draw->_destSpriteY = pDirtyTops[i];
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(DRAW_BLITSURF);
		_objects[i].lastLeft = -1;
	}

	// Update view
	for (order = minOrder; order <= maxOrder; order++) {
		for (i = 0; i < _objCount; i++) {
			pAnimData = _objects[i].pAnimData;
			if (pAnimData->order != order)
				continue;

			if (pNeedRedraw[i]) {
				if (pAnimData->isStatic == 0) {

					_vm->_scenery->updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 2,
					    *(_objects[i].pPosX),
					    *(_objects[i].pPosY), 1);

					if (_vm->_scenery->_toRedrawLeft != -12345) {
						_objects[i].lastLeft =
						    _vm->_scenery->_toRedrawLeft;
						_objects[i].lastTop =
						    _vm->_scenery->_toRedrawTop;
						_objects[i].lastRight =
						    _vm->_scenery->_toRedrawRight;
						_objects[i].lastBottom =
						    _vm->_scenery->_toRedrawBottom;
					} else {
						_objects[i].lastLeft = -1;
					}
				}
				_vm->_scenery->updateStatic(order + 1);
			} else if (pAnimData->isStatic == 0) {
				for (j = 0; j < _objCount; j++) {
					if (pNeedRedraw[j] == 0)
						continue;

					if (pDirtyRights[i] < pDirtyLefts[j])
						continue;

					if (pDirtyRights[j] < pDirtyLefts[i])
						continue;

					if (pDirtyBottoms[i] < pDirtyTops[j])
						continue;

					if (pDirtyBottoms[j] < pDirtyTops[i])
						continue;

					_vm->_scenery->_toRedrawLeft = pDirtyLefts[j];
					_vm->_scenery->_toRedrawRight = pDirtyRights[j];
					_vm->_scenery->_toRedrawTop = pDirtyTops[j];
					_vm->_scenery->_toRedrawBottom = pDirtyBottoms[j];

					_vm->_scenery->updateAnim(pAnimData->layer,
					    pAnimData->frame,
					    pAnimData->animation, 4,
					    *(_objects[i].pPosX),
					    *(_objects[i].pPosY), 1);

					_vm->_scenery->updateStatic(order + 1);
				}
			}
		}
	}

	// Advance animations
	for (i = 0; i < _objCount; i++) {
		pAnimData = _objects[i].pAnimData;
		if (pAnimData->isStatic || pAnimData->isPaused)
			continue;

		if (_objects[i].tick == pAnimData->maxTick) {
			_objects[i].tick = 0;
			if (pAnimData->animType == 4) {
				pAnimData->isPaused = 1;
				pAnimData->frame = 0;
			} else {
				pAnimData->frame++;
				if (pAnimData->frame >=
				    _vm->_scenery->_animations[(int)pAnimData->animation].
						layers[pAnimData->layer].framesCount) {
					switch (pAnimData->animType) {
					case 0:
						pAnimData->frame = 0;
						break;

					case 1:
						pAnimData->frame = 0;

						*(_objects[i].pPosX) =
						    *(_objects[i].pPosX) +
						    _vm->_scenery->_animations[(int)pAnimData->animation].layers[pAnimData->layer].animDeltaX;

						*(_objects[i].pPosY) =
						    *(_objects[i].pPosY) +
						    _vm->_scenery->_animations[(int)pAnimData->animation].layers[pAnimData->layer].animDeltaY;
						break;

					case 2:
						pAnimData->frame = 0;
						pAnimData->animation =
						    pAnimData->newAnimation;
						pAnimData->layer =
						    pAnimData->newLayer;
						break;

					case 3:
						pAnimData->animType = 4;
						pAnimData->frame = 0;
						break;

					case 5:
						pAnimData->isStatic = 1;
						pAnimData->frame = 0;
						break;

					case 6:
						pAnimData->frame--;
						pAnimData->isPaused = 1;
						break;
					}
					pAnimData->newCycle = 1;
				} else {
					pAnimData->newCycle = 0;
				}
			}
		} else {
			_objects[i].tick++;
		}
	}
}

void Mult_v1::freeMult(void) {
	if ((_vm->_anim->_animSurf != 0) && (_vm->_draw->_spritesArray[22] != 0))
		_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);

	delete[] _objects;
	delete[] _renderData;
	delete[] _orderArray;

	_objects = 0;
	_renderData = 0;
	_orderArray = 0;
	_vm->_anim->_animSurf = 0;
	_vm->_draw->_spritesArray[22] = 0;
}

void Mult_v1::playSound(Snd::SoundDesc * soundDesc, int16 repCount, int16 freq,
	    int16 fadeLength) {
	_vm->_snd->playSample(soundDesc, repCount, freq, fadeLength);
}

void Mult_v1::freeMultKeys(void) {
	int i;

	for (i = 0; i < _multData->staticCount; i++) {

		if (_multData->staticLoaded[i] != 0)
			_vm->_scenery->freeStatic(_multData->staticIndices[i]);
	}

	for (i = 0; i < _multData->animCount; i++) {
		if (_multData->animLoaded[i] != 0)
			_vm->_scenery->freeAnim(_multData->animIndices[i]);
	}

	delete[] _multData->staticKeys;

	for (i = 0; i < 4; i++)
		delete[] _multData->animKeys[i];

	delete[] _multData->palFadeKeys;
	delete[] _multData->palKeys;
	delete[] _multData->textKeys;

	for (i = 0; i < _multData->sndSlotsCount; i++) {
		_vm->_game->freeSoundSlot(19 - i);
	}

	delete[] _multData->sndKeys;

	if (_animDataAllocated != 0) {
		delete[] _objects;
		_objects = 0;

		delete[] _renderData;
		_renderData = 0;

		delete[] _animArrayX;
		_animArrayX = 0;

		delete[] _animArrayY;
		_animArrayY = 0;

		delete[] _animArrayData;
		_animArrayData = 0;

		_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
		_vm->_anim->_animSurf = 0;
		_vm->_draw->_spritesArray[22] = 0;

		_animDataAllocated = 0;
	}

	delete _multData;
	_multData = 0;
}

} // End of namespace Gob
