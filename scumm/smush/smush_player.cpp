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
#include "common/file.h"
#include "common/util.h"
#include "common/engine.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse.h"
#include "scumm/imuse_digi.h"
#include "sound/mixer.h"
#include "smush_player.h"
#include "smush_mixer.h"
#include "smush_font.h"
#include "channel.h"
#include "chunk.h"
#include "chunk_type.h"

const int MAX_STRINGS = 200;

class StringResource {
private:

	struct {
		int id;
		char *string;
	} _strings[MAX_STRINGS];

	int _nbStrings;
	int _lastId;
	char *_lastString;

public:

	StringResource() :
		_nbStrings(0),
		_lastId(-1) {
	};
	~StringResource() {
		for(int32 i = 0; i < _nbStrings; i++) {
			delete []_strings[i].string;
		}
	}

	bool init(char *buffer, int32 length) {
		char *def_start = strchr(buffer, '#');
		while(def_start != NULL) {
			char *def_end = strchr(def_start, '\n');
			assert(def_end != NULL);

			char *id_end = def_end;
			while(id_end >= def_start && !isdigit(*(id_end-1))) { 
				id_end--;
			}

			assert(id_end > def_start);
			char *id_start = id_end;
			while(isdigit(*(id_start - 1))) {
				id_start--;
			}

			char idstring[32];
			memcpy(idstring, id_start, id_end - id_start);
			idstring[id_end - id_start] = 0;
			int32 id = atoi(idstring);
			char *data_start = def_end;

			while(*data_start == '\n' || *data_start == '\r') {
				data_start++;
			}
			char *data_end = data_start;

			while(1) {
				if(data_end[-2] == '\r' && data_end[1] == '\n' && data_end[-1] == '\n' && data_end[0] == '\r') {
					break;
				}
				data_end++;
				if(data_end >= buffer + length) {
					data_end = buffer + length;
					break;
				}
			}

			data_end -= 2;
			assert(data_end > data_start);
			char *value = new char[data_end - data_start + 1];
			assert(value);
			memcpy(value, data_start, data_end - data_start);
			value[data_end - data_start] = 0;
			char *line_start = value;
			char *line_end;

			while ((line_end = strchr(line_start, '\n'))) {
				line_start = line_end+1;
				if (line_start[0] == '/' && line_start[1] == '/') {
					line_start += 2;
					if	(line_end[-1] == '\r')
						line_end[-1] = ' ';
					else
						*line_end++ = ' ';
					memmove(line_end, line_start, strlen(line_start)+1);
				}
			}
			_strings[_nbStrings].id = id;
			_strings[_nbStrings].string = value;
			_nbStrings ++;
			def_start = strchr(data_end + 2, '#');
		}
		return true;
	}

	char *get(int id) {
		if(id == _lastId) {
			return _lastString;
		}
		for(int i = 0; i < _nbStrings; i++) {
			if(_strings[i].id == id) {
				_lastId = id;
				_lastString = _strings[i].string;
				return _strings[i].string;
			}
		}
		warning("invalid string id : %d", id);
		_lastId = -1;
		_lastString = "unknown string";
		return _lastString;
	}
};

static StringResource *getStrings(const char *file, const char *directory, bool is_encoded) {
	debug(7, "trying to read text ressources from %s", file);
	File theFile;

	theFile.open(file, directory);
	if (!theFile.isOpen()) {
		return 0;
	}
	int32 length = theFile.size();
	char *filebuffer = new char [length + 1];
	assert(filebuffer);
	theFile.read(filebuffer, length);
	filebuffer[length] = 0;

	if(is_encoded) {
		static const int32 ETRS_HEADER_LENGTH = 16;
		assert(length > ETRS_HEADER_LENGTH);
		Chunk::type type = READ_BE_UINT32(filebuffer);

		if(type != TYPE_ETRS) {
			delete [] filebuffer;
			return getStrings(file, directory, false);
		}

		char *old = filebuffer;
		filebuffer = new char[length - ETRS_HEADER_LENGTH + 1];
		for(int32 i = ETRS_HEADER_LENGTH; i < length; i++) {
			filebuffer[i - ETRS_HEADER_LENGTH] = old[i] ^ 0xCC;
		}
		filebuffer[length - ETRS_HEADER_LENGTH] = '\0';
		delete []old;
		length -= ETRS_HEADER_LENGTH;
	}
	StringResource *sr = new StringResource;
	assert(sr);
	sr->init(filebuffer, length);
	delete []filebuffer;
	return sr;
}

