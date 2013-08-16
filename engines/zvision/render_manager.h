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

#ifndef ZVISION_RENDER_MANAGER_H
#define ZVISION_RENDER_MANAGER_H

#include "common/types.h"
#include "common/rect.h"

#include "graphics/surface.h"

#include "zvision/render_table.h"

class OSystem;

namespace Common {
class String;
class SeekableReadStream;
}

namespace Video {
class VideoDecoder;
}

namespace ZVision {

class RenderManager {
public:
	RenderManager(OSystem *system, const Common::Rect workingWindow, const Graphics::PixelFormat pixelFormat);
	~RenderManager();

private:
	OSystem *_system;
	const Graphics::PixelFormat _pixelFormat;

	Graphics::Surface _backbuffer;
	uint16 *_warpedBackbuffer;

	/** Width of the working window. Saved to prevent extraneous calls to _workingWindow.width() */
	const int _workingWidth;
	/** Height of the working window. Saved to prevent extraneous calls to _workingWindow.height() */
	const int _workingHeight;
	/** 
	 * A Rectangle centered inside the actual window. All in-game coordinates
	 * are given in this coordinate space. Also, all images are clipped to the
	 * edges of this Rectangle
	 */
	const Common::Rect _workingWindow;
	/** Used to warp the background image */
	RenderTable _renderTable;

	Common::SeekableReadStream *_currentBackground;
	/** The (x1,y1) coordinates of the subRectangle of the background that is currently displayed on the screen */
	Common::Point _backgroundOffset;
	/** The width of the current background image */
	uint16 _backgroundWidth;
	/** The height of the current background image */
	uint16 _backgroundHeight;

	/** 
	 * The "velocity" at which the background image is panning. We actually store the inverse of velocity (ms/pixel instead of pixels/ms)
	 * because it allows you to accumulate whole pixels 'steps' instead of rounding pixels every frame
	 */
	int _backgroundInverseVelocity;
	/** Holds any 'leftover' milliseconds between frames */
	uint _accumulatedVelocityMilliseconds;

	byte *_scaledVideoFrameBuffer;

public:
	void initialize();
	/**
	 * Rotates the background image in accordance to the current _backgroundInverseVelocity
	 *
	 * @param deltaTimeInMillis    The amount of time that has passed since the last frame
	 */
	void update(uint deltaTimeInMillis);

	/**
	 * Blits the image or a portion of the image to the backbuffer. Actual screen updates won't happen until the end of the frame.
	 * The image will be clipped to fit inside the working window. Coords are in working window space, not screen space!
	 *
	 * @param fileName        Name of the image file
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param subRectangle    The subrectangle of the image that should be rendered. If this is an empty rectangle, it will blit the entire image. Coords are in working window space, not screen space!
	 */
	void renderImageToBackbuffer(const Common::String &fileName, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle = Common::Rect(0, 0, 0, 0), bool wrap = false);

	/**
	 * Blits the image or a portion of the image to the backbuffer. Actual screen updates won't happen until the end of the frame.
	 * The image will be clipped to fit inside the working window. Coords are in working window space, not screen space!
	 *
	 * @param stream          Stream to read the image data from
	 * @param destinationX    X position where the image should be put. Coords are in working window space, not screen space!
	 * @param destinationY    Y position where the image should be put. Coords are in working window space, not screen space!
	 * @param subRectangle    The subrectangle of the image that should be rendered. If this is an empty rectangle, it will blit the entire image. Coords are in working window space, not screen space!
	 */
	void renderImageToBackbuffer(Common::SeekableReadStream &stream, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle = Common::Rect(0, 0, 0, 0), bool wrap = false);

	/**
	 * Sets the current background image to be used by the RenderManager and immediately
	 * blits it to the screen. (It won't show up until the end of the frame)
	 *
	 * @param fileName    The name of the image file
	 */
	void setBackgroundImage(const Common::String &fileName);

	/**
	 * Set the background position (_backgroundOffset). If the current RenderState is PANORAMA, the offset
	 * will be in the horizontal direction. If the current RenderState is TILT, the offset will be in the
	 * vertical direction. 
	 *
	 * This method will not render anything on the screen. So if nothing else is called that renders the 
	 * background, the change won't be seen until next frame.
	 *
	 * @param offset The amount to offset the background
	 */
	void setBackgroundPosition(int offset);
	
	/**
	 * Set the background scroll velocity. Negative velocities correspond to left / up scrolling and 
	 * positive velocities correspond to right / down scrolling
	 *
	 * @param velocity    Velocity
	 */
	void setBackgroundVelocity(int velocity);

	/**
	 * Converts a point in screen coordinate space to image coordinate space
	 *
	 * @param point    Point in screen coordinate space
	 * @return         Point in image coordinate space
	 */
	const Common::Point screenSpaceToImageSpace(const Common::Point &point);

	RenderTable *getRenderTable();

private:
	/**
	 * Renders a subRectangle of an image to the backbuffer. The destinationRect and SubRect
	 * will be clipped to image bound and to working window bounds
	 *
	 * @param buffer             Pointer to (0, 0) of the image data
	 * @param imageWidth         The width of the original image (not of the subRectangle)
	 * @param imageHeight        The width of the original image (not of the subRectangle)
	 * @param horizontalPitch    The horizontal pitch of the original image
	 * @param destinationX       The x coordinate (in working window space) of where to put the final image
	 * @param destinationY       The y coordinate (in working window space) of where to put the final image
	 * @param subRectangle       A rectangle representing the part of the image that should be rendered
	 * @param wrap               Should the image wrap (tile) if it doesn't completely fill the screen?
	 */
	void renderSubRectToBackbuffer(Graphics::Surface &surface, uint32 destinationX, uint32 destinationY, Common::Rect subRectangle, bool wrap, bool isTransposed);

	void copyTransposedRectToBackbuffer(const uint16 *buffer, int imageWidth, int destinationX, int destinationY, int width, int height, const Common::Rect &subRect);

	void moveBackground(int offset);
};

} // End of namespace ZVision

#endif
