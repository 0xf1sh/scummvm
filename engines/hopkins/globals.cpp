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

#include "common/textconsole.h"
#include "common/file.h"
#include "hopkins/globals.h"
#include "hopkins/files.h"
#include "hopkins/font.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

// Global null pointer. This is needed by the engine to recognise NULL pointers, since
// there are places that differentiate between it and a 0 'error' value
byte *g_PTRNUL;

// Default data for the Hopkins array

const int HOPKINS_PERSO_0[] = {
		0, -2, 0, -3, 0, -6, 0, -1, 0, -3, 0, -3, 0, -5, 0, -3, 0, -6, 0, -3, 0, -3, 0, -3,
		9, -4, 8, -4, 6, -2, 9, -2, 9, -3, 9, -3, 9, -4, 9, -2, 9, -2, 8, -2, 9, -3, 9, -2,
		13, 0, 13, 0, 13, 0, 13, 0, 14, 0, 13, 0, 13, 0, 12, 0, 12, 0, 14, 0, 13, 0, 14, 0,
		10, 3, 9, 3, 10, 4, 8, 2, 7, 1, 10, 2, 9, 2, 7, 4, 7, 3, 8, 0, 9, 1, 9, 1, 0, 4, 0,
		4, 0, 6, 0, 3, 0, 4, 0, 3, 0, 4, 0, 4, 0, 6, 0, 3, 0, 3, 0, 3
};

const int HOPKINS_PERSO_1[] = {
		0, -2, 0, -2, 0, -5, 0, -1, 0, -2, 0, -2, 0, -4, 0, -2, 0, -5, 0, -2, 0, -2, 0, -2,
		11, 0, 10, 0, 11, 0, 11, 0, 11, 0, 11, 0, 12, 0, 11, 0, 9, 0, 10, 0, 11, 0, 11, 0,
		11, 0, 10, 0, 11, 0, 11, 0, 11, 0, 11, 0, 12, 0, 11, 0, 9, 0, 10, 0, 11, 0, 11, 0,
		11, 0, 10, 0, 11, 0, 11, 0, 11, 0, 11, 0, 12, 0, 11, 0, 9, 0, 10, 0, 11, 0, 11, 0,
		0, 3, 0, 3, 0, 5, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 5, 0, 3, 0, 3, 0, 3
};

const int HOPKINS_PERSO_2[] = {
		0, -2, 0, 0, 0, -3, 0, -2, 0, -2, 0, -1, 0, -2, 0, -1, 0, -3, 0, -2, 0, -2, 0, -2,
		8, 0, 9, 0, 5, 0, 9, 0, 7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 7, 0, 6, 0, 9, 0, 8, 0, 9, 0,
		5, 0, 9, 0, 7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 7, 0, 6, 0, 9, 0, 8, 0, 9, 0, 5, 0, 9, 0,
		7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 7, 0, 6, 0, 9, 0, 0, 2, 0, 0, 0, 2, 0, 1, 0, 2, 0, 2,
		0, 2, 0, 2, 0, 2, 0, 1, 0, 2, 0, 2
};