SmushPlayer *player;

void smush_callback(void *ptr) {
	Scumm *scumm = (Scumm *)ptr;
	if (scumm->_smushPlay == false)
		return;

	player->_smushProcessFrame = true;
	player->parseNextFrame();
	player->_smushProcessFrame = false;
}

SmushPlayer::SmushPlayer(Scumm *scumm, int speed, bool subtitles) {
	player = this;
	_version = -1;
	_nbframes = 0;
	_smixer = 0;
	_strings = NULL;
	_skipNext = false;
	_data = NULL;
	_storeFrame = false;
	_width = 0;
	_height = 0;
	_frameBuffer = NULL;
	_sf[0] = NULL;
	_sf[1] = NULL;
	_sf[2] = NULL;
	_sf[3] = NULL;
	_sf[4] = NULL;
	_scumm = scumm;
	_IACTchannel = -1,
	_IACTpos = 0;
	_soundFrequency = 22050;
	_speed = speed;
	_subtitles = subtitles;
	_smushProcessFrame = false;
	
	_mutex = _scumm->_system->create_mutex();
}

SmushPlayer::~SmushPlayer() {
	deinit();
	if (_mutex)
		_scumm->_system->delete_mutex (_mutex);
}

void SmushPlayer::init() {

	_frame = 0;

	_scumm->_sound->pauseBundleMusic(true);
	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->pause(true);
	}
	_scumm->_videoFinished = false;
	_scumm->_insaneState = true;

	_smixer = new SmushMixer(_scumm->_mixer);

	_scumm->setDirtyColors(0, 255);
	_smixer->_silentMixer = _scumm->_silentDigitalImuse;
	_scumm->_smushPlay = true;
	_data = _scumm->virtscr[0].screenPtr + _scumm->virtscr[0].xstart;
	_scumm->_timer->installProcedure(&smush_callback, _speed);

	_alreadyInit = false;
}

void SmushPlayer::deinit() {
	_scumm->_smushPlay = false;
	while (_smushProcessFrame) {}
	_scumm->_timer->releaseProcedure(&smush_callback);

	for(int i = 0; i < 5; i++) {
		if (_sf[i]) {
			delete _sf[i];
			_sf[i] = NULL;
		}
	}

	if(_strings) {
		delete _strings;
		_strings = NULL;
	}

	if(_smixer) {
		_smixer->stop();
		delete _smixer;
		_smixer = NULL;
	}

	_scumm->_insaneState = false;
	_scumm->exitCutscene();
	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->pause(false);
	}
	_scumm->_sound->pauseBundleMusic(false);
	_scumm->_fullRedraw = 1;
}

void SmushPlayer::checkBlock(const Chunk &b, Chunk::type type_expected, uint32 min_size) {
	if(type_expected != b.getType()) {
		error("Chunk type is different from expected : %d != %d", b.getType(), type_expected);
	}
	if(min_size > b.getSize()) {
		error("Chunk size is inferior than minimum required size : %d < %d", b.getSize(), min_size);
	}
}

void SmushPlayer::handleSoundBuffer(int32 track_id, int32 index, int32 max_frames, int32 flags, int32 vol, int32 bal, Chunk &b, int32 size) {
	debug(6, "SmushPlayer::handleSoundBuffer(%d)", track_id);
//	if((flags & 128) == 128) {
//		return;
//	}
//	if((flags & 64) == 64) {
//		return;
//	}
	SmushChannel *c = _smixer->findChannel(track_id);
	if(c == NULL) {
		c = new SaudChannel(track_id, _soundFrequency);
		_smixer->addChannel(c);
	}
	if(index == 0) {
		c->setParameters(max_frames, flags, vol, bal);
	}	else {
		c->checkParameters(index, max_frames, flags, vol, bal);
	}
	c->appendData(b, size);
}

void SmushPlayer::handleSoundFrame(Chunk &b) {
	checkBlock(b, TYPE_PSAD);
	debug(6, "SmushPlayer::handleSoundFrame()");

	int32 track_id = b.getWord();
	int32 index = b.getWord();
	int32 max_frames = b.getWord();
	int32 flags = b.getWord();
	int32 vol = b.getByte();
	int32 bal = b.getChar();
#ifdef DEBUG
	if(index == 0) {
		debug(5, "track_id == %d, max_frames == %d, %d, %d, %d", track_id, max_frames, flags, vol, bal);
	}
#endif
	int32 size = b.getSize() - 10;
	handleSoundBuffer(track_id, index, max_frames, flags, vol, bal, b, size);
}

