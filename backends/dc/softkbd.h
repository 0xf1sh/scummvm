/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2004  Marcus Comstedt
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

#ifndef DC_SOFTKBD_H
#define DC_SOFTKBD_H

#include "label.h"

#define SK_NUM_KEYS 61

class SoftKeyboard : public Interactive
{
 private:
  
  Label labels[2][SK_NUM_KEYS];
  byte shiftState;
  int8 keySel;

 public:
  SoftKeyboard();

  void draw(float x, float y, int transp = 0);
  int key(int k, byte &shiftFlags);
};

#endif /* DC_SOFTKBD_H */
