/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "backends/platform/symbian/src/SymbianOS.h"
#include "backends/platform/symbian/src/SymbianActions.h"
#include "common/config-manager.h"
#include "gui/Actions.h"
#include "gui/Key.h"
#include "gui/message.h"

#include <eikenv.h> // for CEikonEnv::Static() @ Symbian::FatalError()
#include "ESDL/sdlapp.h" // for CSDLApp::GetExecutablePathCStr() @ Symbian::GetExecutablePath()

////////// extern "C" ///////////////////////////////////////////////////

extern Common::ConfigManager *g_config;

namespace Symbian {

// Show a simple Symbian Info win with Msg & exit
void FatalError(const char *msg) {
	TPtrC8 msgPtr((const TUint8 *)msg); 
	TBuf<512> msg16Bit;
	msg16Bit.Copy(msgPtr);
#ifdef S60
#else
	CEikonEnv::Static()->InfoWinL(_L("ScummVM Fatal Error"), msg16Bit);
#endif
	if (g_system)
		g_system->quit();
}

// make this easily available everywhere
char* GetExecutablePath()
{
	return CSDLApp::GetExecutablePathCStr();	
}

} // namespace Symbian {

////////// OSystem_SDL_Symbian //////////////////////////////////////////

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Fullscreen", GFX_NORMAL},
	{0, 0, 0}
};

bool OSystem_SDL_Symbian::hasFeature(Feature f) {
	switch(f) {
		case kFeatureFullscreenMode:
		case kFeatureAspectRatioCorrection:
		case kFeatureAutoComputeDirtyRects:
		case kFeatureCursorHasPalette:
#ifdef  USE_VIBRA_SE_PXXX
		case kFeatureVibration:
#endif
			return true;

		default:
			return false;
	}
}

OSystem_SDL_Symbian::zoneDesc OSystem_SDL_Symbian::_zones[TOTAL_ZONES] = {
        { 0, 0, 320, 145 },
        { 0, 145, 150, 55 },
        { 150, 145, 170, 55 }
};
OSystem_SDL_Symbian::OSystem_SDL_Symbian() :_channels(0),_stereo_mix_buffer(0) {
}

void OSystem_SDL_Symbian::initBackend() {
	ConfMan.setBool("FM_high_quality", false);
#if !defined(S60) || defined(S60V3) // S60 has low quality as default
	ConfMan.setBool("FM_medium_quality", true);
#else
	ConfMan.setBool("FM_medium_quality", false);
#endif
	ConfMan.setInt("joystick_num", 0); // Symbian OS  should have joystick_num set to 0 in the ini file , but uiq devices might refuse opening the joystick
	ConfMan.flushToDisk();
	
	GUI::Actions::init();

	OSystem_SDL::initBackend();
	
	// Initialize global key mapping for Smartphones
	GUI::Actions* actions = GUI::Actions::Instance();

	actions->initInstanceMain(this);	
	actions->loadMapping();
	initZones();
}

OSystem_SDL_Symbian::~OSystem_SDL_Symbian() {
	delete []_stereo_mix_buffer;
}

int OSystem_SDL_Symbian::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

const OSystem::GraphicsMode *OSystem_SDL_Symbian::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

// make sure we always go to normal, even if the string might be set wrong!
bool OSystem_SDL_Symbian::setGraphicsMode(const char * /*name*/) {
	// let parent OSystem_SDL handle it
	return OSystem_SDL::setGraphicsMode(getDefaultGraphicsMode());
}

void OSystem_SDL_Symbian::quitWithErrorMsg(const char *msg) {

	CEikonEnv::Static()->AlertWin(_L("quitWithErrorMsg()")) ;
	
	if (g_system)
		g_system->quit();
}

// Overloaded from SDL_Commmon
void OSystem_SDL_Symbian::quit() {
	delete GUI_Actions::Instance();
	OSystem_SDL::quit();
}

bool OSystem_SDL_Symbian::setSoundCallback(SoundProc proc, void *param) {

	// First save the proc and param
	_sound_proc_param = param; 
	_sound_proc = proc;
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	memset(&desired, 0, sizeof(desired));

	_samplesPerSec = 0;

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");

	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint32 samples = 0x8000;

	for (;;) {
		if ((1000 * samples) / _samplesPerSec < 100)
			break;
		samples >>= 1;
	}

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
#ifdef S60
	desired.callback = symbianMixCallback;
	desired.userdata = this;
#else
	desired.callback = proc;
	desired.userdata = param;
#endif
	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		return false;
	}
	// Note: This should be the obtained output rate, but it seems that at
	// least on some platforms SDL will lie and claim it did get the rate
	// even if it didn't. Probably only happens for "weird" rates, though.
	_samplesPerSec = obtained.freq;
	_channels = obtained.channels;

	// Need to create mixbuffer for stereo mix to downmix
	if(_channels != 2) {
		_stereo_mix_buffer = new byte [obtained.size*2];//*2 for stereo values
	}

	SDL_PauseAudio(0);
	return true;
}

