/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_H
#define PRINCE_H

#include "common/random.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/events.h"

#include "graphics/decoders/bmp.h"

#include "gui/debugger.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "audio/mixer.h"

#include "video/flic_decoder.h"

#include "prince/mob.h"
#include "prince/object.h"


namespace Prince {
#if 0
	bool loadFromStream(Common::SeekableReadStream &stream) {
		ResourceType *resource = new ResourceType();
		while (resource->loadFromStream(stream))
			_list.push_back(resource);
		return true;
	}
#endif
struct PrinceGameDescription;

class PrinceEngine;
class GraphicsMan;
class Script;
class Debugger;
class MusicPlayer;
class VariaTxt;
class Cursor;
class MhwanhDecoder;
class Font;

struct Text {
	const char *_str;
	uint16 _x, _y;
	uint16 _time;
	uint32 _color;

	Text() : _str(NULL), _x(0), _y(0), _time(0), _color(255){
	}
};

struct DebugChannel {

enum Type {
	kScript,
	kEngine 
};

};

class PrinceEngine : public Engine {
protected:
	Common::Error run();

public:
	PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc);
	virtual ~PrinceEngine();

	virtual bool hasFeature(EngineFeature f) const;

	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	const PrinceGameDescription *_gameDescription;
	Video::FlicDecoder _flicPlayer;
	VariaTxt *_variaTxt;

	uint32 _talkTxtSize;
	byte *_talkTxt;

	bool loadLocation(uint16 locationNr);
	bool loadAnim(uint16 animNr, bool loop);
	bool loadVoice(uint32 slot, const Common::String &name);

	void playSample(uint16 sampleId, uint16 loopType);
	void stopSample(uint16 sampleId);

	virtual GUI::Debugger *getDebugger();

	void changeCursor(uint16 curId);
	void printAt(uint32 slot, uint8 color, const char *s, uint16 x, uint16 y);

	static const uint8 MAXTEXTS = 32;
	Text _textSlots[MAXTEXTS];

	uint64 _frameNr;

private:
	bool playNextFrame();
	void keyHandler(Common::Event event);
	void hotspot();
	void scrollCameraRight(int16 delta);
	void scrollCameraLeft(int16 delta);
	void drawScreen();
	void showTexts();
	void init();
	void showLogo();

	uint32 getTextWidth(const char *s);
	void debugEngine(const char *s, ...);

	uint16 _locationNr;

	Common::RandomSource *_rnd;
	Graphics::BitmapDecoder *_roomBmp;
	Cursor *_cursor1;
	Cursor *_cursor2;
	MhwanhDecoder *_walizkaBmp;
	Debugger *_debugger;
	GraphicsMan *_graph;
	Script *_script;
	Font *_font;
	MusicPlayer *_midiPlayer;

	Audio::SoundHandle _soundHandle;
	Common::SeekableReadStream *_voiceStream;
	Common::Array<Mob> _mobList;
	Common::Array<Object *> _objectList;

	uint16 _cameraX;
	uint16 _newCameraX;
	uint16 _sceneWidth;

	bool _flicLooped;
	
	void mainLoop();

};

} // End of namespace Prince

#endif

/* vim: set tabstop=4 noexpandtab: */
