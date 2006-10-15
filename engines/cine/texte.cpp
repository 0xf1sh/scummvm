/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * $URL$
 * $Id$
 *
 */

#include "cine/cine.h"
#include "common/file.h"
#include "cine/various.h"

namespace Cine {

byte *textDataPtr;

byte textTable[256][2][16 * 8];

void generateMask(byte *sprite, byte *mask, uint16 size, byte transparency);

void loadTextData(const char *pFileName, byte *pDestinationBuffer) {
	Common::File pFileHandle;
	uint16 entrySize;
	uint16 numEntry;
	uint16 i;
	byte *tempBuffer;
	uint16 dataSize;

	assert(pFileName);
	assert(pDestinationBuffer);

	pFileHandle.open(pFileName);

	assert(pFileHandle.isOpen());

	entrySize = pFileHandle.readUint16BE();
	numEntry = pFileHandle.readUint16BE();

	dataSize = numEntry * entrySize;
	pFileHandle.read(pDestinationBuffer, numEntry * entrySize);

	tempBuffer = pDestinationBuffer;

	if (g_cine->getGameType() == Cine::GType_FW) {
		dataSize = dataSize / 0x4E;

		loadRelatedPalette(pFileName);

		for (i = 0; i < 0x4E; i++) {
			gfxConvertSpriteToRaw(textTable[i][0], tempBuffer, 16, 8);
			generateMask(textTable[i][0], textTable[i][1], 16 * 8, 0);
			tempBuffer += dataSize;
		}
	} else {
		for (i = 0; i < 90; i++) {
			gfxConvertSpriteToRaw(textTable[i][0], tempBuffer, 8, 8);
			generateMask(textTable[i][0], textTable[i][1], 8 * 8, 0);
			tempBuffer += 0x40;
		}
	}

	pFileHandle.close();
}

} // End of namespace Cine
