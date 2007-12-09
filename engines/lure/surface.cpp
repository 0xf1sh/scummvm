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

#include "lure/decode.h"
#include "lure/events.h"
#include "lure/game.h"
#include "lure/lure.h"
#include "lure/room.h"
#include "lure/screen.h"
#include "lure/sound.h"
#include "lure/strings.h"
#include "lure/surface.h"
#include "common/endian.h"

namespace Lure {

// These variables hold resources commonly used by the Surfaces, and must be initialised and freed
// by the static Surface methods initialise and deinitailse

static MemoryBlock *int_font = NULL;
static MemoryBlock *int_dialog_frame = NULL;
static uint8 fontSize[256];
int numFontChars;

void Surface::initialise() {
	int_font = Disk::getReference().getEntry(FONT_RESOURCE_ID);
	int_dialog_frame = Disk::getReference().getEntry(DIALOG_RESOURCE_ID);

	numFontChars = int_font->size() / 8;
	if (numFontChars > 256)
		error("Font data exceeded maximum allowable size");

	// Calculate the size of each font character
	for (int ctr = 0; ctr < numFontChars; ++ctr) {
		byte *pChar = int_font->data() + (ctr * 8);
		fontSize[ctr] = 0;

		for (int yp = 0; yp < FONT_HEIGHT; ++yp)  {
			byte v = *pChar++;

			for (int xp = 0; xp < FONT_WIDTH; ++xp) {
				if ((v & 0x80) && (xp > fontSize[ctr])) 
					fontSize[ctr] = xp;
				v = (v << 1) & 0xff;
			}
		}

		// If character is empty, like for a space, give a default size
		if (fontSize[ctr] == 0) fontSize[ctr] = 2;
	}
}

void Surface::deinitialise() {
	delete int_font;
	delete int_dialog_frame;
}

/*--------------------------------------------------------------------------*/

Surface::Surface(MemoryBlock *src, uint16 wdth, uint16 hght): _data(src), 
		_width(wdth), _height(hght) {
	if ((uint32) (wdth * hght) != src->size())
		error("Surface dimensions do not match size of passed data");
}

Surface::Surface(uint16 wdth, uint16 hght): _data(Memory::allocate(wdth*hght)),
	_width(wdth), _height(hght) {
}

Surface::~Surface() {
	delete _data;
}

void Surface::loadScreen(uint16 resourceId) {
	MemoryBlock *rawData = Disk::getReference().getEntry(resourceId);
	PictureDecoder decoder;
	MemoryBlock *tmpScreen = decoder.decode(rawData, FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH + 1);
	delete rawData;
	empty();

	_data->copyFrom(tmpScreen, 0, MENUBAR_Y_SIZE * FULL_SCREEN_WIDTH, 
		(FULL_SCREEN_HEIGHT - MENUBAR_Y_SIZE) * FULL_SCREEN_WIDTH);
	
	delete tmpScreen;
}

int Surface::writeChar(uint16 x, uint16 y, uint8 ascii, bool transparent, uint8 colour) {
	byte *const addr = _data->data() + (y * _width) + x;

	if ((ascii < 32) || (ascii >= 32 + numFontChars))
		error("Invalid ascii character passed for display '%d'", ascii);
	
	uint8 v;
	byte *pFont = int_font->data() + ((ascii - 32) * 8);
	byte *pDest;
	uint8 charWidth = 0;

	for (int y1 = 0; y1 < 8; ++y1) {
		v = *pFont++;
		pDest = addr + (y1 * _width);

		for (int x1 = 0; x1 < 8; ++x1, ++pDest) {
			if (v & 0x80) {
				*pDest = colour;
				if (x1+1 > charWidth) charWidth = x1 + 1;
			}
			else if (!transparent) *pDest = 0;
			v = (v << 1) & 0xff;
		}
	}

	return charWidth;
}

void Surface::writeString(uint16 x, uint16 y, Common::String line, bool transparent, 
						  uint8 colour, bool varLength) {
	writeSubstring(x, y, line, line.size(), transparent, colour, varLength);
}

void Surface::writeSubstring(uint16 x, uint16 y, Common::String line, int len, 
		  bool transparent, uint8 colour, bool varLength) {

	const char *sPtr = line.c_str();

	for (int index = 0; (index < len) && (*sPtr != '\0'); ++index, ++sPtr) {
		writeChar(x, y, (uint8) *sPtr, transparent, colour);

		// Move to after the character in preparation for the next character
		if (!varLength) x += FONT_WIDTH;
		else x += fontSize[(uint8)*sPtr - 32] + 2;
	}
}

void Surface::transparentCopyTo(Surface *dest) {
	if (dest->width() != _width) 
		error("Incompatible surface sizes for transparent copy");

	byte *pSrc = _data->data();
	byte *pDest = dest->data().data();
	uint16 numBytes = MIN(_height,dest->height()) * FULL_SCREEN_WIDTH;

	while (numBytes-- > 0) {
		if (*pSrc) *pDest = *pSrc;
		
		++pSrc;
		++pDest;
	}
}

void Surface::copyTo(Surface *dest) {
	copyTo(dest, 0, 0);
}

void Surface::copyTo(Surface *dest, uint16 x, uint16 y) {
	if ((x == 0) && (dest->width() == _width)) {
		// Use fast data transfer
		uint32 dataSize = dest->data().size() - (y * _width);
		if (dataSize > _data->size()) dataSize = _data->size();
		dest->data().copyFrom(_data, 0, y * _width, dataSize);
	} else {
		// Use slower transfer
		Rect rect;
		rect.left = 0; rect.top = 0;
		rect.right = _width-1; rect.bottom = _height-1;
		copyTo(dest, rect, x, y);
	}
}

void Surface::copyTo(Surface *dest, const Rect &srcBounds, 
					 uint16 destX, uint16 destY, int transparentColour) {
	int numBytes = srcBounds.right - srcBounds.left + 1;
	if (destX + numBytes > dest->width())
		numBytes = dest->width() - destX;
	if (numBytes <= 0) return;

	for (uint16 y=0; y<=(srcBounds.bottom-srcBounds.top); ++y) {
		const uint32 srcPos = (srcBounds.top + y) * _width + srcBounds.left;
		const uint32 destPos = (destY+y) * dest->width() + destX;

		if (transparentColour == -1) {
			// No trnnsparent colour, so copy all the bytes of the line
			dest->data().copyFrom(_data, srcPos, destPos, numBytes);
		} else {
			byte *pSrc = _data->data() + srcPos;
			byte *pDest = dest->data().data() + destPos;

			int bytesCtr = numBytes;
			while (bytesCtr-- > 0) {
				if (*pSrc != (uint8) transparentColour)
					*pDest = *pSrc;
				++pSrc;
				++pDest;
			}
		}
	}
}

void Surface::copyFrom(MemoryBlock *src, uint32 destOffset) {
	uint32 size = _data->size() - destOffset;
	if (src->size() > size) size = src->size();
	_data->copyFrom(src, 0, destOffset, size);
}

// fillRect
// Fills a rectangular area with a colour

void Surface::fillRect(const Rect &r, uint8 colour) {
	for (int yp = r.top; yp <= r.bottom; ++yp) {
		byte *const addr = _data->data() + (yp * _width) + r.left;
		memset(addr, colour, r.width());
	}
}

// createDialog
// Forms a dialog encompassing the entire surface

void copyLine(byte *pSrc, byte *pDest, uint16 leftSide, uint16 center, uint16 rightSide) {
	// Left area
	memcpy(pDest, pSrc, leftSide); 
	pSrc += leftSide; pDest += leftSide; 
	// Center area
	memset(pDest, *pSrc, center);
	++pSrc; pDest += center; 
	// Right side
	memcpy(pDest, pSrc, rightSide); 
	pSrc += rightSide; pDest += rightSide; 
}

void Surface::createDialog(bool blackFlag) {
	if ((_width < 20) || (_height < 20)) return;

	byte *pSrc = int_dialog_frame->data();
	byte *pDest = _data->data();
	uint16 xCenter = _width - DIALOG_EDGE_SIZE * 2;
	uint16 yCenter = _height - DIALOG_EDGE_SIZE * 2;
	int y;

	// Dialog top
	for (y = 0; y < 9; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE - 2, xCenter + 2, DIALOG_EDGE_SIZE);
		pSrc += (DIALOG_EDGE_SIZE - 2) + 1 + DIALOG_EDGE_SIZE;
		pDest += _width;
	}

