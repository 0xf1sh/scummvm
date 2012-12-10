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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/files.h"
#include "hopkins/saveload.h"
#include "hopkins/sound.h"
#include "hopkins/talk.h"

namespace Hopkins {

HopkinsEngine *g_vm;

HopkinsEngine::HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Hopkins"), _animationManager() {
	g_vm = this;
	_debugger.setParent(this);
	_animationManager.setParent(this);
	_computerManager.setParent(this);
	_dialogsManager.setParent(this);
	_eventsManager.setParent(this);
	_fileManager.setParent(this);
	_fontManager.setParent(this);
	_globals.setParent(this);
	_graphicsManager.setParent(this);
	_linesManager.setParent(this);
	_menuManager.setParent(this);
	_objectsManager.setParent(this);
	_saveLoadManager.setParent(this);
	_scriptManager.setParent(this);
	_soundManager.setParent(this);
	_talkManager.setParent(this);
}

HopkinsEngine::~HopkinsEngine() {
}

Common::String HopkinsEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool HopkinsEngine::canLoadGameStateCurrently() {
	return !_globals.SORTIE && !_globals.PLAN_FLAG && _eventsManager.souris_flag;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool HopkinsEngine::canSaveGameStateCurrently() {
	return !_globals.SORTIE && !_globals.PLAN_FLAG && _eventsManager.souris_flag;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error HopkinsEngine::loadGameState(int slot) {
	return _saveLoadManager.restore(slot);
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error HopkinsEngine::saveGameState(int slot, const Common::String &desc) {
	return _saveLoadManager.save(slot, desc);
}

Common::Error HopkinsEngine::run() {
	_saveLoadManager.initSaves();

	Common::StringMap iniParams;
	_fileManager.loadIniFile(iniParams);
	processIniParams(iniParams);

	_globals.setConfig();
	_fileManager.initCensorship();
	INIT_SYSTEM();

	_soundManager.WSOUND_INIT();

	if (getPlatform() == Common::kPlatformLinux) {
		if (getIsDemo())
			runLinuxDemo();
		else
			runLinuxFull();
	} else if (getPlatform() == Common::kPlatformWindows) {
		if (getIsDemo())
			runWin95Demo();
		else
			runWin95full();
	} else {
		warning("Unhandled version, switching to linux demo");
		runLinuxDemo();
	}

	return Common::kNoError;
}

bool HopkinsEngine::runWin95Demo() {
	_globals.SVGA = 1;

	_globals.CHARGE_OBJET();
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);

	_globals.HELICO = 0;
	_globals.iRegul = 1;

	warning("TODO Affiche_Version(1)");

	_graphicsManager.DD_LOCK();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_UNLOCK();
	_graphicsManager.Cls_Pal();

	_graphicsManager.LOAD_IMAGE("H2");
	_graphicsManager.FADE_INW();

	if (!_eventsManager.ESC_KEY)
		INTRORUN();

	warning("TODO Fin_Interrupt()");
	warning("TODO TEST = 1;");
	warning("TODO no_vsync = 1;");
	_eventsManager.lItCounter = 0;
	warning("TODO Init_Interrupt_();");

	_globals.iRegul = 1;
	_globals.vitesse = 1;

	for (int i = 1; i < 50; i++) {
		_graphicsManager.SCOPY(_graphicsManager.VESA_SCREEN, 0, 0, 640, 440, _graphicsManager.VESA_BUFFER, 0, 0);
		_eventsManager.VBL();
	}

	_globals.iRegul = 0;
	warning("TODO SPEEDJ = _globals.lItCounter;");
	warning("TODO no_vsync = 0;");
	warning("TODO TEST = 0;");
//	if (SPEEDJ > 475)
	if (_eventsManager.lItCounter > 475)
		_globals.vitesse = 2;
//	if (SPEEDJ > 700)
	if (_eventsManager.lItCounter > 700)
		_globals.vitesse = 3;
	warning("TODO Fin_Interrupt_();");
	warning("TODO Init_Interrupt_();");
	_graphicsManager.FADE_OUTW();
	_globals.iRegul = 1;
	_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.PLANX = _globals.PLANY = 0;
	memset(_globals.SAUVEGARDE, 0, 2000);
	_globals.SORTIE = 0;
	_globals.PASSWORD = true;
	
	if (getLanguage() != Common::PL_POL)
		if (!ADULT())
			return Common::kNoError;

	for (;;) {
		if (_globals.SORTIE == 300)
			_globals.SORTIE = 0;
	
		if (!_globals.SORTIE) {
			_globals.SORTIE = _menuManager.MENU();
			if (_globals.SORTIE == -1) {
				_globals.PERSO = _globals.dos_free2(_globals.PERSO);
				REST_SYSTEM();
				return false;
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals.SORTIE) {
		case 1:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 2);
			break;

		case 3:
			if (!_globals.SAUVEGARDE->data[svField170]) {
				_soundManager.WSOUND(3);
				if (_globals.FR == 1)
					_graphicsManager.LOAD_IMAGE("fondfr");
				if (!_globals.FR)
					_graphicsManager.LOAD_IMAGE("fondan");
				_graphicsManager.FADE_INW();
				_eventsManager.delay(500);
				_graphicsManager.FADE_OUTW();
				_globals.iRegul = 1;
				_soundManager.SPECIAL_SOUND = 2;
				_graphicsManager.DD_LOCK();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_UNLOCK();
				_graphicsManager.Cls_Pal();
				if (!_globals.CENSURE)
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				else
					_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				_soundManager.SPECIAL_SOUND = 0;
				_soundManager.DEL_SAMPLE(1);
				_soundManager.DEL_SAMPLE(2);
				_soundManager.DEL_SAMPLE(3);
				_soundManager.DEL_SAMPLE(4);
				_graphicsManager.FADE_OUTW();
				_globals.SAUVEGARDE->data[svField170] = 1;
			}
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
			break;

		case 4:
			_globals.DESACTIVE_INVENT = true;
			_objectsManager.PLAN_BETA();
			_globals.DESACTIVE_INVENT = false;
			break;

		case 5:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.NOSPRECRAN = true;
			_globals.Max_Perso_Y = 455;
	
			if (_globals.SAUVEGARDE->data[svField80]) {
				if (_globals.SAUVEGARDE->data[svField80] == 1)
					_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
			} else {
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
			}
	
			_globals.NOSPRECRAN = false;
			break;

		case 6:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 460;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			break;

		case 7:
			if (_globals.SAUVEGARDE->data[svField220])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
			break;

		case 8:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
			break;

		case 9:
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Propre = 15;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Ligne_Long = 20;
			if (_globals.SAUVEGARDE->data[svField225])
			  _objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
			else
			  BOOM();
			break;

		case 10:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
			_globals.NOSPRECRAN = false;
			break;

		case 11:
			_globals.NOSPRECRAN = true;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_globals.Max_Propre = 15;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
			_globals.NOSPRECRAN = false;
			break;

		case 12:
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Propre = 15;
			_globals.Max_Perso_Y = 450;
			_globals.Max_Ligne_Long = 20;
			if (_globals.SAUVEGARDE->data[svField225]) {
				if (_globals.FR == 1)
					_graphicsManager.LOAD_IMAGE("ENDFR");
				else if (!_globals.FR)
					_graphicsManager.LOAD_IMAGE("ENDUK");
				_graphicsManager.FADE_INW();
				_eventsManager.MOUSE_ON();
				do
					_eventsManager.VBL();
				while (_eventsManager.BMOUSE() != 1);
				_graphicsManager.FADE_OUTW();
				REST_SYSTEM();
			}
			BOOM();
			break;

		case 13:
		case 14:
		case 15:
			NO_DISPO(11);
			break;

		case 16:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 33:
		case 32:
		case 34: 
			NO_DISPO(4);
			break;

		case 17:
			NO_DISPO(1);
			break;

		case 111:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 112:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 113:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 113;
			_globals.SAUVEGARDE->data[svField5] = _globals.ECRAN;
			_computerManager.showComputer(COMPUTER_HOPKINS);
			_graphicsManager.MODE_VESA();
			break;

		case 114:
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 114;
			_globals.SAUVEGARDE->data[svField5] = _globals.ECRAN;
			_globals.SORTIE = 0;
			_computerManager.showComputer(COMPUTER_SAMANTHAS);
			_graphicsManager.MODE_VESA();
			break;

		case 115:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 115;
			_globals.SAUVEGARDE->data[svField5] = _globals.ECRAN;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.MODE_VESA();
			break;

		case 150:
			_soundManager.WSOUND(28);
			_globals.iRegul = 4; // CHECKME!
			_graphicsManager.DD_LOCK();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_UNLOCK();
			_graphicsManager.Cls_Pal();
			_animationManager.playAnim("JOUR1A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 151:
			_soundManager.WSOUND(28);
			_globals.iRegul = 4; // CHECKME!
			_graphicsManager.DD_LOCK();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_UNLOCK();
			_graphicsManager.Cls_Pal();
			_graphicsManager.LOAD_IMAGE("njour3a");
			_graphicsManager.FADE_INW();
			_eventsManager.delay(5000);
			_graphicsManager.FADE_OUTW();
			_globals.SORTIE = 300;
			_globals.iRegul = 0;
			break;

		case 152:
			_soundManager.WSOUND(28);
			_globals.iRegul = 4; // CHECKME!
			_graphicsManager.DD_LOCK();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_UNLOCK();
			_graphicsManager.Cls_Pal();
			_animationManager.playAnim("JOUR4A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runLinuxDemo() {
	_globals.CHARGE_OBJET();
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);

	_globals.HELICO = 0;
	_eventsManager.MOUSE_OFF();

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();

	_graphicsManager.LOAD_IMAGE("LINUX");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(1500);
	_graphicsManager.FADE_OUTW();

	if (!_globals.internet) {
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("MP.ANM", 10, 16, 200);
	}

	_graphicsManager.LOAD_IMAGE("H2");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(500);
	_graphicsManager.FADE_OUTW();

	if (!_eventsManager.ESC_KEY)
		INTRORUN();
  
	_globals.iRegul = 0;
	_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.PLANX = _globals.PLANY = 0;
	memset(_globals.SAUVEGARDE, 0, 2000);
	_globals.SORTIE = 0;
	_globals.PASSWORD = true;

	for (;;) {
		if (_globals.SORTIE == 300)
			_globals.SORTIE = 0;
	
		if (!_globals.SORTIE) {
			_globals.SORTIE = _menuManager.MENU();
			if (_globals.SORTIE == -1) {
				if (!g_system->getEventManager()->shouldQuit())
					PUBQUIT();
				_globals.PERSO = _globals.dos_free2(_globals.PERSO);
				REST_SYSTEM();
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals.SORTIE) {
		case 17:
		case 18:
		case 19:
		case 20:
		case 22:
		case 23:
		case 24:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 34:
		case 38:
			PASS();
			break;

		case 1:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1);
			break;

		case 3:
			if (!_globals.SAUVEGARDE->data[svField170]) {
				_soundManager.WSOUND(3);
				if (_globals.FR == 1)
					_graphicsManager.LOAD_IMAGE("fondfr");
				if (!_globals.FR)
					_graphicsManager.LOAD_IMAGE("fondan");
				if (_globals.FR == 2)
					_graphicsManager.LOAD_IMAGE("fondes");
				_graphicsManager.FADE_INW();
				_eventsManager.delay(500);
				_graphicsManager.FADE_OUTW();
				_globals.iRegul = 1;
				_soundManager.SPECIAL_SOUND = 2;

				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
				_graphicsManager.Cls_Pal();
				_graphicsManager.FADE_LINUX = 2;
		
				if (!_globals.CENSURE)
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				else
					_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				_soundManager.SPECIAL_SOUND = 0;
				_soundManager.DEL_SAMPLE(1);
				_soundManager.DEL_SAMPLE(2);
				_soundManager.DEL_SAMPLE(3);
				_soundManager.DEL_SAMPLE(4);
				_globals.SAUVEGARDE->data[svField170] = 1;
			}
           
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
			break;
        
		case 4:
			_globals.DESACTIVE_INVENT = true;
			_objectsManager.PLAN_BETA();
			_globals.DESACTIVE_INVENT = false;
			break;

		case 5:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 455;
			_globals.NOSPRECRAN = true;
			if (_globals.SAUVEGARDE->data[svField80]) {
				if (_globals.SAUVEGARDE->data[svField80] == 1)
					_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
			} else {
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
			}
	        
			_globals.NOSPRECRAN = false;
			break;
       
		case 6:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 460;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			break;

		case 7:
			if (_globals.SAUVEGARDE->data[svField220])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
			break;

		case 8:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
			break;

		case 9:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			
			if (!_globals.SAUVEGARDE->data[svField225])
				BOOM();

			_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
			break;

		case 10:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
			_globals.NOSPRECRAN = false;
			break;

		case 11:
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
			_globals.NOSPRECRAN = false;
			break;

		case 12:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			if (_globals.SAUVEGARDE->data[svField225]) {
				_globals.NOSPRECRAN = true;
				_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1);
			} else {
				BOOM();
			}
			break;

		case 13:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1);
			break;

		case 14:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1);
			break;

		case 15:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29);
			_globals.NOSPRECRAN = false;
			break;

		case 16:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;

			if (_globals.SAUVEGARDE->data[svField113] == 1) {
				_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7);
			} else if (!_globals.SAUVEGARDE->data[svField113]) {
				_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7);
			}
			break; 

		case 25:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30);
			break;

		case 26:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30);

		case 33:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8);
			_globals.NOSPRECRAN = false;
			break;

		case 35:
			ENDEMO();
			break;

		case 111:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 112:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 113:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 113;
			_globals.SAUVEGARDE->data[svField5] = 113;
			_computerManager.showComputer(COMPUTER_HOPKINS);

			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.DD_VBL();
			memset(_graphicsManager.VESA_BUFFER, 0, 0x4B000u);
			memset(_graphicsManager.VESA_SCREEN, 0, 0x4B000u);
			_graphicsManager.Cls_Pal();
			_graphicsManager.RESET_SEGMENT_VESA();
			break;

		case 114:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 114;
			_globals.SAUVEGARDE->data[svField5] = 114;
			_computerManager.showComputer(COMPUTER_SAMANTHAS);
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			break;

		case 115:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 115;
			_globals.SAUVEGARDE->data[svField5] = 115;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			break;

		case 150:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;

			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.Cls_Pal();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR1A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 151:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
               
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.Cls_Pal();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR3A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 152:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;

			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.Cls_Pal();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR4A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runWin95full() {
	_globals.SVGA = 2;
//	_SPEED_SCROLL = 4;

	warning("TODO: Init_Interrupt_()");

	_globals.CHARGE_OBJET();
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);
	_globals.HELICO = 0;
	_globals.iRegul = 1;

	warning("TODO: Affiche_Version();");

	_graphicsManager.DD_LOCK();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_UNLOCK();
	_graphicsManager.Cls_Pal();

	_animationManager.playAnim("MP.ANM", 10, 16, 200);
	_graphicsManager.FADE_OUTW();
	if (!_eventsManager.ESC_KEY)
		INTRORUN();
	_graphicsManager.LOAD_IMAGE("H2");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(500);
	_graphicsManager.FADE_OUTW();
	_globals.iRegul = 0;
	_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");

	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.PLANX = _globals.PLANY = 0;
	memset(_globals.SAUVEGARDE, 0, 2000);
	_globals.SORTIE = 0;
	_globals.PASSWORD = true;
	for (;;) {
		if (_globals.SORTIE == 300)
			_globals.SORTIE = 0;

		if (!_globals.SORTIE) {
			_globals.SORTIE = _menuManager.MENU();;
			if (_globals.SORTIE == -1) {
				_globals.PERSO = _globals.dos_free2(_globals.PERSO);
				REST_SYSTEM();
				return false;
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals.SORTIE) {
		case 1:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1);
			break;

		case 3:
			if (!_globals.SAUVEGARDE->data[svField170]) {
				_soundManager.WSOUND(3);
				if (_globals.FR == 1)
					_graphicsManager.LOAD_IMAGE("fondfr");
				else if (!_globals.FR)
					_graphicsManager.LOAD_IMAGE("fondan");
				_graphicsManager.FADE_INW();
				_eventsManager.delay(500);
				_graphicsManager.FADE_OUTW();
				_soundManager.SPECIAL_SOUND = 2;
				_globals.iRegul = 1;
				_graphicsManager.DD_LOCK();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_UNLOCK();
				_graphicsManager.Cls_Pal();
				if (!_globals.CENSURE)
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				else
					_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				_soundManager.SPECIAL_SOUND = 0;
				_soundManager.DEL_SAMPLE(1);
				_soundManager.DEL_SAMPLE(2);
				_soundManager.DEL_SAMPLE(3);
				_soundManager.DEL_SAMPLE(4);
				_graphicsManager.FADE_OUTW();
				_globals.SAUVEGARDE->data[svField170] = 1;
			}
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.NOSPRECRAN = true;
			_globals.Max_Perso_Y = 450;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
			break;

		case 4:
			_globals.DESACTIVE_INVENT = true;
			_objectsManager.PLAN_BETA();
			_globals.DESACTIVE_INVENT = false;
			break;

		case 5:
			_globals.Max_Propre = _globals.SORTIE;
			_globals.Max_Ligne_Long = _globals.SORTIE;
			_globals.Max_Propre_Gen = _globals.SORTIE;
			_globals.NOSPRECRAN = true;
			_globals.Max_Perso_Y = 455;
			if (_globals.SAUVEGARDE->data[svField80]) {
				if (_globals.SAUVEGARDE->data[svField80] == 1)
					_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
			} else {
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
			}
	
			_globals.NOSPRECRAN = false;
			break;

		case 6:
			_globals.Max_Propre = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 460;
			_globals.Max_Ligne_Long = 20;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			break;

		case 7:
			if (_globals.SAUVEGARDE->data[svField220])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
			break;

		case 8:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
			break;

		case 9:
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			if (_globals.SAUVEGARDE->data[svField225])
				_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
			else
				BOOM();
			break;

		case 10:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
			_globals.NOSPRECRAN = false;
			break;

		case 11:
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
			_globals.NOSPRECRAN = false;
			break;

		case 12:
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Perso_Y = 450;
			_globals.Max_Propre = 15;
			if (_globals.SAUVEGARDE->data[svField225]) {
				_globals.NOSPRECRAN = true;
				_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1);
			} else {
				BOOM();
			}
			break;

		case 13:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1);
			break;

		case 14:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1);
			break;

		case 15:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29);
			_globals.NOSPRECRAN = false;
			break;

		case 16:
			_globals.Max_Perso_Y = 450;
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			if (_globals.SAUVEGARDE->data[svField113] == 1) {
				_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7);
			} else if (!_globals.SAUVEGARDE->data[svField113]) {
				_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7);
			}
			break;

		case 17:
			_globals.Max_Propre = 50;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Ligne_Long = 40;
			if (_globals.SAUVEGARDE->data[svField117] == 1) {
				_objectsManager.PERSONAGE2("IM17", "IM17A", "ANIM17", "IM17", 11);
			} else if (!_globals.SAUVEGARDE->data[svField117]) {
				_objectsManager.PERSONAGE2("IM17", "IM17", "ANIM17", "IM17", 11);
			}
			if (_globals.SORTIE == 18) {
				_globals.iRegul = 1;
				_graphicsManager.DD_LOCK();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_UNLOCK();
				_graphicsManager.Cls_Pal();
				_soundManager.WSOUND_OFF();
				_soundManager.WSOUND(29);
				if (_globals.SVGA == 2)
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
				else if (_globals.SVGA == 1)
					_animationManager.playAnim("PURG1.ANM", 12, 18, 50);
				_graphicsManager.FADE_OUTS();
				_globals.iRegul = 0;
			}
			break;

		case 18:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 450;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM18", "IM18", "ANIM18", "IM18", 29);
			break;

		case 19:
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			if (_globals.SAUVEGARDE->data[svField123])
				_objectsManager.PERSONAGE2("IM19", "IM19A", "ANIM19", "IM19", 6);
			else
				_objectsManager.PERSONAGE2("IM19", "IM19", "ANIM19", "IM19", 6);
			break;

		case 20:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 10;
			_globals.Max_Propre_Gen = 8;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM20", "IM20", "ANIM20", "IM20", 6);
			if (_globals.SORTIE == 17) {
				_globals.iRegul = 1;
				_soundManager.WSOUND_OFF();
				_graphicsManager.DD_LOCK();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_UNLOCK();
				_graphicsManager.Cls_Pal();
				_soundManager.WSOUND(6);
				if (_globals.SVGA == 2)
					_animationManager.playAnim("PURG2A.ANM", 12, 18, 50);
				else if (_globals.SVGA == 1)
					_animationManager.playAnim("PURG2.ANM", 12, 18, 50);
				_graphicsManager.FADE_OUTS();
				_globals.iRegul = 0;
			}
			break;

		case 22:
			_globals.Max_Propre = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_globals.Max_Ligne_Long = 20;
			_objectsManager.PERSONAGE2("IM22", "IM22", "ANIM22", "IM22", 6);
			break;

		case 23:
			_globals.Max_Propre = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Ligne_Long = 20;
			_objectsManager.PERSONAGE2("IM23", "IM23", "ANIM23", "IM23", 6);
			break;

		case 24:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			if (_globals.SAUVEGARDE->data[svField181]) {
				if (_globals.SAUVEGARDE->data[svField181] == 1)
					_objectsManager.PERSONAGE2("IM24", "IM24a", "ANIM24", "IM24", 1);
			} else {
				_objectsManager.PERSONAGE2("IM24", "IM24", "ANIM24", "IM24", 1);
			}
			break;

		case 25:
			_globals.Max_Propre = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_globals.Max_Ligne_Long = 20;
			_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30);
			break;

		case 26:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30);
			break;

		case 27:
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre = 10;
			if (_globals.SAUVEGARDE->data[svField177] == 1) {
				_objectsManager.PERSONAGE2("IM27", "IM27A", "ANIM27", "IM27", 27);
			} else if (!_globals.SAUVEGARDE->data[svField177]) {
				_objectsManager.PERSONAGE2("IM27", "IM27", "ANIM27", "IM27", 27);
			}
			break;

		case 28:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			_globals.NOSPRECRAN = true;
			if (_globals.SAUVEGARDE->data[svField166] != 1 || _globals.SAUVEGARDE->data[svField167] != 1)
				_objectsManager.PERSONAGE2("IM28", "IM28", "ANIM28", "IM28", 1);
			else
				_objectsManager.PERSONAGE2("IM28a", "IM28", "ANIM28", "IM28", 1);
			break;

		case 29:
			_globals.Max_Propre = 60;
			_globals.Max_Ligne_Long = 50;
			_globals.Max_Propre_Gen = 50;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM29", "IM29", "ANIM29", "IM29", 1);
			break;

		case 30:
			_globals.Max_Propre = 10;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM30", "IM30", "ANIM30", "IM30", 24);
			_globals.NOSPRECRAN = false;
			break;

		case 31:
			_objectsManager.PERSONAGE("IM31", "IM31", "ANIM31", "IM31", 10);
			break;

		case 32:
			_globals.Max_Propre = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_globals.Max_Ligne_Long = 20;
			_objectsManager.PERSONAGE2("IM32", "IM32", "ANIM32", "IM32", 2);
			break;

		case 33:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8);
			_globals.NOSPRECRAN = false;
			break;

		case 34:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM34", "IM34", "ANIM34", "IM34", 2);
			_globals.NOSPRECRAN = false;
			break;

		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41: {
				_globals.Max_Propre_Gen = 20;
				_globals.fmusic = 13;
				_globals.Max_Propre = 50;
				_globals.Max_Ligne_Long = 40;
				_globals.Max_Perso_Y = 435;
				_globals.DESACTIVE_INVENT = false;
				_globals.FORET = true;
				_globals.NOSPRECRAN = true;
				Common::String im = Common::String::format("IM%d", _globals.SORTIE);
				_soundManager.WSOUND(13);
				if (_globals.FORETSPR == g_PTRNUL) {
					_fileManager.constructFilename(_globals.HOPSYSTEM, "HOPDEG.SPR");
					_globals.FORETSPR = _objectsManager.CHARGE_SPRITE(_globals.NFICHIER);
					_soundManager.CHARGE_SAMPLE(1, "SOUND41.WAV");
				}
				_objectsManager.PERSONAGE2(im, im, "BANDIT", im, 13);
				_globals.NOSPRECRAN = false;
				if (_globals.SORTIE < 35 || _globals.SORTIE > 49) {
					_globals.dos_free2(_globals.FORETSPR);
					_globals.FORETSPR = g_PTRNUL;
					_globals.FORET = false;
					_soundManager.DEL_SAMPLE(1);
				}
				break;
				}
		case 50:
			AVION();
			_globals.SORTIE = 51;
			break;

		case 51:
			_globals.Max_Ligne_Long = 10;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre = 20;
			_objectsManager.PERSONAGE2("IM51", "IM51", "ANIM51", "IM51", 14);
			break;

		case 52:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM52", "IM52", "ANIM52", "IM52", 14);
			break;

		case 54:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM54", "IM54", "ANIM54", "IM54", 14);
			break;

		case 55:
			_globals.Max_Propre = 40;
			_globals.Max_Perso_Y = 460;
			_globals.Max_Ligne_Long = 30;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM55", "IM55", "ANIM55", "IM55", 14);
			break;

		case 56:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM56", "IM56", "ANIM56", "IM56", 14);
			break;

		case 57:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM57", "IM57", "ANIM57", "IM57", 14);
			break;

		case 58:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM58", "IM58", "ANIM58", "IM58", 14);
			break;

		case 59:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM59", "IM59", "ANIM59", "IM59", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 60:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM60", "IM60", "ANIM60", "IM60", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 61:
			if (_globals.SAUVEGARDE->data[svField311] == 1 && !_globals.SAUVEGARDE->data[svField312])
				INCENDIE();
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM61", "IM61", "ANIM61", "IM61", 21);
			break;

		case 62:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM62", "IM62", NULL, "IM62", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 63:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM63", "IM63", "ANIM63", "IM63", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 64:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM64", "IM64", "ANIM64", "IM64", 21);
			break;

		case 65:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM65", "IM65", "ANIM65", "IM65", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 66:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM66", "IM66", "ANIM66", "IM66", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 67:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM67", "IM67", NULL, "IM67", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 68:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM68", "IM68", "ANIM68", "IM68", 21);
			break;

		case 69:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM69", "IM69", "ANIM69", "IM69", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 70:
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre = 8;
			_globals.NOSPRECRAN = true;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Propre_Gen = 20;
			_objectsManager.PERSONAGE2("IM70", "IM70", NULL, "IM70", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 71:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM71", "IM71", "ANIM71", "IM71", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 73:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			if (_globals.SAUVEGARDE->data[svField318] == 1) {
				_objectsManager.PERSONAGE2("IM73", "IM73A", "ANIM73", "IM73", 21);
			} else if (!_globals.SAUVEGARDE->data[svField318]) {
				_objectsManager.PERSONAGE2("IM73", "IM73", "ANIM73", "IM73", 21);
			}
			break;

		case 75:
			BASE();
			break;

		case 77:
			OCEAN(77, "OCEAN01", "OCEAN1", 3, 0, 84, 0, 0, 25);
			break;

		case 78:
			OCEAN(78, "OCEAN02", "OCEAN1", 1, 0, 91, 84, 0, 25);
			break;

		case 79:
			OCEAN(79, "OCEAN03", "OCEAN1", 7, 87, 0, 0, 83, 25);
			break;

		case 80:
			OCEAN(80, "OCEAN04", "OCEAN1", 1, 86, 88, 0, 81, 25);
			break;

		case 81:
			OCEAN(81, "OCEAN05", "OCEAN1", 1, 91, 82, 80, 85, 25);
			break;

		case 82:
			OCEAN(82, "OCEAN06", "OCEAN1", 7, 81, 0, 88, 0, 25);
			break;

		case 83:
			OCEAN(83, "OCEAN07", "OCEAN1", 1, 89, 0, 79, 88, 25);
			break;

		case 84:
			OCEAN(84, "OCEAN08", "OCEAN1", 1, 77, 0, 0, 78, 25);
			break;

		case 85:
			OCEAN(85, "OCEAN09", "OCEAN1", 1, 0, 0, 81, 0, 25);
			break;

		case 86:
			OCEAN(86, "OCEAN10", "OCEAN1", 1, 0, 80, 0, 91, 25);
			break;

		case 87:
			OCEAN(87, "OCEAN11", "OCEAN1", 3, 0, 79, 90, 0, 25);
			break;

		case 88:
			OCEAN(88, "OCEAN12", "OCEAN1", 1, 80, 0, 83, 82, 25);
			break;

		case 89:
			OCEAN(89, "OCEAN13", "OCEAN1", 3, 0, 83, 0, 0, 25);
			break;

		case 90:
			BASED();
			break;

		case 91:
			OCEAN(91, "OCEAN15", "OCEAN1", 3, 78, 81, 86, 0, 25);
			break;

		case 93:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 445;
			if (_globals.SAUVEGARDE->data[svField330])
				_objectsManager.PERSONAGE2("IM93", "IM93c", "ANIM93", "IM93", 29);
			else
				_objectsManager.PERSONAGE2("IM93", "IM93", "ANIM93", "IM93", 29);
			break;

		case 94:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 440;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM94", "IM94", "ANIM94", "IM94", 19);
			break;

		case 95:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = false;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM95", "IM95", "ANIM95", "IM95", 19);
			break;

		case 96:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.NOSPRECRAN = true;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM96", "IM96", "ANIM96", "IM96", 19);
			break;

		case 97:
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre = 5;
			_globals.NOSPRECRAN = true;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM97", "IM97", "ANIM97", "IM97", 19);
			if (_globals.SORTIE == 18) {
				_globals.iRegul = 1;
				_soundManager.WSOUND_OFF();
				_graphicsManager.DD_LOCK();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_UNLOCK();
				_graphicsManager.Cls_Pal();
				_soundManager.WSOUND(6);
				if (_globals.SVGA == 2)
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
				else if (_globals.SVGA == 1)
					_animationManager.playAnim("PURG1.ANM", 12, 18, 50);
				_graphicsManager.FADE_OUTS();
				_globals.iRegul = 0;
			}
			break;

		case 98:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM98", "IM98", "ANIM98", "IM98", 19);
			break;

		case 99:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Perso_Y = 435;
			_globals.Max_Propre_Gen = 5;
			_objectsManager.PERSONAGE2("IM99", "IM99", "ANIM99", "IM99", 19);
			break;

		case 100:
			JOUE_FIN();
			break;

		case 111:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 112:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 113:
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.ECRAN = 113;
			_globals.SAUVEGARDE->data[svField6] = _globals.OLD_ECRAN;
			_globals.SAUVEGARDE->data[svField5] = _globals.ECRAN;
			_globals.SORTIE = 0;
			_computerManager.showComputer(COMPUTER_HOPKINS);
			_graphicsManager.MODE_VESA();
			break;

		case 114:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.ECRAN = 114;
			_globals.SAUVEGARDE->data[svField6] = _globals.OLD_ECRAN;
			_globals.SAUVEGARDE->data[svField5] = _globals.ECRAN;
			_computerManager.showComputer(COMPUTER_SAMANTHAS);
			_graphicsManager.MODE_VESA();
			break;

		case 115:
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.ECRAN = 115;
			_globals.SAUVEGARDE->data[svField6] = _globals.OLD_ECRAN;
			_globals.SAUVEGARDE->data[svField5] = _globals.ECRAN;
			_globals.SORTIE = 0;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.MODE_VESA();
			break;

		case 150:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
			_graphicsManager.DD_LOCK();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_UNLOCK();
			_graphicsManager.Cls_Pal();
			_animationManager.playAnim("JOUR1A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 151:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
			_graphicsManager.DD_LOCK();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_UNLOCK();
			_graphicsManager.Cls_Pal();
			_animationManager.playAnim("JOUR3A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 152:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
			_graphicsManager.DD_LOCK();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_UNLOCK();
			_graphicsManager.Cls_Pal();
			_animationManager.playAnim("JOUR4A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 194:
		case 195:
		case 196:
		case 197:
		case 198:
		case 199:
			_globals.PERSO = _globals.dos_free2(_globals.PERSO);
			_globals.iRegul = 1;
			_soundManager.WSOUND_OFF();
			warning("TODO: heapshrink();");
			_soundManager.WSOUND(23);
			_globals.SORTIE = WBASE();
			_soundManager.WSOUND_OFF();
			warning("TODO: heapshrink();");
			_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
			_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
			_globals.PERSO_TYPE = 0;
			_globals.iRegul = 0;
			_graphicsManager.nbrligne = SCREEN_WIDTH;
			_graphicsManager.MODE_VESA();
			if (_globals.SORTIE == -1)
				error("FIN BASE SOUS MARINE");
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runLinuxFull() {
	_soundManager.WSOUND(16);

	_globals.CHARGE_OBJET();
	_objectsManager.CHANGE_OBJET(14);
	_objectsManager.AJOUTE_OBJET(14);

	_globals.HELICO = 0;
	_eventsManager.MOUSE_OFF();

	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();

	_graphicsManager.LOAD_IMAGE("H2");
	_graphicsManager.FADE_INW();
	_eventsManager.delay(500);

	_globals.vitesse = 2;
	_globals.iRegul = 1;
	_graphicsManager.FADE_LINUX = 2;
	_graphicsManager.FADE_OUTW();

	if (!_eventsManager.ESC_KEY)
		INTRORUN();
  
	_globals.iRegul = 0;
	_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.PLANX = _globals.PLANY = 0;
	memset(_globals.SAUVEGARDE, 0, 2000);
	_globals.SORTIE = 0;
	_globals.PASSWORD = false;

	for (;;) {
		if (_globals.SORTIE == 300)
			_globals.SORTIE = 0;
		if (!_globals.SORTIE) {
			_globals.SORTIE = _menuManager.MENU();
			if (_globals.SORTIE == -1) {
				_globals.PERSO = _globals.dos_free2(_globals.PERSO);
				REST_SYSTEM();
				return true;
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals.SORTIE) {
		case 1:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1);
			break;

		case 3:
			if (!_globals.SAUVEGARDE->data[svField170]) {
				_soundManager.WSOUND(3);
				if (_globals.FR == 1)
					_graphicsManager.LOAD_IMAGE("fondfr");
				else if (!_globals.FR)
					_graphicsManager.LOAD_IMAGE("fondan");
				else if (_globals.FR == 2)
					_graphicsManager.LOAD_IMAGE("fondes");
				_graphicsManager.FADE_INW();
				_eventsManager.delay(500);
				_graphicsManager.FADE_OUTW();
				_globals.iRegul = 1;
				_soundManager.SPECIAL_SOUND = 2;
				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
				_graphicsManager.Cls_Pal();
				_graphicsManager.FADE_LINUX = 2;
				if (!_globals.CENSURE)
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				else
					_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				_soundManager.SPECIAL_SOUND = 0;
				_soundManager.DEL_SAMPLE(1);
				_soundManager.DEL_SAMPLE(2);
				_soundManager.DEL_SAMPLE(3);
				_soundManager.DEL_SAMPLE(4);
				_globals.SAUVEGARDE->data[svField170] = 1;
			}

			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2);
			break;

		case 4:
			_globals.DESACTIVE_INVENT = true;
			_objectsManager.PLAN_BETA();
			_globals.DESACTIVE_INVENT = false;
			break;

		case 5:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 455;
			_globals.NOSPRECRAN = true;
			if (_globals.SAUVEGARDE->data[svField80]) {
				if (_globals.SAUVEGARDE->data[svField80] == 1)
					_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3);
			} else {
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3);
			}
			_globals.NOSPRECRAN = false;
			break;

		case 6:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 460;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2);
			break;

		case 7:
			if (_globals.SAUVEGARDE->data[svField220])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2);
			break;

		case 8:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2);
			break;

		case 9:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			if (!_globals.SAUVEGARDE->data[svField225])
				BOOM();
			_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10);
			break;

		case 10:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9);
			_globals.NOSPRECRAN = false;
			break;

		case 11:
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2);
			_globals.NOSPRECRAN = false;
			break;

		case 12:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 450;
			if (_globals.SAUVEGARDE->data[svField225]) {
				_globals.NOSPRECRAN = true;
				_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1);
			} else {
				BOOM();
			}
			break;

		case 13:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1);
			break;

		case 14:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1);
			break;

		case 15:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29);
			_globals.NOSPRECRAN = false;
			break;

		case 16:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			if (_globals.SAUVEGARDE->data[svField113] == 1) {
				_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7);
			} else if (!_globals.SAUVEGARDE->data[svField113]) {
				_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7);
			}
			break;

		case 17:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			if (_globals.SAUVEGARDE->data[svField117] == 1) {
				_objectsManager.PERSONAGE2("IM17", "IM17A", "ANIM17", "IM17", 11);
			} else if (!_globals.SAUVEGARDE->data[svField117]) {
				_objectsManager.PERSONAGE2("IM17", "IM17", "ANIM17", "IM17", 11);
			}
			if (_globals.SORTIE == 18) {
				_globals.iRegul = 1;
				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
				_graphicsManager.Cls_Pal();
				_soundManager.WSOUND_OFF();
				_soundManager.WSOUND(29);
				_graphicsManager.FADE_LINUX = 2;
				if (_globals.SVGA == 2)
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
				else if (_globals.SVGA == 1)
					_animationManager.playAnim("PURG1.ANM", 12, 18, 50);
				_globals.iRegul = 0;
			}
			break;

		case 18:
			_globals.NOSPRECRAN = true;
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			_objectsManager.PERSONAGE2("IM18", "IM18", "ANIM18", "IM18", 29);
			break;

		case 19:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			if (_globals.SAUVEGARDE->data[svField123])
				_objectsManager.PERSONAGE2("IM19", "IM19A", "ANIM19", "IM19", 6);
			else
				_objectsManager.PERSONAGE2("IM19", "IM19", "ANIM19", "IM19", 6);
			break;

		case 20:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 10;
			_globals.Max_Propre_Gen = 8;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM20", "IM20", "ANIM20", "IM20", 6);
			if (_globals.SORTIE == 17) {
				_globals.iRegul = 1;
				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
				_graphicsManager.Cls_Pal();
				_soundManager.WSOUND_OFF();
				_soundManager.WSOUND(6);
				_graphicsManager.FADE_LINUX = 2;
				if (_globals.SVGA == 2)
					_animationManager.playAnim("PURG2A.ANM", 12, 18, 50);
				else if (_globals.SVGA == 1)
					_animationManager.playAnim("PURG2.ANM", 12, 18, 50);
				_globals.iRegul = 0;
			}
			break;

		case 22:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM22", "IM22", "ANIM22", "IM22", 6);
			break;

		case 23:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM23", "IM23", "ANIM23", "IM23", 6);
			break;

		case 24:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			if (_globals.SAUVEGARDE->data[svField181]) {
				if (_globals.SAUVEGARDE->data[svField181] == 1)
					_objectsManager.PERSONAGE2("IM24", "IM24a", "ANIM24", "IM24", 1);
			} else {
				_objectsManager.PERSONAGE2("IM24", "IM24", "ANIM24", "IM24", 1);
			}
			break;

		case 25:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30);
			break;

		case 26:
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30);
			break;

		case 27:
			_globals.Max_Propre = 10;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			if (_globals.SAUVEGARDE->data[svField177] == 1) {
				_objectsManager.PERSONAGE2("IM27", "IM27A", "ANIM27", "IM27", 27);
			} else if (!_globals.SAUVEGARDE->data[svField177]) {
				_objectsManager.PERSONAGE2("IM27", "IM27", "ANIM27", "IM27", 27);
			}
			break;

		case 28:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 450;
			_globals.NOSPRECRAN = true;
			if (_globals.SAUVEGARDE->data[svField166] != 1 || _globals.SAUVEGARDE->data[svField167] != 1)
				_objectsManager.PERSONAGE2("IM28", "IM28", "ANIM28", "IM28", 1);
			else
				_objectsManager.PERSONAGE2("IM28a", "IM28", "ANIM28", "IM28", 1);
			break;

		case 29:
			_globals.Max_Propre = 60;
			_globals.Max_Ligne_Long = 50;
			_globals.Max_Propre_Gen = 50;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM29", "IM29", "ANIM29", "IM29", 1);
			break;

		case 30:
			_globals.Max_Propre = 10;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM30", "IM30", "ANIM30", "IM30", 24);
			_globals.NOSPRECRAN = false;
			break;

		case 31:
			_objectsManager.PERSONAGE("IM31", "IM31", "ANIM31", "IM31", 10);
			break;

		case 32:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 20;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM32", "IM32", "ANIM32", "IM32", 2);
			break;

		case 33:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8);
			_globals.NOSPRECRAN = false;
			break;

		case 34:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM34", "IM34", "ANIM34", "IM34", 2);
			_globals.NOSPRECRAN = false;
			break;

		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41: {
			_globals.fmusic = 13;
			_globals.Max_Propre = 50;
			_globals.Max_Ligne_Long = 40;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_globals.DESACTIVE_INVENT = false;
			_globals.FORET = true;
			_globals.NOSPRECRAN = true;
			Common::String im = Common::String::format("IM%d", _globals.SORTIE);
			_soundManager.WSOUND(13);
			if (_globals.FORETSPR == g_PTRNUL) {
				_fileManager.constructFilename(_globals.HOPSYSTEM, "HOPDEG.SPR");
				_globals.FORETSPR = _objectsManager.CHARGE_SPRITE(_globals.NFICHIER);
				_soundManager.CHARGE_SAMPLE(1, "SOUND41.WAV");
			}
			_objectsManager.PERSONAGE2(im, im, "BANDIT", im, 13);
			_globals.NOSPRECRAN = false;
			if (_globals.SORTIE < 35 || _globals.SORTIE > 49) {
				_globals.dos_free2(_globals.FORETSPR);
				_globals.FORETSPR = g_PTRNUL;
				_globals.FORET = false;
				_soundManager.DEL_SAMPLE(1);
			}
			break;
			}

		case 50:
			AVION();
			_globals.SORTIE = 51;
			break;

		case 51:
			_globals.Max_Propre = 20;
			_globals.Max_Ligne_Long = 10;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM51", "IM51", "ANIM51", "IM51", 14);
			break;

		case 52:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			_objectsManager.PERSONAGE2("IM52", "IM52", "ANIM52", "IM52", 14);
			break;

		case 54:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM54", "IM54", "ANIM54", "IM54", 14);
			break;

		case 55:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 460;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM55", "IM55", "ANIM55", "IM55", 14);
			break;

		case 56:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM56", "IM56", "ANIM56", "IM56", 14);
			break;

		case 57:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM57", "IM57", "ANIM57", "IM57", 14);
			break;

		case 58:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM58", "IM58", "ANIM58", "IM58", 14);
			break;

		case 59:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM59", "IM59", "ANIM59", "IM59", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 60:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 440;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM60", "IM60", "ANIM60", "IM60", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 61:
			if (_globals.SAUVEGARDE->data[svField311] == 1 && !_globals.SAUVEGARDE->data[svField312])
				INCENDIE();
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM61", "IM61", "ANIM61", "IM61", 21);
			break;

		case 62:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM62", "IM62", NULL, "IM62", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 63:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM63", "IM63", "ANIM63", "IM63", 21);
			_globals.NOSPRECRAN = false;
			break;
										
		case 64:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM64", "IM64", "ANIM64", "IM64", 21);
			break;
		
		case 65:
			_globals.Max_Propre = 40;
			_globals.Max_Ligne_Long = 30;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM65", "IM65", "ANIM65", "IM65", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 66:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM66", "IM66", "ANIM66", "IM66", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 67:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM67", "IM67", NULL, "IM67", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 68:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM68", "IM68", "ANIM68", "IM68", 21);
			break;

		case 69:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM69", "IM69", "ANIM69", "IM69", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 70:
			_globals.Max_Propre = 8;
			_globals.Max_Ligne_Long = 8;
			_globals.Max_Propre_Gen = 20;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM70", "IM70", NULL, "IM70", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 71:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 445;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM71", "IM71", "ANIM71", "IM71", 21);
			_globals.NOSPRECRAN = false;
			break;

		case 73:
			_globals.Max_Propre = 15;
			_globals.Max_Ligne_Long = 15;
			_globals.Max_Propre_Gen = 10;
			_globals.Max_Perso_Y = 445;
			if (_globals.SAUVEGARDE->data[svField318] == 1) {
				_objectsManager.PERSONAGE2("IM73", "IM73A", "ANIM73", "IM73", 21);
			} else if (!_globals.SAUVEGARDE->data[svField318]) {
				_objectsManager.PERSONAGE2("IM73", "IM73", "ANIM73", "IM73", 21);
			}
			break;

		case 75:
			BASE();
			break;

		case 77:
			OCEAN(77, "OCEAN01", "OCEAN1", 3, 0, 84, 0, 0, 25);
			break;

		case 78:
			OCEAN(78, "OCEAN02", "OCEAN1", 1, 0, 91, 84, 0, 25);
			break;

		case 79:
			OCEAN(79, "OCEAN03", "OCEAN1", 7, 87, 0, 0, 83, 25);
			break;

		case 80:
			OCEAN(80, "OCEAN04", "OCEAN1", 1, 86, 88, 0, 81, 25);
			break;

		case 81:
			OCEAN(81, "OCEAN05", "OCEAN1", 1, 91, 82, 80, 85, 25);
			break;

		case 82:
			OCEAN(82, "OCEAN06", "OCEAN1", 7, 81, 0, 88, 0, 25);
			break;

		case 83:
			OCEAN(83, "OCEAN07", "OCEAN1", 1, 89, 0, 79, 88, 25);
			break;

		case 84:
			OCEAN(84, "OCEAN08", "OCEAN1", 1, 77, 0, 0, 78, 25);
			break;

		case 85:
			OCEAN(85, "OCEAN09", "OCEAN1", 1, 0, 0, 81, 0, 25);
			break;

		case 86:
			OCEAN(86, "OCEAN10", "OCEAN1", 1, 0, 80, 0, 91, 25);
			break;

		case 87:
			OCEAN(87, "OCEAN11", "OCEAN1", 3, 0, 79, 90, 0, 25);
			break;

		case 88:
			OCEAN(88, "OCEAN12", "OCEAN1", 1, 80, 0, 83, 82, 25);
			break;

		case 89:
			OCEAN(89, "OCEAN13", "OCEAN1", 3, 0, 83, 0, 0, 25);
			break;

		case 91:
			OCEAN(91, "OCEAN15", "OCEAN1", 3, 78, 81, 86, 0, 25);
			break;

		case 90:
			BASED();
			break;

		case 93:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 445;
			if (_globals.SAUVEGARDE->data[svField330])
				_objectsManager.PERSONAGE2("IM93", "IM93c", "ANIM93", "IM93", 29);
			else
				_objectsManager.PERSONAGE2("IM93", "IM93", "ANIM93", "IM93", 29);
			break;

		case 94:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 440;
			_objectsManager.PERSONAGE2("IM94", "IM94", "ANIM94", "IM94", 19);
			break;

		case 95:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM95", "IM95", "ANIM95", "IM95", 19);
			break;

		case 96:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM96", "IM96", "ANIM96", "IM96", 19);
			break;

		case 97:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 435;
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE2("IM97", "IM97", "ANIM97", "IM97", 19);
			if (_globals.SORTIE == 18) {
				_globals.iRegul = 1;
				_soundManager.WSOUND_OFF();
				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
				_graphicsManager.Cls_Pal();
				_soundManager.WSOUND(6);
				if (_globals.SVGA == 2)
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
				else if (_globals.SVGA == 1)
					_animationManager.playAnim("PURG1.ANM", 12, 18, 50);
				_graphicsManager.FADE_OUTS();
				_globals.iRegul = 0;
			}
			break;

		case 98:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM98", "IM98", "ANIM98", "IM98", 19);
			break;

		case 99:
			_globals.Max_Propre = 5;
			_globals.Max_Ligne_Long = 5;
			_globals.Max_Propre_Gen = 5;
			_globals.Max_Perso_Y = 435;
			_objectsManager.PERSONAGE2("IM99", "IM99", "ANIM99", "IM99", 19);
			break;

		case 100:
			JOUE_FIN();
			break;

		case 111:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 112:
			_globals.NOSPRECRAN = true;
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10);
			_globals.NOSPRECRAN = false;
			break;

		case 113:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 113;
			_globals.SAUVEGARDE->data[svField5] = 113;
			_computerManager.showComputer(COMPUTER_HOPKINS);
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.DD_VBL();
			memset(_graphicsManager.VESA_BUFFER, 0, 0x4B000u);
			memset(_graphicsManager.VESA_SCREEN, 0, 0x4B000u);
			_graphicsManager.Cls_Pal();
			_graphicsManager.RESET_SEGMENT_VESA();
			break;

		case 114:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 114;
			_globals.SAUVEGARDE->data[svField5] = 114;
			_computerManager.showComputer(COMPUTER_SAMANTHAS);
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			break;

		case 115:
			_globals.SORTIE = 0;
			_globals.OLD_ECRAN = _globals.ECRAN;
			_globals.SAUVEGARDE->data[svField6] = _globals.ECRAN;
			_globals.ECRAN = 115;
			_globals.SAUVEGARDE->data[svField5] = 115;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			break;

		case 150:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.Cls_Pal();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR1A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 151:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.Cls_Pal();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR3A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 152:
			_soundManager.WSOUND(16);
			_globals.iRegul = 1;
			_graphicsManager.DD_Lock();
			_graphicsManager.Cls_Video();
			_graphicsManager.DD_Unlock();
			_graphicsManager.Cls_Pal();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR4A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals.SORTIE = 300;
			break;

		case 194:
		case 195:
		case 196:
		case 197:
		case 198:
		case 199:
			_globals.PERSO = _globals.dos_free2(_globals.PERSO);
			_globals.iRegul = 1;
			_soundManager.WSOUND(23);
			_globals.SORTIE = PWBASE();
			_soundManager.WSOUND_OFF();
			_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
			_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
			_globals.PERSO_TYPE = 0;
			_globals.iRegul = 0;
			_graphicsManager.nbrligne = SCREEN_WIDTH;
			break;
		}
	}
	return true;
}

