#ifndef _COMMON_H
#define _COMMON_H

#include "include_asm.h"

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef unsigned char   undefined;
typedef unsigned char   undefined1;
typedef unsigned short  undefined2;
typedef unsigned int    undefined4;

typedef int             s32;
typedef unsigned int    u32;
typedef unsigned char   u8;
typedef signed char     s8;
typedef unsigned short  u16;

// Boolean / null macros
#define TRUE    1
#define FALSE   0
#define NULL    ((void*)0)

#endif