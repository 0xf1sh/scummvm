/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
/*

 Description:   
 
    Action map module - public module header

 Notes: 
*/

#ifndef SAGA_ACTIONMAP_MOD_H_
#define SAGA_ACTIONMAP_MOD_H_

namespace Saga {

int ACTIONMAP_Register(void);
int ACTIONMAP_Init(void);

int ACTIONMAP_Load(const byte * exmap_res, size_t exmap_res_len);
int ACTIONMAP_Draw(R_SURFACE * ds, int color);

int ACTIONMAP_Free(void);
int ACTIONMAP_Shutdown(void);

} // End of namespace Saga

#endif				/* R_ACTIONMAP_MOD_H_ */
/* end "r_actionmap_mod.h" */