bool HopkinsEngine::shouldQuit() const {
	return g_system->getEventManager()->shouldQuit();
}

int HopkinsEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void HopkinsEngine::processIniParams(Common::StringMap &iniParams) {
	_globals.XFULLSCREEN = iniParams["FULLSCREEN"] == "YES";

	_globals.XSETMODE = 1;
	if (iniParams.contains("SETMODE")) {
		int setMode = atoi(iniParams["SETMODE"].c_str());
		_globals.XSETMODE = CLIP(setMode, 1, 5);
	}

	_globals.XZOOM = 0;
	if (_globals.XSETMODE == 5 && iniParams.contains("ZOOM")) {
		int zoom = atoi(iniParams["ZOOM"].c_str());
		_globals.XZOOM = CLIP(zoom, 25, 100);
	}

	_globals.XFORCE16 = iniParams["FORCE16BITS"] == "YES";
	_globals.XFORCE8 = iniParams["FORCE8BITS"] == "YES";
}

void HopkinsEngine::INIT_SYSTEM() {
	// Set graphics mode
	_graphicsManager.SET_MODE(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Synchronise the sound settings from ScummVM
	_soundManager.syncSoundSettings();

	if (getPlatform() == Common::kPlatformLinux)
		_eventsManager.mouse_linux = true;
	else
		_eventsManager.mouse_linux = false;

	switch (_globals.FR) {
	case 0:
		if (!_eventsManager.mouse_linux)
			_fileManager.constructFilename(_globals.HOPSYSTEM, "SOUAN.SPR");
		else 
			_fileManager.constructFilename(_globals.HOPSYSTEM, "LSOUAN.SPR");
		break;
	case 1:
		if (!_eventsManager.mouse_linux)
			_fileManager.constructFilename(_globals.HOPSYSTEM, "SOUFR.SPR");
		else
			_fileManager.constructFilename(_globals.HOPSYSTEM, "LSOUFR.SPR");
		break;
	case 2:
		_fileManager.constructFilename(_globals.HOPSYSTEM, "SOUES.SPR");
		break;
	}
  
	if (_eventsManager.mouse_linux) {
		_eventsManager.souris_sizex = 52;
		_eventsManager.souris_sizey = 32;
	} else {
		_eventsManager.souris_sizex = 34;
		_eventsManager.souris_sizey = 20;
	}
	_eventsManager.pointeur_souris = _fileManager.loadFile(_globals.NFICHIER);

	_globals.clearAll();

	_fileManager.constructFilename(_globals.HOPSYSTEM, "FONTE3.SPR");
	_globals.police = _fileManager.loadFile(_globals.NFICHIER);
	_globals.police_l = 12;
	_globals.police_h = 21;
	_fileManager.constructFilename(_globals.HOPSYSTEM, "ICONE.SPR");
	_globals.ICONE = _fileManager.loadFile(_globals.NFICHIER);
	_fileManager.constructFilename(_globals.HOPSYSTEM, "TETE.SPR");
	_globals.TETE = _fileManager.loadFile(_globals.NFICHIER);
	
	switch (_globals.FR) {
	case 0:
		_fileManager.constructFilename(_globals.HOPLINK, "ZONEAN.TXT");
		_globals.BUF_ZONE = _fileManager.loadFile(_globals.NFICHIER);
		break;
	case 1:
		_fileManager.constructFilename(_globals.HOPLINK, "ZONE01.TXT");
		_globals.BUF_ZONE = _fileManager.loadFile(_globals.NFICHIER);
		break;
	case 2:
		_fileManager.constructFilename(_globals.HOPLINK, "ZONEES.TXT");
		_globals.BUF_ZONE = _fileManager.loadFile(_globals.NFICHIER);
		break;
	}

	_eventsManager.INSTALL_SOURIS();
	_eventsManager.souris_on();
	_eventsManager.souris_flag = false;
	_eventsManager.souris_max();

	_globals.HOPKINS_DATA();

	_eventsManager.ofset_souris_x = 0;
	_eventsManager.ofset_souris_y = 0;
}

void HopkinsEngine::INTRORUN() {
	// Win95 EN demo doesn't include the intro
	if ((getLanguage() == Common::EN_ANY) && (getPlatform() == Common::kPlatformWindows) && (getIsDemo()))
		return;

	byte paletteData[PALETTE_EXT_BLOCK_SIZE];
	byte paletteData2[PALETTE_EXT_BLOCK_SIZE];

	memset(&paletteData, 0, PALETTE_EXT_BLOCK_SIZE);
	_eventsManager.VBL();
	_eventsManager.souris_flag = false;
	_globals.iRegul = 1;
	_eventsManager.VBL();
	_soundManager.WSOUND(16);
	_animationManager._clearAnimationFl = true;
	_animationManager.playAnim("J1.anm", 12, 12, 50);
	if (!_eventsManager.ESC_KEY) {
		_soundManager.VOICE_MIX(1, 3);
		_animationManager.playAnim("J2.anm", 12, 12, 50);

		if (!_eventsManager.ESC_KEY) {
			_soundManager.VOICE_MIX(2, 3);
			_animationManager.playAnim("J3.anm", 12, 12, 50);

			if (!_eventsManager.ESC_KEY) {
				_soundManager.VOICE_MIX(3, 3);
				_graphicsManager.DD_Lock();
				_graphicsManager.Cls_Video();
				_graphicsManager.DD_Unlock();
				_graphicsManager.Cls_Pal();
				_graphicsManager.DD_VBL();
				_soundManager.WSOUND(11);
				_graphicsManager.LOAD_IMAGE("intro1");
				_graphicsManager.SCROLL_ECRAN(0);
				_graphicsManager.ofscroll = 0;
				_graphicsManager.SETCOLOR3(252, 100, 100, 100);
				_graphicsManager.SETCOLOR3(253, 100, 100, 100);
				_graphicsManager.SETCOLOR3(251, 100, 100, 100);
				_graphicsManager.SETCOLOR3(254, 0, 0, 0);
				_globals.BPP_NOAFF = true;
				for (int i = 0; i <= 4; i++)
					_eventsManager.VBL();

				_globals.BPP_NOAFF = false;
				_globals.iRegul = 1;
				_graphicsManager.FADE_INW();
				if (_graphicsManager.DOUBLE_ECRAN == true) {
					_graphicsManager.no_scroll = 2;
					bool v3 = false;
					_graphicsManager.SCROLL = 0;
          
					do {
						_graphicsManager.SCROLL += 2;
						if (_graphicsManager.SCROLL > (SCREEN_WIDTH - 2)) {
							_graphicsManager.SCROLL = SCREEN_WIDTH;
							v3 = true;
						}
            
						if (_eventsManager.XMOUSE() < _graphicsManager.SCROLL + 10)
							_eventsManager.souris_xy(_eventsManager.souris_x + 4, _eventsManager.YMOUSE());
						_eventsManager.VBL();
					} while (!shouldQuit() && !v3 && _graphicsManager.SCROLL != SCREEN_WIDTH);
          
					_eventsManager.VBL();
					_graphicsManager.no_scroll = 0;

					if (shouldQuit())
						return;
				}
        
				_soundManager.VOICE_MIX(4, 3);
				_graphicsManager.FADE_OUTW();
				_graphicsManager.no_scroll = 0;
				_graphicsManager.LOAD_IMAGE("intro2");
				_graphicsManager.SCROLL_ECRAN(0);
				_animationManager.loadAnim("INTRO2");
				_graphicsManager.VISU_ALL();
				_soundManager.WSOUND(23);
				_objectsManager.BOBANIM_OFF(3);
				_objectsManager.BOBANIM_OFF(5);
				_graphicsManager.ofscroll = 0;
				_graphicsManager.SETCOLOR3(252, 100, 100, 100);
				_graphicsManager.SETCOLOR3(253, 100, 100, 100);
				_graphicsManager.SETCOLOR3(251, 100, 100, 100);
				_graphicsManager.SETCOLOR3(254, 0, 0, 0);
				_globals.BPP_NOAFF = true;

				for (int i = 0; i <= 4; i++)
					_eventsManager.VBL();
        
				_globals.BPP_NOAFF = false;
				_globals.iRegul = 1;
				_graphicsManager.FADE_INW();
				for (uint i = 0; i < 200 / _globals.vitesse; ++i)
					_eventsManager.VBL();
        
				_objectsManager.BOBANIM_ON(3);
				_soundManager.VOICE_MIX(5, 3);
				_objectsManager.BOBANIM_OFF(3);
				_eventsManager.VBL();
				memcpy(&paletteData2, _graphicsManager.Palette, 796);
				
				// CHECKME: Useless variables?
				// v21 = *(uint16 *)&_graphicsManager.Palette[796];
				// v22 = _graphicsManager.Palette[798];
				_graphicsManager.setpal_vga256_linux(paletteData, _graphicsManager.VESA_BUFFER);
				_graphicsManager.FIN_VISU();

				if (shouldQuit())
					return;

				_soundManager.SPECIAL_SOUND = 5;
				_graphicsManager.FADE_LINUX = 2;
				_animationManager.playAnim("ELEC.ANM", 10, 26, 200);
				if (shouldQuit())
					return;

				_soundManager.SPECIAL_SOUND = 0;
        
				if (!_eventsManager.ESC_KEY) {
					_graphicsManager.LOAD_IMAGE("intro2");
					_graphicsManager.SCROLL_ECRAN(0);
					_animationManager.loadAnim("INTRO2");
					_graphicsManager.VISU_ALL();
					_soundManager.WSOUND(23);
					_objectsManager.BOBANIM_OFF(3);
					_objectsManager.BOBANIM_OFF(5);
					_objectsManager.BOBANIM_OFF(1);
					_graphicsManager.ofscroll = 0;
					_graphicsManager.SETCOLOR3(252, 100, 100, 100);
					_graphicsManager.SETCOLOR3(253, 100, 100, 100);
					_graphicsManager.SETCOLOR3(251, 100, 100, 100);
					_graphicsManager.SETCOLOR3(254, 0, 0, 0);
					_globals.BPP_NOAFF = true;

					for (int i = 0; i <= 3; i++)
						_eventsManager.VBL();
          
					_globals.BPP_NOAFF = false;
					_globals.iRegul = 1;
					_graphicsManager.setpal_vga256_linux(paletteData2, _graphicsManager.VESA_BUFFER);

					int v9 = 0;
					while (!shouldQuit() && !_eventsManager.ESC_KEY) {
						if (v9 == 12) {
							_objectsManager.BOBANIM_ON(3);
							_eventsManager.VBL();
							_soundManager.VOICE_MIX(6, 3);
							_eventsManager.VBL();
							_objectsManager.BOBANIM_OFF(3);
						}
            
						Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager.Palette[0]);
						
						

						for (int i = 1, v12 = 4 * v9; i <= PALETTE_BLOCK_SIZE; i++) {
							if (_graphicsManager.Palette[i] > v12)
								_graphicsManager.Palette[i] -= v12;
						}

						_graphicsManager.setpal_vga256_linux(_graphicsManager.Palette, _graphicsManager.VESA_BUFFER);


						if (2 * v9 > 1) {
							for (int i = 1; i < 2 * v9; i++)
								_eventsManager.VBL();
						} 
						
						_graphicsManager.setpal_vga256_linux(paletteData2, _graphicsManager.VESA_BUFFER);
						if (20 - v9 > 1) {              
							for (int i = 1; i < 20 - v9; i++)
								_eventsManager.VBL();
						}
            
						v9 += 2;
						if (v9 > 15) {
							_graphicsManager.setpal_vga256_linux(paletteData, _graphicsManager.VESA_BUFFER);
							for (uint j = 1; j < 100 / _globals.vitesse; ++j)
								_eventsManager.VBL();
              
							_objectsManager.BOBANIM_ON(3);
							_soundManager.VOICE_MIX(7, 3);
							_objectsManager.BOBANIM_OFF(3);
							
							for (uint k = 1; k < 60 / _globals.vitesse; ++k)
								_eventsManager.VBL();
							_objectsManager.BOBANIM_ON(5);
							for (uint l = 0; l < 20 / _globals.vitesse; ++l)
								_eventsManager.VBL();

							Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager.Palette[0]);
							_graphicsManager.setpal_vga256_linux(_graphicsManager.Palette, _graphicsManager.VESA_BUFFER);
              
							for (uint m = 0; m < 50 / _globals.vitesse; ++m) {
								if (m == 30 / _globals.vitesse) {
									_objectsManager.BOBANIM_ON(3);
									_soundManager.VOICE_MIX(8, 3);
									_objectsManager.BOBANIM_OFF(3);
								}
                
								_eventsManager.VBL();
							}

							_graphicsManager.FADE_OUTW();
							_graphicsManager.FIN_VISU();
							_animationManager._clearAnimationFl = true;
							_soundManager.WSOUND(3);
							_soundManager.SPECIAL_SOUND = 1;
							_animationManager.playAnim("INTRO1.anm", 10, 24, 18);
							if (shouldQuit())
								return;

							_soundManager.SPECIAL_SOUND = 0;

							if (!_eventsManager.ESC_KEY) {
								_animationManager.playAnim("INTRO2.anm", 10, 24, 18);
								if (shouldQuit())
									return;
                
								if (!_eventsManager.ESC_KEY) {
									_animationManager.playAnim("INTRO3.anm", 10, 24, 200);
									if (shouldQuit())
										return;

									if (!_eventsManager.ESC_KEY) {
										_animationManager._clearAnimationFl = false;
										_graphicsManager.FADE_LINUX = 2;
										_animationManager.playAnim("J4.anm", 12, 12, 1000);
									}
								}
							}
							break;
						}
					}
				}
			}
		}
	}
  
	_eventsManager.ESC_KEY = false;
}