/**
 * The mixer callback function, passed on to OSystem::setSoundCallback().
 * This simply calls the mix() method.
 */
void OSystem_SDL_Symbian::symbianMixCallback(void *s, byte *samples, int len) {
	static_cast <OSystem_SDL_Symbian*>(s)->symbianMix(samples,len);
}


/**
 * Actual mixing implementation
 */
void OSystem_SDL_Symbian::symbianMix(byte *samples, int len) {
	// If not stereo then we need to downmix
	if (_channels != 2) {
	  	_sound_proc(_sound_proc_param, _stereo_mix_buffer, len * 2);
		int16 *bitmixDst = (int16 *)samples;
		int16 *bitmixSrc = (int16 *)_stereo_mix_buffer;

		for (int loop = len / 2; loop >= 0; loop --) {
			*bitmixDst = (*bitmixSrc + *(bitmixSrc + 1)) >> 1;
			bitmixDst++;
			bitmixSrc += 2;
		}
	} else
		_sound_proc(_sound_proc_param, samples, len);
}

/**
 * This is an implementation by the remapKey function
 * @param SDL_Event to remap
 * @param ScumVM event to modify if special result is requested
 * @return true if Event has a valid return status
 */
bool OSystem_SDL_Symbian::remapKey(SDL_Event &ev, Event &event) {
	if (GUI::Actions::Instance()->mappingActive() || ev.key.keysym.sym <= SDLK_UNKNOWN)
		return false;

	for (TInt loop = 0; loop < GUI::ACTION_LAST; loop++) {
		if (GUI::Actions::Instance()->getMapping(loop) == ev.key.keysym.sym && 
			GUI::Actions::Instance()->isEnabled(loop)) {
			// Create proper event instead
			switch(loop) {
			case GUI::ACTION_UP:	
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = -1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;			

			case GUI::ACTION_DOWN:
				if(ev.type == SDL_KEYDOWN) {
					_km.y_vel = 1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;	

			case GUI::ACTION_LEFT:
				if(ev.type == SDL_KEYDOWN) {
					_km.x_vel = -1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHT:
				if(ev.type == SDL_KEYDOWN) {
					_km.x_vel = 1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? EVENT_LBUTTONDOWN : EVENT_LBUTTONUP);
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? EVENT_RBUTTONDOWN : EVENT_RBUTTONUP);
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_ZONE:
				if(ev.type == SDL_KEYDOWN) {
					int i;				
					
					for (i=0; i < TOTAL_ZONES; i++)
						if (_km.x >= _zones[i].x && _km.y >= _zones[i].y &&
							_km.x <= _zones[i].x + _zones[i].width && _km.y <= _zones[i].y + _zones[i].height
							) {
							_mouseXZone[i] = _km.x;
							_mouseYZone[i] = _km.y;
							break;
						}
						_currentZone++;
						if (_currentZone >= TOTAL_ZONES)
							_currentZone = 0;
						event.type = EVENT_MOUSEMOVE;
						fillMouseEvent(event, _mouseXZone[_currentZone], _mouseYZone[_currentZone]);
						SDL_WarpMouse(event.mouse.x, event.mouse.y);					
				}

				return true;

			case GUI::ACTION_SAVE:
			case GUI::ACTION_SKIP:
			case GUI::ACTION_FT_CHEAT:
			case GUI::ACTION_SKIP_TEXT:
			case GUI::ACTION_PAUSE:
				{
					GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
					ev.key.keysym.sym = (SDLKey)key.ascii();
					ev.key.keysym.scancode= key.keycode();
					ev.key.keysym.mod = (SDLMod)key.flags();

					return false;
				}			

			case GUI::ACTION_QUIT:
				{
					GUI::MessageDialog alert("Do you want to quit ?", "Yes", "No");
					if (alert.runModal() == GUI::kMessageOK)
						quit();

					return true;
				}
			}
		}
	}

	return false;
}

void OSystem_SDL_Symbian::setWindowCaption(const char *caption) {
	OSystem_SDL::setWindowCaption(caption);
	check_mappings();
}

void OSystem_SDL_Symbian::check_mappings() {
	if (ConfMan.get("gameid").empty() || GUI::Actions::Instance()->initialized())
		return;

	GUI::Actions::Instance()->initInstanceGame();
}

void OSystem_SDL_Symbian::initZones() {
	int i;

	_currentZone = 0;

	for (i = 0; i < TOTAL_ZONES; i++) {
		_mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2));
		_mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2));
	}
}


/** Vibration support */
#ifdef  USE_VIBRA_SE_PXXX
void OSystem_SDL_Symbian::initializeVibration() {
	_vibrationApi = SonyEricsson::CVibration::NewL();
}

void OSystem_SDL_Symbian::vibrationOn(int vibraLength) {
	// initialize?
	if (!_vibrationApi) _vibrationApi = SonyEricsson::CVibration::NewL();
	// do it!
	_vibrationApi->VibrationOn(1, 1, vibraLength);
}

void OSystem_SDL_Symbian::vibrationOff() {
	_vibrationApi->VibrationOff();
}
#endif //  USE_SE_PXX_VIBRA

