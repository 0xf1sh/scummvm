/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef CODEC1_H
#define CODEC1_H

#include "config.h"

#ifdef DEBUG
# ifndef NO_DEBUG_CODEC1
#  define DEBUG_CODEC1
# endif
#else
# ifdef DEBUG_CODEC1
#  error DEBUG_CODEC1 defined without DEBUG
# endif
#endif

#include "decoder.h"

/*!	@brief ::decoder for codec 1 and 3.

*/
class Codec1Decoder : public Decoder {
public:
	virtual ~Codec1Decoder();
	bool decode(Blitter &, Chunk &);
};

#endif
