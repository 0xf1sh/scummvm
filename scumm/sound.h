/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef SOUND_H
#define SOUND_H

#include "common/scummsys.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

class File;

namespace Scumm {

class ScummEngine;
class ScummFile;

struct MP3OffsetTable;
struct SaveLoadEntry;

enum {
	kTalkSoundID = 10000
};

class Sound {
#ifdef __PALM_OS__
public:
#else
protected:
#endif
	enum SoundMode {
		kVOCMode,
		kMP3Mode,
		kVorbisMode,
		kFlacMode
	};

#ifdef __PALM_OS__
protected:
#endif
	ScummEngine *_vm;

	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos;

	struct {
		int16 sound;
		int32 offset;
		int16 channel;
		int16 flags;
	} _soundQue2[10];

	ScummFile *_sfxFile;
	SoundMode _soundMode;	
	MP3OffsetTable *_offsetTable;	// For compressed audio
	int _numSoundEffects;		// For compressed audio

	uint32 _talk_sound_a1, _talk_sound_a2, _talk_sound_b1, _talk_sound_b2;
	byte _talk_sound_mode, _talk_sound_channel;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[64];
	uint _curSoundPos;

	int _overrideFreq;

	int16 _currentCDSound;
	int16 _currentMusic;
public:
	PlayingSoundHandle _talkChannelHandle;	// Handle of mixer channel actor is talking on
	PlayingSoundHandle _musicChannelHandle;	// Handle of mixer channel music is on

	bool _soundsPaused;
	byte _sfxMode;

public:
	Sound(ScummEngine *parent);
	~Sound();
	void addSoundToQueue(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0);
	void addSoundToQueue2(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0);
	void processSoundQues();
	void setOverrideFreq(int freq);
	void playSound(int soundID, int heOffset, int heChannel, int heFlags);
	void startTalkSound(uint32 offset, uint32 b, int mode, PlayingSoundHandle *handle = NULL);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	int isSoundRunning(int sound) const;
	bool isSoundInUse(int sound) const;
	void stopSound(int sound);
	void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int channel = 0);
	void setupSound();
	void pauseSounds(bool pause);

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int numLoops, int startFrame, int duration);
	void stopCD();
	int pollCD() const;
	void updateCD();
	int getCurrentCDSound() const { return _currentCDSound; }

	// Used by the save/load system:
	const SaveLoadEntry *getSaveLoadEntries();

protected:
	ScummFile *openSfxFile();
	bool isSfxFinished() const;
	void processSfxQueues();

	bool isSoundInQueue(int sound) const;
};

/**
 * An audio stream to which additional data can be appended on-the-fly.
 * Used by SMUSH and iMuseDigital.
 */
class AppendableAudioStream : public AudioStream {
public:
	virtual void append(const byte *data, uint32 len) = 0;
	virtual void finish() = 0;
};

AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags, uint32 len);


} // End of namespace Scumm

#endif
