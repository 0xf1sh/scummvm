/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "common/scummsys.h"

#ifdef USE_MT32EMU

#include "sound/softsynth/mt32/mt32emu.h"

#include "sound/softsynth/emumidi.h"
#include "sound/mpu401.h"

#include "common/util.h"
#include "common/file.h"
#include "common/config-manager.h"

#include "graphics/font.h"
#include "graphics/surface.h"

class MidiChannel_MT32 : public MidiChannel_MPU401 {
	void effectLevel(byte value) { }
	void chorusLevel(byte value) { }
};

class MidiDriver_MT32 : public MidiDriver_Emulated {
private:
	PlayingSoundHandle _handle;
	MidiChannel_MT32 _midiChannels[16];
	uint16 _channelMask;
	MT32Emu::Synth *_synth;

	int _outputRate;

protected:
	void generateSamples(int16 *buf, int len);

public:
	bool _initialising;

	MidiDriver_MT32(SoundMixer *mixer);
	virtual ~MidiDriver_MT32();

	int open();
	void close();
	void send(uint32 b);
	void setPitchBendRange (byte channel, uint range);
	void sysEx(byte *msg, uint16 length);

	uint32 property(int prop, uint32 param);
	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel();

	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _outputRate; }
};

typedef File SFile;

class MT32File: public MT32Emu::File {
	SFile file;
public:
	bool open(const char *filename, OpenMode mode) {
		SFile::AccessMode accessMode = mode == OpenMode_read ? SFile::kFileReadMode : SFile::kFileWriteMode;
		return file.open(filename, accessMode);
	}
	void close() {
		return file.close();
	}
	size_t read(void *in, size_t size) {
		return file.read(in, size);
	}
	bool readLine(char *in, size_t size) {
		return file.gets(in, size) != NULL;
	}
	bool readBit8u(MT32Emu::Bit8u *in) {
		byte b = file.readByte();
		if (file.eof())
			return false;
		*in = b;
		return true;
	}
	size_t write(const void *in, size_t size) {
		return file.write(in, size);
	}
	bool writeBit8u(MT32Emu::Bit8u out) {
		file.writeByte(out);
		return !file.ioFailed();
	}
	bool isEOF() {
		return file.eof();
	}
};

static int eatSystemEvents() {
	OSystem::Event event;
	while (g_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_QUIT:
			return 1;
		default:
			break;
		}
	}
	return 0;
}

static void drawProgress(float progress) {
	Graphics::Surface surf;
	uint32 borderColor = 0x2;
	uint32 fillColor = 0x4;
	surf.w = g_system->getWidth() / 7 * 5;
	surf.h = Graphics::g_scummfont.getFontHeight();
	int x = g_system->getWidth() / 7;
	int y = g_system->getHeight() / 2 - surf.h / 2;
	surf.pitch = surf.w;
	surf.bytesPerPixel = 1;
	surf.pixels = calloc(surf.w, surf.h);
	Common::Rect r(surf.w, surf.h);
	surf.frameRect(r, borderColor);
	r.grow(-1);
	r.right = r.left + (uint16)(r.width() * progress);
	surf.fillRect(r, fillColor);
	g_system->copyRectToScreen((byte *)surf.pixels, surf.pitch, x, y, surf.w, surf.h);
	g_system->updateScreen();
	free(surf.pixels);
}

static void drawMessage(int offset, const Common::String &text) {
	const Graphics::Font &font(Graphics::g_scummfont);
	Graphics::Surface surf;
	uint32 color = 0x2;
	surf.w = g_system->getWidth();
	surf.h = font.getFontHeight();
	surf.pitch = surf.w;
	surf.bytesPerPixel = 1;
	surf.pixels = calloc(surf.w, surf.h);
	font.drawString(&surf, text, 0, 0, surf.w, color, Graphics::kTextAlignCenter);
	int y = g_system->getHeight() / 2 - font.getFontHeight() / 2 + offset * (font.getFontHeight() + 1);
	g_system->copyRectToScreen((byte *)surf.pixels, surf.pitch, 0, y, surf.w, surf.h);
	g_system->updateScreen();
	free(surf.pixels);
}

static MT32Emu::File *MT32_OpenFile(void *userData, const char *filename, MT32Emu::File::OpenMode mode) {
	MT32File *file = new MT32File();
	if (!file->open(filename, mode)) {
		delete file;
		return NULL;
	}
	return file;
}

static void MT32_PrintDebug(void *userData, const char *fmt, va_list list) {
	char buf[512];
	if (((MidiDriver_MT32 *)userData)->_initialising) {
		vsprintf(buf, fmt, list);
		buf[70] = 0; // Truncate to a reasonable length
		drawMessage(1, buf);
	}
	//vdebug(0, fmt, list); // FIXME: Use a higher debug level
}