	// Dialog sides - note that the same source data gets used for all side lines
	for (y = 0; y < yCenter; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE, xCenter, DIALOG_EDGE_SIZE);
		pDest += _width;
	}
	pSrc += DIALOG_EDGE_SIZE * 2 + 1;

	// Dialog bottom
	for (y = 0; y < 9; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE, xCenter + 1, DIALOG_EDGE_SIZE - 1);
		pSrc += DIALOG_EDGE_SIZE + 1 + (DIALOG_EDGE_SIZE - 1);
		pDest += _width;
	}

	// Final processing - if black flag set, clear dialog inside area
	if (blackFlag) {
		Rect r = Rect(DIALOG_EDGE_SIZE, DIALOG_EDGE_SIZE, 
			_width - DIALOG_EDGE_SIZE, _height-DIALOG_EDGE_SIZE);
		fillRect(r, 0);
	}
}

void Surface::copyToScreen(uint16 x, uint16 y) {
	OSystem &system = *g_system;
	system.copyRectToScreen(_data->data(), _width, x, y, _width, _height);
	system.updateScreen();
}

void Surface::centerOnScreen() {
	OSystem &system = *g_system;

	system.copyRectToScreen(_data->data(), _width, 
		(FULL_SCREEN_WIDTH - _width) / 2, (FULL_SCREEN_HEIGHT - _height) / 2,
		_width, _height);
	system.updateScreen();
}

uint16 Surface::textWidth(const char *s, int numChars) {
	uint16 result = 0;
	if (numChars == 0) numChars = strlen(s);

	while (numChars-- > 0) result += fontSize[*s++ - ' '] + 2;
	return result;
}

