/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/singleton.h"

class OSystem;

/**
 * Custome object factory for OSystem.
 */
template <>
OSystem *makeInstance<>();


/**
 * Interface for ScummVM backends. If you want to port ScummVM to a system
 * which is not currently covered by any of our backends, this is the place
 * to start. ScummVM will create an instance of a subclass of this interface
 * and use it to interact with the system.
 *
 * In particular, a backend provides a video surface for ScummVM to draw in;
 * methods to create timers, to handle user input events,
 * control audio CD playback, and sound output.
 */
class OSystem : public Common::Singleton<OSystem> {
public:
	
	/** @name Feature flags */
	//@{
	
	/**
	 * A feature in this context means an ability of the backend which can be
	 * either on or off. Examples include:
	 *  - fullscreen mode
	 *  - aspect ration correction
	 *  - a virtual keyboard for text entry (on PDAs)
	 */
	enum Feature {
		/**
		 * If your backend supports both a windowed and a fullscreen mode,
		 * then this feature flag can be used to switch between the two.
		 */
		kFeatureFullscreenMode,

		/**
		 * Control aspect ratio correction. Aspect ratio correction is used to
		 * correct games running at 320x200 (i.e with an aspect ratio of 8:5),
		 * but which on their original hardware were displayed with the
		 * standard 4:3 ratio (that is, the original graphics used non-square
		 * pixels). When the backend support this, then games running at
		 * 320x200 pixels should be scaled up to 320x240 pixels. For all other
		 * resolutions, ignore this feature flag.
		 * @note You can find utility functions in common/scaler.h which can
		 *       be used to implement aspect ratio correction. In particular,
		 *       stretch200To240() can stretch a rect, including (very fast)
		 *       interpolation, and works in-place.
		 */
		kFeatureAspectRatioCorrection,

		/**
		 * Determine whether a virtual keyboard is too be shown or not.
		 * This would mostly be implemented by backends for hand held devices,
		 * like PocketPC, Palms, Symbian phones like the P800, Zaurus, etc.
		 */
		kFeatureVirtualKeyboard,

		/**
		 * This flag is a bit more obscure: it gives a hint to the backend that
		 * the frontend code is very inefficient in doing screen updates. So
		 * the frontend might do a lot of fullscreen blits even though only a
		 * tiny portion of the actual screen data changed. In that case, it 
		 * might pay off for the backend to compute which parts actual changed,
		 * and then only mark those as dirty.
		 * Implementing this is purely optional, and no harm should arise 
		 * when not doing so (except for decreased speed in said frontends).
		 */
		kFeatureAutoComputeDirtyRects
	};
	
	/**
	 * Determine whether the backend supports the specified feature.
	 */
	virtual bool hasFeature(Feature f) { return false; }

	/**
	 * En-/disable the specified feature. For example, this may be used to
	 * enable fullscreen mode, or to deactivate aspect correction, etc.
	 */
	virtual void setFeatureState(Feature f, bool enable) {}
	
	/**
	 * Query the state of the specified feature. For example, test whether
	 * fullscreen mode is active or not.
	 */
	virtual bool getFeatureState(Feature f) { return false; }

	//@}



	/** @name Graphics */
	//@{

	/**
	 * Description of a graphics mode.
	 */
	struct GraphicsMode {
		/**
		 * The 'name' of the graphics mode. This name is matched when selecting
		 * a mode via the command line, or via the config file.
		 * Examples: "1x", "advmame2x", "hq3x"
		 */
		const char *name;
		/**
		 * Human readable description of the scaler.
		 * Examples: "Normal (no scaling)", "AdvMAME2x", "HQ3x"
		 */
		const char *description;
		/**
		 * ID of the graphics mode. How to use this is completely up to the
		 * backend. This value will be passed to the setGraphicsMode(int)
		 * method by client code.
		 */
		int id;
	};

	/**
	 * Retrieve a list of all graphics modes supported by this backend.
	 * This can be both video modes as well as graphic filters/scalers;
	 * it is completely up to the backend maintainer to decide what is
	 * appropriate here and what not.
	 * The list is terminated by an all-zero entry.
	 * @return a list of supported graphics modes
	 */
	virtual const GraphicsMode *getSupportedGraphicsModes() const = 0;
	