static int MT32_Report(void *userData, MT32Emu::ReportType type, const void *reportData) {
	switch(type) {
	case MT32Emu::ReportType_lcdMessage:
		g_system->displayMessageOnOSD((const char *)reportData);
		break;
	case MT32Emu::ReportType_errorControlROM:
		error("Failed to load MT32_CONTROL.ROM");
		break;
	case MT32Emu::ReportType_errorPCMROM:
		error("Failed to load MT32_PCM.ROM");
		break;
	case MT32Emu::ReportType_progressInit:
		if (((MidiDriver_MT32 *)userData)->_initialising) {
			drawProgress(*((const float *)reportData));
			return eatSystemEvents();
		}
		break;
	default:
		break;
	}
	return 0;
}

////////////////////////////////////////
//
// MidiDriver_MT32
//
////////////////////////////////////////

MidiDriver_MT32::MidiDriver_MT32(SoundMixer *mixer) : MidiDriver_Emulated(mixer) {
	_channelMask = 0xFFFF; // Permit all 16 channels by default
	uint i;
	for (i = 0; i < ARRAYSIZE(_midiChannels); ++i) {
		_midiChannels[i].init(this, i);
	}
	_synth = NULL;
	// A higher baseFreq reduces the length used in generateSamples(),
	// and means that the timer callback will be called more often.
	// That results in more accurate timing.
	_baseFreq = 10000;
	// Unfortunately bugs in the emulator cause inaccurate tuning
	// at rates other than 32KHz, thus we produce data at 32KHz and
	// rely on SoundMixer to convert.
	_outputRate = 32000; //_mixer->getOutputRate();
	_initialising = false;
}

MidiDriver_MT32::~MidiDriver_MT32() {
	if (_synth != NULL)
		delete _synth;
}

int MidiDriver_MT32::open() {
	MT32Emu::SynthProperties prop;

	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();
	
	memset(&prop, 0, sizeof(prop));
	prop.sampleRate = getRate();
	prop.useReverb = true;
	prop.useDefaultReverb = false;
	prop.reverbType = 0;
	prop.reverbTime = 5;
	prop.reverbLevel = 3;
	prop.userData = this;
	prop.printDebug = MT32_PrintDebug;
	prop.report = MT32_Report;
	prop.openFile = MT32_OpenFile;
	_synth = new MT32Emu::Synth();
	_initialising = true;
	const byte dummy_palette[] = {
		0, 0, 0, 0, 
		0, 0, 171, 0, 
		0, 171, 0, 0, 
		0, 171, 171, 0, 
		171, 0, 0, 0
	};

	g_system->setPalette(dummy_palette, 0, 5);
	drawMessage(-1, "Initialising MT-32 Emulator");
	if (!_synth->open(prop))
		return MERR_DEVICE_NOT_AVAILABLE;
	_initialising = false;
	g_system->clearScreen();
	g_system->updateScreen();
	_mixer->playInputStream(&_handle, this, false, -1, 255, 0, false, true);
	return 0;
}

void MidiDriver_MT32::send(uint32 b) {
	_synth->playMsg(b);
}

void MidiDriver_MT32::setPitchBendRange(byte channel, uint range) {
	if (range > 24) {
		printf("setPitchBendRange() called with range > 24: %d", range);
	}
	byte benderRangeSysex[9];
	benderRangeSysex[0] = 0x41; // Roland
	benderRangeSysex[1] = channel;
	benderRangeSysex[2] = 0x16; // MT-32
	benderRangeSysex[3] = 0x12; // Write
	benderRangeSysex[4] = 0x00;
	benderRangeSysex[5] = 0x00;
	benderRangeSysex[6] = 0x04;
	benderRangeSysex[7] = (byte)range;
	benderRangeSysex[8] = MT32Emu::Synth::calcSysexChecksum(&benderRangeSysex[4], 4, 0);
	sysEx(benderRangeSysex, 9);
}

void MidiDriver_MT32::sysEx(byte *msg, uint16 length) {
	if (msg[0] == 0xf0) {
		_synth->playSysex(msg, length);
	} else {
		_synth->playSysexWithoutFraming(msg, length);
	}
}

void MidiDriver_MT32::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	// Detach the player callback handler
	setTimerCallback(NULL, NULL);
	// Detach the mixer callback handler
	_mixer->stopHandle(_handle);

	_synth->close();
	delete _synth;
	_synth = NULL;
}

void MidiDriver_MT32::generateSamples(int16 *data, int len) {
	_synth->render(data, len);
}

uint32 MidiDriver_MT32::property(int prop, uint32 param) {
	switch (prop) {
	case PROP_CHANNEL_MASK:
		_channelMask = param & 0xFFFF;
		return 1;
	}

	return 0;
}

