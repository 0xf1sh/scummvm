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

// Missing string/stdlib/assert declarations for WinCE 2.xx

#if _WIN32_WCE < 300

#define _HEAPOK 0

void *calloc(size_t n, size_t s);
int isdigit(int c);
int isprint(int c);
int isspace(int c);
char *strrchr(const char *s, int c);
char *strdup(const char *s);
int _stricmp( const char *string1, const char *string2 );
int stricmp( const char *string1, const char *string2 );
void assert( void* expression );
void assert( int expression );
long int strtol(const char *nptr, char **endptr, int base);
char *strdup( const char *s);
int _heapchk();

#endif

#ifdef _WIN32_WCE

void *bsearch(const void *, const void *, size_t,
										 size_t, int (*x) (const void *, const void *));
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdarg.h>
#include <fcntl.h>
#include <conio.h>
#include <malloc.h>
#include <assert.h>
#include <mmsystem.h>
#include <ctype.h>
#include <Winuser.h>
#include <direct.h>

void drawError(char*);

#endif
