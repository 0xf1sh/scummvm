/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "base/version.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/opcodes.h"
#include "agi/menu.h"
#include "agi/text.h"

namespace Agi {

#define p0	(p[0])
#define p1	(p[1])
#define p2	(p[2])
#define p3	(p[3])
#define p4	(p[4])
#define p5	(p[5])
#define p6	(p[6])

#define game g_agi->game
#define g_sprites g_agi->_sprites
#define g_sound g_agi->_sound
#define g_text g_agi->_text
#define g_gfx g_agi->_gfx
#define g_picture g_agi->_picture

#define ip	cur_logic->cIP
#define vt	game.view_table[p0]

static struct agi_logic *cur_logic;
static AgiEngine *g_agi;

int timer_hack;			/* Workaround for timer loop in MH1 */

#define _v game.vars
#define cmd(x) static void cmd_##x (uint8 *p)

cmd(increment) {
	if (_v[p0] != 0xff)
		++_v[p0];
}

cmd(decrement) {
	if (_v[p0] != 0)
		--_v[p0];
}

cmd(assignn) {
	_v[p0] = p1;
}

cmd(addn) {
	_v[p0] += p1;
}

cmd(subn) {
	_v[p0] -= p1;
}

cmd(assignv) {
	_v[p0] = _v[p1];
}

cmd(addv) {
	_v[p0] += _v[p1];
}

cmd(subv) {
	_v[p0] -= _v[p1];
}

cmd(mul_n) {
	_v[p0] *= p1;
}

cmd(mul_v) {
	_v[p0] *= _v[p1];
}

cmd(div_n) {
	_v[p0] /= p1;
}

cmd(div_v) {
	_v[p0] /= _v[p1];
}

cmd(random) {
	_v[p2] = g_agi->_rnd->getRandomNumber(p1 - p0) + p0;
}

cmd(lindirectn) {
	_v[_v[p0]] = p1;
}

cmd(lindirectv) {
	_v[_v[p0]] = _v[p1];
}

cmd(rindirect) {
	_v[p0] = _v[_v[p1]];
}

cmd(set) {
	g_agi->setflag(*p, true);
}

cmd(reset) {
	g_agi->setflag(*p, false);
}

cmd(toggle) {
	g_agi->setflag(*p, !g_agi->getflag(*p));
}

cmd(set_v) {
	g_agi->setflag(_v[p0], true);
}

cmd(reset_v) {
	g_agi->setflag(_v[p0], false);
}

cmd(toggle_v) {
	g_agi->setflag(_v[p0], !g_agi->getflag(_v[p0]));
}

cmd(new_room) {
	g_agi->new_room(p0);
}

cmd(new_room_f) {
	g_agi->new_room(_v[p0]);
}

cmd(load_view) {
	g_agi->agiLoadResource(rVIEW, p0);
}

cmd(load_logic) {
	g_agi->agiLoadResource(rLOGIC, p0);
}

cmd(load_sound) {
	g_agi->agiLoadResource(rSOUND, p0);
}

cmd(load_view_f) {
	g_agi->agiLoadResource(rVIEW, _v[p0]);
}

cmd(load_logic_f) {
	g_agi->agiLoadResource(rLOGIC, _v[p0]);
}

cmd(discard_view) {
	g_agi->agiUnloadResource(rVIEW, p0);
}

cmd(object_on_anything) {
	vt.flags &= ~(ON_WATER | ON_LAND);
}

cmd(object_on_land) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= ON_LAND;
}

cmd(object_on_water) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= ON_WATER;
}

cmd(observe_horizon) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags &= ~IGNORE_HORIZON;
}

cmd(ignore_horizon) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= IGNORE_HORIZON;
}

cmd(observe_objs) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags &= ~IGNORE_OBJECTS;
}

cmd(ignore_objs) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= IGNORE_OBJECTS;
}

cmd(observe_blocks) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags &= ~IGNORE_BLOCKS;
}

cmd(ignore_blocks) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= IGNORE_BLOCKS;
}

cmd(set_horizon) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	game.horizon = p0;
}

cmd(get_priority) {
	_v[p1] = vt.priority;
}

cmd(set_priority) {
	vt.flags |= FIXED_PRIORITY;
	vt.priority = p1;
}

cmd(set_priority_f) {
	vt.flags |= FIXED_PRIORITY;
	vt.priority = _v[p1];
}

cmd(release_priority) {
	vt.flags &= ~FIXED_PRIORITY;
}

cmd(set_upper_left) {				/* do nothing (AGI 2.917) */
}

cmd(start_update) {
	g_agi->start_update(&vt);
}

cmd(stop_update) {
	g_agi->stop_update(&vt);
}

cmd(current_view) {
	_v[p1] = vt.current_view;
}

cmd(current_cel) {
	_v[p1] = vt.current_cel;
	debugC(4, kDebugLevelScripts, "v%d=%d", p1, _v[p1]);
}

cmd(current_loop) {
	_v[p1] = vt.current_loop;
}

