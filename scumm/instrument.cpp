/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "stdafx.h"
#include "scumm/scumm.h"
#include "scumm/saveload.h"
#include "scumm/instrument.h"
#include "sound/mididrv.h"

#define NATIVE_MT32 false

static const byte mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
 	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};

static struct {
	char *name;
	byte program;
}
roland_to_gm_map [] = {
	// Monkey Island 2 instruments
	// TODO: Complete
//	{ "badspit   ", ??? },
	{ "Big Drum  ", 116 },
//	{ "burp      ", ??? },
//	{ "dinkfall  ", ??? },
//	{ "Fire Pit  ", ??? },
//	{ "foghorn   ", ??? },
	{ "glop      ",  39 },
//	{ "jacob's la", ??? },
	{ "LeshBass  ",  33 }, 
//	{ "lowsnort  ", ??? },
//	{ "ML explosn", ??? },
	{ "ReggaeBass",  32 },
//	{ "rope fall ", ??? },
//	{ "rumble    ", ??? },
//	{ "SdTrk Bend", ??? },
//	{ "snort     ", ??? },
//	{ "spitting  ", ??? },
	{ "Swell 1   ",  95 },
	{ "Swell 2   ",  95 }
//	{ "thnderclap", ??? },

	// Fate of Atlantis instruments
	// TODO: Build
//	{ "*aah!     ", ??? },
//	{ "*ooh!     ", ??? },
//	{ "*ShotFar4 ", ??? },
//	{ "*splash3  ", ??? },
//	{ "*torpedo5 ", ??? },
//	{ "*whip3    ", ??? },
//	{ "*woodknock", ??? },
//	{ "35 lavabub", ??? },
//	{ "49 bzzt!  ", ??? },
//	{ "applause  ", ??? },
//	{ "Arabongo  ", ??? },
//	{ "Big Drum  ", ??? }, // DUPLICATE (todo: confirm)
//	{ "bodythud1 ", ??? },
//	{ "boneKLOK2 ", ??? },
//	{ "boom10    ", ??? },
//	{ "boom11    ", ??? },
//	{ "boom15    ", ??? },
//	{ "boxclik1a ", ??? },
//	{ "brassbonk3", ??? },
//	{ "carstart  ", ??? },
//	{ "cb tpt 2  ", ??? },
//	{ "cell door ", ??? },
//	{ "chains    ", ??? },
//	{ "crash     ", ??? },
//	{ "crsrt/idl3", ??? },
//	{ "Fire Pit  ", ??? }, // DUPLICATE (todo: confirm)
//	{ "Fzooom    ", ??? },
//	{ "Fzooom 2  ", ??? },
//	{ "ghostwhosh", ??? },
//	{ "glasssmash", ??? },
//	{ "gloop2    ", ??? },
//	{ "gunShotNea", ??? },
//	{ "idoorclse ", ??? },
//	{ "knife     ", ??? },
//	{ "lavacmbl4 ", ??? },
//	{ "Mellow Str", ??? },
//	{ "mtlheater1", ??? },
//	{ "pachinko5 ", ??? },
//	{ "Ping1     ", ??? },
//	{ "rockcrunch", ??? },
//	{ "rumble    ", ??? }, // DUPLICATE (todo: confirm)
//	{ "runngwatr ", ??? },
//	{ "scrape2   ", ??? },
//	{ "snakeHiss ", ??? },
//	{ "snort     ", ??? }, // DUPLICATE (todo: confirm)
//	{ "spindle4  ", ??? },
//	{ "splash2   ", ??? },
//	{ "squirel   ", ??? },
//	{ "steam3    ", ??? },
//	{ "stonwheel6", ??? },
//	{ "street    ", ??? },
//	{ "trickle4  ", ??? }
};



class Instrument_Program : public InstrumentInternal {
private:
	byte _program;
	bool _mt32;

public:
	Instrument_Program (byte program, bool mt32);
	Instrument_Program (Serializer *s);
	void saveOrLoad (Serializer *s);
	void send (MidiChannel *mc);
};