/** 
 * If in demo, displays a 'not available' screen and returns to the city map
 */
void HopkinsEngine::PASS() {
	if (!getIsDemo())
		return;

	if (_globals.FR == 1)
		_graphicsManager.LOAD_IMAGE("ndfr");
	else
		_graphicsManager.LOAD_IMAGE("nduk");
	  
	_graphicsManager.FADE_INW();
	if (_soundManager.VOICEOFF)
		_eventsManager.delay(500);
	else
		_soundManager.VOICE_MIX(628, 4);
		
	_graphicsManager.FADE_OUTW();
	_globals.SORTIE = 4;
}

void HopkinsEngine::NO_DISPO(int sortie) {
	// Use the code of the linux demo instead of the code of the Windows demo.
	// The behavior is somewhat better, and common code is easier to maintain.
	PASS();
	_globals.SORTIE = sortie;
}

void HopkinsEngine::ENDEMO() {
	_soundManager.WSOUND(28);
	if (_globals.FR == 1)
		_graphicsManager.LOAD_IMAGE("endfr");
	else
	    _graphicsManager.LOAD_IMAGE("enduk");
  
	_graphicsManager.FADE_INW();
	_eventsManager.delay(1500);
	_graphicsManager.FADE_OUTW();
	_globals.SORTIE = 0;
}