cmd(last_cel) {
	_v[p1] = vt.loop_data->num_cels - 1;
}

cmd(set_cel) {
	g_agi->set_cel(&vt, p1);
	vt.flags &= ~DONTUPDATE;
}

cmd(set_cel_f) {
	g_agi->set_cel(&vt, _v[p1]);
	vt.flags &= ~DONTUPDATE;
}

cmd(set_view) {
	debugC(4, kDebugLevelScripts, "o%d, %d", p0, p1);
	g_agi->set_view(&vt, p1);
}

cmd(set_view_f) {
	g_agi->set_view(&vt, _v[p1]);
}

cmd(set_loop) {
	g_agi->set_loop(&vt, p1);
}

cmd(set_loop_f) {
	g_agi->set_loop(&vt, _v[p1]);
}

cmd(number_of_loops) {
	_v[p1] = vt.num_loops;
}

cmd(fix_loop) {
	vt.flags |= FIX_LOOP;
}

cmd(release_loop) {
	vt.flags &= ~FIX_LOOP;
}

cmd(step_size) {
	vt.step_size = _v[p1];
}

cmd(step_time) {
	vt.step_time = vt.step_time_count = _v[p1];
}

cmd(cycle_time) {
	vt.cycle_time = vt.cycle_time_count = _v[p1];
}

cmd(stop_cycling) {
	vt.flags &= ~CYCLING;
}

cmd(start_cycling) {
	vt.flags |= CYCLING;
}

cmd(normal_cycle) {
	vt.cycle = CYCLE_NORMAL;
	vt.flags |= CYCLING;
}

cmd(reverse_cycle) {
	vt.cycle = CYCLE_REVERSE;
	vt.flags |= CYCLING;
}

cmd(set_dir) {
	vt.direction = _v[p1];
}

cmd(get_dir) {
	_v[p1] = vt.direction;
}

cmd(get_room_f) {
	_v[p1] = g_agi->object_get_location(_v[p0]);
}

cmd(put) {
	g_agi->object_set_location(p0, _v[p1]);
}

cmd(put_f) {
	g_agi->object_set_location(_v[p0], _v[p1]);
}

cmd(drop) {
	g_agi->object_set_location(p0, 0);
}

cmd(get) {
	g_agi->object_set_location(p0, EGO_OWNED);
}

cmd(get_f) {
	g_agi->object_set_location(_v[p0], EGO_OWNED);
}

cmd(word_to_string) {
	strcpy(game.strings[p0], game.ego_words[p1].word);
}

cmd(open_dialogue) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	game.has_window = true;
}

cmd(close_dialogue) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	game.has_window = false;
}

cmd(close_window) {
	g_agi->close_window();
}

cmd(status_line_on) {
	game.status_line = true;
	g_agi->write_status();
}

cmd(status_line_off) {
	game.status_line = false;
	g_agi->write_status();
}

cmd(show_obj) {
	g_sprites->show_obj(p0);
}

cmd(show_obj_v) {
	g_sprites->show_obj(_v[p0]);
}

cmd(sound) {
	g_sound->start_sound(p0, p1);
}

cmd(stop_sound) {
	g_sound->stop_sound();
}

cmd(menu_input) {
	g_agi->new_input_mode(INPUT_MENU);
}

cmd(enable_item) {
	g_agi->menu->set_item(p0, true);
}

cmd(disable_item) {
	g_agi->menu->set_item(p0, false);
}

cmd(submit_menu) {
	g_agi->menu->submit();
}

cmd(set_scan_start) {
	cur_logic->sIP = cur_logic->cIP;
}

cmd(reset_scan_start) {
	cur_logic->sIP = 2;
}

cmd(save_game) {
	game.simple_save ? g_agi->saveGameSimple() : g_agi->saveGameDialog();
}

cmd(load_game) {
	assert(1);
	game.simple_save ? g_agi->loadGameSimple() : g_agi->loadGameDialog();
}

cmd(init_disk) {				/* do nothing */
}

cmd(log) {				/* do nothing */
}

cmd(trace_on) {				/* do nothing */
}

cmd(trace_info) {				/* do nothing */
}

cmd(show_mem) {
	g_agi->message_box("Enough memory");
}

cmd(init_joy) { /* do nothing */ ;
}

cmd(script_size) {
	report("script.size(%d)\n", p0);
}

cmd(cancel_line) {
	report("cancel.line\n");
}

cmd(obj_status_f) {
	report("obj.status.f\n");
}

/* unknown commands:
 * unk_170: Force savegame name -- j5
 * unk_171: script save -- j5
 * unk_172: script restore -- j5
 * unk_173: Activate keypressed control (ego only moves while key is pressed)
 * unk_174: Change priority table (used in KQ4) -- j5
 * unk_177: Disable menus completely -- j5
 * unk_181: Deactivate keypressed control (default control of ego)
 */
cmd(set_simple) {
	game.simple_save = true;
}

cmd(pop_script) {
	report("pop.script\n");
}

cmd(hold_key) {
	report("hold.key\n");
}

cmd(discard_sound) {
	report("discard.sound\n");
}

