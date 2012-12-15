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

#include "common/system.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/rect.h"
#include "engines/util.h"
#include "hopkins/anim.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

AnimationManager::AnimationManager() {
	_clearAnimationFl = false;
	NO_SEQ = false;
	NO_COUL = false;
}

/**
 * Play Animation
 * @param filename		Filename of animation to play
 * @param rate1			Delay amount before starting animation
 * @param rate2			Delay amount between animation frames
 * @param rate3			Delay amount after animation finishes
 */
void AnimationManager::playAnim(const Common::String &filename, uint32 rate1, uint32 rate2, uint32 rate3) {
	bool breakFlag;
	bool hasScreenCopy;
	byte *screenCopy = NULL;
	byte *screenP = NULL;
	int frameNumber;
	byte *ptr = NULL;
	size_t nbytes;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	hasScreenCopy = false;
	screenP = _vm->_graphicsManager.VESA_SCREEN;
	ptr = _vm->_globals.allocMemory(0x14u);

	_vm->_fileManager.constructFilename(_vm->_globals.HOPANM, filename);
	if (!f.open(_vm->_globals.NFICHIER))
		error("File not found - %s", _vm->_globals.NFICHIER.c_str());

	f.skip(6);
	f.read(_vm->_graphicsManager.Palette, 0x320u);
	f.skip(4);
	nbytes = f.readUint32LE();
	f.skip(14);
	f.read(screenP, nbytes);

	if (_clearAnimationFl) {
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
	}
	if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
		hasScreenCopy = true;
		screenCopy = _vm->_globals.allocMemory(0x4B000u);
		memcpy(screenCopy, screenP, 0x4B000u);
	}
	if (NO_SEQ) {
		if (hasScreenCopy)
			memcpy(screenCopy, _vm->_graphicsManager.VESA_BUFFER, 0x4B000u);
		_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
	} else {
		_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
		_vm->_graphicsManager.DD_Lock();
		if (hasScreenCopy)
			_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(screenP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
	}
	_vm->_eventsManager._rateCounter = 0;
	_vm->_eventsManager._escKeyFl = false;
	_vm->_soundManager.LOAD_ANM_SOUND();

	if (_vm->_globals.iRegul == 1) {
		// Do pre-animation delay
		do {
			if (_vm->_eventsManager._escKeyFl)
				goto EXIT;

			_vm->_eventsManager.refreshEvents();
		} while (!_vm->shouldQuit() && _vm->_eventsManager._rateCounter < rate1);
	}

	_vm->_eventsManager._rateCounter = 0;
	breakFlag = false;
	frameNumber = 0;
	while (!_vm->shouldQuit()) {
		++frameNumber;
		_vm->_soundManager.playAnim_SOUND(frameNumber);

		// Read frame header
		if (f.read(ptr, 16) != 16)
			breakFlag = true;

		if (strncmp((char *)ptr, "IMAGE=", 6))
			breakFlag = true;
		if (breakFlag)
			break;

		f.read(screenP, READ_LE_UINT32(ptr + 8));

		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager._escKeyFl)
					goto EXIT;

				_vm->_eventsManager.refreshEvents();
				_vm->_soundManager.VERIF_SOUND();
			} while (!_vm->shouldQuit() && _vm->_eventsManager._rateCounter < rate2);
		}

		_vm->_eventsManager._rateCounter = 0;
		_vm->_graphicsManager.DD_Lock();
		if (hasScreenCopy) {
			if (*screenP != kByteStop) {
				_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);
				_vm->_graphicsManager.m_scroll16A(screenCopy, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
		} else if (*screenP != kByteStop) {
			_vm->_graphicsManager.Copy_Video_Vbe16(screenP);
		}
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		_vm->_soundManager.VERIF_SOUND();
	}

	if (_vm->_globals.iRegul == 1) {
		// Do post-animation delay
		do {
			if (_vm->_eventsManager._escKeyFl)
				break;

			_vm->_eventsManager.refreshEvents();
			_vm->_soundManager.VERIF_SOUND();
		} while (_vm->_eventsManager._rateCounter < rate3);
	}

	_vm->_eventsManager._rateCounter = 0;
	_vm->_soundManager.VERIF_SOUND();