MidiChannel *MidiDriver_MT32::allocateChannel() {
	MidiChannel_MT32 *chan;
	uint i;

	for (i = 0; i < ARRAYSIZE(_midiChannels); ++i) {
		if (i == 9 || !(_channelMask & (1 << i)))
			continue;
		chan = &_midiChannels[i];
		if (chan->allocate()) {
			return chan;
		}
	}
	return NULL;
}

MidiChannel *MidiDriver_MT32::getPercussionChannel() {
	return &_midiChannels[9];
}

// This code should be used when calling the timer callback from the mixer thread is undesirable.
// Note that it results in less accurate timing.
#if 0
class MidiEvent_MT32 {
public:
	MidiEvent_MT32 *_next;
	uint32 _msg; // 0xFFFFFFFF indicates a sysex message
	byte *_data;
	uint32 _len;

	MidiEvent_MT32(uint32 msg, byte *data, uint32 len) {
		_msg = msg;
		if (len > 0) {
			_data = new byte[len];
			memcpy(_data, data, len);
		}
		_len = len;
		_next = NULL;
	}

	MidiEvent_MT32() {
		if (_len > 0)
			delete _data;
	}
};

class MidiDriver_ThreadedMT32 : public MidiDriver_MT32 {
private:
	OSystem::MutexRef _eventMutex;
	MidiEvent_MT32 *_events;
	Timer::TimerProc _timer_proc;

	void pushMidiEvent(MidiEvent_MT32 *event);
	MidiEvent_MT32 *popMidiEvent();

protected:
	void send(uint32 b);
	void sysEx(byte *msg, uint16 length);

public:
	MidiDriver_ThreadedMT32(SoundMixer *mixer);
	virtual ~MidiDriver_ThreadedMT32();

	void onTimer();
	void close();
	void setTimerCallback(void *timer_param, Timer::TimerProc timer_proc);
};


MidiDriver_ThreadedMT32::MidiDriver_ThreadedMT32(SoundMixer *mixer) : MidiDriver_MT32(mixer) {
	_eventMutex = g_system->createMutex();
	_events = NULL;
	_timer_proc = NULL;
}

MidiDriver_ThreadedMT32::~MidiDriver_ThreadedMT32() {
	g_system->deleteMutex(_eventMutex);
}

void MidiDriver_ThreadedMT32::close() {
	MidiDriver_MT32::close();
	while ((popMidiEvent() != NULL)) {
		// Just eat any leftover events
	}
}

void MidiDriver_ThreadedMT32::setTimerCallback(void *timer_param, Timer::TimerProc timer_proc) {
	if (!_timer_proc || !timer_proc) {
		if (_timer_proc)
			g_timer->removeTimerProc(_timer_proc);
		_timer_proc = timer_proc;
		if (timer_proc)
			g_timer->installTimerProc(timer_proc, getBaseTempo(), timer_param);
	}
}

void MidiDriver_ThreadedMT32::pushMidiEvent(MidiEvent_MT32 *event) {
	g_system->lockMutex(_eventMutex);
	if (_events == NULL) {
		_events = event;
	} else {
		MidiEvent_MT32 *last = _events;
		while (last->_next != NULL)
			last = last->_next;
		last->_next = event;
	}
	g_system->unlockMutex(_eventMutex);
}

MidiEvent_MT32 *MidiDriver_ThreadedMT32::popMidiEvent() {
	MidiEvent_MT32 *event;
	g_system->lockMutex(_eventMutex);
	event = _events;
	if (event != NULL)
		_events = event->_next;
	g_system->unlockMutex(_eventMutex);
	return event;
}

void MidiDriver_ThreadedMT32::send(uint32 b) {
	MidiEvent_MT32 *event = new MidiEvent_MT32(b, NULL, 0);
	pushMidiEvent(event);
}

void MidiDriver_ThreadedMT32::sysEx(byte *msg, uint16 length) {
	MidiEvent_MT32 *event = new MidiEvent_MT32(0xFFFFFFFF, msg, length);
	pushMidiEvent(event);
}

void MidiDriver_ThreadedMT32::onTimer() {
	MidiEvent_MT32 *event;
	while ((event = popMidiEvent()) != NULL) {
		if (event->_msg == 0xFFFFFFFF) {
			MidiDriver_MT32::sysEx(event->_data, event->_len);
		} else {
			MidiDriver_MT32::send(event->_msg);
		}
		delete event;
	}
}
#endif

////////////////////////////////////////
//
// MidiDriver_MT32 factory
//
////////////////////////////////////////

MidiDriver *MidiDriver_MT32_create(SoundMixer *mixer) {
	// HACK: It will stay here until engine plugin loader overhaul
	if (ConfMan.hasKey("extrapath"))                                        
		File::addDefaultDirectory(ConfMan.get("extrapath"));
	return new MidiDriver_MT32(mixer);
}

#endif