cmd(hide_mouse) {
	report("hide.mouse\n");
}

cmd(allow_menu) {
	report("allow.menu\n");
}

cmd(show_mouse) {
	report("show.mouse\n");
}

cmd(fence_mouse) {
	report("fence.mouse\n");
}

cmd(release_key) {
	report("release.key\n");
}

cmd(adj_ego_move_to_x_y) {
	game.view_table[0].flags |= ADJ_EGO_XY;
}

cmd(parse) {
	_v[V_word_not_found] = 0;
	g_agi->setflag(F_entered_cli, false);
	g_agi->setflag(F_said_accepted_input, false);

	g_agi->dictionary_words(g_agi->agi_sprintf(game.strings[p0]));
}

cmd(call) {
	int old_cIP;
	int old_lognum;

	/* CM: we don't save sIP because set.scan.start can be
	 *     used in a called script (fixes xmas demo)
	 */
	old_cIP = cur_logic->cIP;
	old_lognum = game.lognum;

	g_agi->run_logic(p0);

	game.lognum = old_lognum;
	cur_logic = &game.logics[game.lognum];
	cur_logic->cIP = old_cIP;
}

cmd(call_f) {
	cmd_call(&_v[p0]);
}

cmd(draw_pic) {
	debugC(6, kDebugLevelScripts, "=== draw pic %d ===", _v[p0]);
	g_sprites->erase_both();
	g_picture->decode_picture(_v[p0], true);
	g_sprites->blit_both();
	game.picture_shown = 0;
	debugC(6, kDebugLevelScripts, "--- end of draw pic %d ---", _v[p0]);
}

cmd(show_pic) {
	debugC(6, kDebugLevelScripts, "=== show pic ===");
	g_agi->setflag(F_output_mode, false);
	cmd_close_window(NULL);
	g_picture->show_pic();
	game.picture_shown = 1;
	debugC(6, kDebugLevelScripts, "--- end of show pic ---");
}

cmd(load_pic) {
	g_sprites->erase_both();
	g_agi->agiLoadResource(rPICTURE, _v[p0]);
	g_sprites->blit_both();
}

cmd(discard_pic) {
	debugC(6, kDebugLevelScripts, "--- discard pic ---");
	/* do nothing */
}

cmd(overlay_pic) {
	debugC(6, kDebugLevelScripts, "--- overlay pic ---");
	g_sprites->erase_both();
	g_picture->decode_picture(_v[p0], false);
	g_sprites->blit_both();
	game.picture_shown = 0;
	g_sprites->commit_both();
}

cmd(show_pri_screen) {
	g_agi->_debug.priority = 1;
	g_sprites->erase_both();
	g_picture->show_pic();
	g_sprites->blit_both();
	g_agi->wait_key();
	g_agi->_debug.priority = 0;
	g_sprites->erase_both();
	g_picture->show_pic();
	g_sprites->blit_both();
}

cmd(animate_obj) {
	if (vt.flags & ANIMATED)
		return;

	debugC(4, kDebugLevelScripts, "animate vt entry #%d", p0);
	vt.flags = ANIMATED | UPDATE | CYCLING;
	vt.motion = MOTION_NORMAL;
	vt.cycle = CYCLE_NORMAL;
	vt.direction = 0;
}

cmd(unanimate_all) {
	int i;
	for (i = 0; i < MAX_VIEWTABLE; i++)
		game.view_table[i].flags &= ~(ANIMATED | DRAWN);
}

cmd(draw) {
	if (vt.flags & DRAWN)
		return;

	if (vt.y_size <= 0 || vt.x_size <= 0)
		return;

	debugC(4, kDebugLevelScripts, "draw entry %d", vt.entry);

	vt.flags |= UPDATE;
	if (g_agi->agiGetRelease() >= 0x3000) {
		g_agi->set_loop(&vt, vt.current_loop);
		g_agi->set_cel(&vt, vt.current_cel);
	}
	g_agi->fix_position(p0);
	vt.x_pos2 = vt.x_pos;
	vt.y_pos2 = vt.y_pos;
	vt.cel_data_2 = vt.cel_data;
	g_sprites->erase_upd_sprites();
	vt.flags |= DRAWN;

	if (g_agi->agiGetRelease() <= 0x2440)	/* See bug #546562 */
		vt.flags |= ANIMATED;

	g_sprites->blit_upd_sprites();
	vt.flags &= ~DONTUPDATE;

	g_sprites->commit_block(vt.x_pos, vt.y_pos - vt.y_size + 1, vt.x_pos + vt.x_size - 1, vt.y_pos);

	debugC(4, kDebugLevelScripts, "vt entry #%d flags = %02x", p0, vt.flags);
}

cmd(erase) {
	if (~vt.flags & DRAWN)
		return;

	g_sprites->erase_upd_sprites();
	if (vt.flags & UPDATE) {
		vt.flags &= ~DRAWN;
	} else {
		g_sprites->erase_nonupd_sprites();
		vt.flags &= ~DRAWN;
		g_sprites->blit_nonupd_sprites();
	}
	g_sprites->blit_upd_sprites();

	g_sprites->commit_block(vt.x_pos, vt.y_pos - vt.y_size + 1, vt.x_pos + vt.x_size - 1, vt.y_pos);
}

