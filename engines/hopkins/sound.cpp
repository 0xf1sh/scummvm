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

#include "audio/decoders/adpcm_intern.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/sound.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"

namespace Audio {

class APC_ADPCMStream : public Audio::DVI_ADPCMStream {
public:
	APC_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, int rate, int channels) : DVI_ADPCMStream(stream, disposeAfterUse, stream->size(), rate, channels, 0) {
		stream->seek(-12, SEEK_CUR);
		_status.ima_ch[0].last = _startValue[0] = stream->readUint32LE();
		_status.ima_ch[1].last = _startValue[1] = stream->readUint32LE();
		stream->seek(4, SEEK_CUR);
	}

	void reset() {
		DVI_ADPCMStream::reset();
		_status.ima_ch[0].last = _startValue[0];
		_status.ima_ch[1].last = _startValue[1];
	}

private:
	int16 _startValue[2];
};

Audio::RewindableAudioStream *makeAPCStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if (stream->readUint32BE() != MKTAG('C', 'R', 'Y', 'O'))
		return 0;
	if (stream->readUint32BE() != MKTAG('_', 'A', 'P', 'C'))
		return 0;
	stream->readUint32BE(); // version
	stream->readUint32LE(); // out size
	uint32 rate = stream->readUint32LE();
	stream->skip(8); // initial values, will be handled by the class
	bool stereo = stream->readUint32LE() != 0;

	return new APC_ADPCMStream(stream, disposeAfterUse, rate, stereo ? 2 : 1);
}

}

/*------------------------------------------------------------------------*/

namespace Hopkins {

SoundManager::SoundManager() {
	SPECIAL_SOUND = 0;
	SOUNDVOL = 0;
	VOICEVOL = 0;
	MUSICVOL = 0;
	OLD_SOUNDVOL = 0;
	OLD_MUSICVOL = 0;
	OLD_VOICEVOL = 0;
	SOUNDOFF = true;
	MUSICOFF = true;
	VOICEOFF = true;
	TEXTOFF = false;
	SOUND_FLAG = false;
	VBL_MERDE = false;
	SOUND_NUM = 0;
	old_music = 0;
	MOD_FLAG = false;

	for (int i = 0; i < VOICE_COUNT; ++i)
		Common::fill((byte *)&Voice[i], (byte *)&Voice[i] + sizeof(VoiceItem), 0);
	for (int i = 0; i < SWAV_COUNT; ++i)
		Common::fill((byte *)&Swav[i], (byte *)&Swav[i] + sizeof(SwavItem), 0);
	for (int i = 0; i < MWAV_COUNT; ++i)
		Common::fill((byte *)&Mwav[i], (byte *)&Mwav[i] + sizeof(MwavItem), 0);
	for (int i = 0; i < SOUND_COUNT; ++i)
		Common::fill((byte *)&SOUND[i], (byte *)&SOUND[i] + sizeof(SoundItem), 0);
	Common::fill((byte *)&Music, (byte *)&Music + sizeof(MusicItem), 0);
}

SoundManager::~SoundManager() {
	STOP_MUSIC();
	DEL_MUSIC();
	MOD_FLAG = false;
}

void SoundManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
	SPECIAL_SOUND = 0;
}

void SoundManager::WSOUND_INIT() {
	warning("TODO: WSOUND_INIT");
}

void SoundManager::VERIF_SOUND() {
	if (!SOUNDOFF && SOUND_FLAG) {
		if (!VOICE_STAT(1)) {
			STOP_VOICE(1);
			DEL_NWAV(SOUND_NUM);
		}
	}
}

