#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdio.h>
#include <stdlib.h>


typedef __int8 I8;
typedef unsigned __int8 UI8;
typedef __int16 I16;
typedef unsigned __int16 UI16;
typedef __int32 I32;
typedef unsigned __int32 UI32;
typedef unsigned long ULONG;
typedef char CHAR_;
typedef float F32;
typedef double F64;
typedef UI8 PIXCOMP;


#define ARRAY_SIZE( arr ) (sizeof( arr )/sizeof( arr[0] ))

#define FMalloc( iSize ) malloc( iSize )
#define FFree( lpMem ) free( lpMem )


struct RGB
{
	PIXCOMP r, g, b;
};

struct RGBA
{
	PIXCOMP r, g, b, a;
};

#define DWORD_ALIG( w ) ((w * 24 +31) & ~31) /8

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAX_HASH 256
#define MAX_BUFFER 256

#endif