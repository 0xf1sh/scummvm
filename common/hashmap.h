/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

// The hash map (associative array) implementation in this file is
// based on code by Andrew Y. Ng, 1996:

/* 
 * Copyright (c) 1998-2003 Massachusetts Institute of Technology. 
 * This code was developed as part of the Haystack research project 
 * (http://haystack.lcs.mit.edu/). Permission is hereby granted, 
 * free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to 
 * the following conditions: 
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE. 
 */

#ifndef COMMON_HASHMAP_H
#define COMMON_HASHMAP_H

#include "common/stdafx.h"
#include "common/func.h"
#include "common/str.h"
#include "common/util.h"

namespace Common { 

// The table sizes ideally are primes. We use a helper function to find
// suitable table sizes.
uint nextTableSize(uint x);


// Enable the following #define if you want to check how many collisions the
// code produces (many collisions indicate either a bad hash function, or a
// hash table that is too small).
//#define DEBUG_HASH_COLLISIONS

/**
 * HashMap<Key,Val> maps objects of type Key to objects of type Val.
 * For each used Key type, we need an "uint hashit(Key,uint)" function
 * that computes a hash for the given Key object and returns it as an
 * an integer from 0 to hashsize-1, and also an "equality functor".
 * that returns true if if its two arguments are to be considered
 * equal. Also, we assume that "=" works on Val objects for assignment.
 *
 * If aa is an HashMap<Key,Val>, then space is allocated each time aa[key] is
 * referenced, for a new key. If the object is const, then an assertion is
 * triggered instead. Hence if you are not sure whether a key is contained in
 * the map, use contains() first to check for its presence.
 */ 
template <class Key, class Val, class HashFunc = Hash<Key>, class EqualFunc = EqualTo<Key> >
class HashMap {
	friend class const_iterator;
private:
#if defined (PALMOS_MODE)
public:
#endif

	struct Node {
		Key _key;
		Val _value;
		Node() {}
		Node(const Key &key) : _key(key) {}
	};

	Node **_arr;	// hashtable of size arrsize.
	uint _arrsize, _nele;
	
	HashFunc _hash;
	EqualFunc _equal;
	
#ifdef DEBUG_HASH_COLLISIONS
	mutable int _collisions, _lookups;
#endif

	int lookup(const Key &key) const;
	void expand_array(uint newsize);

public:
	class const_iterator {
		typedef const HashMap<Key, Val, HashFunc, EqualFunc> * hashmap_t;
		friend class HashMap<Key, Val, HashFunc, EqualFunc>;
	protected:
		uint _idx;
		hashmap_t _hashmap;
		const_iterator(uint idx, hashmap_t hashmap) : _idx(idx), _hashmap(hashmap) {}

		const Node *deref() const {
			assert(_hashmap != 0);
			Node *node = _hashmap->_arr[_idx];
			assert(node != 0);
			return node;
		}

	public:
		const_iterator() : _idx(0), _hashmap(0) {}

		const Node &operator *() const { return *deref(); }
		const Node *operator->() const { return deref(); }
		bool operator ==(const const_iterator &iter) const { return _idx == iter._idx && _hashmap == iter._hashmap; }
		bool operator !=(const const_iterator &iter) const { return !(*this == iter); }
		const_iterator operator ++() {
			assert(_hashmap);
			do {
				_idx++;
			} while (_idx < _hashmap->_arrsize && _hashmap->_arr[_idx] == 0);
			if (_idx >= _hashmap->_arrsize)
				_idx = (uint)-1;
			
			return *this;
		}
	};

	HashMap();
	~HashMap();

	bool contains(const Key &key) const;

	Val &operator [](const Key &key);
	const Val &operator [](const Key &key) const;
	const Val &queryVal(const Key &key) const;

	void clear(bool shrinkArray = 0);

	size_t erase(const Key &key);

	uint size() const { return _nele; }

	const_iterator	begin() const {
		// Find and return the first non-empty entry
		for (uint ctr = 0; ctr < _arrsize; ++ctr) {
			if (_arr[ctr])
				return const_iterator(ctr, this);
		}
		return end();
	}
	const_iterator	end() const {
		return const_iterator((uint)-1, this);
	}
	
	const_iterator	find(const Key &key) const {
		uint ctr = lookup(key);
		if (_arr[ctr])
			return const_iterator(ctr, this);
		return end();
	}
	
	// TODO: insert() method?

	bool empty() const {
		return (_nele == 0);
	}
};

//-------------------------------------------------------
// HashMap functions

template <class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::HashMap() {
	_arrsize = nextTableSize(0);
	_arr = new Node *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(Node *));

