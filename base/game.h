/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef BASE_GAME_H
#define BASE_GAME_H

#include "common/str.h"
#include "common/array.h"

struct PlainGameDescriptor {
	const char *gameid;
	const char *description;	// TODO: Rename this to "title" or so
};

struct GameDescriptor {
	Common::String gameid;
	Common::String description;	// TODO: Rename this to "title" or so
	
	GameDescriptor() {}
	GameDescriptor(Common::String g, Common::String d) :
		gameid(g), description(d) {}

	/**
	 * This template constructor allows to easily convert structs that mimic
	 * GameDescriptor to a real GameDescriptor instance.
	 *
	 * Normally, one would just subclass GameDescriptor to get this effect much easier.
	 * However, subclassing a struct turns it into a non-POD type. One of the
	 * consequences is that you can't have inline intialized arrays of that type.
	 * But we heavily rely on those, hence we can't subclass GameDescriptor...
	 */
	template <class T>
	GameDescriptor(const T &g) :
		gameid(g.gameid), description(g.description) {}
};

/** List of games. */
class GameList : public Common::Array<GameDescriptor> {
public:
	GameList() {}
	GameList(const GameList &list) : Common::Array<GameDescriptor>(list) {}
	GameList(const PlainGameDescriptor *g) {
		while (g->gameid) {
			push_back(*g);
			g++;
		}
	}
};



class Plugin;

namespace Base {

// TODO: Find a better place for this function.
GameDescriptor findGame(const Common::String &gameName, const Plugin **plugin = NULL);

} // End of namespace Base


#endif