void SoundManager::LOAD_ANM_SOUND() {
	switch (SPECIAL_SOUND) {
	case 2:
		CHARGE_SAMPLE(5, "mitra1.wav");
		CHARGE_SAMPLE(1, "tir2.wav");
		CHARGE_SAMPLE(2, "sound6.wav");
		CHARGE_SAMPLE(3, "sound5.WAV");
		CHARGE_SAMPLE(4, "sound4.WAV");
		break;
	case 5:
		LOAD_WAV("CRIE.WAV", 1);
		break;
	case 14:
		LOAD_WAV("SOUND14.WAV", 1);
		break;
	case 16:
		LOAD_WAV("SOUND16.WAV", 1);
		break;
	case 198:
		LOAD_WAV("SOUND3.WAV", 1);
		break;
	case 199:
		LOAD_WAV("SOUND22.WAV", 1);
		break;
	case 200:
		VOICE_MIX(682, 1);
		break;
	case 208:
		LOAD_WAV("SOUND77.WAV", 1);
		break;
	case 210:
		LOAD_WAV("SOUND78.WAV", 1);
		break;
	case 211:
		LOAD_WAV("SOUND78.WAV", 1);
		break;
	case 229:
		LOAD_WAV("SOUND80.WAV", 1);
		LOAD_WAV("SOUND82.WAV", 2);
		break;
	}
}

void SoundManager::playAnim_SOUND(int soundNumber) {
	if (!_vm->_globals.CENSURE && SPECIAL_SOUND == 2) {
		switch (soundNumber) {
		case 20:
			PLAY_SAMPLE2(5);
			break;
		case 57:
		case 63:
		case 69:
			PLAY_SAMPLE2(1);
			break;
		case 75:
			PLAY_SAMPLE2(2);
			break;
		case 109:
			PLAY_SAMPLE2(3);
			break;
		case 122:
			PLAY_SAMPLE2(4);
			break;
		}
	} else if (SPECIAL_SOUND == 1 && soundNumber == 17)
		PLAY_SOUND("SOUND42.WAV");
	else if (SPECIAL_SOUND == 5 && soundNumber == 19)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 14 && soundNumber == 625)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 16 && soundNumber == 25)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 17) {
		if (soundNumber == 6)
			PLAY_SAMPLE2(1);
		else if (soundNumber == 14)
			PLAY_SAMPLE2(2);
		else if (soundNumber == 67)
			PLAY_SAMPLE2(3);
	} else if (SPECIAL_SOUND == 198 && soundNumber == 15)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 199 && soundNumber == 72)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 208 && soundNumber == 40)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 210 && soundNumber == 2)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 211 && soundNumber == 22)
		PLAY_WAV(1);
	else if (SPECIAL_SOUND == 229) {
		if (soundNumber == 15)
			PLAY_WAV(1);
		else if (soundNumber == 91)
			PLAY_WAV(2);
	}
}

void SoundManager::WSOUND(int soundNumber) {
	if (old_music != soundNumber || !MOD_FLAG) {
		if (MOD_FLAG == 1)
			WSOUND_OFF();

		switch (soundNumber) {
		case 1:
			PLAY_MOD("appar");
			break;
		case 2:
			PLAY_MOD("ville");
			break;
		case 3:
			PLAY_MOD("Rock");
			break;
		case 4:
			PLAY_MOD("polic");
			break;
		case 5:
			PLAY_MOD("deep");
			break;
		case 6:
			PLAY_MOD("purga");
			break;
		case 7:
			PLAY_MOD("rivie");
			break;
		case 8:
			PLAY_MOD("SUSPE");
			break;
		case 9:
			PLAY_MOD("labo");
			break;
		case 10:
			PLAY_MOD("cadav");
			break;
		case 11:
			PLAY_MOD("caban");
			break;
		case 12:
			PLAY_MOD("purg2");
			break;
		case 13:
			PLAY_MOD("foret");
			break;
		case 14:
			PLAY_MOD("ile");
			break;
		case 15:
			PLAY_MOD("ile2");
			break;
		case 16:
			PLAY_MOD("hopki");
			break;
		case 17:
			PLAY_MOD("peur");
			break;
		case 18:
			PLAY_MOD("peur");
			break;
		case 19:
			PLAY_MOD("BASE");
			break;
		case 20:
			PLAY_MOD("cada2");
			break;
		case 21:
			PLAY_MOD("usine");
			break;
		case 22:
			PLAY_MOD("chien");
			break;
		case 23:
			PLAY_MOD("coeur");
			break;
		case 24:
			PLAY_MOD("stand");
			break;
		case 25:
			PLAY_MOD("ocean");
			break;
		case 26:
			PLAY_MOD("base3");
			break;
		case 27:
			PLAY_MOD("gloop");
			break;
		case 28:
			PLAY_MOD("cant");
			break;
		case 29:
			PLAY_MOD("feel");
			break;
		case 30:
			PLAY_MOD("lost");
			break;
		case 31:
			PLAY_MOD("tobac");
			break;
		}

		old_music = soundNumber;
	}
}