void HopkinsEngine::BOOM() {
	_graphicsManager.nbrligne = SCREEN_WIDTH;
	_graphicsManager.SCANLINE(SCREEN_WIDTH);
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();
	
	_globals.iRegul = 1;
	_soundManager.SPECIAL_SOUND = 199;
	_graphicsManager.FADE_LINUX = 2;
	if (_globals.SVGA == 1)
		_animationManager.playAnim("BOMBE2.ANM", 50, 14, 500);
	else if (_globals.SVGA == 2)
		_animationManager.playAnim("BOMBE2A.ANM", 50, 14, 500);
	
	_soundManager.SPECIAL_SOUND = 0;
	_graphicsManager.LOAD_IMAGE("IM15");
	_animationManager.loadAnim("ANIM15");
	_graphicsManager.VISU_ALL();
	_objectsManager.BOBANIM_OFF(7);
	_globals.BPP_NOAFF = true;

	for (int idx = 0; idx < 5; ++idx) {
		_eventsManager.VBL();
	}
  
	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_eventsManager.MOUSE_OFF();
	
	for (int idx = 0; idx < 20; ++idx) {
		_eventsManager.VBL();
	}
  
	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO2("vire.pe2");
	_globals.NOPARLE = false;
	_objectsManager.BOBANIM_ON(7);

	for (int idx = 0; idx < 100; ++idx) {
		_eventsManager.VBL();
	}

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	_globals.iRegul = 0;
	_globals.SORTIE = 151;
}