Globals::Globals() {
	// Set up the special g_PTRNUL variable
	g_PTRNUL = (byte *)malloc(16);
	strcpy((char *)g_PTRNUL, "POINTERNULL");

	// Initialise array properties
	for (int i = 0; i < 6; ++i)
		CACHE_BANQUE[i] = g_PTRNUL;
	for (int i = 0; i < 106; ++i)
		Common::fill((byte *)&ZONEP[i], (byte *)&ZONEP[i] + sizeof(ZonePItem), 0);
	for (int i = 0; i < 100; ++i)
		Common::fill((byte *)&CarreZone[i], (byte *)&CarreZone[i] + sizeof(CarreZoneItem), 0);
	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&Bqe_Anim[i], (byte *)&Bqe_Anim[i] + sizeof(BqeAnimItem), 0);
	for (int i = 0; i < 8; ++i)
		Common::fill((byte *)&Bank[i], (byte *)&Bank[i] + sizeof(BankItem), 0);
	for (int i = 0; i < 36; ++i)
		Common::fill((byte *)&Bob[i], (byte *)&Bob[i] + sizeof(BobItem), 0);
	for (int i = 0; i < 6; ++i)
		Common::fill((byte *)&Liste[i], (byte *)&Liste[i] + sizeof(ListeItem), 0);
	for (int i = 0; i < 35; ++i)
		Common::fill((byte *)&Liste2[i], (byte *)&Liste2[i] + sizeof(Liste2Item), 0);
	for (int i = 0; i < 30; ++i)
		Common::fill((byte *)&BL_ANIM[i], (byte *)&BL_ANIM[i] + sizeof(BlAnimItem), 0);
	for (int i = 0; i < 30; ++i)
		Common::fill((byte *)&VBob[i], (byte *)&VBob[i] + sizeof(VBobItem), 0);
	for (int i = 0; i < 300; ++i)
		Common::fill((byte *)&ObjetW[i], (byte *)&ObjetW[i] + sizeof(ObjetWItem), 0);
	for (int i = 0; i < 250; ++i)
		Common::fill((byte *)&BLOC[i], (byte *)&BLOC[i] + sizeof(BlocItem), 0);
	for (int i = 0; i < 25; ++i)
		Common::fill((byte *)&Cache[i], (byte *)&Cache[i] + sizeof(CacheItem), 0);

	for (int i = 0; i < 101; ++i)
		Common::fill((byte *)&Segment[i], (byte *)&Segment[i] + sizeof(SegmentItem), 0);
	for (int i = 0; i < 105; ++i) {
		BOBZONE[i] = 0;
		BOBZONE_FLAG[i] = false;
	}
	for (int i = 0; i < 500; ++i)
		STAILLE[i] = 0;
	for (int i = 0; i < 32002; ++i)
		super_parcours[i] = 0;
	for (int i = 0; i < 2100; ++i)
		Param[i] = 0;
	for (int i = 0; i < 70; ++i)
		Common::fill((byte *)&Hopkins[i], (byte *)&Hopkins[i] + sizeof(HopkinsItem), 0);

	for (int i = 0; i < 36; ++i)
		_inventory[i] = 0;
	for (int i = 0; i < 51; ++i)
		Common::fill((byte *)&Tri[i], (byte *)&Tri[i] + sizeof(TriItem), 0);

	// Initialise fields
	FR = 0;
	SVGA = 2;
	_internetFl = true;

	PUBEXIT = false;
	_speed = 1;
	INSTALL_TYPE = 1;
	MUSICVOL = 6;
	SOUNDVOL = 6;
	VOICEVOL = 6;
	g_old_anim = 0;
	g_old_sens = 0;
	g_old_sens2 = 0;
	last_sens = 0;
	police_l = police_h = 0;
	TETE = NULL;
	texte_long = 0;
	TEXTE_FORMATE = 0;
	_curObjectIndex = 0;
	NUM_FICHIER_OBJ = 0;
	nbrligne = 0;
	largeur_boite = 0;
	hauteur_boite = 0;
	FORET = false;
	_objectWidth = _objectHeight = 0;
	HELICO = 0;
	_catalogPos = 0;
	_catalogSize = 0;
	_newObjectFl = false;
	iRegul = 0;
	SORTIE = 0;
	PLANX = PLANY = 0;
	PLANI = 0;
	PERSO = 0;
	PASSWORD = false;
	ECRAN = 0;
	NOSPRECRAN = false;
	OLD_ECRAN = 0;
	Max_Propre_Gen = 0;
	Max_Ligne_Long = 0;
	Max_Perso_Y = 0;
	Max_Propre = 0;
	fmusic = 0;
	NBBLOC = 0;
	opt_scrtype = 0;
	opt_scrspeed = 0;
	opt_vitesse = 0;
	opt_sound = 0;
	opt_voice = 0;
	opt_music = 0;
	opt_txt = 0;
	opt_anm = 0;
	NBTRI = 0;
	AFFLI = false;
	AFFIVBL = false;
	NOT_VERIF = 0;
	PERSO_TYPE = 0;
	GOACTION = false;
	NECESSAIRE = false;
	Compteur = 0;
	ACTION_SENS = 0;
	ACTION_SENS = 0;
	SegmentEnCours = 0;
	STOP_BUG = 0;
	MAX_COMPTE = 0;

	Credit_bx = -1;
	Credit_bx1 = -1;
	Credit_by = -1;
	Credit_by1 = -1;
	Credit_y = 0;
	Credit_lignes = 0;
	memset(Credit, 0, 12000);
	Credit_step = 0;
	Credit_l = 0;
	Credit_h = 0;

	OCEAN_SENS = 0;

	// Initialise pointers
	ICONE = NULL;
	BUF_ZONE = NULL;
	CACHE_BANQUE[6] = NULL;
	texte_tmp = NULL;
	SPRITE_ECRAN = NULL;
	SAUVEGARDE = NULL;
	BUFFERTAPE = NULL;
	essai0 = NULL;
	essai1 = NULL;
	essai2 = NULL;
	inventaire2 = NULL;
	GESTE = NULL;
	INVENTAIRE_OBJET = NULL;
	FORETSPR = NULL;
	COUCOU = NULL;
	chemin = NULL;
	cache_souris = NULL;
	BufLig = NULL;
	Bufferdecor = NULL;
	ADR_FICHIER_OBJ = NULL;
	police = NULL;
	PERSO = NULL;
	OPTION_SPR = NULL;

	// Reset flags
	MUSICOFF = false;
	SOUNDOFF = false;
	VOICEOFF = false;
	SOUNDOFF = false;
	MUSICOFF = false;
	VOICEOFF = false;
	CENSURE = false;
	GESTE_FLAG = false;
	BPP_NOAFF = false;
	_disableInventFl = false;
	FLAG_VISIBLE = false;
	netscape = false;
	NOMARCHE = false;
	NO_VISU = false;
	_optionDialogFl = false;
	CACHEFLAG = false;
	NOPARLE = false;
	couleur_40 = 50;

	// Reset indexed variables
	force_to_data_0 = 0;
	oldzone_46 = 0;
	old_x1_65 = 0;
	old_y1_66 = 0;
	old_x2_67 = 0;
	old_y2_68 = 0;
	old_zone_68 = 0;
	old_z_69 = 0;
	old_x_69 = 0;
	old_y_70 = 0;
	compteur_71 = 0;
	zozo_73 = 0;
	j_104 = 0;
}