void SoundManager::WSOUND_OFF() {
	STOP_VOICE(0);
	STOP_VOICE(1);
	STOP_VOICE(2);
	if (_vm->_soundManager.SOUND_FLAG)
		DEL_NWAV(SOUND_NUM);

	for (int i = 1; i <= 48; ++i)
		DEL_SAMPLE_SDL(i);
		
	if (MOD_FLAG) {
		STOP_MUSIC();
		DEL_MUSIC();
		MOD_FLAG = false;
	}
}

void SoundManager::PLAY_MOD(const Common::String &file) {
	if (!MUSICOFF) {
		_vm->_fileManager.constructFilename(_vm->_globals.HOPMUSIC, file);
		if (MOD_FLAG) {
			STOP_MUSIC();
			DEL_MUSIC();
			MOD_FLAG = false;
		}

		LOAD_MUSIC(_vm->_globals.NFICHIER);
		PLAY_MUSIC();
		MOD_FLAG = true;
	}
}

void SoundManager::LOAD_MUSIC(const Common::String &file) {
	if (Music._active)
		DEL_MUSIC();

	Common::File f;
	Common::String filename = Common::String::format("%s.TWA", file.c_str());

	if (!f.open(filename))
		error("Error opening file %s", filename.c_str());

	char s[8];
	int destIndex = 0;
	int mwavIndex;

	bool breakFlag = false;
	do {
		f.read(&s[0], 3);

		if (s[0] == 'x') {
			// End of list reached
			Music._mwavIndexes[destIndex] = -1;
			breakFlag = true;
		} else {
			// Convert two digits to a number
			s[2] = '\0';
			mwavIndex = atol(&s[0]);

			filename = Common::String::format("%s_%s.%s", file.c_str(), &s[0],
				(_vm->getPlatform() == Common::kPlatformWindows) ? "APC" : "WAV");
			LOAD_MSAMPLE(mwavIndex, filename);

			assert(destIndex < MUSIC_WAVE_COUNT);
			Music._mwavIndexes[destIndex++] = mwavIndex;
		}
	} while (!breakFlag);

	f.close();

	Music._active = true;
	Music._isPlaying = false;
	Music._currentIndex = -1;
}

void SoundManager::PLAY_MUSIC() {
	if (Music._active)
		Music._isPlaying = true;
}

void SoundManager::STOP_MUSIC() {
	if (Music._active)
		Music._isPlaying = false;
}

void SoundManager::DEL_MUSIC() {
	if (Music._active) {
		for (int i = 0; i < 50; ++i) {
			DEL_MSAMPLE(i);
		}
	}

	Music._active = false;
	Music._isPlaying = false;
	Music._string = "     ";
	Music._currentIndex = -1;
}

void SoundManager::checkSounds() {
	checkMusic();
	checkVoices();
}

void SoundManager::checkMusic() {
	if (Music._active && Music._isPlaying) {
		int mwavIndex = Music._mwavIndexes[Music._currentIndex];
		if (mwavIndex == -1)
			return;

		if (Music._currentIndex >= 0 && Music._currentIndex < MWAV_COUNT) {
			if (mwavIndex != -1 && !Mwav[mwavIndex]._audioStream->endOfStream())
				// Currently playing wav has not finished, so exit
				return;

			_vm->_mixer->stopHandle(Mwav[mwavIndex]._soundHandle);
		}

		// Time to move to the next index
		if (++Music._currentIndex >= MWAV_COUNT)
			return;

		mwavIndex = Music._mwavIndexes[Music._currentIndex];
		if (mwavIndex == -1) {
			Music._currentIndex = 0;
			mwavIndex = Music._mwavIndexes[Music._currentIndex];
		}			

		int volume = MUSICVOL * 255 / 16;

		Mwav[mwavIndex]._audioStream->rewind();
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &Mwav[mwavIndex]._soundHandle, 
			Mwav[mwavIndex]._audioStream, -1, volume, 0, DisposeAfterUse::NO);
	}
}

