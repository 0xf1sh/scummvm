/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef SIMON_INTERN_H
#define SIMON_INTERN_H

struct Child {
	Child *next;
	uint16 type;
};

struct Child1 : Child {
	uint16 subroutine_id;
	uint16 fr2;
	uint16 array[1];
};

struct Child2 : Child {
	uint16 string_id;
	uint32 avail_props;
	int16 array[1];
};

struct Child3 : Child {
};

struct Child9 : Child {
	uint16 array[4];
};

enum {
	CHILD1_SIZE = sizeof(Child1) - sizeof(uint16),
	CHILD2_SIZE = sizeof(Child2) - sizeof(int16)
};


struct Item {
	uint16 parent;
	uint16 child;
	uint16 sibling;
	int16 unk1;
	int16 unk2;
	int16 unk3;										/* signed int */
	uint16 unk4;
	uint16 xxx_1;									/* unused? */
	Child *children;
};

struct Subroutine {
	uint16 id;										/* subroutine ID */
	uint16 first;									/* offset from subroutine start to first subroutine line */
	Subroutine *next;							/* next subroutine in linked list */
};

struct FillOrCopyDataEntry {
	Item *item;
	uint16 hit_area;
	uint16 xxx_1;
};

struct FillOrCopyData {
	int16 unk1;
	Item *item_ptr;
	FillOrCopyDataEntry e[64];
	int16 unk3, unk4;
	uint16 unk2;
};

struct FillOrCopyStruct {
	byte mode;
	byte flags;
	uint16 x, y;
	uint16 width, height;
	uint16 textColumn, textRow;
	uint8 textColumnOffset, textLength, textMaxLength;
    uint8 fill_color, text_color, unk5;
	FillOrCopyData *fcs_data;
};
// note on text offset: 
// the actual x-coordinate is: textColumn * 8 + textColumnOffset
// the actual y-coordinate is: textRow * 8


enum {
	SUBROUTINE_LINE_SMALL_SIZE = 2,
	SUBROUTINE_LINE_BIG_SIZE = 8,
};

struct SubroutineLine {
	uint16 next;
	int16 cond_a;
	int16 cond_b;
	int16 cond_c;
};

struct TimeEvent {
	uint32 time;
	uint16 subroutine_id;
	TimeEvent *next;
};

struct GameSpecificSettings {
	uint VGA_DELAY_BASE;
	uint TABLE_INDEX_BASE;
	uint TEXT_INDEX_BASE;
	uint NUM_GAME_OFFSETS;
	uint NUM_VIDEO_OP_CODES;
	uint VGA_MEM_SIZE;
	uint TABLES_MEM_SIZE;
	uint NUM_VOICE_RESOURCES;
	uint NUM_EFFECTS_RESOURCES;
	uint MUSIC_INDEX_BASE;
	uint SOUND_INDEX_BASE;
	const char *gme_filename;
	const char *wav_filename;
	const char *voc_filename;
	const char *mp3_filename;
	const char *voc_effects_filename;
	const char *mp3_effects_filename;
 	const char *gamepc_filename;
};

#endif