void Surface::wordWrap(char *text, uint16 width, char **&lines, uint8 &numLines) {
	numLines = 1;
	uint16 lineWidth = 0;
	char *s;
	bool newLine;

	s = text;

	// Scan through the text and insert NULLs to break the line into allowable widths

	while (*s != '\0') {
		char *wordStart = s;
		while (*wordStart == ' ') ++wordStart;
		char *wordEnd = strchr(wordStart, ' ');
		char *wordEnd2 = strchr(wordStart, '\n');
		if ((!wordEnd) || ((wordEnd2) && (wordEnd2 < wordEnd))) {
			wordEnd = wordEnd2;
			newLine = (wordEnd2 != NULL);
		} else {
			newLine = false;
		}

		if (wordEnd) {
			if (!newLine) --wordEnd;
		} else {
			wordEnd = strchr(s, '\0') - 1;
		}

		uint16 wordSize = textWidth(s, (int) (wordEnd - s + 1));

		if (lineWidth + wordSize > width) {
			// Break word onto next line
			*(wordStart - 1) = '\0';
			++numLines;
			lineWidth = 0;
			wordEnd = wordStart - 1;
		} else if (newLine) {
			// Break on newline
			++numLines;
			*wordEnd = '\0';
			lineWidth = 0;
		} else {
			// Add word's length to total for line
			lineWidth += wordSize;
		}

		s = wordEnd+1;
	}

	// Set up a list for the start of each line 
	lines = (char **) Memory::alloc(sizeof(char *) * numLines);
	lines[0] = text;
	for (int ctr = 1; ctr < numLines; ++ctr) 
		lines[ctr] = strchr(lines[ctr-1], 0) + 1;
}

Surface *Surface::newDialog(uint16 width, uint8 numLines, const char **lines, bool varLength, uint8 colour) {
	Surface *s = new Surface(width, (DIALOG_EDGE_SIZE + 3) * 2 + 
		numLines * (FONT_HEIGHT - 1));
	s->createDialog();

	for (uint8 ctr = 0; ctr < numLines; ++ctr)
		s->writeString(DIALOG_EDGE_SIZE + 3, DIALOG_EDGE_SIZE + 2 + 
			(ctr * (FONT_HEIGHT - 1)), lines[ctr], true, colour, varLength);
	return s;
}

Surface *Surface::newDialog(uint16 width, const char *line, uint8 colour) {
	char **lines;
	char *lineCopy = strdup(line);
	uint8 numLines;
	wordWrap(lineCopy, width - (DIALOG_EDGE_SIZE + 3) * 2, lines, numLines);

	// Create the dialog 
	Surface *result = newDialog(width, numLines, const_cast<const char **>(lines), true, colour);

	// Deallocate used resources
	free(lines);
	free(lineCopy);

	return result;
}

Surface *Surface::getScreen(uint16 resourceId) {
	MemoryBlock *block = Disk::getReference().getEntry(resourceId);
	PictureDecoder d;
	MemoryBlock *decodedData = d.decode(block);
	delete block;
	return new Surface(decodedData, FULL_SCREEN_WIDTH, decodedData->size() / FULL_SCREEN_WIDTH);
}