class Instrument_Adlib : public InstrumentInternal {
private:
	struct {
		byte flags_1;
		byte oplvl_1;
		byte atdec_1;
		byte sustrel_1;
		byte waveform_1;
		byte flags_2;
		byte oplvl_2;
		byte atdec_2;
		byte sustrel_2;
		byte waveform_2;
		byte feedback;
		byte flags_a;
		struct { byte a,b,c,d,e,f,g,h; } extra_a;
		byte flags_b;
		struct { byte a,b,c,d,e,f,g,h; } extra_b;
		byte duration;
	} _instrument;

public:
	Instrument_Adlib (byte *data);
	Instrument_Adlib (Serializer *s);
	void saveOrLoad (Serializer *s);
	void send (MidiChannel *mc);
};

class Instrument_Roland : public InstrumentInternal {
private:
	struct {
		byte roland_id;
		byte device_id;
		byte model_id;
		byte command;
		byte address[3];
		struct {
			byte name[10];
			byte partial_struct12;
			byte partial_struct34;
			byte partial_mute;
			byte env_mode;
		} common;
		struct {
			byte wg_pitch_coarse;
			byte wg_pitch_fine;
			byte wg_pitch_keyfollow;
			byte wg_pitch_bender_sw;
			byte wg_waveform_pcm_bank;
			byte wg_pcm_wave_num;
			byte wg_pulse_width;
			byte wg_pw_velo_sens;
			byte p_env_depth;
			byte p_evn_velo_sens;
			byte p_env_time_keyf;
			byte p_env_time[4];
			byte p_env_level[3];
			byte p_env_sustain_level;
			byte end_level;
			byte p_lfo_rate;
			byte p_lfo_depth;
			byte p_lfo_mod_sens;
			byte tvf_cutoff_freq;
			byte tvf_resonance;
			byte tvf_keyfollow;
			byte tvf_bias_point_dir;
			byte tvf_bias_level;
			byte tvf_env_depth;
			byte tvf_env_velo_sens;
			byte tvf_env_depth_keyf;
			byte tvf_env_time_keyf;
			byte tvf_env_time[5];
			byte tvf_env_level[3];
			byte tvf_env_sustain_level;
			byte tva_level;
			byte tva_velo_sens;
			byte tva_bias_point_1;
			byte tva_bias_level_1;
			byte tva_bias_point_2;
			byte tva_bias_level_2;
			byte tva_env_time_keyf;
			byte tva_env_time_v_follow;
			byte tva_env_time[5];
			byte tva_env_level[3];
			byte tva_env_sustain_level;
		} partial[4];
	} _instrument;

	char _instrument_name [11];

public:
	Instrument_Roland (byte *data);
	Instrument_Roland (Serializer *s);
	void saveOrLoad (Serializer *s);
	void send (MidiChannel *mc);
};



////////////////////////////////////////
//
// Instrument class members
//
////////////////////////////////////////

void Instrument::clear()
{
	if (_instrument)
		delete _instrument;
	_instrument = NULL;
	_type = itNone;
}

void Instrument::program (byte program, bool mt32)
{
	clear();
	if (program > 127)
		return;
	_type = itProgram;
	_instrument = new Instrument_Program (program, mt32);
}

void Instrument::adlib (byte *instrument)
{
	clear();
	if (!instrument)
		return;
	_type = itAdlib;
	_instrument = new Instrument_Adlib (instrument);
}

void Instrument::roland (byte *instrument)
{
	clear();
	if (!instrument)
		return;
	_type = itRoland;
	_instrument = new Instrument_Roland (instrument);
}

