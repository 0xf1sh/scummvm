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

#ifndef ZVISION_CONTROL_H
#define ZVISION_CONTROL_H

#include "common/keyboard.h"


namespace Common {
class SeekableReadStream;
struct Point;
class WriteStream;
}

namespace ZVision {

class ZVision;

class Control {
public:
	Control() : _engine(0), _key(0) {}
	Control(ZVision *engine, uint32 key) : _engine(engine), _key(key) {}
	virtual ~Control() {}

	uint32 getKey() {
		return _key;
	}

	virtual void focus() {}
	virtual void unfocus() {}
	/**
	 * Called when LeftMouse is pushed. Default is NOP.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	virtual void onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {}
	/**
	 * Called when LeftMouse is lifted. Default is NOP.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	virtual void onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {}
	/**
	 * Called on every MouseMove. Default is NOP.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 * @return                           Was the cursor changed?
	 */
	virtual bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
		return false;
	}
	/**
	 * Called when a key is pressed. Default is NOP.
	 *
	 * @param keycode    The key that was pressed
	 */
	virtual void onKeyDown(Common::KeyState keyState) {}
	/**
	 * Called when a key is released. Default is NOP.
	 *
	 * @param keycode    The key that was pressed
	 */
	virtual void onKeyUp(Common::KeyState keyState) {}
	/**
	 * Processes the node given the deltaTime since last frame. Default is NOP.
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	virtual bool process(uint32 deltaTimeInMillis) {
		return false;
	}

protected:
	ZVision *_engine;
	uint32 _key;

// Static member functions
public:
	static void parseFlatControl(ZVision *engine);
	static void parsePanoramaControl(ZVision *engine, Common::SeekableReadStream &stream);
	static void parseTiltControl(ZVision *engine, Common::SeekableReadStream &stream);
};

// TODO: Implement InputControl
// TODO: Implement SaveControl
// TODO: Implement SlotControl
// TODO: Implement SafeControl
// TODO: Implement FistControl
// TODO: Implement HotMovieControl
// TODO: Implement PaintControl
// TODO: Implement TilterControl

} // End of namespace ZVision

#endif
