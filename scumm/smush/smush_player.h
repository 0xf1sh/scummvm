/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef SMUSH_PLAYER_H
#define SMUSH_PLAYER_H

#include "common/util.h"
#include "chunk.h"
#include "codec37.h"
#include "codec47.h"

class SmushFont;
class SmushMixer;
class StringResource;

class SmushPlayer {
private:

	Scumm *_scumm;
	int _version;
	int32 _nbframes;
	SmushMixer *_smixer;
	int16 _deltaPal[0x300];
	byte _pal[0x300];
	StringResource *_strings;
	SmushFont *_sf[5];
	Codec37Decoder _codec37;
	Codec47Decoder _codec47;
	int dst_width, dst_height;
	FileChunk *_base;
	byte *_frameBuffer;

	bool _codec37Called;
	bool _skipNext;
	bool _subtitles;
	bool _skips[37];
	int32 _frame;

	int _IACTchannel;
	byte _IACToutput[4096];
	int32 _IACTpos;
	bool _storeFrame;
	int _soundFrequency;
	bool _alreadyInit;
	int _speed;
	bool _outputSound;

public:

	int _width, _height;
	byte *_data;
	bool _smushProcessFrame;
	bool _updateNeeded;

	SmushPlayer(Scumm *, int, bool);
	~SmushPlayer();
	void updatePalette(void);
	void parseNextFrame();
	void init();
	void deinit();
	void setupAnim(const char *file, const char *directory);
	void updateScreen();
	void play(const char *filename, const char *directory);
	void setPalette(byte *palette);

protected:

	bool readString(const char *file, const char *directory);
	void checkBlock(const Chunk &, Chunk::type, uint32 = 0);
	void handleAnimHeader(Chunk &);
	void handleFrame(Chunk &);
	void handleNewPalette(Chunk &);
	void handleFrameObject(Chunk &);
	void handleSoundBuffer(int32, int32, int32, int32, int32, int32, Chunk &, int32);
	void handleImuseBuffer(int32, int32, int32, int32, int32, int32, Chunk &, int32);
	void handleSoundFrame(Chunk &);
	void handleSkip(Chunk &);
	void handleStore(Chunk &);
	void handleFetch(Chunk &);
	void handleImuseAction(Chunk &);
	void handleTextResource(Chunk &);
	void handleDeltaPalette(Chunk &);
	void readPalette(byte *, Chunk &);
};

#endif