void HopkinsEngine::REST_SYSTEM() {
	quitGame();
	_eventsManager.CONTROLE_MES();
}

void HopkinsEngine::PUBQUIT() {
	_globals.PUBEXIT = true;
	_graphicsManager.RESET_SEGMENT_VESA();
	_globals.FORET = false;
	_eventsManager.CASSE = false;
	_globals.DESACTIVE_INVENT = true;
	_globals.FLAG_VISIBLE = false;
	_graphicsManager.LOAD_IMAGE("BOX");
	_soundManager.WSOUND(28);
	_graphicsManager.FADE_INW();
	_eventsManager.MOUSE_ON();
	_eventsManager.CHANGE_MOUSE(0);
	_eventsManager.btsouris = 0;
	_eventsManager.souris_n = 0;
	_globals.netscape = true;

	bool mouseClicked = false;

	// CHECKME: Useless variables ?
	// int xp, yp;
	do {
//		xp = _eventsManager.XMOUSE();
//		yp = _eventsManager.YMOUSE();
		_eventsManager.VBL();
		
		if (_eventsManager.BMOUSE() == 1)
			mouseClicked = true;
	} while (!mouseClicked && !g_system->getEventManager()->shouldQuit());

	// Original tried to open a web browser link here. Since ScummVM doesn't support
	// that, it's being skipped in favour of simply exitting

	_graphicsManager.FADE_OUTW();
}

