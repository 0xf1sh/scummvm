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

// Verb and hitarea handling
#include "stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"

static const char *const verb_names[] = {
	"Walk to",
	"Look at",
	"Open",
	"Move",

	"Consume",
	"Pick up",
	"Close",
	"Use",

	"Talk to",
	"Remove",
	"Wear",
	"Give"
};

static const char *const verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "with what ?",
	"", "", "", "to whom ?"
};


void SimonState::defocusHitarea()
{
	HitArea *last;
	HitArea *ha;

	if (_game & GAME_SIMON2) {
		if (_bit_array[4] & 0x8000) {
			o_unk_120(202);
			_last_hitarea_2_ptr = NULL;
			return;
		}
	}

	last = _hitarea_ptr_5;

	if (last == _hitarea_ptr_7)
		return;

	hitareaChangedHelper();
	_hitarea_ptr_7 = last;

	if (last != NULL && _hitarea_unk_6 &&
			(ha = findHitAreaByID(200)) && (ha->flags & 0x40) && !(last->flags & 0x40))
		focusVerb(last->id);
}

void SimonState::focusVerb(uint hitarea_id)
{
	uint x;
	const char *txt;

	hitarea_id -= 101;

	CHECK_BOUNDS(hitarea_id, verb_prep_names);

	if (_show_preposition) {
		txt = verb_prep_names[hitarea_id];
	} else {
		txt = verb_names[hitarea_id];
	}
	x = (53 - strlen(txt)) * 3;
	showActionString(x, (const byte *)txt);

}

void SimonState::showActionString(uint x, const byte *string)
{
	FillOrCopyStruct *fcs;

	fcs = _fcs_ptr_array_3[1];
	if (fcs == NULL || fcs->text_color == 0)
		return;

	fcs->textColumn = x >> 3;
	fcs->textColumnOffset = x & 7;

	for (; *string; string++)
		video_putchar(fcs, *string);
}


void SimonState::hitareaChangedHelper()
{
	FillOrCopyStruct *fcs;

	if (_game & GAME_SIMON2) {
		if (_bit_array[4] & 0x8000)
			return;
	}

	fcs = _fcs_ptr_array_3[1];
	if (fcs != NULL && fcs->text_color != 0)
		video_fill_or_copy_from_3_to_2(fcs);

	_last_hitarea_2_ptr = NULL;
	_hitarea_ptr_7 = NULL;
}

HitArea *SimonState::findHitAreaByID(uint hitarea_id)
{
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);

	do {
		if (ha->id == hitarea_id)
			return ha;
	} while (ha++, --count);
	return NULL;
}

HitArea *SimonState::findEmptyHitArea()
{
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);

	do {
		if (ha->flags == 0)
			return ha;
	} while (ha++, --count);
	return NULL;
}

void SimonState::clear_hitarea_bit_0x40(uint hitarea)
{
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL)
		ha->flags &= ~0x40;
}

void SimonState::set_hitarea_bit_0x40(uint hitarea)
{
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->flags |= 0x40;
		ha->flags &= ~2;
		if (hitarea == 102)
			hitarea_proc_1();
	}
}

void SimonState::set_hitarea_x_y(uint hitarea, int x, int y)
{
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->x = x;
		ha->y = y;
	}
}

void SimonState::delete_hitarea(uint hitarea)
{
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->flags = 0;
		if (ha == _last_hitarea_2_ptr)
			defocusHitarea();
		_need_hitarea_recalc++;
	}
}

bool SimonState::is_hitarea_0x40_clear(uint hitarea)
{
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha == NULL)
		return false;
	return (ha->flags & 0x40) == 0;
}

void SimonState::addNewHitArea(int id, int x, int y, int width, int height,
															 int flags, int unk3, Item *item_ptr)
{

	HitArea *ha;
	delete_hitarea(id);

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->flags = flags | 0x20;
	ha->id = ha->layer = id;
	ha->unk3 = unk3;
	ha->item_ptr = item_ptr;

	_need_hitarea_recalc++;
}

void SimonState::hitarea_proc_1()
{
	uint id;
	HitArea *ha;

	if (_game & GAME_SIMON2) {
		id = 2;
		if (!(_bit_array[4] & 0x8000))
			id = (_mouse_y >= 136) ? 102 : 101;
	} else {
		id = (_mouse_y >= 136) ? 102 : 101;

	}

	_hitarea_unk_4 = id;

	ha = findHitAreaByID(id);
	if (ha == NULL)
		return;

	if (ha->flags & 0x40) {
		_hitarea_unk_4 = 999;
		_hitarea_ptr_5 = NULL;
	} else {
		_verb_hitarea = ha->unk3;
		handle_verb_hitarea(ha);
	}
}