void Instrument::saveOrLoad (Serializer *s)
{
	if (s->isSaving()) {
		s->saveByte (_type);
		if (_instrument)
			_instrument->saveOrLoad (s);
	} else {
		clear();
		byte type = s->loadByte();
		switch (type) {
		case itNone:
			break;
		case itProgram:
			_instrument = new Instrument_Program (s);
			break;
		case itAdlib:
			_instrument = new Instrument_Adlib (s);
			break;
		case itRoland:
			_instrument = new Instrument_Roland (s);
			break;
		default:
			warning ("No known instrument classification #%d", (int) type);
		}
	}
}



////////////////////////////////////////
//
// Instrument_Program class members
//
////////////////////////////////////////

Instrument_Program::Instrument_Program (byte program, bool mt32) :
_program (program),
_mt32 (mt32)
{
	if (program > 127)
		_program = 255;
}

Instrument_Program::Instrument_Program (Serializer *s)
{
	_program = 255;
	if (!s->isSaving())
		saveOrLoad (s);
}

void Instrument_Program::saveOrLoad (Serializer *s)
{
	_program = s->loadByte();
	_mt32 = (s->loadByte() > 0);
}

void Instrument_Program::send (MidiChannel *mc)
{
	if (_program > 127)
		return;

	if (NATIVE_MT32) // if (mc->device()->mt32device())
		mc->programChange (_mt32 ? _program : _program /*gm_to_mt32 [_program]*/);
	else
		mc->programChange (_mt32 ? mt32_to_gm [_program] : _program);
}



////////////////////////////////////////
//
// Instrument_Adlib class members
//
////////////////////////////////////////

Instrument_Adlib::Instrument_Adlib (byte *data)
{
	memcpy (&_instrument, data, sizeof (_instrument));
}

Instrument_Adlib::Instrument_Adlib (Serializer *s)
{
	if (!s->isSaving())
		saveOrLoad (s);
	else
		memset (&_instrument, 0, sizeof (_instrument));
}

void Instrument_Adlib::saveOrLoad (Serializer *s)
{
	if (s->isSaving())
		s->saveBytes (&_instrument, sizeof (_instrument));
	else
		s->loadBytes (&_instrument, sizeof (_instrument));
}

void Instrument_Adlib::send (MidiChannel *mc)
{
	mc->sysEx_customInstrument ('ADL ', (byte *) &_instrument);
}



////////////////////////////////////////
//
// Instrument_Roland class members
//
////////////////////////////////////////

Instrument_Roland::Instrument_Roland (byte *data)
{
	memcpy (&_instrument, data, sizeof (_instrument));
	memcpy (&_instrument_name, &_instrument.common.name, sizeof (_instrument.common.name));
	_instrument_name[10] = '\0';
}

Instrument_Roland::Instrument_Roland (Serializer *s)
{
	_instrument_name[0] = '\0';
	if (!s->isSaving())
		saveOrLoad (s);
	else
		memset (&_instrument, 0, sizeof (_instrument));
}

void Instrument_Roland::saveOrLoad (Serializer *s)
{
	if (s->isSaving())
		s->saveBytes (&_instrument, sizeof (_instrument));
	else
		s->loadBytes (&_instrument, sizeof (_instrument));
	memcpy (&_instrument_name, &_instrument.common.name, sizeof (_instrument.common.name));
	_instrument_name[10] = '\0';
}

void Instrument_Roland::send (MidiChannel *mc)
{
	if (NATIVE_MT32) { // if (mc->device()->mt32device()) {
		_instrument.device_id = mc->getNumber();
		mc->device()->sysEx ((byte *) &_instrument, sizeof (_instrument));
	} else {
		// Convert to a GM program change.
		byte i;
		for (i = 0; i != ARRAYSIZE(roland_to_gm_map); ++i) {
			if (!memcmp (roland_to_gm_map[i].name, _instrument.common.name, 10)) {
				mc->programChange (roland_to_gm_map[i].program);
				return;
			}
		}
		warning ("MT-32 instrument \"%s\" not supported yet", _instrument_name);
		mc->programChange (0);
	}
}