void HopkinsEngine::INCENDIE() {
	_globals.DESACTIVE_INVENT = true;
	_globals.iRegul = 1;
	_graphicsManager.LOAD_IMAGE("IM71");
	_animationManager.loadAnim("ANIM71");
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_graphicsManager.VISU_ALL();
	_globals.BPP_NOAFF = true;

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.VBL();

	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_globals.iRegul = 1;

	for (int cpt = 0; cpt <= 249; cpt++)
		_eventsManager.VBL();

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("SVGARD1.pe2");
	_globals.NOPARLE = false;

	for (int cpt = 0; cpt <= 49; cpt++)
		_eventsManager.VBL();

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	_globals.SAUVEGARDE->data[svField312] = 1;
	_globals.DESACTIVE_INVENT = false;
}

void HopkinsEngine::BASE() {
	_globals.iRegul = 1;
	_graphicsManager.nbrligne = SCREEN_WIDTH;
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();
	_animationManager._clearAnimationFl = true;
	_soundManager.WSOUND(25);
	if (_globals.SVGA == 1) {
		_animationManager.playAnim("base00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base05.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base30.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base40.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base50.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC05.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("OC30.anm", 10, 18, 18);
		}
	} else if (_globals.SVGA == 2) {
		_animationManager.playAnim("base00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base05a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base20a.anm", 10, 18, 18);
		// CHECKME: The original code was doing the opposite test, which looks like a bug.
		if (!_eventsManager.ESC_KEY) 
			_animationManager.playAnim("base30a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base40a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("base50a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC05a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("OC20a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("OC30a.anm", 10, 18, 18);
		}
	}

	_eventsManager.ESC_KEY = false;
	_animationManager._clearAnimationFl = false;
	_globals.SORTIE = 85;
}

void HopkinsEngine::BASED() {
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();
	_animationManager.NO_SEQ = false;
	_soundManager.WSOUND(26);
	_globals.iRegul = 1;
	_globals.DESACTIVE_INVENT = true;
	_animationManager.NO_COUL = true;
	_graphicsManager.FADE_LINUX = 2;
	_animationManager.playSequence("abase.seq", 50, 15, 50);
	_animationManager.NO_COUL = false;
	_graphicsManager.LOAD_IMAGE("IM92");
	_animationManager.loadAnim("ANIM92");
	_graphicsManager.VISU_ALL();
	_objectsManager.INILINK("IM92");
	_globals.BPP_NOAFF = true;

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.VBL();

	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_globals.CACHE_ON();

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(8) != 22);

	_graphicsManager.FADE_OUTW();
	_graphicsManager.FIN_VISU();
	_globals.RESET_CACHE();
	_globals.DESACTIVE_INVENT = false;
	_globals.SORTIE = 93;
	_globals.iRegul = 0;
}

