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

#ifndef ZVISION_RENDER_TABLE_H
#define ZVISION_RENDER_TABLE_H

#include "common/types.h"
#include "common/rect.h"

#include "zvision/vector2.h"

namespace ZVision {

class RenderTable {
public:
	RenderTable(uint32 numRows, uint32 numColumns);
	~RenderTable();

public:
	enum RenderState {
		PANORAMA,
		TILT,
		FLAT
	};

private:
	uint32 _numColumns, _numRows;
	Vector2 *_internalBuffer;
	RenderState _renderState;

	struct {
		float fieldOfView;
		float linearScale;
	} _panoramaOptions;

	// TODO: See if tilt and panorama need to have separate options
	struct {
		float fieldOfView;
		float linearScale;
	} _tiltOptions;

public:
	RenderState getRenderState() { return _renderState; }
	void setRenderState(RenderState newState);
	void mutateImage(uint16 *sourceBuffer, uint16* destBuffer, uint32 imageWidth, uint32 imageHeight, Common::Rect subRectangle, Common::Rect destRectangle);

private:
	void generatePanoramaLookupTable();
	void generateTiltLookupTable();
};

} // End of namespace ZVision

#endif