	/**
	 * Return the ID of the 'default' graphics mode. What exactly this means
	 * is up to the backend. This mode is set by the client code when no user
	 * overrides are present (i.e. if no custom graphics mode is selected via
	 * the command line or a config file).
	 *
	 * @return the ID of the 'default' graphics mode
	 */
	virtual int getDefaultGraphicsMode() const = 0;
	
	/**
	 * Switch to the specified graphics mode. If switching to the new mode
	 * failed, this method returns false.
	 *
	 * @param mode	the ID of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 */
	virtual bool setGraphicsMode(int mode) = 0;

	/**
	 * Switch to the graphics mode with the given name. If 'name' is unknown,
	 * or if switching to the new mode failed, this method returns false.
	 *
	 * @param name	the name of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 * @note This is implemented via the setGraphicsMode(int) method, as well
	 *       as getSupportedGraphicsModes() and getDefaultGraphicsMode().
	 *       In particular, backends do not have to overload this!
	 */
	bool setGraphicsMode(const char *name);

	/**
	 * Determine which graphics mode is currently active.
	 * @return the active graphics mode
	 */
	virtual int getGraphicsMode() const = 0;

	/**
	 * Set the size of the virtual screen. Typical sizes include:
	 *  - 320x200 (e.g. for most SCUMM games, and Simon)
	 *  - 320x240 (e.g. for FM-TOWN SCUMM games)
	 *  - 640x480 (e.g. for Curse of Monkey Island)
	 *
	 * This is the resolution for which the client code generates data;
	 * this is not necessarily equal to the actual display size. For example,
	 * a backend may magnify the graphics to fit on screen (see also the
	 * GraphicsMode); stretch the data to perform aspect ratio correction;
	 * or shrink it to fit on small screens (in cell phones).
	 *
	 * @param width		the new virtual screen width
	 * @param height	the new virtual screen height
	 */
	virtual void initSize(uint width, uint height) = 0;

	/**
	 * Begin a new GFX transaction, which is a sequence of GFX mode changes.
	 * The idea behind GFX transactions is to make it possible to activate
	 * several different GFX changes at once as a "batch" operation. For
	 * example, assume we are running in 320x200 with a 2x scaler (thus using
	 * 640x400 pixels in total). Now, we want to switch to 640x400 with the 1x
	 * scaler. Without transactions, we have to choose whether we want to first
	 * switch the scaler mode, or first to 640x400 mode. In either case,
	 * depending on the backend implementation, some ugliness may result.
	 * E.g. the window might briefly switch to 320x200 or 1280x800.
	 * Using transactions, this can be avoided.
	 *
	 * @note Transaction support is optional, and the default implementations
	 *       of the relevant methods simply do nothing.
	 * @see endGFXTransaction
	 */
	virtual void beginGFXTransaction() {};


	/**
	 * End (and thereby commit) the current GFX transaction.
	 * @see beginGFXTransaction
	 */
	virtual void endGFXTransaction() {};


	/**
	 * Returns the currently set virtual screen height.
	 * @see initSize
	 * @return the currently set virtual screen height
	 */
	virtual int16 getHeight() = 0;

	/**
	 * Returns the currently set virtual screen width.
	 * @see initSize
	 * @return the currently set virtual screen width
	 */
	virtual int16 getWidth() = 0;

	/**
	 * Replace the specified range of the palette with new colors.
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * The palette data is specified in interleaved RGBA format. That is, the
	 * first byte of the memory block 'colors' points at is the red component
	 * of the first new color; the second byte the blue component of the first
	 * new color; the third byte the green component, the last byte to the alpha
	 * (transparency) value. Then the second color starts, and so on. So memory
	 * looks like this: R1-G1-B1-A1-R2-G2-B2-A2-R3-...
	 *
	 * @param colors	the new colors, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note It is an error if start+num exceeds 256, behaviour is undefined
	 *       in that case (the backend may ignore it silently or assert).
	 * @note The alpha value is not actually used, and future revisions of this
	 *       API are probably going to remove it.
	 */
	virtual void setPalette(const byte *colors, uint start, uint num) = 0;