void HopkinsEngine::JOUE_FIN() {
	_globals.PERSO = _globals.dos_free2(_globals.PERSO);
	_dialogsManager.VIRE_INVENT = true;
	_globals.DESACTIVE_INVENT = true;
	_graphicsManager.ofscroll = 0;
	_globals.PLAN_FLAG = false;
	_globals.iRegul = 1;
	_soundManager.WSOUND(26);
	_globals.chemin = (int16 *)g_PTRNUL;
	_globals.NOMARCHE = true;
	_globals.SORTIE = 0;
	_globals.AFFLI = false;
	_globals.AFFIVBL = false;
	_soundManager.CHARGE_SAMPLE(1, "SOUND90.WAV");
	_graphicsManager.LOAD_IMAGE("IM100");
	_animationManager.loadAnim("ANIM100");
	_graphicsManager.VISU_ALL();
	_eventsManager.MOUSE_ON();
	_objectsManager.BOBANIM_OFF(7);
	_objectsManager.BOBANIM_OFF(8);
	_objectsManager.BOBANIM_OFF(9);
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_eventsManager.CHANGE_MOUSE(0);
	_globals.BPP_NOAFF = true;

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.VBL();

	_globals.BPP_NOAFF = false;
	_graphicsManager.FADE_INW();
	_globals.iRegul = 1;

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(6) != 54);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("GM4.PE2");
	_globals.DESACTIVE_INVENT = true;
	_objectsManager.BOBANIM_OFF(6);
	_objectsManager.BOBANIM_OFF(10);
	_objectsManager.BOBANIM_ON(9);
	_objectsManager.BOBANIM_ON(7);

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 54);

	_soundManager.PLAY_SAMPLE2(1);

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 65);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("DUELB4.PE2");
	_eventsManager.MOUSE_OFF();
	_globals.DESACTIVE_INVENT = true;

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 72);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("DUELH1.PE2");

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 81);

	_globals.NOPARLE = true;
	_talkManager.PARLER_PERSO("DUELB5.PE2");

	do
		_eventsManager.VBL();
	while (_objectsManager.BOBPOSI(7) != 120);

	_objectsManager.BOBANIM_OFF(7);
	if (_globals.SAUVEGARDE->data[svField135] == 1) {
		_soundManager.SPECIAL_SOUND = 200;
		_soundManager.VBL_MERDE = true;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("BERM.ANM", 100, 24, 300);
		_graphicsManager.FIN_VISU();
		_soundManager.DEL_SAMPLE(1);
		_graphicsManager.LOAD_IMAGE("PLAN3");
		_graphicsManager.FADE_INW();

		_eventsManager.lItCounter = 0;
		if (!_eventsManager.ESC_KEY) {
			do
				_eventsManager.CONTROLE_MES();
			while (_eventsManager.lItCounter < 2000 / _globals.vitesse && !_eventsManager.ESC_KEY);
		}
		_eventsManager.ESC_KEY = false;
		_graphicsManager.FADE_OUTW();
		_globals.iRegul = 1;
		_soundManager.SPECIAL_SOUND = 0;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("JOUR2A.anm", 12, 12, 1000);
		_soundManager.WSOUND(11);
		_graphicsManager.DD_Lock();
		_graphicsManager.Cls_Video();
		_graphicsManager.DD_Unlock();
		_graphicsManager.Cls_Pal();
		_animationManager.playAnim("FF1a.anm", 18, 18, 9);
		_animationManager.playAnim("FF1a.anm", 9, 18, 9);
		_animationManager.playAnim("FF1a.anm", 9, 18, 18);
		_animationManager.playAnim("FF1a.anm", 9, 18, 9);
		_animationManager.playAnim("FF2a.anm", 24, 24, 100);
		Credits();
		_globals.iRegul = 0;
		_globals.SORTIE = 300;
		_dialogsManager.VIRE_INVENT = false;
		_globals.DESACTIVE_INVENT = false;
	} else {
		_soundManager.SPECIAL_SOUND = 200;
		_soundManager.VBL_MERDE = true;
		_animationManager.playAnim2("BERM.ANM", 100, 24, 300);
		_objectsManager.BOBANIM_OFF(7);
		_objectsManager.BOBANIM_ON(8);
		_globals.NOPARLE = true;
		_talkManager.PARLER_PERSO("GM5.PE2");
		_globals.DESACTIVE_INVENT = true;

		do
			_eventsManager.VBL();
		while (_objectsManager.BOBPOSI(8) != 5);

		_soundManager.PLAY_SOUND2("SOUND41.WAV");

		do
			_eventsManager.VBL();
		while (_objectsManager.BOBPOSI(8) != 21);

		_graphicsManager.FADE_OUTW();
		_graphicsManager.FIN_VISU();
		_soundManager.DEL_SAMPLE(1);
		_soundManager.WSOUND(16);
		_globals.iRegul = 1;
		_soundManager.SPECIAL_SOUND = 0;
		_dialogsManager.VIRE_INVENT = false;
		_globals.DESACTIVE_INVENT = false;
		_animationManager.playAnim("JOUR4A.anm", 12, 12, 1000);
		_globals.iRegul = 0;
		_globals.SORTIE = 300;
	}
	_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
	_globals.iRegul = 0;
}

void HopkinsEngine::AVION() {
	_soundManager.WSOUND(28);
	_globals.iRegul = 1;
	_globals.nbrligne = SCREEN_WIDTH;
	_graphicsManager.DD_Lock();
	_graphicsManager.Cls_Video();
	_graphicsManager.DD_Unlock();
	_graphicsManager.Cls_Pal();

	_animationManager._clearAnimationFl = false;
	if (_globals.SVGA == 1) {
		_animationManager.playAnim("aerop00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop30.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop40.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop50.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop60.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop70.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans00.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans10.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans15.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans20.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans30.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans40.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("PARA00.anm", 9, 9, 9);
		}
	} else if (_globals.SVGA == 2) {
		_animationManager.playAnim("aerop00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("serop10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop20a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop30a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop40a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop50a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop60a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("aerop70a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans00a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans10a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans15a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans20a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans30a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY)
			_animationManager.playAnim("trans40a.anm", 10, 18, 18);
		if (!_eventsManager.ESC_KEY) {
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("PARA00a.anm", 9, 9, 9);
		}
	}

	_eventsManager.ESC_KEY = 0;
	_animationManager._clearAnimationFl = false;
}

int HopkinsEngine::PWBASE() {
	_globals.DESACTIVE_INVENT = true;
	_graphicsManager.LOAD_IMAGE("PBASE");
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_eventsManager.CHANGE_MOUSE(0);
	_graphicsManager.FADE_INW();
	bool loopCond = false;
	int zone;
	do {
		if (shouldQuit())
			return 0;

		int mouseButton = _eventsManager.BMOUSE();
		int posX = _eventsManager.XMOUSE();
		int posY = _eventsManager.YMOUSE();
		zone = 0;
		if ((posX - 181 <= 16) && (posY - 66 <= 22) &&
		    (posX - 181 >= 0) && (posY - 66 >= 0))
			zone = 1;
		if ((posX - 353 <= 22) && (posY - 116 <= 19) &&
		    (posX - 353 >= 0) && (posY - 116 >= 0))
			zone = 2;
		if ((posX - 483 <= 20) && (posY - 250 <= 25) &&
		    (posX - 483 >= 0) && (posY - 250 >= 0))
			zone = 3;
		if ((posX - 471 <= 27) && (posY - 326 <= 20) &&
		    (posX - 471 >= 0) && (posY - 326 >= 0))
			zone = 4;
		if ((posX - 162 <= 21) && (posY - 365 <= 23) &&
		    (posX - 162 >= 0) && (posY - 365 >= 0))
			zone = 5;
		if ((posX - 106 <= 20) && (posY - 267 <= 26) &&
		    (posX - 106 >= 0) && (posY - 267 >= 0))
			zone = 6;
		if (zone) {
			_eventsManager.CHANGE_MOUSE(4);
			_globals.couleur_40 += 25;
			if (_globals.couleur_40 > 100)
				_globals.couleur_40 = 0;
			_graphicsManager.SETCOLOR4(251, _globals.couleur_40, _globals.couleur_40, _globals.couleur_40);
		} else {
			_eventsManager.CHANGE_MOUSE(0);
			_graphicsManager.SETCOLOR4(251, 100, 100, 100);
		}
		_eventsManager.VBL();
		if ((mouseButton == 1) && zone)
			loopCond = true;
	} while (!loopCond);

	_globals.DESACTIVE_INVENT = false;
	_graphicsManager.FADE_OUTW();

	int result;
	switch (zone) {
	case 1:
		result = 94;
		break;
	case 2:
		result = 95;
		break;
	case 3:
		result = 96;
		break;
	case 4:
		result = 97;
		break;
	case 5:
		result = 98;
		break;
	case 6:
		result = 99;
		break;
	default:
		result = 0;
		break;
	}
	return result;
}

int HopkinsEngine::WBASE() {
	warning("STUB: WBASE()");
	return 300;
}

void HopkinsEngine::Charge_Credits() {
	_globals.Credit_y = 440;
	_globals.Credit_l = 10;
	_globals.Credit_h = 40;
	_globals.Credit_step = 45;
	switch (_globals.FR) {
	case 0:
		_fileManager.constructFilename(_globals.HOPLINK, "CREAN.TXT");
		break;
	case 1:
		_fileManager.constructFilename(_globals.HOPLINK, "CREFR.TXT");
		break;
	case 2:
		_fileManager.constructFilename(_globals.HOPLINK, "CREES.TXT");
		break;
	default:
		error("Charge_Credits(): Unhandled language");
		break;
	}

	byte *bufPtr = _fileManager.loadFile(_globals.NFICHIER);
	byte *curPtr = bufPtr;
	int idxLines = 0;
	bool loopCond = false;
	do {
		if (*curPtr == '%') {
			if (curPtr[1] == '%') {
				loopCond = true;
				break;
			}
			_globals.Credit[idxLines]._colour = curPtr[1];
			_globals.Credit[idxLines]._actvFl = true;
			_globals.Credit[idxLines]._linePosY = _globals.Credit_y + idxLines * _globals.Credit_step;
			int idxBuf = 0;
			for (;;) {
				byte curChar = curPtr[idxBuf + 3];
				if (curChar == '%' || curChar == 10)
					break;
				_globals.Credit[idxLines]._line[idxBuf] = curChar;
				idxBuf++;
				if (idxBuf >= 49)
					break;
			}
			_globals.Credit[idxLines]._line[idxBuf] = 0;
			_globals.Credit[idxLines]._lineSize = idxBuf - 1;
			curPtr = curPtr + idxBuf + 2;
			++idxLines;
		} else {
			curPtr++;
		}
		_globals.Credit_lignes = idxLines;
	} while (!loopCond);

/* Useless
	v5 = 0;
	if (_globals.Credit_lignes > 0) {
		do
			++v5;
		while (v5 < _globals.Credit_lignes);
	}
*/
	_globals.dos_free2(bufPtr);
}

void HopkinsEngine::CREDIT_AFFICHE(int startPosY, byte *buffer, char colour) {
	byte *bufPtr = buffer;
	int strWidth = 0;
	byte curChar;
	for (;;) {
		curChar = *bufPtr++;
		if (!curChar)
			break;
		if (curChar > 31)
			strWidth += _objectsManager.getWidth(_globals.police, curChar - 32);
	}
	int startPosX = 320 - strWidth / 2;
	int endPosX = strWidth + startPosX;
	int endPosY = startPosY + 12;
	if ((_globals.Credit_bx == -1) && (_globals.Credit_bx1 == -1) && (_globals.Credit_by == -1) && (_globals.Credit_by1 == -1)) {
		_globals.Credit_bx = startPosX;
		_globals.Credit_bx1 = endPosX;
		_globals.Credit_by = startPosY;
		_globals.Credit_by1 = endPosY;
	}
	if (startPosX < _globals.Credit_bx)
		_globals.Credit_bx = startPosX;
	if (endPosX > _globals.Credit_bx1)
		_globals.Credit_bx1 = endPosX;
	if (_globals.Credit_by > startPosY)
		_globals.Credit_by = startPosY;
	if (endPosY > _globals.Credit_by1)
		_globals.Credit_by1 = endPosY;

	bufPtr = buffer;
	for (;;) {
		curChar = *bufPtr++;
		if (!curChar)
			break;
		if (curChar > 31) {
			_graphicsManager.Affiche_Fonte(_graphicsManager.VESA_BUFFER, _globals.police, startPosX, startPosY, curChar - 32, colour);
			startPosX += _objectsManager.getWidth(_globals.police, curChar - 32);
		}
	}
}

void HopkinsEngine::Credits() {
	Charge_Credits();
	_globals.Credit_y = 436;
	_graphicsManager.LOAD_IMAGE("GENERIC");
	_graphicsManager.FADE_INW();
	_soundManager.WSOUND(28);
	_eventsManager.souris_flag = false;
	_globals.iRegul = 3;
	_globals.Credit_bx = _globals.Credit_bx1 = _globals.Credit_by = _globals.Credit_by1 = -1;
	int soundId = 28;
	do {
		for (int i = 0; i < _globals.Credit_lignes; ++i) {
			if (_globals.Credit[i]._actvFl) {
				int nextY = _globals.Credit_y + i * _globals.Credit_step;
				_globals.Credit[i]._linePosY = nextY;

				if ((nextY - 21  >= 0) && (nextY - 21 <= 418)) {
					int col = 0;
					switch (_globals.Credit[i]._colour) {
					case '1':
						col = 163;
						break;
					case '2':
						col = 161;
						break;
					case '3':
						col = 162;
						break;
					default:
						warning("Unknown colour, default to col #1");
						col = 163;
						break;
					}
					if (_globals.Credit[i]._lineSize != -1)
						CREDIT_AFFICHE(nextY, _globals.Credit[i]._line, col);
				}
			}
		}
		--_globals.Credit_y;
		if (_globals.Credit_bx != -1 || _globals.Credit_bx1 != -1 || _globals.Credit_by != -1 || _globals.Credit_by1 != -1) {
			_eventsManager.VBL();
			_graphicsManager.SCOPY(_graphicsManager.VESA_SCREEN, 60, 50, 520, 380, _graphicsManager.VESA_BUFFER, 60, 50);
		} else {
			_eventsManager.VBL();
		}
		if ( _globals.Credit[_globals.Credit_lignes - 1]._linePosY <= 39) {
			_globals.Credit_y = 440;
			++soundId;
			if (soundId > 31)
				soundId = 28;
			_soundManager.WSOUND(soundId);
		}
		_globals.Credit_bx = -1;
		_globals.Credit_bx1 = -1;
		_globals.Credit_by = -1;
		_globals.Credit_by1 = -1;
	} while ((_eventsManager.BMOUSE() != 1) && (!g_system->getEventManager()->shouldQuit()));
	_graphicsManager.FADE_OUTW();
	_globals.iRegul = 1;
	_eventsManager.souris_flag = true;
}

void HopkinsEngine::BTOCEAN() {
	_fontManager.TEXTE_OFF(9);
	if (_eventsManager.btsouris == 16) {
		_eventsManager.XMOUSE();
		if (_objectsManager.NUMZONE > 0) {
			int oldPosX = _eventsManager.XMOUSE();
			int oldPosY = _eventsManager.YMOUSE();
			bool displAnim = false;
			if (_objectsManager.NUMZONE == 1) {
				if (_globals.OCEAN_SENS == 3)
					_objectsManager.SPACTION(_globals.PERSO, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 1)
					_objectsManager.SPACTION(_globals.PERSO, "27,26,25,24,23,22,21,20,19,18,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 5)
					_objectsManager.SPACTION(_globals.PERSO, "9,10,11,12,13,14,15,16,17,18,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 7;
				_globals.SORTIE = 1;
				int oldX = _objectsManager.XSPR(0);
				for (;;) {
					if (_globals.vitesse == 1)
						oldX -= 2;
					else if (_globals.vitesse == 2)
						oldX -= 4;
					else if (_globals.vitesse == 3)
						oldX -= 6;
					_objectsManager.SETXSPR(0, oldX);
					OCEAN_HOME();
					_eventsManager.VBL();
					if (_eventsManager.BMOUSE() == 1) {
						if (oldPosX == _eventsManager.XMOUSE()) {
							if (_eventsManager.YMOUSE() == oldPosY)
								break;
						}
					}
					if (oldX <= -100)
						goto LABEL_22;
				}
				displAnim = true;
			}
LABEL_22:
			if (_objectsManager.NUMZONE == 2) {
				if (_globals.OCEAN_SENS == 7)
					_objectsManager.SPACTION(_globals.PERSO, "18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 1)
					_objectsManager.SPACTION(_globals.PERSO, "27,28,29,30,31,32,33,34,35,36,-1,", 0, 0, 6, 0);
				if (_globals.OCEAN_SENS == 5)
					_objectsManager.SPACTION(_globals.PERSO, "9,8,7,6,5,4,3,2,1,0,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 3;
				_globals.SORTIE = 2;
				int oldX = _objectsManager.XSPR(0);
				for (;;) {
					if (_globals.vitesse == 1)
						oldX += 2;
					else if (_globals.vitesse == 2)
						oldX += 4;
					else if (_globals.vitesse == 3)
						oldX += 6;
					_objectsManager.SETXSPR(0, oldX);
					OCEAN_HOME();
					_eventsManager.VBL();
					if (_eventsManager.BMOUSE() == 1) {
						if (oldPosX == _eventsManager.XMOUSE()) {
							if (_eventsManager.YMOUSE() == oldPosY)
								break;
						}
					}
					if (oldX > 499)
						goto LABEL_41;
				}
				displAnim = true;
			}
LABEL_41:
			if (_objectsManager.NUMZONE == 3) {
				if (_globals.OCEAN_SENS == 3) {
					int oldX = _objectsManager.XSPR(0);
					do {
						if (_globals.vitesse == 1)
							oldX += 2;
						else if (_globals.vitesse == 2)
							oldX += 4;
						else if (_globals.vitesse == 3)
							oldX += 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY) {
									displAnim = true;
									goto LABEL_57;
								}
							}
						}
					} while (oldX <= 235);
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "36,35,34,33,32,31,30,29,28,27,-1,", 0, 0, 6, 0);
				}
LABEL_57:
				if (_globals.OCEAN_SENS == 7) {
					int oldX = _objectsManager.XSPR(0);
					do {
						if (_globals.vitesse == 1)
							oldX -= 2;
						else if (_globals.vitesse == 2)
							oldX -= 4;
						else if (_globals.vitesse == 3)
							oldX -= 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY) {
									displAnim = true;
									goto LABEL_72;
								}
							}
						}
					} while (oldX > 236);
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "18,19,20,21,22,23,24,25,26,27,-1,", 0, 0, 6, 0);
				}
LABEL_72:
				if (_globals.OCEAN_SENS == 5)
					_objectsManager.SPACTION(_globals.PERSO, "9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 1;
				_globals.SORTIE = 3;
			}
			if (_objectsManager.NUMZONE == 4) {
				if (_globals.OCEAN_SENS == 3) {
					int oldX = _objectsManager.XSPR(0);
					do {
						if (_globals.vitesse == 1)
							oldX += 2;
						else if (_globals.vitesse == 2)
							oldX += 4;
						else if (_globals.vitesse == 3)
							oldX += 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY) {
									displAnim = true;
									goto LABEL_91;
								}
							}
						}
					} while (oldX <= 235);
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "0,1,2,3,4,5,6,7,8,9,-1,", 0, 0, 6, 0);
				}