Globals::~Globals() {
	free(ICONE);
	freeMemory(TETE);
	freeMemory(police);
	freeMemory(BUF_ZONE);
	freeMemory(CACHE_BANQUE[6]);
	freeMemory(texte_tmp);
	freeMemory(SPRITE_ECRAN);
	freeMemory((byte *)SAUVEGARDE);
	freeMemory(BUFFERTAPE);
	freeMemory(inventaire2);
	freeMemory(GESTE);
	freeMemory(INVENTAIRE_OBJET);
	freeMemory(FORETSPR);
	freeMemory(COUCOU);
	freeMemory(cache_souris);
	freeMemory(Bufferdecor);
	freeMemory(ADR_FICHIER_OBJ);
	freeMemory(PERSO);

	CLEAR_VBOB();

	free(g_PTRNUL);
}

void Globals::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void Globals::setConfig() {
	HOPIMAGE = "BUFFER";
	HOPANIM = "ANIM";
	HOPLINK = "LINK";
	HOPSAVE = "SAVE";
	HOPSOUND = "SOUND";
	HOPMUSIC = "MUSIC";
	HOPVOICE = "VOICE";
	HOPANM = "ANM";
	HOPTSVGA = "TSVGA";
	HOPSVGA = "SVGA";
	HOPVGA = "VGA";
	HOPSEQ = "SEQ";
	HOPSYSTEM = "SYSTEM";

	// CHECKME: Should be in Globals() but it doesn't work
	// The Polish version is a translation of the English version. The filenames are the same.
	switch (_vm->getLanguage()) {
	case Common::EN_ANY:
	case Common::PL_POL:
		FR = 0;
		break;
	case Common::FR_FRA:
		FR = 1;
		break;
	case Common::ES_ESP:
		FR = 2;
		break;
	default:
		warning("Unknown language in internal language mapping");
		break;
	}

	if (_vm->getIsDemo())
		_internetFl = true;
	else
		_internetFl = false;
	// End of CHECKME

	switch (FR) {
	case 0:
		FICH_ZONE = "ZONEAN.TXT";
		FICH_TEXTE = "TEXTEAN.TXT";
		break;
	case 1:
		FICH_ZONE = "ZONE01.TXT";
		FICH_TEXTE = "TEXTE01.TXT";
		break;
	case 2:
		FICH_ZONE = "ZONEES.TXT";
		FICH_TEXTE = "TEXTEES.TXT";
		break;
	}
}