	/**
	 * Blit a bitmap to the virtual screen.
	 * The real screen will not immediately be updated to reflect the changes.
	 * Client code has to to call updateScreen to ensure any changes are
	 * visible to the user. This can be used to optimize drawing and reduce
	 * flicker.
	 * @see updateScreen
	 */
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) = 0;
	
	/**
	 * Clear the screen to black.
	 */
	virtual void clearScreen() {}

	/** Update the dirty areas of the screen. */
	virtual void updateScreen() = 0;

	/**
	 * Set current shake position, a feature needed for some SCUMM screen effects.
	 * The effect causes the displayed graphics to be shifted upwards by the specified 
	 * (always positive) offset. The area at the bottom of the screen which is moved
	 * into view by this is filled by black. This does not cause any graphic data to
	 * be lost - that is, to restore the original view, the game engine only has to
	 * call this method again with a 0 offset. No calls to copyRectToScreen are necessary.
	 * @param shakeOffset	the shake offset
	 *
	 * @todo This is a rather special screen effect, only used by the SCUMM
	 *       frontend - we should consider removing it from the backend API
	 *       and instead implement the functionality in the frontend.
	 */
	virtual void setShakePos(int shakeOffset) = 0;

	//@}



	/** @name Overlay */
	//@{
	virtual void showOverlay() = 0;
	virtual void hideOverlay() = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(OverlayColor *buf, int pitch) = 0;
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual int16 getOverlayHeight()  { return getHeight(); }
	virtual int16 getOverlayWidth()   { return getWidth(); }

	/**
	* Convert the given RGB triplet into an OverlayColor. A OverlayColor can
	 * be 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation generates a 16 bit color value, in the 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @see colorToRGB
	 */
	virtual OverlayColor RGBToColor(uint8 r, uint8 g, uint8 b) {
		return ((((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | ((b >> 3) & 0x1F));
	}

	/**
	 * Convert the given OverlayColor into a RGB triplet. An OverlayColor can
	 * be 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation takes a 16 bit color value and assumes it to be in 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @see RGBToColor
	 */
	virtual void colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
		r = (((color >> 11) & 0x1F) << 3);
		g = (((color >> 5) & 0x3F) << 2);
		b = ((color&0x1F) << 3);
	}

	//@}



	/** @name Mouse */
	//@{

	/** Show or hide the mouse cursor. */
	virtual bool showMouse(bool visible) = 0;

	/** 
	 * Move ("warp") the mouse cursor to the specified position in virtual 
	 * screen coordinates.
	 * @param x		the new x position of the mouse
	 * @param y		the new x position of the mouse
	 */
	virtual void warpMouse(int x, int y) = 0;

	/**
	 * Set the bitmap used for drawing the cursor.
	 *
	 * @param buf		the pixmap data to be used (8bit/pixel)
	 * @param w			width of the mouse cursor
	 * @param h			height of the mouse cursor
	 * @param hotspotX	horizontal offset from the left side to the hotspot
	 * @param hotspotY	vertical offset from the top side to the hotspot
	 * @param keycolor	transparency color index
	 */
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255) = 0;

	//@}



	/** @name Events and Time */
	//@{

	typedef int (*TimerProc)(int interval);

	/**
	 * The types of events backends may generate.
	 * @see Event
	 *
	 * @todo Merge EVENT_LBUTTONDOWN, EVENT_RBUTTONDOWN and EVENT_WHEELDOWN;
	 *       likewiese EVENT_LBUTTONUP, EVENT_RBUTTONUP, EVENT_WHEELUP.
	 *       To do that, we just have to add a field to the Event which
	 *       indicates which button was pressed.
	 */
	enum EventType {
		/** A key was pressed, details in Event::kbd. */
		EVENT_KEYDOWN = 1,
		/** A key was released, details in Event::kbd. */
		EVENT_KEYUP = 2,
		/** The mouse moved, details in Event::mouse. */
		EVENT_MOUSEMOVE = 3,
		EVENT_LBUTTONDOWN = 4,
		EVENT_LBUTTONUP = 5,
		EVENT_RBUTTONDOWN = 6,
		EVENT_RBUTTONUP = 7,
		EVENT_WHEELUP = 8,
		EVENT_WHEELDOWN = 9,

		EVENT_QUIT = 10,
		EVENT_SCREEN_CHANGED = 11
	};

	/**
	 * Keyboard modifier flags, used for Event::kbd::flags.
	 */
	enum {
		KBD_CTRL  = 1 << 0,
		KBD_ALT   = 1 << 1,
		KBD_SHIFT = 1 << 2
	};

	/**
	 * Data structure for an event. A pointer to an instance of Event
	 * can be passed to pollEvent.
	 * @todo Rework/document this structure. It should be made 100% clear which
	 *       field is valid for which event type.
	 *       Implementation wise, we might want to use the classic
	 *       union-of-structs trick. It goes roughly like this:
	 *       struct BasicEvent {
	 *       	EventType type;
	 *       };
	 *       struct MouseMovedEvent : BasicEvent {
	 *       	Common::Point pos;
	 *       };
	 *       struct MouseButtonEvent : MouseMovedEvent {
	 *       	int button;
	 *       };
	 *       struct KeyEvent : BasicEvent {
	 *       	...
	 *       };
	 *       ...
	 *       union Event {
	 *          EventType type;
	 *       	MouseMovedEvent mouse;
	 *       	MouseButtonEvent button;
	 *       	KeyEvent key;
	 *       	...
	 *       };
	 */
	struct Event {
		/** The type of the event. */
		EventType type;
		/**
		  * Keyboard data; only valid for keyboard events (EVENT_KEYDOWN and
		  * EVENT_KEYUP). For all other event types, content is undefined.
		  */
		struct {
			/**
			 * Abstract key code (will be the same for any given key regardless
			 * of modifiers being held at the same time.
			 * For example, this is the same for both 'A' and Shift-'A'.
			 * @todo Document which values are to be used for non-ASCII keys
			 * like F1-F10. For now, let's just say that our primary backend
			 * is the SDL one, and it uses the values SDL uses... so until
			 * we fix this, your best bet is to get a copy of SDL_keysym.h
			 * and look at that, if you want to find out a key code.
			 */
			int keycode;
			/**
			 * ASCII-value of the pressed key (if any). 
			 * This depends on modifiers, i.e. pressing the 'A' key results in
			 * different values here depending on the status of shift, alt and
			 * caps lock.
			 * For the function keys F1-F9, values of 315-323 are used.
			 */
			uint16 ascii;
			/**
			 * Status of the modifier keys. Bits are set in this for each 
			 * pressed modifier
			 * @see KBD_CTRL, KBD_ALT, KBD_SHIFT
			 */
			byte flags;
		} kbd;
		/**
		 * The mouse coordinates, in virtual screen coordinates. Only valid
		 * for mouse events.
		 * Virtual screen coordinatest means: the coordinate system of the
		 * screen area as defined by the most recent call to initSize().
		 */
		Common::Point mouse;
	};

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(Event &event) = 0;

	/** Get the number of milliseconds since the program was started. */
	virtual uint32 getMillis() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	/**
	 * Set the timer callback, a function which is periodically invoked by the
	 * backend. This can for example be done via a background thread.
	 * There is at most one active timer; if this method is called while there
	 * is already an active timer, then the new timer callback should replace
	 * the previous one. In particular, passing a callback pointer value of 0
	 * is legal and can be used to clear the current timer callback.
	 * @see Common::Timer
	 * @note The implementation of this method must be 'atomic' in the sense
	 *       that when the method returns, the previously set callback must
	 *       not be in use anymore (in particular, if timers are implemented
	 *       via threads, then it must be ensured that the timer thread is
	 *       not using the old callback function anymore).
	 *
	 * @param callback	pointer to the callback. May be 0 to reset the timer
	 * @param interval	the interval (in milliseconds) between invocations
	 *                  of the callback
	 */
	virtual void setTimerCallback(TimerProc callback, int interval) = 0;

	//@}



	/**
	 * @name Mutex handling
	 * Historically, the OSystem API used to have a method which allowed
	 * creating threads. Hence mutex support was needed for thread syncing.
	 * To ease portability, though, we decided to remove the threading API.
	 * Instead, we now use timers (see setTimerCallback() and Common::Timer).
	 * But since those may be implemented using threads (and in fact, that's
	 * how our primary backend, the SDL one, does it on many systems), we
	 * still have to do mutex syncing in our timer callbacks.
	 *
	 * Hence backends which do not use threads to implement the timers simply
	 * can use dummy implementations for these methods.
	 */
	//@{

	typedef struct Mutex *MutexRef;

	/**
	 * Create a new mutex.
	 * @return the newly created mutex, or 0 if an error occured.
	 */
	virtual MutexRef createMutex() = 0;

	/**
	 * Lock the given mutex.
	 * @param mutex	the mutex to lock.
	 */
	virtual void lockMutex(MutexRef mutex) = 0;

	/**
	 * Unlock the given mutex.
	 * @param mutex	the mutex to unlock.
	 */
	virtual void unlockMutex(MutexRef mutex) = 0;

	/**
	 * Delete the given mutex. Make sure the mutex is unlocked before you delete it.
	 * If you delete a locked mutex, the behavior is undefined, in particular, your
	 * program may crash.
	 * @param mutex	the mutex to delete.
	 */
	virtual void deleteMutex(MutexRef mutex) = 0;

	//@}



	/** @name Sound */
	//@{
	typedef void (*SoundProc)(void *param, byte *buf, int len);

	/**
	 * Set the audio callback which is invoked whenever samples need to be generated.
	 * Currently, only the 16-bit signed mode is ever used for Simon & Scumm
	 * @param proc		pointer to the callback.
	 * @param param		an arbitrary parameter which is stored and passed to proc.
	 */
	virtual bool setSoundCallback(SoundProc proc, void *param) = 0;

	/**
	 * Remove any audio callback previously set via setSoundCallback, thus effectively
	 * stopping all audio output immediately.
	 * @see setSoundCallback
	 */
	virtual void clearSoundCallback() = 0;

	/**
	 * Determine the output sample rate. Audio data provided by the sound
	 * callback will be played using this rate.
	 * @note Client code other than the sound mixer should _not_ use this
	 *       method. Instead, call SoundMixer::getOutputRate()!
	 * @return the output sample rate
	 */
	virtual int getOutputSampleRate() const = 0;

	//@}



	/**
	 * @name Audio CD
	 * The methods in this group deal with Audio CD playback.
	 */
	//@{

	/**
	 * Initialise the specified CD drive for audio playback.
	 * @return true if the CD drive was inited succesfully
	 */
	virtual bool openCD(int drive) = 0;

	/**
	 * Poll CD status.
	 * @return true if CD audio is playing
	 */
	virtual bool pollCD() = 0;

	/**
	 * Start audio CD playback. 
	 * @param track			the track to play.
	 * @param num_loops		how often playback should be repeated (-1 = infinitely often).
	 * @param start_frame	the frame at which playback should start (75 frames = 1 second).
	 * @param duration		the number of frames to play.
	 */
	virtual void playCD(int track, int num_loops, int start_frame, int duration) = 0;

	/**
	 * Stop audio CD playback.
	 */
	virtual void stopCD() = 0;

	/**
	 * Update cdrom audio status.
	 */
	virtual void updateCD() = 0;

	//@}



	/** @name Miscellaneous */
	//@{
	/** Quit (exit) the application. */
	virtual void quit() = 0;

	/**
	 * Set a window caption or any other comparable status display to the
	 * given value.
	 * @param caption	the window caption to use from now on
	 */
	virtual void setWindowCaption(const char *caption) {}

	/**
	 * Display a message in an 'on screen display'. That is, display it in a
	 * fashion where it is visible on or near the screen (e.g. in a transparent
	 * rectangle over the regular screen content; or in a message box beneath
	 * it; etc.).
	 *
	 * @note There is a default implementation which uses a TimedMessageDialog
	 *       to display the message. Hence implementing this is optional.
	 *
	 * @param msg	the message to display on screen
	 */
	virtual void displayMessageOnOSD(const char *msg);

	/** Savefile management. */
	virtual SaveFileManager *getSavefileManager();

	//@}
};

/** The global OSystem instance. Inited in main(). */
#define g_system	(&OSystem::instance())

namespace Common {

/**
 * Auxillary class to (un)lock a mutex on the stack.
 */
class StackLock {
	OSystem::MutexRef _mutex;
	const char *_mutexName;

	void lock();
	void unlock();
public:
	StackLock(OSystem::MutexRef mutex, const char *mutexName = NULL);
	~StackLock();
};

}	// End of namespace Common


#endif 