void SoundManager::checkVoices() {
	// Check the status of each voice.
	bool hasActiveVoice = false;
	for (int i = 0; i < VOICE_COUNT; ++i) {
		VOICE_STAT(i);
		hasActiveVoice |= Voice[i]._status != 0;
	}

	if (!hasActiveVoice && SOUND_FLAG) {
		SOUND_FLAG = false;
		SOUND_NUM = 0;
	}
}

void SoundManager::LOAD_MSAMPLE(int mwavIndex, const Common::String &file) {
	if (!Mwav[mwavIndex]._active) {
		Common::File f;
		if (!f.open(file)) {
			// Fallback from WAV to APC...
			if (!f.open(setExtension(file, ".APC")))
				error("Could not open %s for reading", file.c_str());
		}

		Mwav[mwavIndex]._audioStream = makeSoundStream(f.readStream(f.size()));
		Mwav[mwavIndex]._active = true;

		f.close();
	}
}

void SoundManager::DEL_MSAMPLE(int mwavIndex) {
	if (Mwav[mwavIndex]._active) {
		Mwav[mwavIndex]._active = false;
		_vm->_mixer->stopHandle(Mwav[mwavIndex]._soundHandle);

		delete Mwav[mwavIndex]._audioStream;
		Mwav[mwavIndex]._audioStream = NULL;
	}
}

bool SoundManager::VOICE_MIX(int voiceId, int voiceMode) {
	int fileNumber; 
	int oldMusicVol; 
	bool breakFlag; 
	Common::String prefix;
	Common::String filename;
	Common::File f;
	size_t catPos, catLen;

	fileNumber = voiceId;
	if (VOICEOFF == 1)
		return false;

	if ((unsigned int)(voiceMode - 1) <= 1
	        && (voiceId == 4
	            || voiceId == 16
	            || voiceId == 121
	            || voiceId == 142
	            || voiceId == 182
	            || voiceId == 191
	            || voiceId == 212
	            || voiceId == 225
	            || voiceId == 239
	            || voiceId == 245
	            || voiceId == 297
	            || voiceId == 308
	            || voiceId == 333
	            || voiceId == 348
	            || voiceId == 352
	            || voiceId == 358
	            || voiceId == 364
	            || voiceId == 371
	            || voiceId == 394
	            || voiceId == 414
	            || voiceId == 429
	            || voiceId == 442
	            || voiceId == 446
	            || voiceId == 461
	            || voiceId == 468
	            || voiceId == 476
	            || voiceId == 484
	            || voiceId == 491
	            || voiceId == 497
	            || voiceId == 501
	            || voiceId == 511
	            || voiceId == 520
	            || voiceId == 536
	            || voiceId == 554
	            || voiceId == 566
	            || voiceId == 573
	            || voiceId == 632
	            || voiceId == 645))
		fileNumber = 684;
	
	if ((unsigned int)(voiceMode - 1) <= 1) {
		prefix = "DF";
	}
	if (voiceMode == 3) {
		prefix = "IF";
	}
	if (voiceMode == 4) {
		prefix = "TF";
	}
	if (voiceMode == 5) {
		prefix = "OF";
	}

	filename = Common::String::format("%s%d", prefix.c_str(), fileNumber);
	
	if (!_vm->_fileManager.searchCat(filename + ".WAV", 9)) {
		if (_vm->_globals.FR == 1)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "RES_VFR.RES");
		else if (!_vm->_globals.FR)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "RES_VAN.RES");
		else if (_vm->_globals.FR == 2)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "RES_VES.RES");

		catPos = _vm->_globals.CAT_POSI;
		catLen = _vm->_globals.CAT_TAILLE;
	} else if (!_vm->_fileManager.searchCat(filename + ".APC", 9)) {
		if (_vm->_globals.FR == 1)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "RES_VFR.RES");
		else if (!_vm->_globals.FR)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "RES_VAN.RES");
		else if (_vm->_globals.FR == 2)
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, "RES_VES.RES");

		catPos = _vm->_globals.CAT_POSI;
		catLen = _vm->_globals.CAT_TAILLE;
	} else {		
		_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, filename + ".WAV");
		if (!f.exists(_vm->_globals.NFICHIER)) {
			_vm->_fileManager.constructFilename(_vm->_globals.HOPVOICE, filename + ".APC");
			if (!f.exists(_vm->_globals.NFICHIER))
				return false;
		}

		catPos = 0;
		catLen = 0;
	}

	SDL_LVOICE(catPos, catLen);
	oldMusicVol = MUSICVOL;
	if (!MUSICOFF && MUSICVOL > 2)
		MUSICVOL = (signed int)((long double)MUSICVOL - (long double)MUSICVOL / 100.0 * 45.0);

	PLAY_VOICE_SDL();

	// Loop for playing voice
	breakFlag = 0;
	do {
		if (SPECIAL_SOUND != 4 && !VBL_MERDE)
			_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.BMOUSE())
			break;
		_vm->_eventsManager.CONTROLE_MES();
		if (_vm->_eventsManager.ESC_KEY)
			break;
		if (!VOICE_STAT(2))
			breakFlag = true;
	} while (!_vm->shouldQuit() && !breakFlag);


	STOP_VOICE(2);
	DEL_SAMPLE_SDL(20);
	MUSICVOL = oldMusicVol;
	_vm->_eventsManager.ESC_KEY = false;
	VBL_MERDE = 0;
	return true;
}