void Globals::clearAll() {
	// TODO: The original allocated an explicit memory block for the null pointer
	// to point to. For now, we're seeing if the NULL value will do as well

	for (int idx = 0; idx < 6; ++idx)
		CACHE_BANQUE[idx] = g_PTRNUL;

	nbrligne = 80;
	INIT_ANIM();

	texte_tmp = g_PTRNUL;
	texte_long = 0;
	police = g_PTRNUL;
	police_h = 0;
	police_l = 0;
	hauteur_boite = 0;
	largeur_boite = 0;

	_vm->_fontManager.clearAll();

	INIT_VBOB();
	ADR_FICHIER_OBJ = g_PTRNUL;
	NUM_FICHIER_OBJ = 0;
	Bufferdecor = g_PTRNUL;
	_vm->_eventsManager._objectBuf = g_PTRNUL;
	_vm->_dialogsManager._inventWin1 = g_PTRNUL;
	_vm->_dialogsManager._inventBuf2 = g_PTRNUL;
	COUCOU = g_PTRNUL;
	SPRITE_ECRAN = g_PTRNUL;
	SAUVEGARDE = (Sauvegarde *)g_PTRNUL;
	_curObjectIndex = 0;

	for (int idx = 0; idx < 105; ++idx) {
		ZONEP[idx]._destX = 0;
		ZONEP[idx]._destY = 0;
		ZONEP[idx].field4 = 0;
	}

	essai0 = (int16 *)g_PTRNUL;
	essai1 = (int16 *)g_PTRNUL;
	essai2 = (int16 *)g_PTRNUL;
	BufLig = (int16 *)g_PTRNUL;
	chemin = (int16 *)g_PTRNUL;

	for (int idx = 0; idx < 400; ++idx) {
		_vm->_linesManager.Ligne[idx].field0 = 0;
		_vm->_linesManager.Ligne[idx].field2 = 0;
		_vm->_linesManager.Ligne[idx].field4 = 0;
		_vm->_linesManager.Ligne[idx].field6 = 0;
		_vm->_linesManager.Ligne[idx].field8 = 0;
		_vm->_linesManager.Ligne[idx].lineData = (int16 *)g_PTRNUL;

		_vm->_linesManager.LigneZone[idx].count = 0;
		_vm->_linesManager.LigneZone[idx].field2 = 0;
		_vm->_linesManager.LigneZone[idx].zoneData = (int16 *)g_PTRNUL;
	}

	for (int idx = 0; idx < 100; ++idx) {
		CarreZone[idx].field0 = 0;
	}

	texte_long = 0;
	texte_tmp = g_PTRNUL;
	BUFFERTAPE = allocMemory(85000);

	SAUVEGARDE = (Sauvegarde *)malloc(sizeof(Sauvegarde));
	memset(SAUVEGARDE, 0, sizeof(Sauvegarde));

	essai0 = (int16 *)BUFFERTAPE;
	essai1 = (int16 *)(BUFFERTAPE + 25000);
	essai2 = (int16 *)(BUFFERTAPE + 50000);
	BufLig = (int16 *)(BUFFERTAPE + 75000);
	largeur_boite = 240;
	TEXTE_FORMATE = 300;

	_vm->_eventsManager._objectBuf = allocMemory(2500);
	INVENTAIRE_OBJET = allocMemory(2500);

	ADR_FICHIER_OBJ = g_PTRNUL;
	FORETSPR = g_PTRNUL;
	FORET = false;

	cache_souris = allocMemory(2500);
	GESTE = g_PTRNUL;
	GESTE_FLAG = false;
}

void Globals::HOPKINS_DATA() {
	assert(PERSO_TYPE >= 0 && PERSO_TYPE <= 2);

	const int *srcList[] = { HOPKINS_PERSO_0, HOPKINS_PERSO_1, HOPKINS_PERSO_2 };
	const int *srcP = srcList[PERSO_TYPE];

	for (int idx = 0; idx < 240 / 4; ++idx) {
		Hopkins[idx].field0 = *srcP++;
		Hopkins[idx].field2 = *srcP++;
	}

	g_old_anim = -1;
	g_old_sens = -1;
}

void Globals::INIT_ANIM() {
	for (int idx = 0; idx < 35; ++idx) {
		Bqe_Anim[idx]._data = g_PTRNUL;
		Bqe_Anim[idx].field4 = 0;
	}

	for (int idx = 0; idx < 8; ++idx) {
		Bank[idx]._data = g_PTRNUL;
		Bank[idx].field4 = 0;
		Bank[idx]._filename = "";
		Bank[idx]._fileHeader = 0;
		Bank[idx].field1C = 0;
	}
}

void Globals::INIT_VBOB() {
	for (int idx = 0; idx < 30; ++idx) {
		VBob[idx].field4 = 0;
		VBob[idx]._xp = 0;
		VBob[idx]._yp = 0;
		VBob[idx]._frameIndex = 0;
		VBob[idx]._surface = g_PTRNUL;
		VBob[idx]._spriteData = g_PTRNUL;
		VBob[idx]._oldSpriteData = g_PTRNUL;
	}
}

