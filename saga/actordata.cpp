/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Actor data table
#include "saga/saga.h"
#include "saga/actordata.h"

namespace Saga {

ActorTableData ITE_ActorTable[ITE_ACTORCOUNT] = {
	// Original used so called permanent actors for first three and that was designed by 
	// EXTENDED object flag. They contained frames in more than one resource. We use 
	// different technique here see "Apppending to sprite list" in loadActorResources()

//       flags     name scene    x     y    z  spr  frm scp  col 
//    ------------ ---- ----  ---- ----- ---- ---- ---- --- ---- -- -- --
	{ kProtagonist | kExtended,
					 0,   1,    0,    0,   0,  37, 135,  0,   1,  0, 0, 0},	// map party
	// spr and frm numbers taken from permanent actors list
	{ kFollower | kExtended,
					 1,   0,    0,    0,   0,  45, 177,  1, 132,  0, 0, 0},	// Okk
	{ kFollower | kExtended,
					 2,   0,    0,    0,   0,  48, 143,  2, 161,  0, 0, 0},	// Eeah
	{ 0,             3,   0,  240,  480,   0, 115, 206,  0,  25,  0, 0, 0},	// albino ferret
	{ 0,             4,  17,  368,  400,   0, 115, 206,  4,  49,  0, 0, 0},	// moneychanger
	{ 0,             5,  11,  552,  412,   0,  54, 152,  1, 171,  0, 0, 0},	// Sist
	{ 0,            17,   2, 1192,  888,   0,  57, 153, 17,  49,  0, 0, 0},	// worker ferret 1
	{ 0,            17,   2,  816, 1052,   0,  57, 153, 18,  49,  0, 0, 0},	// worker ferret 2
	{ 0,            17,   2,  928,  932,   0,  58, 153, 19,  49,  0, 0, 0},	// worker ferret 3
	{ 0,            17,   2, 1416, 1160,   0,  58, 153, 20,  49,  0, 0, 0},	// worker ferret 4
	{ 0,            19,  49, 1592, 1336,   0,  92, 175, 15, 162,  0, 0, 0},	// faire merchant 1 (bear)
	{ 0,            20,  49,  744,  824,   0,  63, 156, 19, 112,  0, 4, 4},	// faire merchant 2 (ferret)
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire merchant 3
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire merchant 4
	{ 0,             9,  49, 1560, 1624,   0,  94, 147, 18, 132,  0, 4, 4},	// faire goer 1a (rat)
	{ 0,            56,  49, 1384,  792,   0,  95, 193, 20,  72,  0, 0, 0},	// faire goer 1b (otter)
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 2a
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0},	// faire goer 2b
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0}, 	// faire goer 3a
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0}, 	// faire goer 3b
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0}, 	// faire goer 4a
	{ 0,            19,   0, 1592, 1336,   0,  92, 175,  0, 171,  0, 0, 0}, 	// faire goer 4b
	{ 0,            18,  32,  764,  448,   0,  55, 150,  0,  48, 10, 4, 4}, 	// Scorry
	{ 0,            35,  32,    0,    0,   0,  56, 151,  0, 112,  0, 0, 0}, 	// grand puzzler
	{ 0,            36,  32,    0,    0,   0, 105, 142,  0, 155,  0, 0, 0}, 	// Rhene
	{ 0,            32,  32,    0,    0,   0,  91, 190,  0,  98,  0, 0, 0}, 	// elk captain
	{ 0,            31,  32,    0,    0,   0,  90, 189,  0, 171,  0, 0, 0}, 	// elk guard 1
	{ 0,            31,  32,    0,    0,   0,  90, 189,  0, 171,  0, 0, 0}, 	// elk guard 2
	{ 0,            31,  32,    0,    0,   0,  90, 189,  0, 171,  0, 0, 0}, 	// elk guard 3
	{ 0,            31,  32,    0,    0,   0,  79, 172,  0,  18,  0, 0, 0}, 	// boar sergeant
	{ 0,            21,  50,  664,  400,   0,  76, 171,  2,  74,  0, 4, 4}, 	// boar sentry 1
	{ 0,            21,  50,  892,  428,   0,  76, 171,  2,  74,  0, 4, 4}, 	// boar sentry 2
	{ 0,             9,  51,  904,  936,   0,  51, 145, 35,   5,  0, 0, 0}, 	// hall rat 1
	{ 0,             9,  51,  872,  840,   0,  51, 145, 36,   5,  0, 0, 0}, 	// hall rat 2
	{ 0,             9,  51, 1432,  344,   0,  51, 145, 37,   5,  0, 0, 0}, 	// hall rat 3
	{ 0,             9,  51,  664,  472,   0,  51, 145, 38,   5,  0, 0, 0}, 	// hall rat 4
	{ 0,            10,  51, 1368, 1464,   0,  80, 146, 39, 147,  0, 0, 0}, 	// book rat 1
	{ 0,            10,  51, 1416, 1624,   0,  80, 146, 40, 147,  0, 0, 0}, 	// book rat 2
	{ 0,            10,  51, 1752,  120,   0,  80, 146, 41, 147,  0, 0, 0}, 	// book rat 3
	{ 0,            10,  51,  984,  408,   0,  80, 146, 42, 147,  0, 0, 0}, 	// book rat 4
	{ 0,            14,  52,  856,  376,   0,  82, 174,  8,  73,  0, 0, 0}, 	// grounds servant 1
	{ 0,            14,  52,  808,  664,   0,  82, 174,  9,  73,  0, 0, 0}, 	// grounds servant 2
	{ 0,            14,  52,  440,  568,   0,  82, 174, 10,  73,  0, 0, 0}, 	// grounds servant 3
	{ 0,            14,  52,  392,  776,   0,  82, 174, 11,  73,  0, 0, 0}, 	// grounds servant 4
	{ 0,            21,   4,  240,  384,   0,  79, 172,  0,  18,  0, 2, 2}, 	// boar sentry 3 (by doorway)
	{ 0,            23,   4,  636,  268,   0,  77, 173,  0,  74,  0, 4, 4}, 	// boar courtier
	{ 0,            22,   4,  900,  320,   0,  78, 179,  0,  60,  0, 4, 4}, 	// boar king
	{ 0,            14,   4,  788,  264,   0,  75, 170,  0, 171,  0, 2, 2}, 	// boar servant 1
	{ 0,            14,   4, 1088,  264,   0,  75, 170,  0, 171,  0, 6, 6}, 	// boar servant 2
	{ 0,            24,  19,  728,  396,   0,  65, 181, 47, 146,  0, 6, 6}, 	// glass master
	{ 0,            24,  21,  -20,  -20,   0,  66, 182,  0, 146,  0, 4, 4}, 	// glass master (with orb)
	{ kCycle,       25,  19,  372,  464,   0,  67, 183, 73, 146,  0, 2, 2}, 	// glass worker
	{ 0,            26,   5,  564,  476,  27,  53, 149,  1,   5,  0, 4, 4}, 	// door rat
	{ kCycle,       27,  31,  868,  344,   0,  81, 180,  0, 171,  0, 4, 4}, 	// bees
	{ 0,            28,  73,  568,  380,   0,  83, 176, 30, 120,  0, 4, 4}, 	// fortune teller
	{ 0,            14,   7,  808,  480,   0,  82, 174,  9,  73,  0, 0, 0}, 	// orb messenger
	{ 0,            29,  10,  508,  432,   0,  84, 186,  6, 112,  0, 4, 4}, 	// elk king
	{ 0,            33,  10,  676,  420,   0,  86, 184,  6, 171,  0, 4, 4}, 	// elk chancellor
	{ 0,            30,  10,  388,  452,   0,  88, 185,  6, 171,  0, 4, 4}, 	// elk courtier 1
	{ 0,            30,  10,  608,  444,   0,  89, 185,  6, 171,  0, 4, 4}, 	// elk courtier 2
	{ 0,            31,  10,  192,  468,   0,  90, 189,  6, 171,  0, 4, 4}, 	// elk throne guard 1
	{ 0,            31,  10,  772,  432,   0,  90, 189,  6, 171,  0, 4, 4}, 	// elk throne guard 2
	{ 0,            14,  10, 1340,  444,   0,  87, 188,  6, 171,  0, 4, 4}, 	// elk servant
	{ 0,            20,  18,  808,  360,   7,  60, 154, 64,  88,  0, 4, 4}, 	// hardware ferret
	{ 0,            34,  49, 1128, 1256,   0,  96, 191, 16,  35,  0, 4, 4}, 	// porcupine
	{ 0,            34,  49, 1384,  792,   0,  93, 192, 17,  66,  0, 4, 4}, 	// faire ram
	{ 0,            24,  21,    0,  -40,   0,  65, 181, 50, 146,  0, 6, 6}, 	// glass master 2
	{ 0,             3,  21,    0,  -40,   0,  64, 158, 49, 112,  0, 0, 0}, 	// Sakka
	{ 0,            17,  21,    0,  -40,   0,  62, 157, 74,  48,  0, 0, 0}, 	// lodge ferret 1
	{ 0,            17,  21,    0,  -40,   0,  62, 157, 74,  49,  0, 0, 0}, 	// lodge ferret 2
	{ 0,            17,  21,    0,  -40,   0,  62, 157, 74,  50,  0, 0, 0}, 	// lodge ferret 3
	{ 0,            12, 244, 1056,  504,   0, 107, 167, 21, 124,  0, 6, 6}, 	// Elara
	{ 0,             8,  33,  248,  440,   0,  68, 169, 14, 112,  0, 0, 0}, 	// Tycho
	{ 0,            11,  23,  308,  424,   0, 106, 166,  6,  48,  0, 2, 2}, 	// Alamma
	{ 0,            17,   2, 1864, 1336,   0,  58, 153, 21,  49,  0, 0, 0}, 	// worker ferret 5
	{ 0,            17,   2,  760,  216,   0,  58, 153, 22,  49,  0, 0, 0}, 	// worker ferret 6
	{ 0,            44,  29,    0,    0,   0,  72, 159,  0, 112,  0, 0, 0}, 	// Prince
	{ 0,            45,  29,    0,    0,   0,  71, 163,  0, 146,  0, 6, 6}, 	// harem girl 1
	{ 0,            45,  29,    0,    0,   0,  71, 163,  0, 124,  0, 2, 2}, 	// harem girl 2
	{ 0,            45,  29,    0,    0,   0,  71, 163,  0, 169,  0, 0, 0}, 	// harem girl 3
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0}, 	// dog sergeant
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0}, 	// throne dog guard 1
	{ 0,             7, 257,  552,  408,   0,  70, 165,  0,   4,  0, 2, 2}, 	// throne dog guard 2
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0}, 	// throne dog guard 3
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0}, 	// throne dog guard 4
	{ 0,             7, 257,  712,  380,   0,  69, 164,  0,   4,  0, 4, 4}, 	// throne dog guard 5
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0}, 	// throne dog guard 6
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0}, 	// throne dog guard 7
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0}, 	// throne dog guard 8
	{ 0,             7,  29,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0}, 	// throne dog guard 9
	{ 0,             7,   0,    0,    0,   0,  69, 164,  0,   4,  0, 0, 0}, 	// throne dog guard 10
	{ 0,             7,  29,    0,    0,   0,  70, 165,  0,   4,  0, 0, 0}, 	// throne dog guard 11
	{ 0,            47,  30,    0,    0,   0, 102, 199,  1, 186,  0, 0, 0}, 	// old wolf ferryman
	{ 0,            48,  69,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0}, 	// cat village wildcat
	{ 0,            49,  69,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0}, 	// cat village attendant
	{ 0,            50,  69,    0,    0,   0, 111, 203, 16,  67,  0, 0, 0}, 	// cat village Prowwa
	{ 0,            51,  20,    0,    0,   0, 112, 204, 15,  26,  0, 0, 0}, 	// Prowwa hut Mirrhp
	{ 0,            50,  20,    0,    0,   0, 111, 203, 14,  67,  0, 0, 0}, 	// Prowwa hut Prowwa
	{ 0,            49,  20,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0}, 	// Prowwa hut attendant
	{ 0,            48, 256,    0,    0,   0, 109, 202, 35,  26,  0, 0, 0}, 	// wildcat sentry
	{ 0,            21,  32,    0,    0,   0,  76, 171,  0, 171,  0, 0, 0}, 	// boar warrior 1
	{ 0,            21,  32,    0,    0,   0,  76, 171,  0, 171,  0, 0, 0}, 	// boar warrior 2
	{ 0,            21,  32,    0,    0,   0,  76, 171,  0, 171,  0, 0, 0}, 	// boar warrior 3
	{ 0,            52,  15,  152,  400,   0, 108, 168, 19,  48, 10, 2, 2}, 	// Alamma's voice
	{ 0,            47, 251,  640,  360,   0, 113, 205,  5, 186, 10, 2, 2}, 	// ferry on ocean
	{ 0,            41,  75,  152,  400,   0, 100, 197,  5,  81,  0, 0, 0}, 	// Shiala
	{ 0,            44,   9,    0,    0,   0,  73, 160, 54, 112,  0, 0, 0}, 	// Prince (asleep)
	{ 0,             0,  22,  -20,  -20,   0, 118, 209,  0, 171,  0, 0, 0}, 	// Rif and Eeah (at rockslide)
	{ 0,             1,  22,    0,    0,   0, 119, 210,  0, 171,  0, 0, 0}, 	// Okk (at rockslide)
	{ 0,             0,  22,  -20,  -20,   0, 118, 209,  0, 171,  0, 0, 0}, 	// Rif and Eeah (at rockslide w. rope)
	{ 0,             1,  22,    0,    0,   0, 119, 210,  0, 171,  0, 0, 0}, 	// Okk (at rockslide w. rope)
	{ 0,            53,  42,  640,  400,   0, 104, 201,  8, 141,  0, 0, 0}, 	// Kylas Honeyfoot
	{ 0,            54,  21,  -20,  -20,   0, 120, 211, 48, 238,  0, 0, 0}, 	// Orb of Hands
	{ 0,             0,   4,  -20,  -20,   0,  42, 140,  0,   1,  0, 0, 0}, 	// Rif (muddy)
	{ 0,            26,   5,  -20,  -20,  27,  52, 148,  1,   5,  0, 4, 4}, 	// door rat (standing)
	{ 0,            36,   4,  -20,  -20,   0, 116, 207,  0, 155,  0, 0, 0}, 	// boar with Rhene 1
	{ 0,            36,   0,  -20,  -20,   0, 117, 208,  0, 155,  0, 0, 0}, 	// boar with Rhene 2
	{ 0,            46, 252,  -20,  -20,   0,  74, 162, 29,  34,  0, 0, 0}, 	// dog jailer
	{ 0,             0,  32,  -20,  -20,   0,  41, 137,  0,   1,  0, 0, 0}, 	// Rif (tourney)
	{ 0,             0, 259,  -20,  -20,   0,  44, 138,  0,   1,  0, 0, 0}, 	// cliff rat
	{ 0,             0,   5,  -20,  -20,   0,  43, 139,  0,   1,  0, 0, 0}, 	// Rif (cloaked)
	{ 0,             0,  31,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0}, 	// Rif (oak tree scene)
	{ 0,             0, 252,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0}, 	// Rif (jail cell scene)
	{ 0,             0,  15,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0}, 	// Rif (outside Alamma's)
	{ 0,             0,  20,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0}, 	// Rif (sick tent)
	{ 0,             0,  25,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0}, 	// Rif (gem room)
	{ 0,             0, 272,  -20,  -20,   0,  40, 141,  0,   1,  0, 0, 0}, 	// Rif (dragon maze)
	{ 0,             0,  50,  -20,  -20,   0,  39, 136,  0,   1,  0, 0, 0}, 	// Rif (boar entry gate)
	{ 0,            50,  71,  -20,  -20,   0, 111, 203,  0,  67,  0, 0, 0}, 	// Prowwa (dog castle back)
	{ 0,            50, 274,  -20,  -20,   0, 111, 203,  0,  67,  0, 0, 0}, 	// Prowwa (cat festival)
	{ 0,            50, 274,  -20,  -20,   0, 110, 212,  0, 171,  0, 0, 0}, 	// cat festival dancer 1
	{ 0,            50, 274,  -20,  -20,   0, 110, 212,  0, 171,  0, 0, 0}, 	// cat festival dancer 2
	{ 0,            50, 274,  -20,  -20,   0, 110, 212,  0, 171,  0, 0, 0}, 	// cat festival dancer 3
	{ 0,            57, 272,  909,  909,  48, 121, 213,  0, 171,  0, 0, 0}, 	// komodo dragon
	{ 0,            58,  15,  -20,  -20,   0, 122, 214,  0, 171,  0, 0, 0}, 	// letter from Elara
	{ 0,            37, 246,  -20,  -20,   0,  97, 194,  0, 141,  0, 0, 0}, 	// Gar (wolves' cage)
	{ 0,            38, 246,  -20,  -20,   0,  98, 195,  0,  27,  0, 0, 0}, 	// Wrah (wolves' cage)
	{ 0,            59, 246,  -20,  -20,   0, 103, 200,  0,  26,  0, 0, 0}, 	// Chota (wolves' cage)
	{ 0,            41, 245,  -20,  -20,   0, 100, 197,  0,  81,  0, 0, 0}, 	// Shiala (wolves' cage)
	{ 0,            47, 250,  640,  360,   0, 114, 205,  0, 186, 10, 2, 2}, 	// ferry on ocean
	{ 0,             0, 278,  -20,  -20,   0,  40, 141,  0,   1,  0, 0, 0}, 	// Rif (falling in tunnel trap door)
	{ 0,             0, 272,  -20,  -20,   0,  40, 141,  0,   1,  0, 0, 0}, 	// Rif (falling in dragon maze)
	{ 0,            41,  77,  -20,  -20,   0, 100, 197, 24,  81,  0, 0, 0}, 	// Shiala (grotto)
	{ 0,            37, 261,  -20,  -20,   0,  97, 194,  0, 141,  0, 0, 0}, 	// Gar (ambush)
	{ 0,            38, 261,  -20,  -20,   0,  98, 195,  0,  27,  0, 0, 0}, 	// Wrah (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0}, 	// dark claw wolf (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0}, 	// dark claw wolf (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0}, 	// dark claw wolf (ambush)
	{ 0,            39, 261,  -20,  -20,   0,  99, 196,  0,   5,  0, 0, 0}, 	// dark claw wolf (ambush)
	{ 0,            59, 279,  -20,  -20,   0, 103, 200,  0,  26,  0, 0, 0}, 	// Chota (top of dam)
	{ 0,            38, 279,  -20,  -20,   0,  98, 195,  0,  27,  0, 0, 0}, 	// Wrah (top of dam)
	{ 0,            42,  77,  -20,  -20,   0, 101, 198, 25, 171,  0, 0, 0}, 	// Shiala's spear
	{ 0,            59, 281,  -20,  -20,   0, 103, 200, 26,  26,  0, 0, 0}, 	// Chota (lab)
	{ 0,            59, 279,  -20,  -20,   0, 123, 215,  0,   1,  0, 0, 0}, 	// Rif (finale)
	{ 0,            59, 279,  -20,  -20,   0, 123, 215,  0, 132,  0, 0, 0}, 	// Okk (finale)
	{ 0,            59, 279,  -20,  -20,   0, 123, 215,  0, 161,  0, 0, 0}, 	// Eeah (finale)
	{ 0,            54, 279,  -20,  -20,   0, 120, 211,  0, 133,  0, 6, 6}, 	// Orb of Storms (top of dam)
	{ 0,            44,   9,  -20,  -20,   0, 124, 161,  0, 171,  0, 6, 6}, 	// Prince's snores
	{ 0,             7, 255,  588,  252,   0,  70, 165,  0,   3,  0, 2, 2}, 	// hall dog guard 1
	{ 0,             7, 255,  696,  252,   0,  70, 165,  0,   5,  0, 6, 6}, 	// hall dog guard 2
	{ 0,            36,   4,    0,    0,   0, 105, 142,  0, 155,  0, 0, 0}, 	// Rhene
	{ 0,            44, 272, 1124, 1124, 120,  72, 159,  0, 112,  0, 0, 0}, 	// Prince (dragon maze)
	{ 0,             7, 272, 1124, 1108, 120,  70, 165,  0,   4,  0, 0, 0}, 	// dog heckler 1 (dragon maze)
	{ 0,             7, 272, 1108, 1124, 120,  70, 165,  0,   4,  0, 0, 0}, 	// dog heckler 2 (dragon maze)
	{ 0,            29, 288,  508,  432,   0,  85, 187,  0, 112,  0, 4, 4}, 	// elk king (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0,  99,  0, 4, 4}, 	// crowd voice 1 (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0,  98,  0, 4, 4}, 	// crowd voice 2 (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0, 104,  0, 4, 4}, 	// crowd voice 3 (finale)
	{ 0,            29,   0,  508,  432,   0,  84, 186,  0,  99,  0, 4, 4}, 	// crowd voice 4 (finale)
	{ 0,            36, 288,    0,    0,   0, 105, 142,  0, 155,  0, 0, 0}, 	// Rhene (finale)
	{ 0,             1,  27,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0}, 	// Okk (temple gate)
	{ 0,             1, 252,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0}, 	// Okk (jail cell)
	{ 0,             1,  25,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0}, 	// Okk (gem room)
	{ 0,             1, 259,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0}, 	// Okk (cliff)
	{ 0,             1, 279,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0}, 	// Okk (dam top)
	{ 0,             1, 273,  -20,  -20,   0,  47, 178,  0, 132,  0, 0, 0}, 	// Okk (human ruins)
	{ 0,             1,  26,  -20,  -20,   0,   8, 178,  0, 171,  0, 0, 0}, 	// puzzle pieces
	{ 0,             1,   0,  -20,  -20,   0,   0,   0,  0,  50,  0, 0, 0}, 	// poker dog 1
	{ 0,             1,   0,  -20,  -20,   0,   0,   0,  0,  82,  0, 0, 0}, 	// poker dog 2
	{ 0,             1,   0,  -20,  -20,   0,   0,   0,  0,  35,  0, 0, 0}, 	// poker dog 3
	{ 0,             9,  74,  -20,  -20,   0,  51, 145,  0,   5,  0, 0, 0}  	// sundial rat
};

} // End of namespace Saga
