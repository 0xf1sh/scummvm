/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include <SDL.h>

// Uncomment this to enable the 'on screen display' code.
#define USE_OSD	1

class OSystem_SDL : public OSystem {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	// Set the size of the video bitmap.
	// Typically, 320x200
	void initSize(uint w, uint h);

	// Set colors of the palette
	void setPalette(const byte *colors, uint start, uint num);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h);

	void move_screen(int dx, int dy, int height);

	// Update the dirty areas of the screen
	void updateScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);
	
	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	void warpMouse(int x, int y);

	// Set the bitmap that's used when drawing the cursor.
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y);

	// Shaking is used in SCUMM. Set current shake position.
	void set_shake_pos(int shake_pos);

	// Get the number of milliseconds since the program was started.
	uint32 get_msecs();

	// Delay for a specified amount of milliseconds
	void delay_msecs(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);

	// Set function that generates samples 
	bool setSoundCallback(SoundProc proc, void *param);
	
	void clearSoundCallback();

	// Poll CD status
	// Returns true if cd audio is playing
	bool poll_cdrom();

	// Play CD audio track
	void play_cdrom(int track, int num_loops, int start_frame, int duration);

	// Stop CD audio track
	void stop_cdrom();

	// Update CD audio status
	void update_cdrom();

	// Quit
	void quit();


	// Add a callback timer
	void setTimerCallback(TimerProc callback, int timer);

	// Mutex handling
	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Overlay
	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getHeight();
	virtual int16 getWidth();

	// Methods that convert RGB to/from colors suitable for the overlay.
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b);
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b);


	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	
	virtual void setWindowCaption(const char *caption);
	virtual bool openCD(int drive);
	virtual int getOutputSampleRate() const;

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

#ifdef USE_OSD
	void displayMessageOnOSD(const char *msg);
#endif

protected:
	void init_intern();

#ifdef USE_OSD
	SDL_Surface *_osdSurface;
	Uint8 _osdAlpha;			// Transparency level of the OSD
	uint32 _osdFadeStartTime;	// When to start the fade out
	enum {
		kOSDFadeOutDelay = 2 * 1000,	// Delay before the OSD is faded out (in milliseconds)
		kOSDFadeOutDuration = 500,		// Duration of the OSD fade out (in milliseconds)
		kOSDColorKey = 1,
		kOSDInitialAlpha = 80			// Initial alpha level, in percent
	};
#endif

	// hardware screen
	SDL_Surface *_hwscreen;

	// unseen game screen
	SDL_Surface *_screen;
	int _screenWidth, _screenHeight;

	// temporary screen (for scalers/overlay)
	SDL_Surface *_tmpscreen;
	bool _overlayVisible;

	// CD Audio
	SDL_CD *_cdrom;
	int cd_track, cd_num_loops, cd_start_frame, cd_duration;
	Uint32 cd_end_time, cd_stop_time;

	enum {
		DF_WANT_RECT_OPTIM			= 1 << 0,
		DF_UPDATE_EXPAND_1_PIXEL	= 1 << 1
	};

	bool _forceFull; // Force full redraw on next updateScreen
	ScalerProc *_scaler_proc;
	int _scaleFactor;
	int _mode;
	bool _full_screen;
	uint32 _mode_flags;
	bool _modeChanged;

	/** True if aspect ratio correction is enabled. */
	bool _adjustAspectRatio;

	enum {
		NUM_DIRTY_RECT = 100,

		MAX_MOUSE_W = 80,
		MAX_MOUSE_H = 80,
		MAX_SCALING = 3
	};

	// Dirty rect managment
	SDL_Rect _dirty_rect_list[NUM_DIRTY_RECT];
	int _num_dirty_rects;
	uint32 *_dirty_checksums;
	bool cksum_valid;
	int CKSUM_NUM;

	// Keyboard mouse emulation
	struct KbdMouse {	
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	} km;

	struct MousePos {
		int16 x, y, w, h;
		MousePos() : x(0), y(0), w(0), h(0) {}
	};

	// mouse
	bool _mouseVisible;
	bool _mouseDrawn;
	const byte *_mouseData;
	byte *_mouseBackup;
	MousePos _mouseCurState;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;

	// joystick
	SDL_Joystick *_joystick;

	// Shake mode
	int _currentShakePos;
	int _newShakePos;

	// Palette data
	SDL_Color *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;
	
	/**
	 * Mutex which prevents multiple threads from interfering with each other
	 * when accessing the screen.
	 */
	MutexRef _graphicsMutex;


	void add_dirty_rgn_auto(const byte *buf);
	void mk_checksums(const byte *buf);

	void add_dirty_rect(int x, int y, int w, int h);

	void draw_mouse();
	void undraw_mouse();
	/** Set the position of the virtual mouse cursor. */
	void set_mouse_pos(int x, int y);
	void fillMouseEvent(Event &event, int x, int y);
	void toggleMouseGrab();


	virtual void internUpdateScreen();

	void load_gfx_mode();
	void unload_gfx_mode();
	void hotswap_gfx_mode();
	
	void setFullscreenMode(bool enable);

	bool save_screenshot(const char *filename);
	
	int effectiveScreenHeight() { return (_adjustAspectRatio ? 240 : _screenHeight) * _scaleFactor; }

	void setup_icon();
	void kbd_mouse();
	void init_joystick(int joystick_num) { _joystick = SDL_JoystickOpen(joystick_num); }
};

#endif
