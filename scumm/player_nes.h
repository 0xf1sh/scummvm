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

#ifndef PLAYER_NES_H
#define PLAYER_NES_H

#include "common/scummsys.h"
#include "scumm/music.h"
#include "sound/audiostream.h"

class SoundMixer;

namespace Scumm {

class ScummEngine;

/**
 * Scumm NES sound/music driver.
 */
class Player_NES : public AudioStream, public MusicEngine {
public:
	Player_NES(ScummEngine *scumm);
	virtual ~Player_NES();

	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _sample_rate; }

private:

	void sound_play();
	void playSFX(int nr);
	void playMusic();
	byte fetchSoundByte(int nr);
	void chainCommand(int chan);
	void checkSilenceChannels(int chan);

	void APU_writeChannel(int chan, int offset, byte value);
	void APU_writeControl(byte value);
	byte APU_readStatus();

	void do_mix(int16 *buf, uint len);

	ScummEngine *_vm;
	SoundMixer *_mixer;
	int _sample_rate;
	int _samples_per_frame;
	int _current_sample;

	static const int MAXVOLUME = 0x7F;
	static const int NUMSLOTS = 3;
	static const int NUMCHANS = 4;

	struct slot {
		int framesleft;
		int id;
		int type;
		byte *data;
		int offset;
	} _slot[NUMSLOTS];

	struct mchan {
		int command;
		int framedelay;
		int pitch;
		int volume;
		int voldelta;
		int envflags;
		int cmdlock;
	} _mchan[NUMCHANS];

	bool isSFXplaying, wasSFXplaying;

	byte *dataStart;
	int numNotes;
	byte *auxData1;
	byte *auxData2;

	byte *soundptr;
};

} // End of namespace Scumm

#endif