EXIT:
	if (_vm->_graphicsManager.FADE_LINUX == 2 && !hasScreenCopy) {
		screenCopy = _vm->_globals.allocMemory(0x4B000u);

		f.seek(0);
		f.skip(6);
		f.read(_vm->_graphicsManager.Palette, 0x320u);
		f.skip(4);
		nbytes = f.readUint32LE();
		f.skip(14);
		f.read(screenP, nbytes);

		memcpy(screenCopy, screenP, 0x4B000u);

		breakFlag = false;
		do {
			memset(ptr, 0, 20);

			if (f.read(ptr, 16) != 16)
				breakFlag = true;
			if (strncmp((char *)ptr, "IMAGE=", 6))
				breakFlag = true;

			if (!breakFlag) {
				f.read(screenP, READ_LE_UINT32(ptr + 8));
				if (*screenP != kByteStop)
					_vm->_graphicsManager.Copy_WinScan_Vbe3(screenP, screenCopy);
			}
		} while (breakFlag);
		_vm->_graphicsManager.FADE_OUTW_LINUX(screenCopy);
		screenCopy = _vm->_globals.freeMemory(screenCopy);
	}
	if (hasScreenCopy) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.FADE_OUTW_LINUX(screenCopy);
		screenCopy = _vm->_globals.freeMemory(screenCopy);
	}

	_vm->_graphicsManager.FADE_LINUX = 0;
	f.close();
	ptr = _vm->_globals.freeMemory(ptr);
	_vm->_graphicsManager.NOLOCK = false;
}

/**
 * Play Animation, type 2
 */
void AnimationManager::playAnim2(const Common::String &filename, uint32 a2, uint32 a3, uint32 a4) {
	int v5;
	int v8;
	byte *ptr;
	int v11;
	byte *v12;
	byte *v13;
	int v15;
	size_t nbytes;
	byte buf[6];
	Common::File f;

	if (_vm->shouldQuit())
		return;

	v8 = 0;
	while (!_vm->shouldQuit()) {
		memcpy(_vm->_graphicsManager.OLD_PAL, _vm->_graphicsManager.Palette, 0x301u);

		_vm->_fileManager.constructLinuxFilename("TEMP.SCR");

		if (_vm->_graphicsManager.nbrligne == SCREEN_WIDTH)
			_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x4B000u);
		else if (_vm->_graphicsManager.nbrligne == 1280)
			_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x96000u);
		if (!_vm->_graphicsManager.nbrligne)
			_vm->_graphicsManager.ofscroll = 0;

		v12 = _vm->_graphicsManager.VESA_SCREEN;
		v13 = _vm->_globals.allocMemory(0x14u);
		_vm->_fileManager.constructFilename(_vm->_globals.HOPANM, filename);

		if (!f.open(_vm->_globals.NFICHIER))
			error("Error opening file - %s", _vm->_globals.NFICHIER.c_str());

		f.read(&buf, 6);
		f.read(_vm->_graphicsManager.Palette, 0x320u);
		f.read(&buf, 4);
		nbytes = f.readUint32LE();
		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.read(v12, nbytes);

		_vm->_graphicsManager.Cls_Pal();
		v11 = _vm->_graphicsManager.SCROLL;
		_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH);
		_vm->_graphicsManager.SCROLL_ECRAN(0);
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.max_x = SCREEN_WIDTH;
		if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
			v8 = 1;
			ptr = _vm->_globals.allocMemory(0x4B000u);
			memcpy(ptr, v12, 0x4B000u);
		}
		if (_vm->_animationManager.NO_SEQ) {
			if (v8 == 1)
				memcpy(ptr, _vm->_graphicsManager.VESA_BUFFER, 0x4B000u);
			_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
		} else {
			_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
			_vm->_graphicsManager.DD_Lock();
			if (v8)
				_vm->_graphicsManager.m_scroll16A(ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager.m_scroll16(v12, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			_vm->_graphicsManager.DD_Unlock();
			_vm->_graphicsManager.DD_VBL();
		}
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul != 1)
			break;
		for (;;) {
			if (_vm->_eventsManager._escKeyFl == true)
				goto LABEL_114;
			if (redrawAnim() == true)
				break;
			_vm->_eventsManager.refreshEvents();
			if (_vm->_eventsManager._rateCounter >= a2)
				goto LABEL_48;
		}
		if (_vm->_graphicsManager.NOLOCK == true)
			goto LABEL_114;
		if (v8 == 1)
			ptr = _vm->_globals.freeMemory(ptr);
		_vm->_globals.freeMemory(v13);
		f.close();

		_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
		g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

		memcpy(_vm->_graphicsManager.Palette, _vm->_graphicsManager.OLD_PAL, 0x301u);
		_vm->_graphicsManager.Cls_Pal();
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.SCROLL = v11;
		_vm->_graphicsManager.SCROLL_ECRAN(v11);
		if (_vm->_graphicsManager.DOUBLE_ECRAN == true) {
			_vm->_graphicsManager.SCANLINE(0x500u);
			_vm->_graphicsManager.max_x = 1280;
			_vm->_graphicsManager.DD_Lock();
			if (_vm->_graphicsManager.SDL_ECHELLE)
				_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		} else {
			_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH * 2);
			_vm->_graphicsManager.max_x = SCREEN_WIDTH;
			_vm->_graphicsManager.DD_Lock();
			_vm->_graphicsManager.Cls_Video();
			if (_vm->_graphicsManager.SDL_ECHELLE)
				_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		}
