/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2004  Marcus Comstedt
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

#include <common/system.h>
#include <ronin/soundcommon.h>

#define NUM_BUFFERS 4
#define SOUND_BUFFER_SHIFT 3

class Interactive
{
 public:
  virtual int key(int k, byte &shiftFlags) = 0;
  virtual void mouse(int x, int y) = 0;
};

#include "softkbd.h"

class OSystem_Dreamcast : public OSystem {

 public:
  OSystem_Dreamcast();


  // Determine whether the backend supports the specified feature.
  bool hasFeature(Feature f);

  // En-/disable the specified feature.
  void setFeatureState(Feature f, bool enable);

  // Query the state of the specified feature.
  bool getFeatureState(Feature f);

  // Retrieve a list of all graphics modes supported by this backend.
  const GraphicsMode *getSupportedGraphicsModes() const;

  // Return the ID of the 'default' graphics mode.
  int getDefaultGraphicsMode() const;

  // Switch to the specified graphics mode.
  bool setGraphicsMode(int mode);

  // Determine which graphics mode is currently active.
  int getGraphicsMode() const;

  // Set colors of the palette
  void setPalette(const byte *colors, uint start, uint num);

  // Set the size of the video bitmap.
  // Typically, 320x200
  void initSize(uint w, uint h, int overlayScale);
  int16 getHeight() { return _screen_h; }
  int16 getWidth() { return _screen_w; }

  // Draw a bitmap to screen.
  // The screen will not be updated to reflect the new bitmap
  void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

  // Update the dirty areas of the screen
  void updateScreen();

  // Either show or hide the mouse cursor
  bool showMouse(bool visible);

  // Move ("warp") the mouse cursor to the specified position.
  void warpMouse(int x, int y);

  // Set the bitmap that's used when drawing the cursor.
  void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor);
  
  // Shaking is used in SCUMM. Set current shake position.
  void setShakePos(int shake_pos);

  // Get the number of milliseconds since the program was started.
  uint32 getMillis();
  
  // Delay for a specified amount of milliseconds
  void delayMillis(uint msecs);
  
  // Get the next event.
  // Returns true if an event was retrieved.	
  bool pollEvent(Event &event);
  
  // Set function that generates samples 
  bool setSoundCallback(SoundProc proc, void *param);
  void clearSoundCallback();

  // Determine the output sample rate. Audio data provided by the sound
  // callback will be played using this rate.
  int getOutputSampleRate() const;
		
  // Initialise the specified CD drive for audio playback.
  bool openCD(int drive);

  // Poll cdrom status
  // Returns true if cd audio is playing
  bool pollCD();

  // Play cdrom audio track
  void playCD(int track, int num_loops, int start_frame, int duration);
  
  // Stop cdrom audio track
  void stopCD();

  // Update cdrom audio status
  void updateCD();

  // Quit
  void quit();

  // Overlay
  void showOverlay();
  void hideOverlay();
  void clearOverlay();
  void grabOverlay(int16 *buf, int pitch);
  void copyRectToOverlay(const int16 *buf, int pitch, int x, int y, int w, int h);

  // Add a callback timer
  void setTimerCallback(TimerProc callback, int timer);

  // Mutex handling
  MutexRef createMutex();
  void lockMutex(MutexRef mutex);
  void unlockMutex(MutexRef mutex);
  void deleteMutex(MutexRef mutex);

  // Set a window caption or any other comparable status display to the
  // given value.
  void setWindowCaption(const char *caption);

  // Savefile handling
  SaveFileManager *getSavefileManager();


  // Extra SoftKbd support
  void mouseToSoftKbd(int x, int y, int &rx, int &ry) const;


  static OSystem *create();


 private:

  SoftKeyboard _softkbd;

  int _ms_cur_x, _ms_cur_y, _ms_cur_w, _ms_cur_h, _ms_old_x, _ms_old_y;
  int _ms_hotspot_x, _ms_hotspot_y, _ms_visible, _devpoll;
  int _current_shake_pos, _screen_w, _screen_h;
  int _overlay_x, _overlay_y;
  unsigned char *_ms_buf;
  unsigned char _ms_keycolor;
  SoundProc _sound_proc;
  void *_sound_proc_param;
  bool _overlay_visible, _overlay_dirty, _screen_dirty;
  int _screen_buffer, _overlay_buffer, _mouse_buffer;
  bool _aspect_stretch, _softkbd_on;
  float _overlay_fade, _xscale, _yscale, _top_offset;
  int _softkbd_motion;

  uint32 _timer_duration, _timer_next_expiry;
  bool _timer_active;
  int (*_timer_callback) (int);

  unsigned char *screen;
  unsigned short *mouse;
  unsigned short *overlay;
  void *screen_tx[NUM_BUFFERS];
  void *mouse_tx[NUM_BUFFERS];
  void *ovl_tx[NUM_BUFFERS];
  unsigned short palette[256];

  int temp_sound_buffer[RING_BUFFER_SAMPLES>>SOUND_BUFFER_SHIFT];

  void checkSound();

  void drawMouse(int xdraw, int ydraw, int w, int h,
		 unsigned char *buf, bool visible);

  void setScaling();
};


extern int handleInput(struct mapledev *pad,
		       int &mouse_x, int &mouse_y,
		       byte &shiftFlags, Interactive *inter = NULL);
extern void initSound();
extern bool selectGame(char *&, char *&, class Icon &);