cmd(position) {
	vt.x_pos = vt.x_pos2 = p1;
	vt.y_pos = vt.y_pos2 = p2;
}

cmd(position_f) {
	vt.x_pos = vt.x_pos2 = _v[p1];
	vt.y_pos = vt.y_pos2 = _v[p2];
}

cmd(get_posn) {
	game.vars[p1] = (unsigned char)vt.x_pos;
	game.vars[p2] = (unsigned char)vt.y_pos;
}

cmd(reposition) {
	int dx = (int8) _v[p1], dy = (int8) _v[p2];

	debugC(4, kDebugLevelScripts, "dx=%d, dy=%d", dx, dy);
	vt.flags |= UPDATE_POS;

	if (dx < 0 && vt.x_pos < -dx)
		vt.x_pos = 0;
	else
		vt.x_pos += dx;

	if (dy < 0 && vt.y_pos < -dy)
		vt.y_pos = 0;
	else
		vt.y_pos += dy;

	g_agi->fix_position(p0);
}

cmd(reposition_to) {
	vt.x_pos = p1;
	vt.y_pos = p2;
	vt.flags |= UPDATE_POS;
	g_agi->fix_position(p0);
}

cmd(reposition_to_f) {
	vt.x_pos = _v[p1];
	vt.y_pos = _v[p2];
	vt.flags |= UPDATE_POS;
	g_agi->fix_position(p0);
}

cmd(add_to_pic) {
	g_sprites->add_to_pic(p0, p1, p2, p3, p4, p5, p6);
}

cmd(add_to_pic_f) {
	g_sprites->add_to_pic(_v[p0], _v[p1], _v[p2], _v[p3], _v[p4], _v[p5], _v[p6]);
}

cmd(force_update) {
	g_sprites->erase_both();
	g_sprites->blit_both();
	g_sprites->commit_both();
}

cmd(reverse_loop) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = CYCLE_REV_LOOP;
	vt.flags |= (DONTUPDATE | UPDATE | CYCLING);
	vt.parm1 = p1;
	g_agi->setflag(p1, false);
}

cmd(end_of_loop) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = CYCLE_END_OF_LOOP;
	vt.flags |= (DONTUPDATE | UPDATE | CYCLING);
	vt.parm1 = p1;
	g_agi->setflag(p1, false);
}

cmd(block) {
	debugC(4, kDebugLevelScripts, "x1=%d, y1=%d, x2=%d, y2=%d", p0, p1, p2, p3);
	game.block.active = true;
	game.block.x1 = p0;
	game.block.y1 = p1;
	game.block.x2 = p2;
	game.block.y2 = p3;
}

cmd(unblock) {
	game.block.active = false;
}

cmd(normal_motion) {
	vt.motion = MOTION_NORMAL;
}

cmd(stop_motion) {
	vt.direction = 0;
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		/* ego only */
		_v[V_ego_dir] = 0;
		game.player_control = false;
	}
}

cmd(start_motion) {
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		/* ego only */
		_v[V_ego_dir] = 0;
		game.player_control = true;
	}
}

cmd(player_control) {
	game.player_control = true;
	game.view_table[0].motion = MOTION_NORMAL;
}

cmd(program_control) {
	game.player_control = false;
}

cmd(follow_ego) {
	vt.motion = MOTION_FOLLOW_EGO;
	vt.parm1 = p1 > vt.step_size ? p1 : vt.step_size;
	vt.parm2 = p2;
	vt.parm3 = 0xff;
	g_agi->setflag(p2, false);
	vt.flags |= UPDATE;
}

cmd(move_obj) {
	/* _D (_D_WARN "o=%d, x=%d, y=%d, s=%d, f=%d", p0, p1, p2, p3, p4); */

	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = p1;
	vt.parm2 = p2;
	vt.parm3 = vt.step_size;
	vt.parm4 = p4;

	if (p3 != 0)
		vt.step_size = p3;

	g_agi->setflag(p4, false);
	vt.flags |= UPDATE;

	if (p0 == 0)
		game.player_control = false;

	/* AGI 2.272 (ddp, xmas) doesn't call move_obj! */
	if (g_agi->agiGetRelease() > 0x2272)
		g_agi->move_obj(&vt);
}

cmd(move_obj_f) {
	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = _v[p1];
	vt.parm2 = _v[p2];
	vt.parm3 = vt.step_size;
	vt.parm4 = p4;

	if (_v[p3] != 0)
		vt.step_size = _v[p3];

	g_agi->setflag(p4, false);
	vt.flags |= UPDATE;

	if (p0 == 0)
		game.player_control = false;

	/* AGI 2.272 (ddp, xmas) doesn't call move_obj! */
	if (g_agi->agiGetRelease() > 0x2272)
		g_agi->move_obj(&vt);
}