	_nele = 0;
	
#ifdef DEBUG_HASH_COLLISIONS
	_collisions = 0;
	_lookups = 0;
#endif
}

template <class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::~HashMap() {
	uint ctr;

	for (ctr = 0; ctr < _arrsize; ctr++)
		if (_arr[ctr] != NULL)
			delete _arr[ctr];

	delete[] _arr;
}

template <class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::clear(bool shrinkArray) {
	for (uint ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] != NULL) {
			delete _arr[ctr];
			_arr[ctr] = NULL;
		}
	}

	if (shrinkArray && _arrsize > nextTableSize(0)) {
		delete[] _arr;

		_arrsize = nextTableSize(0);
		_arr = new Node *[_arrsize];
		assert(_arr != NULL);
		memset(_arr, 0, _arrsize * sizeof(Node *));
	}

	_nele = 0;
}

template <class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::expand_array(uint newsize) {
	assert(newsize > _arrsize);
	Node **old_arr;
	uint old_arrsize, old_nele, ctr, dex;

	old_nele = _nele;
	old_arr = _arr;
	old_arrsize = _arrsize;

	// allocate a new array 
	_arrsize = newsize;
	_arr = new Node *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(Node *));

	_nele = 0;

	// rehash all the old elements
	for (ctr = 0; ctr < old_arrsize; ctr++) {
		if (old_arr[ctr] == NULL)
			continue;

		// Insert the element from the old table into the new table.
		// Since we know that no key exists twice in the old table, we
		// can do this slightly better than by calling lookup, since we
		// don't have to call _equal().
		dex = _hash(old_arr[ctr]->_key) % _arrsize;
		while (_arr[dex] != NULL) {
			dex = (dex + 1) % _arrsize;
		}

		_arr[dex] = old_arr[ctr];
		_nele++;
	}

	// Perform a sanity check: Old number of elements should match the new one!
	assert(_nele == old_nele);

	delete[] old_arr;

	return;
}

template <class Key, class Val, class HashFunc, class EqualFunc>
int HashMap<Key, Val, HashFunc, EqualFunc>::lookup(const Key &key) const {
	uint ctr = _hash(key) % _arrsize;

	while (_arr[ctr] != NULL && !_equal(_arr[ctr]->_key, key)) {
		ctr = (ctr + 1) % _arrsize;

#ifdef DEBUG_HASH_COLLISIONS
		_collisions++;
#endif
	}
	
#ifdef DEBUG_HASH_COLLISIONS
	_lookups++;
	fprintf(stderr, "collisions %d, lookups %d, ratio %f in HashMap %p; size %d num elements %d\n",
		_collisions, _lookups, ((double) _collisions / (double)_lookups),
		(const void *)this, _arrsize, _nele);
#endif

	return ctr;
}

template <class Key, class Val, class HashFunc, class EqualFunc>
bool HashMap<Key, Val, HashFunc, EqualFunc>::contains(const Key &key) const {
	uint ctr = lookup(key);
	return (_arr[ctr] != NULL);
}

template <class Key, class Val, class HashFunc, class EqualFunc>
Val &HashMap<Key, Val, HashFunc, EqualFunc>::operator [](const Key &key) {
	uint ctr = lookup(key);

	if (_arr[ctr] == NULL) {
		_arr[ctr] = new Node(key);
		_nele++;

		// Keep the load factor below 75%.
		if (_nele > _arrsize * 75 / 100) {
			expand_array(nextTableSize(_arrsize));
			ctr = lookup(key);
		}
	}

	return _arr[ctr]->_value;
}

template <class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::operator [](const Key &key) const {
	return queryVal(key);
}

template <class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::queryVal(const Key &key) const {
	uint ctr = lookup(key);
	assert(_arr[ctr] != NULL);
	return _arr[ctr]->_value;
}

template <class Key, class Val, class HashFunc, class EqualFunc>
size_t HashMap<Key, Val, HashFunc, EqualFunc>::erase(const Key &key) {
	// This is based on code in the Wikipedia article on Hash tables.
	uint i = lookup(key);
	if (_arr[i] == NULL)
		return 0; // key wasn't present, so no work has to be done
	uint j = i;
	while (true) {
		j = (j + 1) % _arrsize;
		if (_arr[j] == NULL)
			break;
		uint k = _hash(_arr[j]->_key) % _arrsize;
		if ((j > i && (k <= i || k > j)) ||
		    (j < i && (k <= i && k > j)) ) {
			_arr[i] = _arr[j];
			i = j;
		}
	}
	_arr[i] = NULL;
	return 1;
}

}	// End of namespace Common

#endif