void SoundManager::DEL_SAMPLE(int soundIndex) {
	if (VOICE_STAT(1) == 1)
		STOP_VOICE(1);
	if (VOICE_STAT(2) == 2)
		STOP_VOICE(2);
	if (VOICE_STAT(3) == 3)
		STOP_VOICE(3);
	DEL_SAMPLE_SDL(soundIndex);
	SOUND[soundIndex]._active = false;
}

void SoundManager::PLAY_SOUND(const Common::String &file) {
	if (!SOUNDOFF) {
		if (SOUND_FLAG)
			DEL_NWAV(SOUND_NUM);
		LOAD_NWAV(file, 1);
		PLAY_NWAV(1);
	}
}

void SoundManager::PLAY_SOUND2(const Common::String &file) {
	if (!SOUNDOFF) {
		LOAD_NWAV(file, 1);
		PLAY_NWAV(1);
	}
}

void SoundManager::MODSetSampleVolume() {
	// No implementatoin needed
}

void SoundManager::MODSetVoiceVolume() {
	// No implementatoin needed
}

void SoundManager::MODSetMusicVolume(int volume) {
	// No implementatoin needed
}

void SoundManager::CHARGE_SAMPLE(int wavIndex, const Common::String &file) {
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSOUND, file);
	LOAD_SAMPLE2_SDL(wavIndex, _vm->_globals.NFICHIER, 0);
	SOUND[wavIndex]._active = true;
}

void SoundManager::PLAY_SAMPLE(int wavIndex, int voiceMode) {
	if (!SOUNDOFF && SOUND[wavIndex]._active) {
		if (SOUND_FLAG)
			DEL_NWAV(SOUND_NUM);
		if (voiceMode == 5) {
			if (VOICE_STAT(1) == 1)
				STOP_VOICE(1);
			PLAY_SAMPLE_SDL(1, wavIndex);
		}
		if (voiceMode == 6) {
			if (VOICE_STAT(2) == 1)
				STOP_VOICE(1);
			PLAY_SAMPLE_SDL(2, wavIndex);
		}
		if (voiceMode == 7) {
			if (VOICE_STAT(3) == 1)
				STOP_VOICE(1);
			PLAY_SAMPLE_SDL(3, wavIndex);
		}
		if (voiceMode == 8) {
			if (VOICE_STAT(1) == 1)
				STOP_VOICE(1);
			PLAY_SAMPLE_SDL(1, wavIndex);
		}
	}
}

