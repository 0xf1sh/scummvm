/* Copyright (C) 1994-2004 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/d_sound.h"

namespace Sword2 {

int32 Logic::fnTestFunction(int32 *params) {
	// params:	0 address of a flag
	return IR_CONT;
}

int32 Logic::fnTestFlags(int32 *params) {
	// params:	0 value of flag
	return IR_CONT;
}

int32 Logic::fnGosub(int32 *params) {
	// params:	0 id of script

	// Hurray, script subroutines. Logic goes up - pc is saved for current
	// level.
	logicUp(params[0]);
	return IR_GOSUB;
}

int32 Logic::fnNewScript(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of script

	_scriptVars[PLAYER_ACTION] = 0;		// must clear this
	logicReplace(params[0]);
	return IR_TERMINATE;
}

int32 Logic::fnInteract(int32 *params) {
	// Run targets action on a subroutine. Called by player on his base
	// level 0 idle, for example.

	// params:	0 id of target from which we derive action script
	//		  reference

	_scriptVars[PLAYER_ACTION] = 0;		// must clear this
	logicUp((params[0] < 16) + 2);		// 3rd script of clicked on id

	// Out, up and around again - pc is saved for current level to be
	// returned to.
	return IR_GOSUB;
}

int32 Logic::fnPreLoad(int32 *params) {
	// Forces a resource into memory before it's "officially" opened for
	// use. eg. if an anim needs to run on smoothly from another,
	// "preloading" gets it into memory in advance to avoid the cacheing
	// delay that normally occurs before the first frame.

	// params:	0 resource to preload

	_vm->_resman->openResource(params[0]);
	_vm->_resman->closeResource(params[0]);
	return IR_CONT;
}

int32 Logic::fnPreFetch(int32 *params) {
	// Go fetch resource in the background.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

int32 Logic::fnFetchWait(int32 *params) {
	// Fetches a resource in the background but prevents the script from
	// continuing until the resource is in memory.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

int32 Logic::fnRelease(int32 *params) {
	// Releases a resource from memory. Used for freeing memory for
	// sprites that have just been used and will not be used again.
	// Sometimes it is better to kick out a sprite straight away so that
	// the memory can be used for more frequent animations.

	// params:	0 resource to release [guess]

	return IR_CONT;
}

int32 Logic::fnRandom(int32 *params) {
	// params:	0 min
	//		1 max

	_scriptVars[RESULT] = _vm->_rnd.getRandomNumberRng(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	// NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle
	// quit, 2 gives 2 cycles, etc.

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	if (ob_logic->looping == 0) {
		ob_logic->looping = 1;
		ob_logic->pause = params[1];
	}

	if (ob_logic->pause) {
		ob_logic->pause--;
		return IR_REPEAT;
	}

	ob_logic->looping = 0;
	return IR_CONT;
}

int32 Logic::fnRandomPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 minimum number of game-cycles to pause
	//		2 maximum number of game-cycles to pause

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);
	int32 pars[2];

	if (ob_logic->looping == 0) {
		pars[0] = params[1];
		pars[1] = params[2];
		fnRandom(pars);
		pars[1] = _scriptVars[RESULT];
	}

	pars[0] = params[0];
	return fnPause(pars);
}

int32 Logic::fnPassGraph(int32 *params) {
	// makes an engine local copy of passed ObjectGraphic - run script 4
	// of an object to request this used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params:	0 pointer to an ObjectGraphic structure

	warning("fnPassGraph() is a no-op now");
	return IR_CONT;
}

int32 Logic::fnPassMega(int32 *params) {
	// makes an engine local copy of passed graphic_structure and
	// mega_structure - run script 4 of an object to request this
	// used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params: 	0 pointer to a mega structure

	memcpy(&_engineMega, _vm->_memory->decodePtr(params[0]), sizeof(ObjectMega));
	return IR_CONT;
}

int32 Logic::fnSetValue(int32 *params) {
	// temp. function!

	// used for setting far-referenced megaset resource field in mega
	// object, from start script

	// params:	0 pointer to object's mega structure
	//		1 value to set it to

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[0]);

	ob_mega->megaset_res = params[1];
	return IR_CONT;
}

#ifdef _SWORD2_DEBUG
#define BLACK	0
#define WHITE	1
#define RED	2
#define GREEN	3
#define BLUE	4

static uint8 black[4]	= {  0,    0,   0,   0 };
static uint8 white[4]	= { 255, 255, 255,   0 };
static uint8 red[4]	= { 255,   0,   0,   0 };
static uint8 green[4]	= {   0, 255,   0,   0 };
static uint8 blue[4]	= {   0,   0, 255,   0 };
#endif

int32 Logic::fnFlash(int32 *params) {
	// flash colour 0 (ie. border) - useful during script development
	// eg. fnFlash(BLUE) where a text line is missed; RED when some code
	// missing, etc

	// params:	0 colour to flash

#ifdef _SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case WHITE:
		_vm->_graphics->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_graphics->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_graphics->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_graphics->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}

	// There used to be a busy-wait loop here, so I don't know how long
	// the delay was meant to be. Probably doesn't matter much.

	_vm->_graphics->updateDisplay();
	_vm->_system->delay_msecs(250);
	_vm->_graphics->setPalette(0, 1, black, RDPAL_INSTANT);
#endif

	return IR_CONT;
}


int32 Logic::fnColour(int32 *params) {
	// set border colour - useful during script development
	// eg. set to colour during a timer situation, then black when timed
	// out

	// params	0: colour (see defines above)

#ifdef _SWORD2_DEBUG
	// what colour?
	switch (params[0]) {
	case BLACK:
		_vm->_graphics->setPalette(0, 1, black, RDPAL_INSTANT);
		break;
	case WHITE:
		_vm->_graphics->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_graphics->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_graphics->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_graphics->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}
#endif

	return IR_CONT;
}

int32 Logic::fnDisplayMsg(int32 *params) {
	// Display a message to the user on the screen.

	// params:	0 Text number of message to be displayed.

	uint32 local_text = params[0] & 0xffff;
	uint32 text_res = params[0] / SIZE;

	// Display message for three seconds.

	// +2 to skip the encoded text number in the first 2 chars; 3 is
	// duration in seconds

	_vm->displayMsg(_vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text) + 2, 3);
	_vm->_resman->closeResource(text_res);

	return IR_CONT;
}

int32 Logic::fnResetGlobals(int32 *params) {
	// fnResetGlobals is used by the demo - so it can loop back & restart
	// itself

	// params:	none

	int32 size;
	uint32 *globals;

	size = _vm->_resman->fetchLen(1);
	size -= sizeof(StandardHeader);

	debug(5, "globals size: %d", size);

	globals = (uint32 *) ((byte *) _vm->_resman->openResource(1) + sizeof(StandardHeader));

	// blank each global variable
	memset(globals, 0, size);

	_vm->_resman->closeResource(1);

	// all objects but george
	_vm->_resman->killAllObjects(false);

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from fnInitBackground

	// switch on scrolling (2 means first time on screen)
	_vm->_thisScreen.scroll_flag = 2;

	return IR_CONT;
}

// FIXME:
//
// The original credits used a different font. I think it's stored in the
// font.clu file, but I don't know how to interpret it.
//
// The original used the entire screen. This version cuts off the top and
// bottom of the screen, because that's where the menus would usually be.
//
// The original had some sort of smoke effect at the bottom of the screen.

enum {
	LINE_LEFT,
	LINE_CENTER,
	LINE_RIGHT
};

struct CreditsLine {
	char *str;
	byte type;
	int top;
	int height;
	byte *sprite;
};

#define CREDITS_FONT_HEIGHT 25
#define CREDITS_LINE_SPACING 20

int32 Logic::fnPlayCredits(int32 *params) {
	uint32 loopingMusicId = _vm->_loopingMusicId;

	// This function just quits the game if this is the playable demo, ie.
	// credits are NOT played in the demo any more!

	// params:	none

	if (_scriptVars[DEMO]) {
		_vm->closeGame();
		return IR_STOP;
	}

	// Prepare for the credits by fading down, stoping the music, etc.

	_vm->setMouse(0);

	_vm->_sound->muteFx(true);
	_vm->_sound->muteSpeech(true);

	_vm->_graphics->waitForFade();
	_vm->_graphics->fadeDown();
	_vm->_graphics->waitForFade();

	_vm->_graphics->closeMenuImmediately();

	// There are three files which I believe are involved in showing the
	// credits:
	//
	// credits.bmp  - The "Smacker" logo, stored as follows:
	//
	//     width     2 bytes, little endian
	//     height    2 bytes, little endian
	//     palette   3 * 256 bytes
	//     data      width * height bytes
	//
	//     Note that the maximum colour component in the palette is 0x3F.
	//     This is the same resolution as the _paletteMatch table. I doubt
	//     that this is a coincidence, but let's use the image palette
	//     directly anyway, just to be safe.
	//
	// credits.clu  - The credits text
	//
	//     This is simply a text file with CRLF line endings.
	//     '^' is not shown, but used to mark the center of the line.
	//     '@' is used as a placeholder for the "Smacker" logo. At least
	//     when it appears alone.
	//     Remaining lines are centered.
	//
	// fonts.clu    - The credits font?
	//
	//     FIXME: At this time I don't know how to interpret fonts.clu. For
	//     now, let's just the standard speech font instead.

	SpriteInfo spriteInfo;
	File f;
	int i;

	// Read the "Smacker" logo

	uint16 logoWidth = 0;
	uint16 logoHeight = 0;
	byte *logoData = NULL;
	byte palette[256 * 4];

	if (f.open("credits.bmp")) {
		logoWidth = f.readUint16LE();
		logoHeight = f.readUint16LE();

		for (i = 0; i < 256; i++) {
			palette[i * 4 + 0] = f.readByte() << 2;
			palette[i * 4 + 1] = f.readByte() << 2;
			palette[i * 4 + 2] = f.readByte() << 2;
			palette[i * 4 + 3] = 0;
		}

		logoData = (byte *) malloc(logoWidth * logoHeight);

		f.read(logoData, logoWidth * logoHeight);
		f.close();
	} else {
		warning("Can't find credits.bmp");
		memset(palette, 0, sizeof(palette));
		palette[14 * 4 + 0] = 252;
		palette[14 * 4 + 1] = 252;
		palette[14 * 4 + 2] = 252;
		palette[14 * 4 + 3] = 0;
	}

	_vm->_graphics->setPalette(0, 256, palette, RDPAL_INSTANT);

	// Read the credits text

	// This should be plenty
	CreditsLine creditsLines[350];

	for (i = 0; i < ARRAYSIZE(creditsLines); i++) {
		creditsLines[i].str = NULL;
		creditsLines[i].sprite = NULL;
	}

	char textLine[80];

	if (!f.open("credits.clu")) {
		warning("Can't find credits.clu");
		return IR_CONT;
	}

	int lineTop = 400;
	int lineCount = 0;
	int pos = 0;

	textLine[0] = 0;

	int paragraphStart = 0;
	bool hasCenterMark = false;

	while (1) {
		if (lineCount >= ARRAYSIZE(creditsLines)) {
			warning("Too many credits lines");
			break;
		}

		byte b = f.readByte();

		if (f.ioFailed())
			break;

		// Remember that the current paragraph has at least once center
		// mark. If a paragraph has no center marks, it should be
		// centered.

		if (b == '^')
			hasCenterMark = true;

		if (b == '^' && pos != 0) {
			textLine[pos] = 0;

			creditsLines[lineCount].top = lineTop;
			creditsLines[lineCount].height = CREDITS_FONT_HEIGHT;
			creditsLines[lineCount].type = LINE_LEFT;
			creditsLines[lineCount].str = strdup(textLine);

			lineCount++;
			textLine[0] = '^';
			pos = 1;
		} else if (b == 0x0a) {
			creditsLines[lineCount].top = lineTop;

			if (textLine[0] == '^') {
				creditsLines[lineCount].str = strdup(textLine + 1);
				creditsLines[lineCount].type = LINE_RIGHT;
			} else {
				creditsLines[lineCount].str = strdup(textLine);
				creditsLines[lineCount].type = LINE_LEFT;
			}

			if (strcmp(textLine, "@") == 0) {
				creditsLines[lineCount].height = logoHeight;
				lineTop += logoHeight;
			} else {
				creditsLines[lineCount].height = CREDITS_FONT_HEIGHT;
				lineTop += CREDITS_LINE_SPACING;
			}

			if (strlen(textLine) > 0)
				lineCount++;
			else {
				if (!hasCenterMark)
					for (int j = paragraphStart; j < lineCount; j++)
						creditsLines[j].type = LINE_CENTER;

				paragraphStart = lineCount;
				hasCenterMark = false;
			}

			pos = 0;
		} else if (b == 0x0d) {
			textLine[pos++] = 0;
		} else
			textLine[pos++] = b;
	}

	f.close();

	// The paragraph detection above won't find the last paragraph, so we
	// have to deal with it separately.

	if (!hasCenterMark)
		for (int j = paragraphStart; j < lineCount; j++)
			creditsLines[j].type = LINE_CENTER;

	// We could easily add some ScummVM stuff to the credits, if we wanted
	// to. On the other hand, anyone with the attention span to actually
	// read all the credits probably already knows. :-)

	// Start the music and roll the credits

	// The credits music (which can also be heard briefly in the "carib"
	// cutscene) is played once.

	int32 pars[2];

	pars[0] = 309;
	pars[1] = FX_SPOT;
	fnPlayMusic(pars);

	_vm->_graphics->clearScene();
	_vm->_graphics->fadeUp(0);

	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight = 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;

	int startLine = 0;
	int scrollPos = 0;

	bool abortCredits = false;

	int scrollSteps = lineTop + CREDITS_FONT_HEIGHT;
	uint32 musicStart = _vm->_system->get_msecs();

	// Ideally the music should last just a tiny bit longer than the
	// credits. Note that musicTimeRemaining() will return 0 if the music
	// is muted, so we need a sensible fallback for that case.

	uint32 musicLength = MAX((int32) (1000 * (_vm->_sound->musicTimeRemaining() - 3)), 25 * (int32) scrollSteps);

	while (scrollPos < scrollSteps && !_vm->_quit) {
		bool foundStartLine = false;

		_vm->_graphics->clearScene();

		for (i = startLine; i < lineCount; i++) {
			// Free any sprites that have scrolled off the screen

			if (creditsLines[i].top + creditsLines[i].height < scrollPos) {
				if (creditsLines[i].sprite) {
					free(creditsLines[i].sprite);
					creditsLines[i].sprite = NULL;
					debug(2, "Freeing sprite '%s'", creditsLines[i].str);
				}
				if (creditsLines[i].str) {
					free(creditsLines[i].str);
					creditsLines[i].str = NULL;
				}
			} else if (creditsLines[i].top < scrollPos + 400) {
				if (!foundStartLine) {
					startLine = i;
					foundStartLine = true;
				}

				if (!creditsLines[i].sprite) {
					debug(2, "Creating sprite '%s'", creditsLines[i].str);
					creditsLines[i].sprite = _vm->_fontRenderer->makeTextSprite((byte *) creditsLines[i].str, 600, 14, _vm->_speechFontId, 0);
				}

				FrameHeader *frame = (FrameHeader *) creditsLines[i].sprite;

				spriteInfo.y = creditsLines[i].top - scrollPos;
				spriteInfo.w = frame->width;
				spriteInfo.h = frame->height;
				spriteInfo.data = creditsLines[i].sprite + sizeof(FrameHeader);

				switch (creditsLines[i].type) {
				case LINE_LEFT:
					spriteInfo.x = 640 / 2 - 5 - frame->width;
					break;
				case LINE_RIGHT:
					spriteInfo.x = 640 / 2 + 5;
					break;
				case LINE_CENTER:
					if (strcmp(creditsLines[i].str, "@") == 0) {
						spriteInfo.data = logoData;
						spriteInfo.x = (640 - logoWidth) / 2;
						spriteInfo.w = logoWidth;
						spriteInfo.h = logoHeight;
					} else
						spriteInfo.x = (640 - frame->width) / 2;
					break;
				}

				if (spriteInfo.data)
					_vm->_graphics->drawSprite(&spriteInfo);
			} else
				break;
		}

		_vm->_graphics->updateDisplay();

		KeyboardEvent *ke = _vm->keyboardEvent();

		if (ke && ke->keycode == 27) {
			if (!abortCredits) {
				abortCredits = true;
				_vm->_graphics->fadeDown();
			}
		}

		if (abortCredits && _vm->_graphics->getFadeStatus() == RDFADE_BLACK)
			break;

		_vm->sleepUntil(musicStart + (musicLength * scrollPos) / scrollSteps);
		scrollPos++;
	}

	// We're done. Clean up and try to put everything back where it was
	// before the credits.

	for (i = 0; i < lineCount; i++) {
		if (creditsLines[i].str)
			free(creditsLines[i].str);
		if (creditsLines[i].sprite)
			free(creditsLines[i].sprite);
	}

	if (logoData)
		free(logoData);

	if (!abortCredits) {
		// The music should either have stopped or be about to stop, so
		// wait for it to really happen.

		while (_vm->_sound->musicTimeRemaining() && !_vm->_quit) {
			_vm->_graphics->updateDisplay(false);
			_vm->_system->delay_msecs(100);
		}
	}

	if (_vm->_quit)
		return IR_CONT;

	_vm->_sound->muteFx(false);
	_vm->_sound->muteSpeech(false);

	if (loopingMusicId) {
		pars[0] = loopingMusicId;
		pars[1] = FX_LOOP;
		fnPlayMusic(pars);
	} else
		fnStopMusic(NULL);

	_vm->_thisScreen.new_palette = 99;

	if (!_vm->_mouseStatus || _choosing)
		_vm->setMouse(NORMAL_MOUSE_ID);

	if (_scriptVars[DEAD])
		_vm->buildSystemMenu();

	return IR_CONT;
}

} // End of namespace Sword2