LABEL_112:
		_vm->_graphicsManager.DD_Unlock();
		_vm->_eventsManager.VBL();
		_vm->_graphicsManager.FADE_INS();
	}
LABEL_48:
	_vm->_eventsManager._rateCounter = 0;
	v5 = 0;
	v15 = 0;
	for (;;) {
		++v15;
		_vm->_soundManager.playAnim_SOUND(v15);
		memset(&buf, 0, 6u);
		memset(v13, 0, 0x13u);

		if (f.read(v13, 0x10) != 0x10)
			v5 = -1;

		if (strncmp((const char *)v13, "IMAGE=", 6))
			v5 = -1;

		if (v5)
			goto LABEL_88;
		f.read(v12, READ_LE_UINT32(v13 + 8));
		if (_vm->_globals.iRegul == 1)
			break;
LABEL_77:
		_vm->_eventsManager._rateCounter = 0;
		_vm->_graphicsManager.DD_Lock();
		if (v8) {
			if (*v12 != kByteStop) {
				_vm->_graphicsManager.Copy_WinScan_Vbe3(v12, ptr);
				_vm->_graphicsManager.m_scroll16A(ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
		} else if (*v12 != kByteStop) {
			_vm->_graphicsManager.Copy_Video_Vbe16(v12);
		}
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		_vm->_soundManager.VERIF_SOUND();
LABEL_88:
		if (v5 == -1) {
			if (_vm->_globals.iRegul == 1) {
				while (_vm->_eventsManager._escKeyFl != true) {
					if (redrawAnim() == true) {
						if (_vm->_graphicsManager.NOLOCK == true)
							goto LABEL_114;
						if (v8 == 1)
							ptr = _vm->_globals.freeMemory(ptr);
						_vm->_globals.freeMemory(v13);
						f.close();

						_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
						g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

						memcpy(_vm->_graphicsManager.Palette, _vm->_graphicsManager.OLD_PAL, 0x301u);
						_vm->_graphicsManager.Cls_Pal();
						_vm->_graphicsManager.DD_Lock();
						_vm->_graphicsManager.Cls_Video();
						_vm->_graphicsManager.DD_Unlock();
						_vm->_graphicsManager.SCROLL = v11;
						_vm->_graphicsManager.SCROLL_ECRAN(v11);
						if (_vm->_graphicsManager.DOUBLE_ECRAN == true) {
							_vm->_graphicsManager.SCANLINE(0x500u);
							_vm->_graphicsManager.max_x = 1280;
							_vm->_graphicsManager.DD_Lock();
							if (_vm->_graphicsManager.SDL_ECHELLE)
								_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
							else
								_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
						} else {
							_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH * 2);
							_vm->_graphicsManager.max_x = SCREEN_WIDTH;
							_vm->_graphicsManager.DD_Lock();
							_vm->_graphicsManager.Cls_Video();
							if (_vm->_graphicsManager.SDL_ECHELLE)
								_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
							else
								_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
						}
						goto LABEL_112;
					}
					_vm->_eventsManager.refreshEvents();
					_vm->_soundManager.VERIF_SOUND();
					if (_vm->_eventsManager._rateCounter >= a4)
						goto LABEL_114;
				}
			}
			goto LABEL_114;
		}
	}
	while (_vm->_eventsManager._escKeyFl != true) {
		if (redrawAnim() == true) {
			if (_vm->_graphicsManager.NOLOCK == true)
				break;
			if (v8 == 1)
				ptr = _vm->_globals.freeMemory(ptr);
			_vm->_globals.freeMemory(v13);
			f.close();

			_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
			g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

			memcpy(_vm->_graphicsManager.Palette, _vm->_graphicsManager.OLD_PAL, 0x301u);
			_vm->_graphicsManager.Cls_Pal();
			_vm->_graphicsManager.DD_Lock();
			_vm->_graphicsManager.Cls_Video();
			_vm->_graphicsManager.DD_Unlock();
			_vm->_graphicsManager.SCROLL = v11;
			_vm->_graphicsManager.SCROLL_ECRAN(v11);
			if (_vm->_graphicsManager.DOUBLE_ECRAN == true) {
				_vm->_graphicsManager.SCANLINE(0x500u);
				_vm->_graphicsManager.max_x = 1280;
				_vm->_graphicsManager.DD_Lock();
				if (_vm->_graphicsManager.SDL_ECHELLE)
					_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				else
					_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			} else {
				_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH * 2);
				_vm->_graphicsManager.max_x = SCREEN_WIDTH;
				_vm->_graphicsManager.DD_Lock();
				_vm->_graphicsManager.Cls_Video();
				if (_vm->_graphicsManager.SDL_ECHELLE)
					_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				else
					_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
			goto LABEL_112;
		}
		_vm->_eventsManager.refreshEvents();
		_vm->_soundManager.VERIF_SOUND();
		if (_vm->_eventsManager._rateCounter >= a3)
			goto LABEL_77;
	}
LABEL_114:
	_vm->_graphicsManager.NOLOCK = false;
	f.close();

	if (_vm->_graphicsManager.FADE_LINUX == 2 && !v8) {
		byte *ptra;
		ptra = _vm->_globals.allocMemory(0x4B000u);

		f.seek(0);
		f.read(&buf, 6);
		f.read(_vm->_graphicsManager.Palette, 0x320u);
		f.read(&buf, 4u);
		nbytes = f.readUint32LE();

		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.read(v12, nbytes);
		memcpy(ptra, v12, 0x4B000u);

		int v6 = 0;
		do {
			memset(&buf, 0, 6u);
			memset(v13, 0, 0x13u);
			if (f.read(v13, 16) != 16)
				v6 = -1;
			if (strncmp((const char *)v13, "IMAGE=", 6))
				v6 = -1;

			if (!v6) {
				f.read(v12, READ_LE_UINT32(v13 + 8));
				if (*v12 != kByteStop)
					_vm->_graphicsManager.Copy_WinScan_Vbe3(v12, ptra);
			}
		} while (v6 != -1);
		_vm->_graphicsManager.FADE_OUTW_LINUX(ptra);
		ptra = _vm->_globals.freeMemory(ptra);
	}
	if (v8 == 1) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.FADE_OUTW_LINUX(ptr);
		_vm->_globals.freeMemory(ptr);
	}
	_vm->_graphicsManager.FADE_LINUX = 0;
	_vm->_globals.freeMemory(v13);

	_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	memcpy(_vm->_graphicsManager.Palette, _vm->_graphicsManager.OLD_PAL, 0x301u);
	_vm->_graphicsManager.Cls_Pal();
	_vm->_graphicsManager.DD_Lock();
	_vm->_graphicsManager.Cls_Video();
	_vm->_graphicsManager.DD_Unlock();
	_vm->_graphicsManager.SCROLL = v11;
	_vm->_graphicsManager.SCROLL_ECRAN(v11);
	if (_vm->_graphicsManager.DOUBLE_ECRAN == true) {
		_vm->_graphicsManager.SCANLINE(0x500u);
		_vm->_graphicsManager.max_x = 1280;
		_vm->_graphicsManager.DD_Lock();
		if (_vm->_graphicsManager.SDL_ECHELLE)
			_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, _vm->_eventsManager._startPos.x, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	} else {
		_vm->_graphicsManager.SCANLINE(SCREEN_WIDTH);
		_vm->_graphicsManager.max_x = SCREEN_WIDTH;
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		if (_vm->_graphicsManager.SDL_ECHELLE)
			_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	}
	_vm->_graphicsManager.DD_Unlock();
	_vm->_graphicsManager.FADE_INS();
	_vm->_graphicsManager.DD_VBL();
}