void SoundManager::PLAY_SAMPLE2(int idx) {
	if (!SOUNDOFF && SOUND[idx]._active) {
		if (SOUND_FLAG)
			DEL_NWAV(SOUND_NUM);
		if (VOICE_STAT(1) == 1)
			STOP_VOICE(1);
		PLAY_SAMPLE_SDL(1, idx);
	}
}

void SoundManager::LOAD_WAV(const Common::String &file, int wavIndex) {
	LOAD_NWAV(file, wavIndex);
}

void SoundManager::PLAY_WAV(int wavIndex) {
	PLAY_NWAV(wavIndex);
}

int SoundManager::VOICE_STAT(int voiceIndex) {
	if (Voice[voiceIndex]._status) {
		int wavIndex = Voice[voiceIndex]._wavIndex;
		if (Swav[wavIndex]._audioStream != NULL && Swav[wavIndex]._audioStream->endOfStream())
			STOP_VOICE(voiceIndex);
	}

	return Voice[voiceIndex]._status;
}

void SoundManager::STOP_VOICE(int voiceIndex) {
	if (Voice[voiceIndex]._status) {
		Voice[voiceIndex]._status = 0;
		int wavIndex = Voice[voiceIndex]._wavIndex;
		if (Swav[wavIndex]._active) {
			if (Swav[wavIndex].freeSample)
				DEL_SAMPLE_SDL(wavIndex);
		}
	}
	Voice[voiceIndex].fieldC = 0;
	Voice[voiceIndex]._status = 0;
	Voice[voiceIndex].field14 = 0;
}

void SoundManager::SDL_LVOICE(size_t filePosition, size_t entryLength) {
	if (!SDL_LoadVoice(_vm->_globals.NFICHIER, filePosition, entryLength, Swav[20]))
		error("Couldn't load the sample %s", _vm->_globals.NFICHIER.c_str());

	Swav[20]._active = true;
}
	
void SoundManager::PLAY_VOICE_SDL() {
	if (!Swav[20]._active)
		error("Bad handle");

	if (!Voice[2]._status) {
		int wavIndex = Voice[2]._wavIndex;
		if (Swav[wavIndex]._active && Swav[wavIndex].freeSample)
			DEL_SAMPLE_SDL(wavIndex);
	}

	PLAY_SAMPLE_SDL(2, 20);
}

bool SoundManager::DEL_SAMPLE_SDL(int wavIndex) {
	if (Swav[wavIndex]._active) {
		_vm->_mixer->stopHandle(Swav[wavIndex]._soundHandle);
		delete Swav[wavIndex]._audioStream;
		Swav[wavIndex]._audioStream = NULL;
		Swav[wavIndex]._active = false;

		return true;
	} else {
		return false;
	}
}

bool SoundManager::SDL_LoadVoice(const Common::String &filename, size_t fileOffset, size_t entryLength, SwavItem &item) {
	Common::File f;
	if (!f.open(filename)) {
		// Fallback from WAV to APC...
		if (!f.open(setExtension(filename, ".APC")))
			error("Could not open %s for reading", filename.c_str());
	}

	f.seek(fileOffset);
	item._audioStream = makeSoundStream(f.readStream((entryLength == 0) ? f.size() : entryLength));
	f.close();

	return true;
}

void SoundManager::LOAD_SAMPLE2_SDL(int wavIndex, const Common::String &filename, bool freeSample) {
	if (Swav[wavIndex]._active)
		DEL_SAMPLE_SDL(wavIndex);

	SDL_LoadVoice(filename, 0, 0, Swav[wavIndex]);
	Swav[wavIndex]._active = true;
	Swav[wavIndex].freeSample = freeSample;
}

void SoundManager::LOAD_NWAV(const Common::String &file, int wavIndex) {
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSOUND, file);
	LOAD_SAMPLE2_SDL(wavIndex, _vm->_globals.NFICHIER, 1);
}

void SoundManager::PLAY_NWAV(int wavIndex) {
	if (!SOUND_FLAG && !SOUNDOFF) {
		SOUND_FLAG = true;
		SOUND_NUM = wavIndex;
		PLAY_SAMPLE_SDL(1, wavIndex);
	}
}

