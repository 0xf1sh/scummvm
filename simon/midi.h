/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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

#ifndef SIMON_MIDI_H
#define SIMON_MIDI_H

#include "sound/mididrv.h"
#include "sound/midiparser.h"

class File;
class OSystem;

struct MusicInfo {
	MidiParser *parser;
	byte * data;
	byte   num_songs;         // For Type 1 SMF resources
	byte * songs[16];         // For Type 1 SMF resources
	uint32 song_sizes[16];    // For Type 1 SMF resources
	MidiChannel *channel[16]; // Dynamic remapping of channels to resolve conflicts

	MusicInfo() { clear(); }
	void clear() {
		parser = 0; data = 0; num_songs = 0;
		memset (songs, 0, sizeof (songs));
		memset (song_sizes, 0, sizeof (song_sizes));
		memset (channel, 0, sizeof (channel));
	}
};

class MidiPlayer : public MidiDriver {
protected:
	OSystem *_system;
	void *_mutex;
	MidiDriver *_driver;

	MusicInfo _music;
	MusicInfo _sfx;
	MusicInfo *_current; // Allows us to establish current context for operations.

	// These are maintained for both music and SFX
	byte _volumeTable[16]; // 0-127
	byte _masterVolume;    // 0-255
	bool _paused;

	// These are only used for music.
	byte _currentTrack;
	bool _loopTrack;
	byte _queuedTrack;
	bool _loopQueuedTrack;

	static void onTimer (void *data);
	void clearConstructs();
	void clearConstructs (MusicInfo &info);

public:
	bool _enable_sfx;

public:
	MidiPlayer (OSystem *system);
	virtual ~MidiPlayer();

	void loadSMF (File *in, int song, bool sfx = false);
	void loadMultipleSMF (File *in, bool sfx = false);
	void loadXMIDI (File *in, bool sfx = false);

	void setLoop (bool loop);
	void startTrack(int track);
	void queueTrack (int track, bool loop);
	bool isPlaying (bool check_queued = false) { return (_currentTrack != 255 && (_queuedTrack != 255 || !check_queued)); }

	void stop();
	void pause (bool b);

	int  get_volume() { return _masterVolume; }
	void set_volume (int volume);
	void set_driver (MidiDriver *md);

public:
	// MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent (byte type, byte *data, uint16 length);

	// Timing functions - MidiDriver now operates timers
	void setTimerCallback (void *timer_param, void (*timer_proc) (void *)) { }
	uint32 getBaseTempo (void) { return _driver ? _driver->getBaseTempo() : 0; }

	// Channel allocation functions
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }
};

#endif