bool AnimationManager::redrawAnim() {
	return false;
}

/**
 * Load Animation
 */
void AnimationManager::loadAnim(const Common::String &animName) {
	byte v20[15];
	char header[10];
	char filename1[15];
	char filename2[15];
	char filename3[15];
	char filename4[15];
	char filename5[15];
	char filename6[15];

	clearAnim();

	Common::String filename = animName + ".ANI";
	_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, filename);

	Common::File f;
	if (!f.open(_vm->_globals.NFICHIER))
		error("Failed to open %s", _vm->_globals.NFICHIER.c_str());

	int filesize = f.size();
	int nbytes = filesize - 115;
	f.read(header, 10);
	f.read(v20, 15);
	f.read(filename1, 15);
	f.read(filename2, 15);
	f.read(filename3, 15);
	f.read(filename4, 15);
	f.read(filename5, 15);
	f.read(filename6, 15);

	if (header[0] != 'A' || header[1] != 'N' || header[2] != 'I' || header[3] != 'S')
		error("File incompatible with this soft.");

	const char *files[6] = { &filename1[0], &filename2[0], &filename3[0], &filename4[0],
			&filename5[0], &filename6[0] };

	for (int idx = 1; idx <= 6; ++idx) {
		if (files[idx - 1][0]) {
			_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, files[idx - 1]);

			if (!f.exists(_vm->_globals.NFICHIER))
				error("File not found");
			if (loadSpriteBank(idx, files[idx - 1]))
				error("File not compatible with this soft.");
		}
	}

	byte *data = _vm->_globals.allocMemory(nbytes + 1);
	f.read(data, nbytes);
	f.close();

	for (int idx = 1; idx <= 20; ++idx) {
		searchAnim(data, idx, nbytes);
	}

	_vm->_globals.freeMemory(data);
}

/**
 * Clear animation
 */