LABEL_91:
				if (_globals.OCEAN_SENS == 7) {
					int oldX = _objectsManager.XSPR(0);
					for (;;) {
						if (_globals.vitesse == 1)
							oldX -= 2;
						else if (_globals.vitesse == 2)
							oldX -= 4;
						else if (_globals.vitesse == 3)
							oldX -= 6;
						_objectsManager.SETXSPR(0, oldX);
						OCEAN_HOME();
						_eventsManager.VBL();
						if (_eventsManager.BMOUSE() == 1) {
							if (oldPosX == _eventsManager.XMOUSE()) {
								if (_eventsManager.YMOUSE() == oldPosY)
									break;
							}
						}
						if (oldX <= 236) {
							if (!displAnim)
								_objectsManager.SPACTION(_globals.PERSO, "18,17,16,15,14,13,12,11,10,9,-1,", 0, 0, 6, 0);
							break;
						}
					}
				}
				if (_globals.OCEAN_SENS == 1)
					_objectsManager.SPACTION(_globals.PERSO, "27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,-1,", 0, 0, 6, 0);
				_globals.OCEAN_SENS = 5;
				_globals.SORTIE = 4;
			}
		}
	}
}

void HopkinsEngine::OCEAN_HOME() {
	if (_globals.OCEAN_SENS == 3)
		_objectsManager.SETANISPR(0, 0);
	if (_globals.OCEAN_SENS == 7)
		_objectsManager.SETANISPR(0, 18);
	if (_globals.OCEAN_SENS == 1)
		_objectsManager.SETANISPR(0, 27);
	if (_globals.OCEAN_SENS == 5)
		_objectsManager.SETANISPR(0, 9);
}

void HopkinsEngine::OCEAN(int16 a1, Common::String a2, Common::String a3, int16 a4, int16 exit1, int16 exit2, int16 exit3, int16 exit4, int16 a9) {
	_globals.PLAN_FLAG = false;
	_graphicsManager.NOFADE = false;
	_globals.NOMARCHE = false;
	_globals.SORTIE = 0;
	_globals.AFFLI = false;
	_globals.AFFIVBL = true;
	_globals.DESACTIVE_INVENT = true;
	_soundManager.WSOUND(a9);
	_fileManager.constructFilename(_globals.HOPSYSTEM, "VAISSEAU.SPR");
	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	if (a2.size())
		_graphicsManager.LOAD_IMAGE(a2);

	if ((a1 != 77) && (a1 != 84) && (a1 != 91))
		_objectsManager.INILINK("ocean");
	else if (a1 == 77)
		_objectsManager.INILINK("IM77");
	else if (a1 == 84)
		_objectsManager.INILINK("IM84");
	else if (a1 == 91)
		_objectsManager.INILINK("IM91");

	if (!exit1)
		_objectsManager.ZONE_OFF(1);
	if (!exit2)
		_objectsManager.ZONE_OFF(2);
	if (!exit3)
		_objectsManager.ZONE_OFF(3);
	if (!exit4)
		_objectsManager.ZONE_OFF(4);
	if (!_globals.OCEAN_SENS)
		_globals.OCEAN_SENS = a4;
	if (_globals.OCEAN_SENS == 5) {
		_objectsManager.PERX = 236;
		_objectsManager.PERI = 9;
	}
	if (_globals.OCEAN_SENS == 1) {
		_objectsManager.PERX = 236;
		_objectsManager.PERI = 27;
	}
	if (_globals.OCEAN_SENS == 7) {
		_objectsManager.PERX = 415;
		_objectsManager.PERI = 18;
	}
	if (_globals.OCEAN_SENS == 3) {
		_objectsManager.PERX = -20;
		_objectsManager.PERI = 0;
	}
	_objectsManager.SPRITE(_globals.PERSO, _objectsManager.PERX, 110, 0, _objectsManager.PERI, 0, 0, 0, 0);
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_objectsManager.SPRITE_ON(0);
	_globals.chemin = (int16 *)g_PTRNUL;
	_eventsManager.MOUSE_ON();
	_eventsManager.CHANGE_MOUSE(4);

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.VBL();

	if (!_graphicsManager.NOFADE)
		_graphicsManager.FADE_INW();
	_graphicsManager.NOFADE = false;
	_globals.iRegul = 1;

	bool loopCond = false;
	do {
		int mouseButton = _eventsManager.BMOUSE();
		if (mouseButton && mouseButton == 1)
			BTOCEAN();
		_objectsManager.VERIFZONE();
		OCEAN_HOME();
		_eventsManager.VBL();
		if (_globals.SORTIE)
			loopCond = true;
	} while (!loopCond);

	if (_globals.SORTIE == 1)
		_globals.SORTIE = exit1;
	if (_globals.SORTIE == 2)
		_globals.SORTIE = exit2;
	if (_globals.SORTIE == 3)
		_globals.SORTIE = exit3;
	if (_globals.SORTIE == 4)
		_globals.SORTIE = exit4;
	_graphicsManager.FADE_OUTW();
	_objectsManager.SPRITE_OFF(0);
	_globals.AFFLI = false;
	_objectsManager.CLEAR_ECRAN();
	_fileManager.constructFilename(_globals.HOPSYSTEM, "PERSO.SPR");
	_globals.PERSO = _fileManager.loadFile(_globals.NFICHIER);
	_globals.PERSO_TYPE = 0;
}

void HopkinsEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_soundManager.syncSoundSettings();
}

bool HopkinsEngine::ADULT() {
	int xp, yp;
	int buttonIndex;

	_graphicsManager.min_x = 0;
	_graphicsManager.min_y = 0;
	_graphicsManager.max_x = SCREEN_WIDTH;
	_graphicsManager.max_y = SCREEN_HEIGHT - 1;
	_eventsManager.CASSE = false;
	_globals.FORET = false;
	_globals.FLAG_VISIBLE = false;
	_globals.DESACTIVE_INVENT = true;
	_globals.SORTIE = false;

	_graphicsManager.LOAD_IMAGE("ADULT");
	_graphicsManager.FADE_INW();
	_eventsManager.MOUSE_ON();
	_eventsManager.CHANGE_MOUSE(0);
	_eventsManager.btsouris = false;
	_eventsManager.souris_n = false;

	do {
		xp = _eventsManager.XMOUSE();
		yp = _eventsManager.YMOUSE();

		buttonIndex = 0;
		if (xp >= 37 && xp <= 169 && yp >= 406 && yp <= 445)
			buttonIndex = 2;
		else if (xp >= 424 && xp <= 602 && yp >= 406 && yp <= 445)
			buttonIndex = 1;

		_eventsManager.VBL();
	} while (!shouldQuit() && (buttonIndex == 0 || _eventsManager.BMOUSE() != 1));
	
	_globals.DESACTIVE_INVENT = false;
	_globals.FLAG_VISIBLE = false;
	_graphicsManager.FADE_OUTW();

	if (buttonIndex != 2) {
		// Quit game
		return false;
	} else {
		// Continue
		_graphicsManager.min_x = 0;
		_graphicsManager.max_y = 20;
		_graphicsManager.max_x = SCREEN_WIDTH;
		_graphicsManager.max_y = SCREEN_HEIGHT - 20;
		return true;
	}
}

} // End of namespace Hopkins