bool Surface::getString(Common::String &line, int maxSize, bool isNumeric, bool varLength, int16 x, int16 y) {
	OSystem &system = *g_system;
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();
	uint8 bgColour = *(screen.screen().data().data() + (y * FULL_SCREEN_WIDTH) + x);
	String newLine(line);
	bool abortFlag = false;
	bool refreshFlag = false;

	bool vKbdFlag = g_system->hasFeature(OSystem::kFeatureVirtualKeyboard);
	if (!vKbdFlag)
		mouse.cursorOff();

	// Insert a cursor character at the end of the string
	newLine.insertChar('_', newLine.size());

	while (!abortFlag) {
		// Display the string
		screen.screen().writeString(x, y, newLine, true, DIALOG_TEXT_COLOUR, varLength);
		screen.update();
		int stringSize = textWidth(newLine.c_str());

		// Loop until the input string changes
		refreshFlag = false;
		while (!refreshFlag && !abortFlag) {
			abortFlag = events.quitFlag;
			if (abortFlag) break;

			while (events.pollEvent()) {
				if (events.type() == Common::EVENT_KEYDOWN) {
					char ch = events.event().kbd.ascii;
					uint16 keycode = events.event().kbd.keycode;

					if ((keycode == Common::KEYCODE_RETURN) || (keycode == Common::KEYCODE_KP_ENTER)) {
						// Return character
						screen.screen().fillRect(
							Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
						screen.update();
						newLine.deleteLastChar();
						line = newLine;
						if (!vKbdFlag)
							mouse.cursorOn();
						return true;
					}
					else if (keycode == Common::KEYCODE_ESCAPE) {
						// Escape character
						screen.screen().fillRect(
							Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
						screen.update();
						abortFlag = true;
					} else if (keycode == Common::KEYCODE_BACKSPACE) {
						// Delete the last character
						if (newLine.size() == 1) continue;

						screen.screen().fillRect(
							Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
						newLine.deleteChar(newLine.size() - 2);
						refreshFlag = true;

					} else if ((ch >= ' ') && (stringSize + 8 < maxSize)) {
						if (((ch >= '0') && (ch <= '9')) || !isNumeric) {
							screen.screen().fillRect(
								Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
							newLine.insertChar(ch, newLine.size() - 1);
							refreshFlag = true;
						}
					}
				}
			}

			system.updateScreen();
			system.delayMillis(10);
		}
	}

	if (!vKbdFlag)
		mouse.cursorOn();

	return false;
}


/*--------------------------------------------------------------------------*/

void Dialog::show(const char *text) {
	debugC(ERROR_BASIC, kLureDebugStrings, "Dialog::show text=%s", text);
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	Room &room = Room::getReference();
	mouse.cursorOff();

	room.update();
	debugC(ERROR_DETAILED, kLureDebugStrings, "Dialog::show creating dialog");
	Surface *s = Surface::newDialog(INFO_DIALOG_WIDTH, text);
	debugC(ERROR_DETAILED, kLureDebugStrings, "Dialog::show created dialog");
	s->copyToScreen(INFO_DIALOG_X, INFO_DIALOG_Y);
	debugC(ERROR_DETAILED, kLureDebugStrings, "Dialog::show copied to screen");

	// Wait for a keypress or mouse button
	Events::getReference().waitForPress();

	screen.update();
	mouse.cursorOn();

	delete s;
}

void Dialog::show(uint16 stringId, const char *hotspotName, const char *characterName) {
	debugC(ERROR_BASIC, kLureDebugStrings, "Hotspot::showMessage stringId=%xh hotspot=%s, character=%s",
		stringId, hotspotName, characterName);
	char buffer[MAX_DESC_SIZE];
	StringData &sl = StringData::getReference();

	sl.getString(stringId, buffer, hotspotName, characterName);
	show(buffer);
}

void Dialog::show(uint16 stringId) {
	show(stringId, NULL, NULL);
}

/*--------------------------------------------------------------------------*/

TalkDialog::TalkDialog(uint16 characterId, uint16 destCharacterId, uint16 activeItemId, uint16 descId) {
	debugC(ERROR_DETAILED, kLureDebugAnimations, "TalkDialog(chars=%xh/%xh, item=%d, str=%d", 
		characterId, destCharacterId, activeItemId, descId);
	StringData &strings = StringData::getReference();
	Resources &res = Resources::getReference();
	char srcCharName[MAX_DESC_SIZE];
	char destCharName[MAX_DESC_SIZE];
	char itemName[MAX_DESC_SIZE];
	int characterArticle, hotspotArticle = 3;

	_characterId = characterId;
	_destCharacterId = destCharacterId;
	_activeItemId = activeItemId;
	_descId = descId;

	HotspotData *talkingChar = res.getHotspot(characterId);
	HotspotData *destCharacter = (destCharacterId == 0) ? NULL : 
		res.getHotspot(destCharacterId);
	HotspotData *itemHotspot = (activeItemId == 0) ? NULL :
		res.getHotspot(activeItemId);
	assert(talkingChar);

	strings.getString(talkingChar->nameId & 0x1fff, srcCharName);
	characterArticle = (talkingChar->nameId >> 13) + 1;

	strcpy(destCharName, "");
	if (destCharacter != NULL)
		strings.getString(destCharacter->nameId, destCharName);
	strcpy(itemName, "");
	if (itemHotspot != NULL) {
		strings.getString(itemHotspot->nameId & 0x1fff, itemName);
		hotspotArticle = (itemHotspot->nameId >> 13) - 1;
	}

	strings.getString(descId, _desc, itemName, destCharName, hotspotArticle, characterArticle);

	// Apply word wrapping to figure out the needed size of the dialog
	Surface::wordWrap(_desc, TALK_DIALOG_WIDTH - (TALK_DIALOG_EDGE_SIZE + 3) * 2,
		_lines, _numLines);
	_endLine = 0; _endIndex = 0;

	_surface = new Surface(TALK_DIALOG_WIDTH, 
		(_numLines + 1) * FONT_HEIGHT + TALK_DIALOG_EDGE_SIZE * 4);

	// Draw the dialog
	byte *pSrc = res.getTalkDialogData().data();
	byte *pDest = _surface->data().data();
	int xPos, yPos;

	// Handle the dialog top
	for (yPos = 0; yPos < TALK_DIALOG_EDGE_SIZE; ++yPos) {
		*pDest++ = *pSrc++;
		*pDest++ = *pSrc++;

		for (xPos = 0; xPos < TALK_DIALOG_WIDTH - TALK_DIALOG_EDGE_SIZE - 2; ++xPos) 
			*pDest++ = *pSrc;
		++pSrc;

		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrc++;
	}

	// Handle the middle section
	for (yPos = 0; yPos < _surface->height() - TALK_DIALOG_EDGE_SIZE * 2; ++yPos) {
		byte *pSrcTemp = pSrc;

		// Left edge
		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrcTemp++;
		
		// Middle section
		for (xPos = 0; xPos < _surface->width() - TALK_DIALOG_EDGE_SIZE * 2; ++xPos)
			*pDest++ = *pSrcTemp;
		++pSrcTemp;

		// Right edge
		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrcTemp++;
	}

	//  Bottom section
	pSrc += TALK_DIALOG_EDGE_SIZE * 2 + 1;
	for (yPos = 0; yPos < TALK_DIALOG_EDGE_SIZE; ++yPos) {
		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrc++;

		for (xPos = 0; xPos < TALK_DIALOG_WIDTH - TALK_DIALOG_EDGE_SIZE - 2; ++xPos) 
			*pDest++ = *pSrc;
		++pSrc;

		*pDest++ = *pSrc++;
		*pDest++ = *pSrc++;
	}

	_wordCountdown = 0;

	// Write out the character name
	uint16 charWidth = Surface::textWidth(srcCharName);
	_surface->writeString((TALK_DIALOG_WIDTH-charWidth)/2, TALK_DIALOG_EDGE_SIZE + 2,
		srcCharName, true, DIALOG_WHITE_COLOUR);
}

TalkDialog::~TalkDialog() {
	Memory::dealloc(_lines);
	delete _surface;
}

void TalkDialog::copyTo(Surface *dest, uint16 x, uint16 y) {
	if (_endLine < _numLines) {
		if (_wordCountdown > 0) {
			// Handle delay between words
			--_wordCountdown;

		} else {
			// Set a delay before the next word is displayed
			Game &game = Game::getReference();
			_wordCountdown = game.fastTextFlag() ? 0 : 1;

			// Scan forward to find the next word break
			char ch = '\0';
			bool wordFlag = false;

			while (!wordFlag) {
				ch = _lines[_endLine][++_endIndex];
				wordFlag = (ch == ' ') || (ch == '\0');
			}

			// Write out the completed portion of the current line
			_surface->writeSubstring(TALK_DIALOG_EDGE_SIZE + 2, 
				TALK_DIALOG_EDGE_SIZE + 4 + (_endLine + 1) * FONT_HEIGHT,
				_lines[_endLine], _endIndex, true);

			// If at end of line, move to next line for next time
			if (ch == '\0') {
				++_endLine;
				_endIndex = -1;
			}
		}
	}

	_surface->copyTo(dest, x, y);
}

void TalkDialog::saveToStream(Common::WriteStream *stream) {
	stream->writeUint16LE(_characterId);
	stream->writeUint16LE(_destCharacterId);
	stream->writeUint16LE(_activeItemId);
	stream->writeUint16LE(_descId);
	stream->writeSint16LE(_endLine);
	stream->writeSint16LE(_endIndex);
	stream->writeSint16LE(_wordCountdown);

}

TalkDialog *TalkDialog::loadFromStream(Common::ReadStream *stream) {
	uint16 characterId = stream->readUint16LE();
	if (characterId == 0)
		return NULL;

	uint16 destCharacterId = stream->readUint16LE();
	uint16 activeItemId = stream->readUint16LE();
	uint16 descId = stream->readUint16LE(); 

	TalkDialog *dialog = new TalkDialog(characterId, destCharacterId, activeItemId, descId);
	dialog->_endLine = stream->readSint16LE();
	dialog->_endIndex = stream->readSint16LE();
	dialog->_wordCountdown = stream->readSint16LE();
	return dialog;
}

/*--------------------------------------------------------------------------*/

#define SR_SEPARATOR_Y 21
#define SR_SEPARATOR_X 5
#define SR_SEPARATOR_HEIGHT 5
#define SR_SAVEGAME_NAMES_Y (SR_SEPARATOR_Y + SR_SEPARATOR_HEIGHT + 1)

void SaveRestoreDialog::toggleHightlight(int xs, int xe, int ys, int ye) {
	Screen &screen = Screen::getReference();
	byte *addr = screen.screen().data().data() + FULL_SCREEN_WIDTH * ys + xs;

	for (int y = 0; y < ye - ys + 1; ++y, addr += FULL_SCREEN_WIDTH) {
		for (int x = 0; x < xe - xs + 1; ++x) {
			if (addr[x] == DIALOG_TEXT_COLOUR) addr[x] = DIALOG_WHITE_COLOUR;
			else if (addr[x] == DIALOG_WHITE_COLOUR) addr[x] = DIALOG_TEXT_COLOUR;
		}
	}

	screen.update();
}

bool SaveRestoreDialog::show(bool saveDialog) {
	OSystem &system = *g_system;
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Resources &res = Resources::getReference();
	LureEngine &engine = LureEngine::getReference();
	int selectedLine = -1;
	int index;

	// Figure out a list of present savegames
	String **saveNames = (String **)Memory::alloc(sizeof(String *) * MAX_SAVEGAME_SLOTS);
	int numSaves = 0;
	while ((numSaves < MAX_SAVEGAME_SLOTS) && 
		((saveNames[numSaves] = engine.detectSave(numSaves + 1)) != NULL))
		++numSaves;

	// For the save dialog, if all the slots have not been used up, create a 
	// blank entry for a new savegame
	if (saveDialog && (numSaves < MAX_SAVEGAME_SLOTS))
		saveNames[numSaves++] = new String();

	// For the restore dialog, if there are no savegames, return immediately
	if (!saveDialog && (numSaves == 0)) {
		Memory::dealloc(saveNames);
		return false;
	}

	Surface *s = new Surface(INFO_DIALOG_WIDTH, SR_SAVEGAME_NAMES_Y + 
		numSaves * FONT_HEIGHT + FONT_HEIGHT + 2);

	// Create the outer dialog and dividing line
	s->createDialog();
	byte *pDest = s->data().data() + (s->width() * SR_SEPARATOR_Y) + SR_SEPARATOR_X;
	uint8 rowColours[5] = {*(pDest-2), *(pDest-1), *(pDest-1), *(pDest-2), *(pDest+1)};
	for (int y = 0; y < SR_SEPARATOR_HEIGHT; ++y, pDest += s->width())
		memset(pDest, rowColours[y], s->width() - 12);

	// Create title line
	Common::String title(res.stringList().getString(
		saveDialog ? S_SAVE_GAME : S_RESTORE_GAME));
	s->writeString((s->width() - s->textWidth(title.c_str())) / 2, FONT_HEIGHT+2, title, true);

	// Write out any existing save names
	for (index = 0; index < numSaves; ++index)
		s->writeString(DIALOG_EDGE_SIZE, SR_SAVEGAME_NAMES_Y + (index * 8), saveNames[index]->c_str(), true);

	// Display the dialog
	s->copyTo(&screen.screen(), SAVE_DIALOG_X, SAVE_DIALOG_Y);
	screen.update();
	mouse.pushCursorNum(CURSOR_ARROW);
	Sound.pause();

	bool abortFlag = false;
	bool doneFlag = false;
	while (!abortFlag && !doneFlag) {
		// Provide highlighting of lines to select a save slot
		while (!abortFlag && !(mouse.lButton() && (selectedLine != -1)) && !mouse.rButton()) {
			abortFlag = events.quitFlag;
			if (abortFlag) break;

			while (events.pollEvent()) {
				if ((events.type() == Common::EVENT_KEYDOWN) &&
					(events.event().kbd.keycode == Common::KEYCODE_ESCAPE)) {
					abortFlag = true;
					break;
				}
				if (events.type() == Common::EVENT_MOUSEMOVE) {
					// Mouse movement
					int lineNum;
					if ((mouse.x() < (SAVE_DIALOG_X + DIALOG_EDGE_SIZE)) ||
						(mouse.x() >= (SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE)) ||
						(mouse.y() < SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y) ||
						(mouse.y() >= SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + numSaves * FONT_HEIGHT))
						// Outside displayed lines
						lineNum = -1;
					else
						lineNum = (mouse.y() - (SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y)) / FONT_HEIGHT;

					if (lineNum != selectedLine) {
						if (selectedLine != -1)
							// Deselect previously selected line
							toggleHightlight(SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
								SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + (selectedLine + 1) * FONT_HEIGHT - 1);

						// Highlight new line
						selectedLine = lineNum;
						if (selectedLine != -1)
							toggleHightlight(SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
								SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + (selectedLine + 1) * FONT_HEIGHT - 1);
					}
				}
			}

			system.updateScreen();
			system.delayMillis(10);
		}

		// Deselect selected row
		if (selectedLine != -1) 
			toggleHightlight(SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
				SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE,
				SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
				SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + (selectedLine + 1) * FONT_HEIGHT - 1);

		if (mouse.lButton() || mouse.rButton()) {
			abortFlag = mouse.rButton();
			mouse.waitForRelease();
		}
		if (abortFlag) break;

		// If in save mode, allow the entry of a new savename
		if (saveDialog) {
			if (!screen.screen().getString(*saveNames[selectedLine], 
				INFO_DIALOG_WIDTH - (DIALOG_EDGE_SIZE * 2), 
				false, true, SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
				SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT)) {
				// Aborted out of name selection, so restore old name and 
				// go back to slot selection
				screen.screen().writeString(
					SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
					SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
                    saveNames[selectedLine]->c_str(), true, DIALOG_TEXT_COLOUR, true);
				selectedLine = -1;
				continue;
			}
		}
		doneFlag = true;
	}

	delete s;
	Sound.resume();

	if (doneFlag) {
		// Handle save or restore
		if (saveDialog)
			doneFlag = engine.saveGame(selectedLine + 1, *saveNames[selectedLine]);
		else
			doneFlag = engine.loadGame(selectedLine + 1);
	}

	mouse.popCursor();

	// Free savegame caption list
	for (index = 0; index < numSaves; ++index) delete saveNames[index];
	Memory::dealloc(saveNames);

	return doneFlag;
}

/*--------------------------------------------------------------------------*/

struct RestartRecordPos {
	int16 x, y;
};

struct RestartRecord {
	Common::Language Language;
	int16 width, height;
	RestartRecordPos BtnRestart;
	RestartRecordPos BtnRestore;
};

static const RestartRecord buttonBounds[] = {
	{ EN_ANY, 48, 14, { 118, 152 }, { 168, 152 } },
	{ DE_DEU, 48, 14, { 106, 152 }, { 168, 152 } },
	{ UNK_LANG, 48, 14, { 112, 152 }, { 168, 152 } }
};


bool RestartRestoreDialog::show() {
	Resources &res = Resources::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	LureEngine &engine = LureEngine::getReference();

	Sound.killSounds();
	Sound.musicInterface_Play(60, 0);
	mouse.setCursorNum(CURSOR_ARROW);

	// See if there are any savegames that can be restored
	String *firstSave = engine.detectSave(1);
	bool restartFlag = (firstSave == NULL);
	int highlightedButton = -1;

	if (!restartFlag) {
		Memory::dealloc(firstSave);

		// Get the correct button bounds record to use
		const RestartRecord *btnRecord = &buttonBounds[0];
		while ((btnRecord->Language != engine.getLanguage()) && 
			   (btnRecord->Language != UNK_LANG))
			++btnRecord;

		// Fade out the screen
		screen.paletteFadeOut(RES_PALETTE_ENTRIES);

		// Get the palette that will be used, and first fade out the prior screen
		Palette p(RESTART_RESOURCE_ID - 1);

		// Turn on the mouse
		mouse.cursorOn();

		// Load the restore/restart screen image
		Surface *s = Surface::getScreen(RESTART_RESOURCE_ID);
		s->copyTo(&screen.screen(), 0, MENUBAR_Y_SIZE);
		delete s;

		res.activeHotspots().clear();
		Hotspot *btnHotspot = new Hotspot();

		// Restart button
		btnHotspot->setSize(btnRecord->width, btnRecord->height);
		btnHotspot->setPosition(btnRecord->BtnRestart.x, btnRecord->BtnRestart.y);
		btnHotspot->setAnimation(0x184B);
		btnHotspot->copyTo(&screen.screen());

		// Restore button
		btnHotspot->setFrameNumber(1);
		btnHotspot->setPosition(btnRecord->BtnRestore.x, btnRecord->BtnRestore.y);
		btnHotspot->copyTo(&screen.screen());

		screen.update();
		screen.paletteFadeIn(&p);

		// Event loop for making selection
		bool buttonPressed = false;

		while (!events.quitFlag) {
			// Handle events
			while (events.pollEvent()) {
				if ((events.type() == Common::EVENT_LBUTTONDOWN) && (highlightedButton != -1)) {
					mouse.waitForRelease();
					buttonPressed = true;
					break;
				}
			}

			if (buttonPressed)
				break;

			// Check if the pointer is over either button
			int currentButton = -1;
			if ((mouse.y() >= btnRecord->BtnRestart.y) &&
				(mouse.y() < btnRecord->BtnRestart.y + btnRecord->height)) {
				// Check whether the Restart or Restore button is highlighted
				if ((mouse.x() >= btnRecord->BtnRestart.x) &&
					(mouse.x() < btnRecord->BtnRestart.x + btnRecord->width))
					currentButton = 0;
				else if ((mouse.x() >= btnRecord->BtnRestore.x) &&
					(mouse.x() < btnRecord->BtnRestore.x + btnRecord->width))
					currentButton = 1;
			}

			// Take care of highlighting as the selected button changes
			if (currentButton != highlightedButton) {
				highlightedButton = currentButton;

				// Restart button
				btnHotspot->setFrameNumber((highlightedButton == 0) ? 2 : 0);
				btnHotspot->setPosition(btnRecord->BtnRestart.x, btnRecord->BtnRestart.y);
				btnHotspot->copyTo(&screen.screen());

				// Restore button
				btnHotspot->setFrameNumber((highlightedButton == 1) ? 3 : 1);
				btnHotspot->setPosition(btnRecord->BtnRestore.x, btnRecord->BtnRestore.y);
				btnHotspot->copyTo(&screen.screen());
			}


			screen.update();
			g_system->delayMillis(10);
		}

		restartFlag = highlightedButton == 0;
		delete btnHotspot;
	}

	Sound.killSounds();

	if (!restartFlag && !events.quitFlag) {
		// Need to show Restore game dialog
		if (!SaveRestoreDialog::show(false))
			// User cancelled, so fall back on Restart
			restartFlag = true;
	}

	return restartFlag;
}

/*--------------------------------------------------------------------------*/

struct ItemDesc {
	Common::Language language;
	int16 x, y;
	uint16 width, height;
	uint16 animId;
	uint8 startColour;
};

#define PROT_SPR_HEADER 0x1830
#define WORDING_HEADER 0x1839
#define NUMBER_HEADER 0x1842

static const ItemDesc copyProtectElements[] = {
	{UNK_LANG, 104, 96, 32, 48, PROT_SPR_HEADER, 0},
	{UNK_LANG, 179, 96, 32, 48, PROT_SPR_HEADER, 0},
	
	{EN_ANY, 57, 40, 208, 40, WORDING_HEADER, 32},
	{FR_FRA, 57, 40, 208, 40, WORDING_HEADER, 32},
	{DE_DEU, 39, 40, 208, 40, WORDING_HEADER, 32},
	{NL_NLD, 57, 40, 208, 40, WORDING_HEADER, 32},
	{ES_ESP, 57, 40, 208, 40, WORDING_HEADER, 32},
	{IT_ITA, 57, 40, 208, 40, WORDING_HEADER, 32},

	{UNK_LANG, 138, 168, 16, 8, NUMBER_HEADER, 32},
	{UNK_LANG, 145, 168, 16, 8, NUMBER_HEADER, 32},
	{UNK_LANG, 164, 168, 16, 8, NUMBER_HEADER, 32},
	{UNK_LANG, 171, 168, 16, 8, NUMBER_HEADER, 32},
	{UNK_LANG, 0, 0, 0, 0, 0, 0}
};

int pageNumbers[20] = {
	4, 10, 16, 22, 5, 11, 17, 23, 6, 12, 18, 7, 13, 19, 8, 14, 20, 9, 15, 21};

CopyProtectionDialog::CopyProtectionDialog() {
	// Get objects for the screen
	LureEngine &engine = LureEngine::getReference();

	const ItemDesc *ptr = &copyProtectElements[0];
	while ((ptr->width != 0) || (ptr->height != 0)) {
		if ((ptr->language == UNK_LANG) || (ptr->language == engine.getLanguage())) {
			if (ptr->animId == 0) break; //***DEBUG***
			Hotspot *h = new Hotspot();
			h->setPosition(ptr->x, ptr->y);
			h->setSize(ptr->width, ptr->height);
			h->setColourOffset(ptr->startColour);
			h->setAnimation(ptr->animId);

			_hotspots.push_back(h);
		}

		++ptr;
	}
}

bool CopyProtectionDialog::show() {
	Screen &screen = Screen::getReference();
	Events &events = Events::getReference();
	Common::RandomSource rnd;

	screen.setPaletteEmpty();
	Palette p(COPY_PROTECTION_RESOURCE_ID - 1);

	for (int tryCounter = 0; tryCounter < 3; ++tryCounter) {
		// Copy the base screen to the output screen
		Surface *s = Surface::getScreen(COPY_PROTECTION_RESOURCE_ID);
		s->copyTo(&screen.screen(), 0, MENUBAR_Y_SIZE);
		delete s;

		// Add wording header and display screen
		_hotspots[2]->setFrameNumber(1);
		_hotspots[2]->copyTo(&screen.screen());
		screen.update();
		screen.setPalette(&p);

		// Cycle through displaying different characters until a key or mouse button is pressed
		do {
			chooseCharacters();
		} while (!events.interruptableDelay(100));

		// Change title text to selection
		_hotspots[2]->setFrameNumber(0);
		_hotspots[2]->copyTo(&screen.screen());
		screen.update();

		// Clear any prior try
		_charIndex = 0;

		while (!events.quitFlag) {
			while (events.pollEvent() && (_charIndex < 4)) {
				if (events.type() == Common::EVENT_KEYDOWN) { 
					if ((events.event().kbd.keycode == Common::KEYCODE_BACKSPACE) && (_charIndex > 0)) {
						// Remove the last number typed
						--_charIndex;
						_hotspots[_charIndex + 3]->setFrameNumber(10);   // Blank space
						_hotspots[_charIndex + 3]->copyTo(&screen.screen());

						screen.update();
					} else if ((events.event().kbd.keycode >= Common::KEYCODE_0) &&
								(events.event().kbd.keycode <= Common::KEYCODE_9)) {
						// Number pressed
						_hotspots[_charIndex + 3]->setFrameNumber(events.event().kbd.ascii - '0');
						_hotspots[_charIndex + 3]->copyTo(&screen.screen());

						++_charIndex;
					}

					screen.update();
				}
			}

			g_system->delayMillis(10);
			if (_charIndex == 4)
				break;
		}

		if (events.quitFlag)
			return false;

		// At this point, two page numbers have been entered - validate them
		int page1 = (_hotspots[3]->frameNumber() * 10) + _hotspots[4]->frameNumber();
		int page2 = (_hotspots[5]->frameNumber() * 10) + _hotspots[6]->frameNumber();
		
		if ((page1 == pageNumbers[_hotspots[0]->frameNumber()]) &&
			(page2 == pageNumbers[_hotspots[1]->frameNumber()]))
			return true;
	}

	// Copy protection failed
	return false;
}

void CopyProtectionDialog::chooseCharacters() {
	Screen &screen = Screen::getReference();
	int char1 = _rnd.getRandomNumber(19);
	int char2 = _rnd.getRandomNumber(19);

	_hotspots[0]->setFrameNumber(char1);
	_hotspots[0]->copyTo(&screen.screen());
	_hotspots[1]->setFrameNumber(char2);
	_hotspots[1]->copyTo(&screen.screen());

	screen.update();
}

} // end of namespace Lure