void AnimationManager::clearAnim() {
	for (int idx = 0; idx < 35; ++idx) {
		_vm->_globals.Bqe_Anim[idx].data = _vm->_globals.freeMemory(_vm->_globals.Bqe_Anim[idx].data);
		_vm->_globals.Bqe_Anim[idx].field4 = 0;
	}

	for (int idx = 0; idx < 8; ++idx) {
		_vm->_globals.Bank[idx].data = _vm->_globals.freeMemory(_vm->_globals.Bank[idx].data);
		_vm->_globals.Bank[idx].field4 = 0;
		_vm->_globals.Bank[idx].filename1 = "";
		_vm->_globals.Bank[idx].fileHeader = 0;
		_vm->_globals.Bank[idx].field1C = 0;
	}
}

/**
 * Load Sprite Bank
 */
int AnimationManager::loadSpriteBank(int idx, const Common::String &filename) {
	byte *v3;
	byte *v4;
	byte *v13;
	byte *ptr;
	byte *v19;
	int result = 0;
	_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, filename);
	_vm->_globals.Bank[idx].field1C = _vm->_fileManager.fileSize(_vm->_globals.NFICHIER);
	_vm->_globals.Bank[idx].field4 = 1;
	_vm->_globals.Bank[idx].filename1 = filename;
	_vm->_globals.Bank[idx].filename2 = _vm->_globals.REP_SPR;

	v3 = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	v4 = v3;

	_vm->_globals.Bank[idx].fileHeader = 0;
	if (*(v3 + 1) == 'L' && *(v3 + 2) == 'E')
	    _vm->_globals.Bank[idx].fileHeader = 1;
	if (*(v3 + 1) == 'O' && *(v3 + 2) == 'R')
		_vm->_globals.Bank[idx].fileHeader = 2;

	if (_vm->_globals.Bank[idx].fileHeader) {
		_vm->_globals.Bank[idx].data = v3;

		bool loopCond = false;
		int v8 = 0;
		int width;
		int height;
		do {
			ptr = v4;
			width = _vm->_objectsManager.getWidth(v4, v8);
			height = _vm->_objectsManager.getHeight(ptr, v8);
			v4 = ptr;
			if (!width && !height)
				loopCond = true;
			if (!loopCond)
				++v8;
			if (v8 > 249)
				loopCond = true;
		} while (!loopCond);

		if (v8 <= 249) {
			_vm->_globals.Bank[idx].field1A = v8;

			Common::String ofsFilename = _vm->_globals.Bank[idx].filename1;
			char ch;
			do {
				ch = ofsFilename.lastChar();
				ofsFilename.deleteLastChar();
			} while (ch != '.');
			ofsFilename += ".OFS";

			_vm->_fileManager.constructFilename(_vm->_globals.HOPANIM, ofsFilename);
			Common::File f;
			if (f.exists(_vm->_globals.NFICHIER)) {
				v19 = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
				v13 = v19;

				if (_vm->_globals.Bank[idx].field1A > 0) {
					for (int objIdx = 0; objIdx < _vm->_globals.Bank[idx].field1A; ++objIdx) {
						int x1 = (int16)READ_LE_UINT16(v13);
						int y1 = (int16)READ_LE_UINT16(v13 + 2);
						int x2 = (int16)READ_LE_UINT16(v13 + 4);
						int y2 = (int16)READ_LE_UINT16(v13 + 6);
						v13 += 8;

						_vm->_objectsManager.set_offsetxy(_vm->_globals.Bank[idx].data, objIdx, x1, y1, 0);
						if (_vm->_globals.Bank[idx].fileHeader == 2)
							_vm->_objectsManager.set_offsetxy(_vm->_globals.Bank[idx].data, objIdx, x2, y2, 1);
					}
				}

				_vm->_globals.freeMemory(v19);
			}

			result = 0;
		} else {
			_vm->_globals.freeMemory(ptr);
			_vm->_globals.Bank[idx].field4 = 0;
			result = -2;
		}
	} else {
		_vm->_globals.freeMemory(v3);
		_vm->_globals.Bank[idx].field4 = 0;
		result = -1;
	}

	return result;
}

/**
 * Search Animation
 */