void SmushPlayer::handleSkip(Chunk &b) {
	checkBlock(b, TYPE_SKIP, 4);
	int32 code = b.getDword();
	debug(6, "SmushPlayer::handleSkip(%d)", code);
	if(code >= 0 && code < 37)
		_skipNext = _skips[code];
	else
		_skipNext = true;
}

void SmushPlayer::handleStore(Chunk &b) {
	debug(6, "SmushPlayer::handleStore()");
	checkBlock(b, TYPE_STOR, 4);
	_storeFrame = true;
}

void SmushPlayer::handleFetch(Chunk &b) {
	debug(6, "SmushPlayer::handleFetch()");
	checkBlock(b, TYPE_FTCH, 6);

	if (_frameBuffer != NULL) {
		memcpy(_data, _frameBuffer, _width * _height);
	}
}

void SmushPlayer::handleImuseBuffer(int32 track_id, int32 index, int32 nbframes, int32 size, int32 unk1, int32 track_flags, Chunk &b, int32 bsize) {
	int32 track = (track_flags << 16) | track_id;

	SmushChannel *c = _smixer->findChannel(track);
	if(c == 0) {
		c = new ImuseChannel(track, _soundFrequency);
		_smixer->addChannel(c);
	}
	if(index == 0)
		c->setParameters(nbframes, size, track_flags, unk1);
	else
		c->checkParameters(index, nbframes, size, track_flags, unk1);
	c->appendData(b, bsize);
}

void SmushPlayer::handleImuseAction(Chunk &b) {
	checkBlock(b, TYPE_IACT, 8);
	debug(6, "SmushPlayer::handleImuseAction()");

	int code;
	code = b.getWord();
	int flags = b.getWord();
	int unknown = b.getShort();
	int track_flags = b.getWord();

	assert(flags == 46 && unknown == 0);
	int track_id = b.getWord();
	int index = b.getWord();
	int nbframes = b.getWord();
	int32 size = b.getDword();
	int32 bsize = b.getSize() - 18;

	if (g_scumm->_gameId != GID_CMI) {
		handleImuseBuffer(track_id, index, nbframes, size, unknown, track_flags, b, bsize);
	} else {
		byte output_data[4096];
		byte *src = (byte *)malloc(bsize);
		b.read(src, bsize);
		byte *d_src = src;
		byte value;

		do {
			if (bsize == 0)
				break;
			if (_IACTpos >= 2) {
				int32 len = READ_BE_UINT16(_IACToutput) + 2;
				len -= _IACTpos;
				if (len > bsize) {
					memcpy(_IACToutput + _IACTpos, d_src, bsize);
					_IACTpos += bsize;
					bsize = 0;
				} else {
					memcpy(_IACToutput + _IACTpos, d_src, len);
					byte *dst = output_data;
					byte *d_src2 = _IACToutput;
					d_src2 += 2;
					int32 count = 1024;
					byte variable1 = *d_src2++;
					byte variable2 = variable1 >> 4;
					variable1 &= 0x0f;
					do {
						value = *(d_src2++);
						if (value == 0x80) {
							*dst++ = *d_src2++;
							*dst++ = *d_src2++;
						} else {
							int16 val = (int8)value << variable2;
							*dst++ = val>> 8;
							*dst++ = (byte)(val);
						}
						value = *(d_src2++);
						if (value == 0x80) {
							*dst++ = *d_src2++;
							*dst++ = *d_src2++;
						} else {
							int16 val = (int8)value << variable1;
							*dst++ = val>> 8;
							*dst++ = (byte)(val);
						}
					} while (--count);

					if (_IACTchannel == -1) {
						_IACTchannel = _scumm->_mixer->playStream(NULL, -1, output_data, 0x1000, 22050,
															SoundMixer::FLAG_STEREO | SoundMixer::FLAG_16BITS, -1, 200000);
					} else {
						_scumm->_mixer->append(_IACTchannel, output_data, 0x1000, 22050,
															SoundMixer::FLAG_STEREO | SoundMixer::FLAG_16BITS);
					}

					bsize -= len;
					d_src += len;
					_IACTpos = 0;
				}
			} else {
				if (bsize == 1) {
					if (_IACTpos != 0) {
						*(_IACToutput + 1) = *d_src++;
						_IACTpos = 2;
						bsize--;
						continue;
					}
					bsize = 0;
					*(_IACToutput + 0) = *d_src;
					_IACTpos = 1;
					continue;
				} else if (_IACTpos == 0) {
					*(_IACToutput + 0) = *d_src++;
					bsize--;
				}
				*(_IACToutput + 1) = *d_src++;
				_IACTpos = 2;
				bsize--;
			}	
		} while (bsize != 0);
	
		free(src);
	}
}

