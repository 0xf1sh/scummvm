// stdafx.h : Includedatei f�r Standardsystem-Includedateien,
// oder projektspezifische Includedateien, die h�ufig benutzt, aber
// in unregelm��igen Abst�nden ge�ndert werden.
//

#pragma once


#include <iostream>
#include <tchar.h>


typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct CptObj {
	uint16 *data;
	uint32 len;
	char *dbgName;
	uint16 type;
	//uint16 id;
};
// TODO: Verweisen Sie hier auf zus�tzliche Header, die Ihr Programm erfordert
