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

#ifndef MADS_PALETTE_H
#define MADS_PALETTE_H

#include "common/scummsys.h"

namespace MADS {

class MADSEngine;

struct RGB4 {
	byte r;
	byte g;
	byte b;
	byte u;
};

/**
 * Used to store a list of RGB values
 */
class RGBList {
private:
	int _size;
	byte *_data;
	byte *_palIndexes;
	bool _freeData;
public:
	/** 
	 * Constructor
	 */
	RGBList(int numEntries = 256, byte *srcData = NULL, bool freeData = true);

	/**
	 * Destructor
	 */
	~RGBList();

	/**
	 * Returns the raw data containing the RGB values as 3 bytes per entry
	 */
	byte *data() { return _data; }

	/**
	 * Returns the list of palette indexes each RGB tuple maps to in the current palette
	 */
	byte *palIndexes() { return _palIndexes; }

	/**
	 * Returns the size of the palette
	 */
	int size() const { return _size; }
};

#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

class Palette {
private:
	/**
	 * Initialises a stanadrd range of colours for the given palette
	 */
	void initRange(byte *palette);
protected:
	MADSEngine *_vm;
	bool _colorsChanged;

	bool _fading_in_progress;
	byte _originalPalette[PALETTE_COUNT * 4];
	byte _fadedPalette[PALETTE_COUNT * 4];
	int _usageCount[PALETTE_COUNT];

	void reset();
public:
	byte _mainPalette[PALETTE_SIZE];
	RGB4 _gamePalette[PALETTE_COUNT];
public:
	/**
	 * Constructor
	 */
	Palette(MADSEngine *vm);
	
	/**
	 * Destructor
	 */
	virtual ~Palette() {}

	/**
	 * Sets a new palette
	 */
	void setPalette(const byte *colors, uint start, uint num);

	/**
	 * Returns a subset of the currently loaded palette
	 */
	void grabPalette(byte *colors, uint start, uint num);
	
	/**
	 * Returns the palette index in the palette that most closely matches the
	 * specified RGB pair
	 */
	uint8 palIndexFromRgb(byte r, byte g, byte b, byte *paletteData = nullptr);

	// Methods used for reference counting color usage
	/**
	 * Resets the usage counts for the palette
	 */
	void resetColorCounts();

	/**
	 * Blocks out a range of the palette from being used
	 */
	void blockRange(int startIndex, int size);

	/**
	 * Adds the data of an RGBList into the current palette and increment usage counts.
	 */
	void addRange(RGBList *list);

	/**
	 * Delets a range from the current palette, dercementing the accompanying usage counts.
	 */
	void deleteRange(RGBList *list);

	/**
	 * Deletes all loaded RGB lists are their usage references.
	 */
	void deleteAllRanges();

	// Virtual method table
	/**
	 * Decode a palette and return it, without affecting the Palette itself
	 */
	byte *decodePalette(Common::SeekableReadStream *palStream, int *numColors);

	/**
	 * Loads a palette from a stream
	 */
	int loadPalette(Common::SeekableReadStream *palStream, int indexStart = 0);

	/**
	 * Sets a small set of system/core colors needed by the game
	 */
	void setSystemPalette();

	/**
	 * Update a range of an arbitrary palette
	 */
	static void setGradient(byte *palette, int start, int count, int rgbValue1, int rgbValue2);

	/**
	 * Resets the game palette
	 */
	void resetGamePalette(int v1, int v2);

	/**
	 * Set the first four palette entries with preset values
	 */
	void setLowRange();

	// Color indexes
	uint8 BLACK;
	uint8 BLUE;
	uint8 GREEN;
	uint8 CYAN;
	uint8 RED;
	uint8 VIOLET;
	uint8 BROWN;
	uint8 LIGHT_GRAY;
	uint8 DARK_GRAY;
	uint8 LIGHT_BLUE;
	uint8 LIGHT_GREEN;
	uint8 LIGHT_CYAN;
	uint8 LIGHT_RED;
	uint8 PINK;
	uint8 YELLOW;
	uint8 WHITE;
};

} // End of namespace MADS

#endif /* MADS_PALETTE_H */