void AnimationManager::searchAnim(const byte *data, int animIndex, int count) {
	int v3;
	const byte *v5;
	int v6;
	int v7;
	int v8;
	byte *v9;
	int v10;
	int v11;
	int v12;
	int v13;
	int v15;
	int v16;
	int v17;
	int v19;
	int v20;
	int v21;
	int v22;
	const byte *v23;
	int v;

	v21 = 0;
	v3 = 0;
	v19 = animIndex;
	do {
		v20 = *(v21 + data);
		if (v20 == 'A' && *(data + v21 + 1) == 'N' && *(data + v21 + 2) == 'I' && *(data + v21 + 3) == 'M') {
			int entryIndex = *(data + v21 + 4);
			if (animIndex == entryIndex) {
				v5 = v21 + data + 5;
				v6 = v21 + 5;
				v7 = 0;
				v8 = 0;
				do {
					if (*v5 == 'A' && *(v5 + 1) == 'N' && *(v5 + 2) == 'I' && *(v5 + 3) == 'M')
						v8 = 1;
					if (*v5 == 'F' && *(v5 + 1) == 'I' && *(v5 + 2) == 'N')
						v8 = 1;
					if (count < v6) {
						_vm->_globals.Bqe_Anim[animIndex].field4 = 0;
						_vm->_globals.Bqe_Anim[v19].data = g_PTRNUL;
						return;
					}
					++v6;
					++v7;
					++v5;
				} while (v8 != 1);
				_vm->_globals.Bqe_Anim[v19].data = _vm->_globals.allocMemory(v7 + 50);
				_vm->_globals.Bqe_Anim[animIndex].field4 = 1;
				memcpy(_vm->_globals.Bqe_Anim[v19].data, v21 + data + 5, 0x14u);

				byte *dataP = _vm->_globals.Bqe_Anim[v19].data;
				v9 = dataP + 20;
				v23 = v21 + data + 25;
				v10 = READ_LE_UINT16(v21 + data + 25);
				v11 = READ_LE_UINT16(v21 + data + 27);
				v22 = READ_LE_UINT16(v21 + data + 29);
				v12 = READ_LE_UINT16(v21 + data + 31);
				v13 = *(v21 + data + 33);
				*(dataP + 29) = *(v21 + data + 34);
				WRITE_LE_UINT16(dataP + 20, v10);
				WRITE_LE_UINT16(dataP + 22, v11);
				WRITE_LE_UINT16(dataP + 24, v22);
				WRITE_LE_UINT16(dataP + 26, v12);
				*(dataP + 28) = v13;

				for (int v14 = 1; v14 <= 4999; v14++) {
					v9 += 10;
					v23 += 10;
					if (!v22)
						break;

					v = READ_LE_UINT16(v23);
					v15 = READ_LE_UINT16(v23 + 2);
					v22 = READ_LE_UINT16(v23 + 4);
					v16 = READ_LE_UINT16(v23 + 6);
					v17 = *(v23 + 8);
					*(v9 + 9) = *(v23 + 9);
					WRITE_LE_UINT16(v9, v);
					WRITE_LE_UINT16(v9 + 2, v15);
					WRITE_LE_UINT16(v9 + 4, v22);
					WRITE_LE_UINT16(v9 + 6, v16);
					*(v9 + 8) = v17;
				}
				v3 = 1;
			}
		}
		if (v20 == 'F' && *(data + v21 + 1) == 'I' && *(data + v21 + 2) == 'N')
			v3 = 1;
		++v21;
	} while (v21 <= count && v3 != 1);
}

/**
 * Play sequence
 */