cmd(wander) {
	if (p0 == 0)
		game.player_control = false;
	vt.motion = MOTION_WANDER;
	vt.flags |= UPDATE;
}

cmd(set_game_id) {
	if (cur_logic->texts && (p0 - 1) <= cur_logic->num_texts)
		strncpy(game.id, cur_logic->texts[p0 - 1], 8);
	else
		game.id[0] = 0;

	report("Game ID: \"%s\"\n", game.id);
}

cmd(pause) {
	int tmp = game.clock_enabled;
	const char *b[] = { "Continue", NULL };

	game.clock_enabled = false;
	g_agi->selection_box("  Game is paused.  \n\n\n", b);
	game.clock_enabled = tmp;
}

cmd(set_menu) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, cur_logic->num_texts);
	if (cur_logic->texts != NULL && p0 < cur_logic->num_texts)
		g_agi->menu->add(cur_logic->texts[p0 - 1]);
}

cmd(set_menu_item) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, cur_logic->num_texts);
	if (cur_logic->texts != NULL && p0 <= cur_logic->num_texts)
		g_agi->menu->add_item(cur_logic->texts[p0 - 1], p1);
}

cmd(version) {
	char ver_msg[64];
	char ver2_msg[] =
	    "\n"
	    "                               \n\n"
	    "  Emulating Sierra AGI v%x.%03x\n";
	char ver3_msg[] =
	    "\n"
	    "                             \n\n"
	    "  Emulating AGI v%x.002.%03x\n";
	/* no Sierra as it wraps textbox */
	char *r, *q;
	int ver, maj, min;
	char msg[256];
	int gap;
	int len;

	sprintf(ver_msg, TITLE " v%s", gScummVMVersion);

	ver = g_agi->agiGetRelease();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	q = maj == 2 ? ver2_msg : ver3_msg;
	r = strchr(q + 1, '\n');

	/* insert our version into the other version */
	len = strlen(ver_msg);
	gap = r - q;
	if(gap < 0)
		gap = 0;
	else
		gap = (gap - len) / 2;

	strncpy(q + 1 + gap, ver_msg, strlen(ver_msg));
	sprintf(msg, q, maj, min);
	g_agi->message_box(msg);
}

cmd(configure_screen) {
	game.line_min_print = p0;
	game.line_user_input = p1;
	game.line_status = p2;
}

cmd(text_screen) {
	debugC(4, kDebugLevelScripts, "switching to text mode");
	game.gfx_mode = false;
	/*
	 * Simulates the "bright background bit" of the PC video
	 * controller.
	 */
	if (game.color_bg)
		game.color_bg |= 0x08;
	g_gfx->clearScreen(game.color_bg);
}

cmd(graphics) {
	debugC(4, kDebugLevelScripts, "switching to graphics mode");
	if (!game.gfx_mode) {
		game.gfx_mode = true;
		g_gfx->clearScreen(0);
		g_picture->show_pic();
		g_agi->write_status();
		g_agi->write_prompt();
	}
}

cmd(set_text_attribute) {
	game.color_fg = p0;
	game.color_bg = p1;

	if (game.gfx_mode) {
		if (game.color_bg != 0) {
			game.color_fg = 0;
			game.color_bg = 15;
		}
	}
}

cmd(status) {
	g_agi->inventory();
}

cmd(quit) {
	const char *buttons[] = { "Quit", "Continue", NULL };

	g_sound->stop_sound();
	if (p0) {
		game.quit_prog_now = true;
	} else {
		if (g_agi->selection_box
				(" Quit the game, or continue? \n\n\n", buttons) == 0) {
			game.quit_prog_now = true;
		}
	}
}

cmd(restart_game) {
	const char *buttons[] = { "Restart", "Continue", NULL };
	int sel;

	g_sound->stop_sound();
	sel = g_agi->getflag(F_auto_restart) ? 1 :
		g_agi->selection_box(" Restart game, or continue? \n\n\n", buttons);

	if (sel == 0) {
		game.quit_prog_now = 0xff;
		g_agi->setflag(F_restart_game, true);
		g_agi->menu->enable_all();
	}
}

cmd(distance) {
	int16 x1, y1, x2, y2, d;
	struct vt_entry *v0 = &game.view_table[p0];
	struct vt_entry *v1 = &game.view_table[p1];

	if (v0->flags & DRAWN && v1->flags & DRAWN) {
		x1 = v0->x_pos + v0->x_size / 2;
		y1 = v0->y_pos;
		x2 = v1->x_pos + v1->x_size / 2;
		y2 = v1->y_pos;
		d = abs(x1 - x2) + abs(y1 - y2);
		if (d > 0xfe)
			d = 0xfe;
	} else {
		d = 0xff;
	}
	_v[p2] = (unsigned char)d;
}

cmd(accept_input) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "input normal");
	g_agi->new_input_mode(INPUT_NORMAL);
	game.input_enabled = true;
}

cmd(prevent_input) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "no input");
	g_agi->new_input_mode(INPUT_NONE);
	game.input_enabled = false;
}