void SmushPlayer::handleTextResource(Chunk &b) {
	int pos_x = b.getShort();
	int pos_y = b.getShort();
	int flags = b.getShort();
	int left = b.getShort();
	int top = b.getShort();
	int width = b.getShort();
	/*int32 height =*/ b.getShort();
	/*int32 unk2 =*/ b.getWord();

	char *str, *string = NULL, *string2 = NULL;
	if (b.getType() == TYPE_TEXT) {
		string = (char *)malloc(b.getSize() - 16);
		str = string;
		b.read(string, b.getSize() - 16);
	} else {
		int string_id = b.getWord();
		if(!_strings)
			return;
		str = _strings->get(string_id);
	}

	// if subtitles disabled and bit 3 is set, then do not draw
	if((!_subtitles) && ((flags & 8) == 8))
		return;

	SmushFont *sf = _sf[0];
	int color = 15;
	while(*str == '/') {
		str++; // For Full Throttle text resources
	}

	if (_scumm->_gameId == GID_CMI) {
		_scumm->translateText((byte*)str - 1, _scumm->_transText);
		while(*str++ != '/');
		string2 = (char *)_scumm->_transText;

		// If string2 contains formatting information there probably
		// wasn't any translation for it in the language.tab file. In
		// that case, pretend there is no string2.
		if (string2[0] == '^')
			string2[0] = 0;
	}

	while(str[0] == '^') {
		switch(str[1]) {
		case 'f':
			{
				int id = str[3] - '0';
				str += 4;
				sf = _sf[id]; 
			}
			break;
		case 'c':
			{
				color = str[4] - '0' + 10 *(str[3] - '0');
				str += 5;
			}
			break;
		default:
			error("invalid escape code in text string");
		}
	}

	assert(sf != NULL);
	sf->setColor(color);

	if (_scumm->_gameId != GID_CMI) {
		string2 = str;
	}
	if (_scumm->_gameId == GID_CMI) {
		if (string2[0] == 0) {
			string2 = str;
		}
	}

	// flags:
	// bit 0 - center				1
	// bit 1 - not used			2
	// bit 2 - ???					4
	// bit 3 - wrap around	8
	switch (flags) {
		case 0: 
			sf->drawStringAbsolute(string2, _data, _width, pos_x, pos_y);
			break;
		case 1:
			sf->drawStringCentered(string2, _data, _width, _height, MAX(pos_y, top), left, width, pos_x);
			break;
		case 4:
			sf->drawStringAbsolute(string2, _data, _width, pos_x, pos_y);
			break;
		case 5:
			sf->drawStringCentered(string2, _data, _width, _height, MAX(pos_y, top), left, width, pos_x);
			break;
		case 8:
			sf->drawStringWrap(string2, _data, _width, _height, pos_x, MAX(pos_y, top), width);
			break;
		case 9:
			sf->drawStringCentered(string2, _data, _width, _height, MAX(pos_y, top), left, width, pos_x);
			break;
		case 12:
			sf->drawStringWrap(string2, _data, _width, _height, pos_x, MAX(pos_y, top), width);
			break;
		case 13:
			sf->drawStringWrapCentered(string2, _data, _width, _height, pos_x, MAX(pos_y, top), width);
			break;
		default:
			warning("SmushPlayer::handleTextResource. Not handled flags: %d\n", flags);
	}

	if (string != NULL) {
		free (string);
	}
}

bool SmushPlayer::readString(const char *file, const char *directory) {
	const char *i = strrchr(file, '.');
	if(i == NULL) {
		error("invalid filename : %s", file);
	}
	char fname[260];
	memcpy(fname, file, i - file);
	strcpy(fname + (i - file), ".trs");
	if((_strings = getStrings(fname, directory, false)) != 0) {
		return true;
	}

	if((_strings = getStrings("digtxt.trs", directory, true)) != 0) {
		return true;
	}
	return false;
}