void Globals::CLEAR_VBOB() {
	for (int idx = 0; idx < 30; ++idx) {
		VBob[idx].field4 = 0;
		VBob[idx]._xp = 0;
		VBob[idx]._yp = 0;
		VBob[idx]._frameIndex = 0;
		VBob[idx]._surface = freeMemory(VBob[idx]._surface);
		VBob[idx]._spriteData = g_PTRNUL;
		VBob[idx]._oldSpriteData = g_PTRNUL;
	}
}

// Load Object
void Globals::CHARGE_OBJET() {
	_vm->_fileManager.constructFilename(HOPSYSTEM, "OBJET.DAT");
	byte *data = _vm->_fileManager.loadFile(NFICHIER);
	byte *srcP = data;

	for (int idx = 0; idx < 300; ++idx) {
		ObjetW[idx].field0 = *srcP++;
		ObjetW[idx]._idx = *srcP++;
		ObjetW[idx].field2 = *srcP++;
		ObjetW[idx].field3 = *srcP++;
		ObjetW[idx].field4 = *srcP++;
		ObjetW[idx].field5 = *srcP++;
		ObjetW[idx].field6 = *srcP++;
		ObjetW[idx].field7 = *srcP++;
	}

	free(data);
}

byte *Globals::allocMemory(int count) {
	byte *result = (byte *)malloc(count);
	if (!result)
		result = g_PTRNUL;
	return result;
}

byte *Globals::freeMemory(byte *p) {
	if (p != g_PTRNUL)
		free(p);
	return g_PTRNUL;
}

// Reset Cache
void Globals::RESET_CACHE() {

	for (int idx = 1; idx <= 5; ++idx) {
		CACHE_BANQUE[idx] = freeMemory(CACHE_BANQUE[idx]);
	}

	for (int idx = 0; idx <= 20; ++idx) {
		Cache[idx]._spriteData = g_PTRNUL;
		Cache[idx]._x = 0;
		Cache[idx]._y = 0;
		Cache[idx]._spriteIndex = 0;
		Cache[idx].fieldA = 0;
		Cache[idx]._width = 0;
		Cache[idx]._height = 0;
		Cache[idx].field10 = false;
		Cache[idx].field14 = 0;
	}

	CACHEFLAG = false;
}

void Globals::CACHE_ON() {
	CACHEFLAG = true;
}

// TODO: Find why some calls have a parameter value
void Globals::CACHE_OFF(int v1) {
	CACHEFLAG = false;
}

void Globals::CACHE_SUB(int idx) {
	Cache[idx].fieldA = 0;
}

void Globals::CACHE_ADD(int idx) {
	Cache[idx].fieldA = 1;
}

// Load Cache
void Globals::loadCache(const Common::String &file) {
	byte *v2 = g_PTRNUL;
	byte *spriteData;
	byte *ptr;
	Common::String v16;
	Common::File f;

	RESET_CACHE();
	_vm->_fileManager.constructFilename(HOPLINK, file);
	ptr = _vm->_fileManager.loadFile(NFICHIER);
	v16 = Common::String((const char *)ptr);

	_vm->_fileManager.constructFilename(HOPLINK, v16);

	if (f.exists(NFICHIER)) {
		spriteData = _vm->_fileManager.loadFile(NFICHIER);
		CACHE_BANQUE[1] = spriteData;
		int v15 = 60;
		for (int i = 0; i <= 21; i++) {
			int v11 = (int16)READ_LE_UINT16((uint16 *)ptr + v15);
			int v4 = (int16)READ_LE_UINT16((uint16 *)ptr + v15 + 1);
			int v5 = (int16)READ_LE_UINT16((uint16 *)ptr + v15 + 2);
			int v6 = i;
			Cache[v6].field14 = (int16)READ_LE_UINT16((uint16 *)ptr + v15 + 4);
			Cache[v6]._spriteIndex = v11;
			Cache[v6]._x = v4;
			Cache[v6]._y = v5;
			if (spriteData == g_PTRNUL) {
				Cache[i].fieldA = 0;
			} else {
				int v8 = _vm->_objectsManager.getWidth(spriteData, v11);
				int v9 = _vm->_objectsManager.getHeight(spriteData, v11);
				Cache[i]._spriteData = spriteData;
				Cache[i]._width = v8;
				Cache[i]._height = v9;
				Cache[i].fieldA = 1;
			}

			if ( !Cache[i]._x && !Cache[i]._y && !Cache[i]._spriteIndex)
				Cache[i].fieldA = 0;
			v15 += 5;
		}
		CACHE_ON();
		v2 = ptr;
	}
	freeMemory(v2);
}

void Globals::B_CACHE_OFF(int idx) {
	Bob[idx].field34 = true;
}


} // End of namespace Hopkins