cmd(get_string) {
	int tex, row, col;

	debugC(4, kDebugLevelScripts, "%d %d %d %d %d", p0, p1, p2, p3, p4);

	tex = p1 - 1;
	row = p2;
	col = p3;

	/* Workaround for SQLC bug.
	 * See bug #792125 for details
	 */
	if (row > 24)
		row = 24;
	if (col > 39)
		col = 39;

	g_agi->new_input_mode(INPUT_GETSTRING);

	if (cur_logic->texts != NULL && cur_logic->num_texts >= tex) {
		int len = strlen(cur_logic->texts[tex]);
		g_agi->print_text(cur_logic->texts[tex], 0, col, row, len, game.color_fg, game.color_bg);
		g_agi->get_string(col + len - 1, row, p4, p0);

		/* SGEO: display input char */
		g_gfx->printCharacter((col + len), row, game.cursor_char, game.color_fg, game.color_bg);
	}

	do {
		g_agi->main_cycle();
	} while (game.input_mode == INPUT_GETSTRING);
}

cmd(get_num) {
	debugC(4, kDebugLevelScripts, "%d %d", p0, p1);
	g_agi->new_input_mode(INPUT_GETSTRING);

	if (cur_logic->texts != NULL && cur_logic->num_texts >= (p0 - 1)) {
		int len = strlen(cur_logic->texts[p0 - 1]);
		g_agi->print_text(cur_logic->texts[p0 - 1], 0, 0, 22, len, game.color_fg, game.color_bg);
		g_agi->get_string(len - 1, 22, 3, MAX_STRINGS);

		/* CM: display input char */
		g_gfx->printCharacter((p3 + len), 22, game.cursor_char, game.color_fg, game.color_bg);
	}

	do {
		g_agi->main_cycle();
	} while (game.input_mode == INPUT_GETSTRING);

	_v[p1] = atoi(game.strings[MAX_STRINGS]);
	debugC(4, kDebugLevelScripts, "[%s] -> %d", game.strings[MAX_STRINGS], _v[p1]);
	g_agi->clear_lines(22, 22, game.color_bg);
	g_agi->flush_lines(22, 22);
}

cmd(set_cursor_char) {
	if (cur_logic->texts != NULL && (p0 - 1) <= cur_logic->num_texts) {
		game.cursor_char = *cur_logic->texts[p0 - 1];
	} else {
		/* default */
		game.cursor_char = '_';
	}
}

cmd(set_key) {
	int key;

	debugC(4, kDebugLevelScripts, "%d %d %d", p0, p1, p2);

	if (game.ev_keyp[p2].data != 0)	/* TBC sets c23 (ESC) twice! */
		return;

	key = 256 * p1 + p0;
	game.ev_keyp[p2].data = key;
	game.ev_keyp[p2].occured = false;
}

cmd(set_string) {
	/* CM: to avoid crash in Groza (str = 150) */
	if (p0 > MAX_STRINGS)
		return;
	strcpy(game.strings[p0], cur_logic->texts[p1 - 1]);
}

cmd(display) {
	g_agi->print_text(cur_logic->texts[p2 - 1], p1, 0, p0, 40, game.color_fg, game.color_bg);
}

cmd(display_f) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	g_agi->print_text(cur_logic->texts[_v[p2] - 1], _v[p1], 0, _v[p0], 40, game.color_fg, game.color_bg);
}