void SmushPlayer::readPalette(byte *out, Chunk &in) {
	in.read(out, 0x300);
}

static byte delta_color(byte org_color, int16 delta_color) {
	int16 t;
	t = (((int32)(org_color) << 7) + org_color + delta_color) >> 7;
	if (t > 255)
		t = 255;
	if (t < 0)
		t = 0;
	return (byte)t;
}

void SmushPlayer::handleDeltaPalette(Chunk &b) {
	checkBlock(b, TYPE_XPAL);
	debug(6, "SmushPlayer::handleDeltaPalette()");

	if(b.getSize() == 0x300 * 3 + 4) {

		b.getWord();
		b.getWord();

		for(int i = 0; i < 0x300; i++) {
			_deltaPal[i] = b.getWord();
		}
		readPalette(_pal, b);
		setPalette(_pal);
	} else if(b.getSize() == 6) {

		b.getWord();
		b.getWord();
		b.getWord();

		for(int i = 0; i < 0x300; i++) {
			_pal[i] = delta_color(_pal[i], _deltaPal[i]);
		}
		setPalette(_pal);
	} else {
		error("SmushPlayer::handleDeltaPalette() Wrong size for DeltaPalette");
	}
}

void SmushPlayer::handleNewPalette(Chunk &b) {
	checkBlock(b, TYPE_NPAL, 0x300);
	debug(6, "SmushPlayer::handleNewPalette()");

	readPalette(_pal, b);
	setPalette(_pal);
}

extern void smush_decode_codec1(byte *dst, byte *src, int height);

void SmushPlayer::handleFrameObject(Chunk &b) {
	checkBlock(b, TYPE_FOBJ, 14);
	if(_skipNext) {
		_skipNext = false;
		return;
	}

	int codec = b.getWord();
	b.getWord(); // left
	b.getWord(); // top
	int width = b.getWord();
	int height = b.getWord();

	if((height != _scumm->_realHeight) || (width != _scumm->_realWidth))
		return;

	if(_alreadyInit == false) {
		_codec37.init(width, height);
		_codec47.init(width, height);
		_alreadyInit = true;
	}

	_width = width;
	_height = height;
	b.getWord();
	b.getWord();


	int32 chunk_size = b.getSize() - 14;
	byte *chunk_buffer = (byte *)malloc(chunk_size);
	assert(chunk_buffer);
	b.read(chunk_buffer, chunk_size);

	switch (codec) {
	case 1:
	case 3:
		smush_decode_codec1(_data, chunk_buffer, _height);
		break;
	case 37:
		_codec37.decode(_data, chunk_buffer);
		break;
	case 47:
		_codec47.decode(_data, chunk_buffer);
		break;
	default:
		error("Invalid codec for frame object : %d", (int)codec);
	}

	if (_storeFrame == true) {
		if (_frameBuffer == NULL) {
			_frameBuffer = (byte *)malloc(_width * _height);
		}
		memcpy(_frameBuffer, _data, _width * _height);
		_storeFrame = false;
	}

	free(chunk_buffer);
}

void SmushPlayer::handleFrame(Chunk &b) {
	checkBlock(b, TYPE_FRME);
	debug(6, "SmushPlayer::handleFrame(%d)", _frame);
	_skipNext = false;

	uint32 start_time, end_time;
	start_time = _scumm->_system->get_msecs();

	while(!b.eof()) {
		Chunk *sub = b.subBlock();
		if(sub->getSize() & 1) b.seek(1);
		switch(sub->getType()) {
			case TYPE_NPAL:
				handleNewPalette(*sub);
				break;
			case TYPE_FOBJ:
				handleFrameObject(*sub);
				break;
			case TYPE_PSAD:
				handleSoundFrame(*sub);
				break;
			case TYPE_TRES:
				handleTextResource(*sub);
				break;
			case TYPE_XPAL:
				handleDeltaPalette(*sub);
				break;
			case TYPE_IACT:
				handleImuseAction(*sub);
				break;
			case TYPE_STOR:
				handleStore(*sub);
				break;
			case TYPE_FTCH:
				handleFetch(*sub);
				break;
			case TYPE_SKIP:
				handleSkip(*sub);
				break;
			case TYPE_TEXT:
				handleTextResource(*sub);
				break;
			default:
				error("Unknown frame subChunk found : %s, %d", Chunk::ChunkString(sub->getType()), sub->getSize());
		}
		delete sub;
	}

	end_time = _scumm->_system->get_msecs();

	updateScreen();
	_smixer->handleFrame();

	debug(5, "Smush stats: FRME( %03d ), Limit(%d)", end_time - start_time, _speed / 1000);

	_frame++;
}

