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

#include <stdafx.h>

#include "sound/mixer.h"

class SmushChannel;

class SmushMixer {
private:

	SoundMixer *_mixer;
	struct {
		int id;
		SmushChannel *chan;
		bool first;
		int mixer_index;
	} _channels[SoundMixer::NUM_CHANNELS];

	int _nextIndex;
	int _soundFrequency;

public:

	SmushMixer(SoundMixer *);
	virtual ~SmushMixer();
	SmushChannel *findChannel(int32 track);
	bool addChannel(SmushChannel *c);
	bool handleFrame();
	bool stop();
	bool update();
	bool _silentMixer;
};