cmd(clear_text_rect) {
	int c, x1, y1, x2, y2;

	if ((c = p4) != 0)
		c = 15;
	x1 = p1 * CHAR_COLS;
	y1 = p0 * CHAR_LINES;
	x2 = (p3 + 1) * CHAR_COLS - 1;
	y2 = (p2 + 1) * CHAR_LINES - 1;

	/* Added to prevent crash with x2 = 40 in the iigs demo */
	if (x1 > GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (x2 > GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y1 > GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (y2 > GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	g_gfx->drawRectangle(x1, y1, x2, y2, c);
	g_gfx->flushBlock(x1, y1, x2, y2);
}

cmd(toggle_monitor) {
	report("toggle.monitor\n");
}

cmd(echo_line) {
	strcpy((char *)game.input_buffer, (const char *)game.echo_buffer);
	game.cursor_pos = strlen((char *)game.input_buffer);
	game.has_prompt = 0;
}

cmd(clear_lines) {
	uint8 l;

	/* Residence 44 calls clear.lines(24,0,0), see bug #558423 */
	l = p1 ? p1 : p0;

	g_agi->clear_lines(p0, l, p2);
	g_agi->flush_lines(p0, l);
}

cmd(print) {
	int n = p0 < 1 ? 1 : p0;
	g_agi->print(cur_logic->texts[n - 1], 0, 0, 0);
}

cmd(print_f) {
	int n = _v[p0] < 1 ? 1 : _v[p0];
	g_agi->print(cur_logic->texts[n - 1], 0, 0, 0);
}

cmd(print_at) {
	int n = p0 < 1 ? 1 : p0;
	debugC(4, kDebugLevelScripts, "%d %d %d %d", p0, p1, p2, p3);
	g_agi->print(cur_logic->texts[n - 1], p1, p2, p3);
}

cmd(print_at_v) {
	int n = _v[p0] < 1 ? 1 : _v[p0];
	g_agi->print(cur_logic->texts[n - 1], p1, p2, p3);
}

cmd(push_script) {
	if (g_agi->opt.agimouse) {
		game.vars[27] = g_mouse.button;
		game.vars[28] = g_mouse.x / 2;
		game.vars[29] = g_mouse.y;
	} else
		report("push.script\n");
}

cmd(set_pri_base) {
	int i, x, pri;

	report("Priority base set to %d\n", p0);

	/* game.alt_pri = true; */
	x = (_HEIGHT - p0) * _HEIGHT / 10;

	for (i = 0; i < _HEIGHT; i++) {
		pri = (i - p0) < 0 ? 4 : (i - p0) * _HEIGHT / x + 5;
		if (pri > 15)
			pri = 15;
		game.pri_table[i] = pri;
	}
}

cmd(mouse_posn) {
	_v[p0] = WIN_TO_PIC_X(g_mouse.x);
	_v[p1] = WIN_TO_PIC_Y(g_mouse.y);
}

cmd(shake_screen) {
	int i;

	/* AGI Mouse 1.1 uses shake.screen values between 100 and 109 to
	 * set the palette.
	 */
	if (g_agi->opt.agimouse && p0 >= 100 && p0 < 110) {
		report("not implemented: AGI Mouse palettes\n");
		return;
	} else
		g_gfx->shakeStart();

	g_sprites->commit_both();		/* Fixes SQ1 demo */
	for (i = 4 * p0; i; i--) {
		g_gfx->shakeScreen(i & 1);
		g_gfx->flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
		g_agi->main_cycle();
	}
	g_gfx->shakeEnd();
}

static void (*agi_command[183]) (uint8 *) = {
	NULL,			/* 0x00 */
	cmd_increment,
	cmd_decrement,
	cmd_assignn,
	cmd_assignv,
	cmd_addn,
	cmd_addv,
	cmd_subn,
	cmd_subv,	/* 0x08 */
	cmd_lindirectv,
	cmd_rindirect,
	cmd_lindirectn,
	cmd_set,
	cmd_reset,
	cmd_toggle,
	cmd_set_v,
	cmd_reset_v,	/* 0x10 */
	cmd_toggle_v,
	cmd_new_room,
	cmd_new_room_f,
	cmd_load_logic,
	cmd_load_logic_f,
	cmd_call,
	cmd_call_f,
	cmd_load_pic,	/* 0x18 */
	cmd_draw_pic,
	cmd_show_pic,
	cmd_discard_pic,
	cmd_overlay_pic,
	cmd_show_pri_screen,
	cmd_load_view,
	cmd_load_view_f,
	cmd_discard_view,	/* 0x20 */
	cmd_animate_obj,
	cmd_unanimate_all,
	cmd_draw,
	cmd_erase,
	cmd_position,
	cmd_position_f,
	cmd_get_posn,
	cmd_reposition,	/* 0x28 */
	cmd_set_view,
	cmd_set_view_f,
	cmd_set_loop,
	cmd_set_loop_f,
	cmd_fix_loop,
	cmd_release_loop,
	cmd_set_cel,
	cmd_set_cel_f,	/* 0x30 */
	cmd_last_cel,
	cmd_current_cel,
	cmd_current_loop,
	cmd_current_view,
	cmd_number_of_loops,
	cmd_set_priority,
	cmd_set_priority_f,
	cmd_release_priority,	/* 0x38 */
	cmd_get_priority,
	cmd_stop_update,
	cmd_start_update,
	cmd_force_update,
	cmd_ignore_horizon,
	cmd_observe_horizon,
	cmd_set_horizon,
	cmd_object_on_water,	/* 0x40 */
	cmd_object_on_land,
	cmd_object_on_anything,
	cmd_ignore_objs,
	cmd_observe_objs,
	cmd_distance,
	cmd_stop_cycling,
	cmd_start_cycling,
	cmd_normal_cycle,	/* 0x48 */
	cmd_end_of_loop,
	cmd_reverse_cycle,
	cmd_reverse_loop,
	cmd_cycle_time,
	cmd_stop_motion,
	cmd_start_motion,
	cmd_step_size,
	cmd_step_time,	/* 0x50 */
	cmd_move_obj,
	cmd_move_obj_f,
	cmd_follow_ego,
	cmd_wander,
	cmd_normal_motion,
	cmd_set_dir,
	cmd_get_dir,
	cmd_ignore_blocks,	/* 0x58 */
	cmd_observe_blocks,
	cmd_block,
	cmd_unblock,
	cmd_get,
	cmd_get_f,
	cmd_drop,
	cmd_put,
	cmd_put_f,	/* 0x60 */
	cmd_get_room_f,
	cmd_load_sound,
	cmd_sound,
	cmd_stop_sound,
	cmd_print,
	cmd_print_f,
	cmd_display,
	cmd_display_f,	/* 0x68 */
	cmd_clear_lines,
	cmd_text_screen,
	cmd_graphics,
	cmd_set_cursor_char,
	cmd_set_text_attribute,
	cmd_shake_screen,
	cmd_configure_screen,
	cmd_status_line_on,	/* 0x70 */
	cmd_status_line_off,
	cmd_set_string,
	cmd_get_string,
	cmd_word_to_string,
	cmd_parse,
	cmd_get_num,
	cmd_prevent_input,
	cmd_accept_input,	/* 0x78 */
	cmd_set_key,
	cmd_add_to_pic,
	cmd_add_to_pic_f,
	cmd_status,
	cmd_save_game,
	cmd_load_game,
	cmd_init_disk,
	cmd_restart_game,	/* 0x80 */
	cmd_show_obj,
	cmd_random,
	cmd_program_control,
	cmd_player_control,
	cmd_obj_status_f,
	cmd_quit,
	cmd_show_mem,
	cmd_pause,	/* 0x88 */
	cmd_echo_line,
	cmd_cancel_line,
	cmd_init_joy,
	cmd_toggle_monitor,
	cmd_version,
	cmd_script_size,
	cmd_set_game_id,
	cmd_log,	/* 0x90 */
	cmd_set_scan_start,
	cmd_reset_scan_start,
	cmd_reposition_to,
	cmd_reposition_to_f,
	cmd_trace_on,
	cmd_trace_info,
	cmd_print_at,
	cmd_print_at_v,	/* 0x98 */
	cmd_discard_view,
	cmd_clear_text_rect,
	cmd_set_upper_left,
	cmd_set_menu,
	cmd_set_menu_item,
	cmd_submit_menu,
	cmd_enable_item,
	cmd_disable_item,	/* 0xa0 */
	cmd_menu_input,
	cmd_show_obj_v,
	cmd_open_dialogue,
	cmd_close_dialogue,
	cmd_mul_n,
	cmd_mul_v,
	cmd_div_n,
	cmd_div_v,	/* 0xa8 */
	cmd_close_window,
	cmd_set_simple,
	cmd_push_script,
	cmd_pop_script,
	cmd_hold_key,
	cmd_set_pri_base,
	cmd_discard_sound,
	cmd_hide_mouse,	/* 0xb0 */
	cmd_allow_menu,
	cmd_show_mouse,
	cmd_fence_mouse,
	cmd_mouse_posn,
	cmd_release_key,
	cmd_adj_ego_move_to_x_y
};

#define CMD_BSIZE 12

/**
 * Execute a logic script
 * @param n  Number of the logic resource to execute
 */
int AgiEngine::run_logic(int n) {
	uint8 op = 0;
	uint8 p[CMD_BSIZE] = { 0 };
	uint8 *code = NULL;
	g_agi = this;
	int num = 0;

	/* If logic not loaded, load it */
	if (~game.dir_logic[n].flags & RES_LOADED) {
		debugC(4, kDebugLevelScripts, "logic %d not loaded!", n);
		agiLoadResource(rLOGIC, n);
	}

	game.lognum = n;
	cur_logic = &game.logics[game.lognum];

	code = cur_logic->data;
	cur_logic->cIP = cur_logic->sIP;

	timer_hack = 0;
	while (ip < game.logics[n].size && !game.quit_prog_now) {
		if (g_agi->_debug.enabled) {
			if (g_agi->_debug.steps > 0) {
				if (g_agi->_debug.logic0 || n) {
					debug_console(n, lCOMMAND_MODE, NULL);
					g_agi->_debug.steps--;
				}
			} else {
				_sprites->blit_both();
				do {
					main_cycle();
				} while (!g_agi->_debug.steps && g_agi->_debug.enabled);
				_sprites->erase_both();
			}
		}

		switch (op = *(code + ip++)) {
		case 0xff:	/* if (open/close) */
			test_if_code(n);
			break;
		case 0xfe:	/* goto */
			/* +2 covers goto size */
			ip += 2 + ((int16)READ_LE_UINT16(code + ip));
			/* timer must keep running even in goto loops,
			 * but AGI engine can't do that :(
			 */
			if (timer_hack > 20) {
				g_gfx->pollTimer();
				g_agi->update_timer();
				timer_hack = 0;
			}
			break;
		case 0x00:	/* return */
			return 1;
		default:
			num = logic_names_cmd[op].num_args;
			memmove(p, code + ip, num);
			memset(p + num, 0, CMD_BSIZE - num);

			debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logic_names_cmd[op].name, p[0], p[1], p[2]);
			agi_command[op] (p);
			ip += num;
		}

		if (game.exit_all_logics)
			break;
	}

	return 0;		/* after executing new.room() */
}

void AgiEngine::execute_agi_command(uint8 op, uint8 *p) {
	debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logic_names_cmd[op].name, p[0], p[1], p[2]);
	g_agi = this;

	agi_command[op] (p);
}

} // End of namespace Agi