void AnimationManager::playSequence(const Common::String &file, uint32 rate1, uint32 rate2, uint32 rate3) {
	bool readError;
	int v7;
	byte *ptr = NULL;
	byte *v9;
	byte *v10;
	int soundNumber;
	size_t nbytes;
	Common::File f;

	if (_vm->shouldQuit())
		return;

	v7 = 0;
	_vm->_eventsManager._mouseFl = false;
	if (!NO_COUL) {
		_vm->_eventsManager.VBL();

		_vm->_fileManager.constructLinuxFilename("TEMP.SCR");
		if (_vm->_graphicsManager.nbrligne == SCREEN_WIDTH)
			_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x4B000u);
		else if (_vm->_graphicsManager.nbrligne == (SCREEN_WIDTH * 2))
			_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x96000u);
		if (!_vm->_graphicsManager.nbrligne)
			_vm->_graphicsManager.ofscroll = 0;
	}
	v9 = _vm->_graphicsManager.VESA_SCREEN;
	v10 = _vm->_globals.allocMemory(0x16u);
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSEQ, file);
	if (!f.open(_vm->_globals.NFICHIER))
		error("Error opening file - %s", _vm->_globals.NFICHIER.c_str());

	f.skip(6);
	f.read(_vm->_graphicsManager.Palette, 0x320u);
	f.skip(4);
	nbytes = f.readUint32LE();
	f.skip(14);
	f.read(v9, nbytes);

	if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
		v7 = 1;
		ptr = _vm->_globals.allocMemory(0x4B000u);
		memcpy(ptr, v9, 0x4B000u);
	}
	if (_vm->_animationManager.NO_SEQ) {
		if (v7 == 1)
			memcpy(ptr, _vm->_graphicsManager.VESA_BUFFER, 0x4B000u);
		if (!_vm->getIsDemo()) {
			_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
			_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
		}
		_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
	} else {
		_vm->_graphicsManager.DD_Lock();
		if (v7)
			_vm->_graphicsManager.m_scroll16A(ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(v9, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
	}
	if (_vm->getIsDemo()) {
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager._escKeyFl == true) {
					if (!_vm->_eventsManager._disableEscKeyFl)
						goto LABEL_59;
					_vm->_eventsManager._escKeyFl = false;
				}
				_vm->_eventsManager.refreshEvents();
				_vm->_soundManager.VERIF_SOUND();
			} while (_vm->_eventsManager._rateCounter < rate1);
		}
	} else {
		if (NO_COUL)
			_vm->_graphicsManager.FADE_INW_LINUX(v9);
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul == 1) {
			do {
				if (_vm->_eventsManager._escKeyFl) {
					if (!_vm->_eventsManager._disableEscKeyFl)
						goto LABEL_59;
					_vm->_eventsManager._escKeyFl = false;
				}
				_vm->_eventsManager.refreshEvents();
				_vm->_soundManager.VERIF_SOUND();
			} while (_vm->_eventsManager._rateCounter < rate1);
		}
	}
	_vm->_eventsManager._rateCounter = 0;
	readError = false;
	soundNumber = 0;
	do {
		++soundNumber;
		_vm->_soundManager.playAnim_SOUND(soundNumber);
		memset(v10, 0, 0x13u);
		if (f.read(v10, 16) != 16)
			readError = true;

		if (strncmp((const char *)v10, "IMAGE=", 6))
			readError = true;
		if (!readError) {
			f.read(v9, READ_LE_UINT32(v10 + 8));
			if (_vm->_globals.iRegul == 1) {
				do {
					if (_vm->_eventsManager._escKeyFl == true) {
						if (!_vm->_eventsManager._disableEscKeyFl)
							goto LABEL_59;
						_vm->_eventsManager._escKeyFl = false;
					}
					_vm->_eventsManager.refreshEvents();
					_vm->_soundManager.VERIF_SOUND();
				} while (_vm->_eventsManager._rateCounter < rate2);
			}
			_vm->_eventsManager._rateCounter = 0;
			_vm->_graphicsManager.DD_Lock();
			if (v7) {
				if (*v9 != kByteStop) {
					_vm->_graphicsManager.Copy_WinScan_Vbe(v9, ptr);
					_vm->_graphicsManager.m_scroll16A(ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
				}
			} else if (*v9 != kByteStop) {
				_vm->_graphicsManager.Copy_Video_Vbe16a(v9);
			}
			_vm->_graphicsManager.DD_Unlock();
			_vm->_graphicsManager.DD_VBL();
			_vm->_soundManager.VERIF_SOUND();
		}
	} while (!readError);

	if (_vm->_globals.iRegul == 1) {
		do {
			if (_vm->_eventsManager._escKeyFl == true) {
				if (!_vm->_eventsManager._disableEscKeyFl)
					goto LABEL_59;
				_vm->_eventsManager._escKeyFl = false;
			}
			_vm->_eventsManager.refreshEvents();
			_vm->_soundManager.VERIF_SOUND();
		} while (_vm->_eventsManager._rateCounter < rate3);
	}
	_vm->_eventsManager._rateCounter = 0;
LABEL_59:
	_vm->_graphicsManager.NOLOCK = false;
	f.close();

	if (!NO_COUL) {
		_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
		g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

		_vm->_eventsManager._mouseFl = true;
	}
	if (v7 == 1)
		_vm->_globals.freeMemory(ptr);
	_vm->_globals.freeMemory(v10);
}

/**
 * Play Sequence type 2
 */