void SimonState::handle_verb_hitarea(HitArea * ha)
{
	HitArea *tmp = _hitarea_ptr_5;

	if (ha == tmp)
		return;

	if (!(_game & GAME_SIMON2)) {
		if (tmp != NULL) {
			tmp->flags |= 8;
			video_toggle_colors(tmp, 0xd5, 0xd0, 0xd5, 0xA);
		}

		if (ha->flags & 2)
			video_toggle_colors(ha, 0xda, 0xd5, 0xd5, 5);
		else
			video_toggle_colors(ha, 0xdf, 0xda, 0xda, 0xA);

		ha->flags &= ~(2 + 8);

	} else {
		if (ha->id < 101)
			return;
		_mouse_cursor = ha->id - 101;
		_need_hitarea_recalc++;

	}

	_hitarea_ptr_5 = ha;
}

void SimonState::hitarea_leave(HitArea * ha)
{
	if (!(_game & GAME_SIMON2)) {
		video_toggle_colors(ha, 0xdf, 0xd5, 0xda, 5);
	} else {
		video_toggle_colors(ha, 0xe7, 0xe5, 0xe6, 1);
	}
}

void SimonState::leaveHitAreaById(uint hitarea_id)
{
	HitArea *ha = findHitAreaByID(hitarea_id);
	if (ha)
		hitarea_leave(ha);
}

void SimonState::handle_unk2_hitarea(FillOrCopyStruct *fcs)
{
	uint index;

	index = get_fcs_ptr_3_index(fcs);

	if (fcs->fcs_data->unk1 == 0)
		return;

	lock();
	fcs_unk_proc_1(index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1 - 1, fcs->fcs_data->unk2);
	unlock();
}

void SimonState::handle_unk_hitarea(FillOrCopyStruct *fcs)
{
	uint index;

	index = get_fcs_ptr_3_index(fcs);

	lock();
	fcs_unk_proc_1(index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1 + 1, fcs->fcs_data->unk2);
	unlock();
}

void SimonState::setup_hitarea_from_pos(uint x, uint y, uint mode)
{
	HitArea *best_ha;
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);
	uint16 layer = 0;
	uint16 x_ = x;
	const uint16 y_ = y;

	if (_game & GAME_SIMON2) {
		if (_bit_array[4] & 0x8000 || y < 134) {
			x_ += _x_scroll * 8;
		}
	}

	best_ha = NULL;

	do {
		if (ha->flags & 0x20) {
			if (!(ha->flags & 0x40)) {
				if (x_ >= ha->x && y_ >= ha->y &&
						x_ - ha->x < ha->width && y_ - ha->y < ha->height && layer <= ha->layer) {
					layer = ha->layer;
					best_ha = ha;
				} else {
					if (ha->flags & 2) {
						hitarea_leave(ha);
						ha->flags &= ~2;
					}
				}
			} else {
				ha->flags &= ~2;
			}
		}
	} while (ha++, --count);

	if (best_ha == NULL) {
		defocusHitarea();
		return;
	}

	if (mode != 0 && mode != 3) {
		_last_hitarea = best_ha;
		_variableArray[1] = x;
		_variableArray[2] = y;
	}

	if (best_ha->flags & 4) {
		defocusHitarea();
	} else if (best_ha != _last_hitarea_2_ptr) {
		new_current_hitarea(best_ha);
	}

	if (best_ha->flags & 8 && !(best_ha->flags & 2)) {
		hitarea_leave(best_ha);
		best_ha->flags |= 2;
	}

	return;
}

void SimonState::new_current_hitarea(HitArea * ha)
{
	bool result;

	hitareaChangedHelper();
	if (ha->flags & 1) {
		result = hitarea_proc_2(ha->flags >> 8);
	} else {
		result = hitarea_proc_3(ha->item_ptr);
	}

	if (result)
		_last_hitarea_2_ptr = ha;
}

bool SimonState::hitarea_proc_2(uint a)
{
	uint x;
	const byte *string_ptr;

	if (_game & GAME_SIMON2) {
		if (_bit_array[4] & 0x8000) {
			Subroutine *sub;
			_variableArray[84] = a;
			sub = getSubroutineByID(5003);
			if (sub != NULL)
				startSubroutineEx(sub);
			return true;
		}
	}

	if (a >= 20)
		return false;

	string_ptr = getStringPtrByID(_stringid_array_2[a]);
	// Arisme : hack for long strings in the French version
	if ((strlen((const char*)string_ptr) - 1) <= 53)
		x = (53 - (strlen((const char *)string_ptr) - 1)) * 3;
	else
		x = 0;
	showActionString(x, string_ptr);

	return true;
}

bool SimonState::hitarea_proc_3(Item *item)
{
	Child2 *child2;
	uint x;
	const byte *string_ptr;

	if (item == 0 || item == _dummy_item_2 || item == _dummy_item_3)
		return false;

	child2 = (Child2 *)findChildOfType(item, 2);
	if (child2 == NULL)
		return false;

	string_ptr = getStringPtrByID(child2->string_id);
	// Arisme : hack for long strings in the French version
	if ((strlen((const char*)string_ptr) - 1) <= 53)
		x = (53 - (strlen((const char *)string_ptr) - 1)) * 3;
	else
		x = 0;
	showActionString(x, string_ptr);
	
	return true;
}