void SmushPlayer::handleAnimHeader(Chunk &b) {
	checkBlock(b, TYPE_AHDR, 0x300 + 6);
	debug(6, "SmushPlayer::handleAnimHeader()");

	_version = b.getWord();
	_nbframes = b.getWord();
	b.getWord();
	readPalette(_pal, b);
	setPalette(_pal);
}

void SmushPlayer::setupAnim(const char *file, const char *directory) {
	_base = new FileChunk(file, directory);
	Chunk *sub = _base->subBlock();
	checkBlock(*sub, TYPE_AHDR);
	handleAnimHeader(*sub);

	readString(file, directory);

	if (_scumm->_gameId == GID_FT) {
		_sf[0] = new SmushFont(true, false);
		_sf[2] = new SmushFont(true, false);
		_sf[0]->loadFont("scummfnt.nut", directory);
		_sf[2]->loadFont("titlfnt.nut", directory);
	} else if (_scumm->_gameId == GID_DIG) {
		for(int i = 0; i < 4; i++) {
			char file_font[11];
			sprintf((char *)&file_font, "font%d.nut", i);
			_sf[i] = new SmushFont(i != 0, false);
			_sf[i]->loadFont(file_font, directory);
		}
	} else if (_scumm->_gameId == GID_CMI) {
		for(int i = 0; i < 5; i++) {
			char file_font[11];
			sprintf((char *)&file_font, "font%d.nut", i);
			_sf[i] = new SmushFont(false, true);
			_sf[i]->loadFont(file_font, directory);
		}
	} else {
		error("SmushPlayer::init() Unknown font setup for game");
	}	

	delete sub;
}

void SmushPlayer::parseNextFrame() {
	Chunk *sub = _base->subBlock();
	if (_base->eof()) {
		_scumm->_videoFinished = true;
		return;
	}

	switch(sub->getType()) {
		case TYPE_FRME:
			handleFrame(*sub);
			break;
		default:
			error("Unknown Chunk found : %d, %d", sub->getType(), sub->getSize());
	}
	delete sub;
}

void SmushPlayer::setPalette(byte *palette) {
	byte palette_colors[1024];
	byte *p = palette_colors;
	byte *data = palette;

	for (int i = 0; i != 256; i++, data += 3, p += 4) {
		p[0] = data[0]; // red
		p[1] = data[1]; // green
		p[2] = data[2]; // blue
		p[3] = 0;
	}

	_scumm->_system->set_palette(palette_colors, 0, 256);
}

void SmushPlayer::updateScreen() {
	_scumm->_system->lock_mutex(_mutex);

	uint32 end_time, start_time = _scumm->_system->get_msecs();
	_scumm->_system->copy_rect(_data, _width, 0, 0, _width, _height);
	_updateNeeded = true;
	end_time = _scumm->_system->get_msecs();
	debug(4, "Smush stats: updateScreen( %03d )", end_time - start_time);

	_scumm->_system->unlock_mutex(_mutex);
}

void SmushPlayer::play(const char *filename, const char *directory) {
	File f;
	f.open(filename, directory);
	if(f.isOpen() == false) {
		warning("SmushPlayer::setupAnim() File not found %s", filename);
		return;
	}

	_updateNeeded = false;

	setupAnim(filename, directory);
	init();

	while (true) {
		_scumm->_system->lock_mutex(_mutex);
		_scumm->parseEvents();
		_scumm->processKbd();
		if(_updateNeeded == true) {
			
			uint32 end_time, start_time = _scumm->_system->get_msecs();
			_scumm->_system->update_screen();
			_updateNeeded = false;
			end_time = _scumm->_system->get_msecs();
			debug(4, "Smush stats: BackendUpdateScreen( %03d )", end_time - start_time);

		}
		_scumm->_system->unlock_mutex(_mutex);
		if (_scumm->_videoFinished == true)
			break;
		if (_scumm->_saveLoadFlag)
			break;
		_scumm->_system->delay_msecs(10);
	};

	deinit();
}