void AnimationManager::playSequence2(const Common::String &file, uint32 rate1, uint32 rate2, uint32 rate3) {
	bool v4;
	int v7;
	byte *ptr = NULL;
	byte *v10;
	byte *v11 = NULL;
	int v13;
	size_t nbytes;
	Common::File f;

	v7 = 0;
	for (;;) {
		if (_vm->shouldQuit())
			return;

		_vm->_eventsManager._mouseFl = false;
		v10 = _vm->_graphicsManager.VESA_SCREEN;
		v11 = _vm->_globals.allocMemory(0x16u);
		_vm->_fileManager.constructFilename(_vm->_globals.HOPSEQ, file);

		if (!f.open(_vm->_globals.NFICHIER))
			error("File not found - %s", _vm->_globals.NFICHIER.c_str());

		f.skip(6);
		f.read(_vm->_graphicsManager.Palette, 0x320u);
		f.skip(4);
		nbytes = f.readUint32LE();
		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.read(v10, nbytes);

		if (_vm->_graphicsManager.WinScan / 2 > SCREEN_WIDTH) {
			v7 = 1;
			ptr = _vm->_globals.allocMemory(0x4B000u);
			memcpy((void *)ptr, v10, 0x4B000u);
		}
		if (_vm->_animationManager.NO_SEQ) {
			if (v7 == 1) {
				assert(ptr != NULL);
				memcpy((void *)ptr, _vm->_graphicsManager.VESA_BUFFER, 0x4B000u);
			}
			_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
		} else {
			_vm->_graphicsManager.DD_Lock();
			_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
			if (v7)
				_vm->_graphicsManager.m_scroll16A(ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			else
				_vm->_graphicsManager.m_scroll16(v10, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			_vm->_graphicsManager.DD_Unlock();
			_vm->_graphicsManager.DD_VBL();
		}
		_vm->_eventsManager._rateCounter = 0;
		_vm->_eventsManager._escKeyFl = false;
		_vm->_soundManager.LOAD_ANM_SOUND();
		if (_vm->_globals.iRegul != 1)
			break;
		while (!_vm->shouldQuit()) {
			if (_vm->_eventsManager._escKeyFl == true)
				goto LABEL_54;
			if (redrawAnim() == true)
				break;
			_vm->_eventsManager.refreshEvents();
			_vm->_soundManager.VERIF_SOUND();
			if (_vm->_eventsManager._rateCounter >= rate1)
				goto LABEL_23;
		}
LABEL_48:
		if (_vm->_graphicsManager.NOLOCK == true)
			goto LABEL_54;
		if (v7 == 1)
			ptr = _vm->_globals.freeMemory(ptr);
		_vm->_globals.freeMemory(v11);
		f.close();
	}
LABEL_23:
	_vm->_eventsManager._rateCounter = 0;
	v4 = false;
	v13 = 0;
	while (!_vm->shouldQuit()) {
		_vm->_soundManager.playAnim_SOUND(v13++);

		memset(v11, 0, 0x13u);
		if (f.read(v11, 16) != 16)
			v4 = true;

		if (strncmp((const char *)v11, "IMAGE=", 6))
			v4 = true;
		if (v4)
			goto LABEL_44;
		f.read(v10, READ_LE_UINT32(v11 + 8));
		if (_vm->_globals.iRegul == 1)
			break;
LABEL_33:
		_vm->_eventsManager._rateCounter = 0;
		_vm->_graphicsManager.DD_Lock();
		if (v7) {
			if (*v10 != kByteStop) {
				_vm->_graphicsManager.Copy_WinScan_Vbe(v10, ptr);
				_vm->_graphicsManager.m_scroll16A(ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
			}
		} else if (*v10 != kByteStop) {
			_vm->_graphicsManager.Copy_Video_Vbe16a(v10);
		}
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		_vm->_soundManager.VERIF_SOUND();
LABEL_44:
		if (v4) {
			if (_vm->_globals.iRegul == 1) {
				while (_vm->_eventsManager._escKeyFl != true) {
					if (redrawAnim() == true)
						goto LABEL_48;
					_vm->_eventsManager.refreshEvents();
					_vm->_soundManager.VERIF_SOUND();
					if (_vm->_eventsManager._rateCounter >= rate3)
						goto LABEL_53;
				}
			} else {
LABEL_53:
				_vm->_eventsManager._rateCounter = 0;
			}
			goto LABEL_54;
		}
	}
	while (_vm->_eventsManager._escKeyFl != true) {
		_vm->_eventsManager.refreshEvents();
		if (redrawAnim() == true)
			goto LABEL_48;
		if (_vm->_eventsManager._rateCounter >= rate2)
			goto LABEL_33;
	}
LABEL_54:
	if (_vm->_graphicsManager.FADE_LINUX == 2 && !v7) {
		byte *ptra = _vm->_globals.allocMemory(0x4B000u);

		f.seek(0);
		f.skip(6);
		f.read(_vm->_graphicsManager.Palette, 0x320u);
		f.skip(4);
		nbytes = f.readUint32LE();

		f.readUint32LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.read(v10, nbytes);

		memcpy(ptra, v10, 0x4B000u);
		bool v5 = false;
		do {
			memset(v11, 0, 0x13u);
			if (f.read(v11, 16) != 16)
				v5 = true;

			if (strncmp((const char *)v11, "IMAGE=", 6))
				v5 = true;
			if (!v5) {
				f.read(v10, READ_LE_UINT32(v11 + 8));
				if (*v10 != kByteStop)
					_vm->_graphicsManager.Copy_WinScan_Vbe(v10, ptra);
			}
		} while (!v5);
		_vm->_graphicsManager.FADE_OUTW_LINUX(ptra);
		ptra = _vm->_globals.freeMemory(ptra);
	}
	if (v7 == 1) {
		if (_vm->_graphicsManager.FADE_LINUX == 2)
			_vm->_graphicsManager.FADE_OUTW_LINUX(ptr);
		_vm->_globals.freeMemory(ptr);
	}
	_vm->_graphicsManager.FADE_LINUX = 0;

	f.close();
	_vm->_globals.freeMemory(v11);
	_vm->_eventsManager._mouseFl = true;
}

} // End of namespace Hopkins