void SoundManager::DEL_NWAV(int wavIndex) {
	if (DEL_SAMPLE_SDL(wavIndex)) {
		if (VOICE_STAT(1) == 1)
			STOP_VOICE(1);

		SOUND_NUM = 0;
		SOUND_FLAG = false;
	}
}

void SoundManager::PLAY_SAMPLE_SDL(int voiceIndex, int wavIndex) {
	if (!Swav[wavIndex]._active)
		warning("Bad handle");

	if (Voice[voiceIndex]._status == 1 && Swav[wavIndex]._active && Swav[wavIndex].freeSample)
		DEL_SAMPLE_SDL(wavIndex);

	Voice[voiceIndex].fieldC = 0;
	Voice[voiceIndex]._status = 1;
	Voice[voiceIndex].field14 = 4;
	Voice[voiceIndex]._wavIndex = wavIndex;
	
	int volume = (voiceIndex == 2) ? VOICEVOL * 255 / 16 : SOUNDVOL * 255 / 16;

	// Start the voice playing
	Swav[wavIndex]._audioStream->rewind();
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &Swav[wavIndex]._soundHandle, 
		Swav[wavIndex]._audioStream, -1, volume, 0, DisposeAfterUse::NO);
}

void SoundManager::syncSoundSettings() {
	bool muteAll = false;
	if (ConfMan.hasKey("mute"))
		muteAll = ConfMan.getBool("mute");

	// Update the mute settings
	MUSICOFF = muteAll || (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute"));
	SOUNDOFF = muteAll || (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute"));
	VOICEOFF = muteAll || (ConfMan.hasKey("speech_mute") && ConfMan.getBool("speech_mute"));

	// Update the volume levels
	MUSICVOL = MIN(255, ConfMan.getInt("music_volume")) * 16 / 255;
	SOUNDVOL = MIN(255, ConfMan.getInt("sfx_volume")) * 16 / 255;
	VOICEVOL = MIN(255, ConfMan.getInt("speech_volume")) * 16 / 255;

	// Update any active sounds
	for (int idx = 0; idx < SWAV_COUNT; ++idx) {
		if (Swav[idx]._active) {
			int volume = (idx == 20) ? (VOICEVOL * 255 / 16) : (SOUNDVOL * 255 / 16);
			_vm->_mixer->setChannelVolume(Swav[idx]._soundHandle, volume);
		}
	}
	for (int idx = 0; idx < MWAV_COUNT; ++idx) {
		if (Mwav[idx]._active) {
			_vm->_mixer->setChannelVolume(Mwav[idx]._soundHandle, MUSICVOL * 255 / 16);
		}
	}
}

void SoundManager::updateScummVMSoundSettings() {
	ConfMan.setBool("mute", MUSICOFF && SOUNDOFF && VOICEOFF);
	ConfMan.setBool("music_mute", MUSICOFF);
	ConfMan.setBool("sfx_mute", SOUNDOFF);
	ConfMan.setBool("speech_mute", VOICEOFF);

	ConfMan.setInt("music_volume", MUSICVOL * 255 / 16);
	ConfMan.setInt("sfx_volume", SOUNDVOL * 255 / 16);
	ConfMan.setInt("speech_volume", VOICEVOL * 255 / 16);

	ConfMan.flushToDisk();
}

Audio::RewindableAudioStream *SoundManager::makeSoundStream(Common::SeekableReadStream *stream) {
	if (_vm->getPlatform() == Common::kPlatformWindows)
		return Audio::makeAPCStream(stream, DisposeAfterUse::YES);	
	else
		return Audio::makeWAVStream(stream, DisposeAfterUse::YES);	
}

// Blatant rip from gob engine. Hi DrMcCoy!
Common::String SoundManager::setExtension(const Common::String &str, const Common::String &ext) {
	if (str.empty())
		return str;

	const char *dot = strrchr(str.c_str(), '.');
	if (dot)
		return Common::String(str.c_str(), dot - str.c_str()) + ext;

	return str + ext;
}
} // End of namespace Hopkins
