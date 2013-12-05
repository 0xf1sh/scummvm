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

#ifndef PRINCE_DETAIL_ANIMATION_H
#define PRINCE_DETAIL_ANIMATION_H

#include "graphics/surface.h"

namespace Prince { namespace Detail {

class Animation {
public:
	Animation(byte *data, uint32 dataSize);
	~Animation();
	int16 getLoopCount() const;
	int16 getBaseX() const;
	int16 getBaseY() const;
	uint getPhaseCount() const;
	uint getFrameCount() const;
	int16 getPhaseOffsetX(uint phaseIndex) const;
	int16 getPhaseOffsetY(uint phaseIndex) const;
	int16 getPhaseFrameIndex(uint phaseIndex) const;
	Graphics::Surface *getFrame(uint frameIndex);
protected:
	byte *_data;
	uint32 _dataSize;
	byte *getPhaseEntry(uint phaseIndex) const;
};

} } 

#endif
