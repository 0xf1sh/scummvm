/* ScummVM - Scumm Interpreter
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

#ifndef WINCE_SDL_H
#define WINCE_SDL_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/scaler.h"
#include "backends/intern.h"
#include "backends/sdl/sdl-common.h"

#include "CEgui.h"
#include "CEkeys.h"
#include "CEDevice.h"
#include "CEScaler.h"

#include <SDL.h>

class OSystem_WINCE3 : public OSystem_SDL {
public:
	OSystem_WINCE3();

	// Update the dirty areas of the screen
	void internUpdateScreen();

	void initSize(uint w, uint h);

	// Overloaded from SDL_Common (toolbar handling)
	bool poll_event(Event *event);
	// Overloaded from SDL_Common (toolbar handling)
	void draw_mouse();
	// Overloaded from SDL_Common (mouse and new scaler handling)
	void fillMouseEvent(Event &event, int x, int y);
	// Overloaded from SDL_Common (new scaler handling)
	void add_dirty_rect(int x, int y, int w, int h);
	// Overloaded from SDL_Common (new scaler handling)
	void warpMouse(int x, int y);
	// Overloaded from SDL_Commmon
	void quit();
	// Overloaded from SDL_Commmon (master volume and sample rate subtleties)
	bool setSoundCallback(SoundProc proc, void *param);

	// GUI and action stuff
	void swap_panel_visibility();
	void swap_panel();
	void swap_sound_master();
	void add_right_click();
	void swap_mouse_visibility();
	void swap_freeLook();
	void swap_zoom_up();
	void swap_zoom_down();

#ifdef WIN32_PLATFORM_WFSP
	// Smartphone actions
	void add_left_click();
	void move_cursor_up();
	void move_cursor_down();
	void move_cursor_left();
	void move_cursor_right();
	void switch_zone();
#endif

	static int getScreenWidth();
	static int getScreenHeight();
	static void initScreenInfos();
	static bool isOzone();

protected:
	void load_gfx_mode();
	void unload_gfx_mode();
	void hotswap_gfx_mode();
	bool save_screenshot(const char *filename);

	
	const GraphicsMode *getSupportedGraphicsModes() const;	
	bool setGraphicsMode(int mode);
	//int getGraphicsMode() const;	
	int getDefaultGraphicsMode() const;

	void setWindowCaption(const char *caption);
	bool openCD(int drive);
	int getOutputSampleRate() const;

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

private:

#ifdef USE_VORBIS
	bool checkOggHighSampleRate();
#endif

	static void private_sound_proc(void *param, byte *buf, int len);
	static SoundProc _originalSoundProc;

	bool update_scalers();
	void create_toolbar();
	void update_game_settings();
	void check_mappings();
	void update_keyboard();
	void get_sample_rate();	

	void retrieve_mouse_location(int &x, int &y);

	CEGUI::ToolbarHandler _toolbarHandler;

	SDL_Surface *_toolbarLow;	// toolbar 320x40
	SDL_Surface *_toolbarHigh;  // toolbar 640x80
	bool _toolbarHighDrawn;		// cache toolbar 640x80

	uint16 _sampleRate;			// current audio sample rate

	bool _freeLook;				// freeLook mode (do not send mouse button events)

	bool _forceHideMouse;		// force invisible mouse cursor

	bool _forcePanelInvisible;  // force panel visibility for some cases
	bool _panelVisible;			// panel visibility
	bool _panelStateForced;		// panel visibility forced by external call
	
	bool _panelInitialized;		// only initialize the toolbar once

	bool _monkeyKeyboard;		// forced keyboard for Monkey Island copy protection
	static bool _soundMaster;	// turn off sound after all calculations
								// static since needed by the SDL callback
	bool _orientationLandscape; // current orientation
	bool _newOrientation;		// new orientation

	bool _saveToolbarState;		// save visibility when forced
	String _saveActiveToolbar;	// save active toolbar when forced

	bool _saveToolbarZoom;		// save visibility when zooming 
	bool _zoomUp;				// zooming up mode
	bool _zoomDown;				// zooming down mode

	int _scaleFactorXm;
	int _scaleFactorXd;
	int _scaleFactorYm;
	int _scaleFactorYd;
	bool _scalersChanged;

	static int _platformScreenWidth;
	static int _platformScreenHeight;
	static bool _isOzone;
};

#endif
