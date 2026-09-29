#ifndef _COMMON_H
#define _COMMON_H

#include "include_asm.h"
#include "ps1_types.h"

typedef unsigned char   undefined;
typedef unsigned char   undefined1;
typedef unsigned short  undefined2;
typedef unsigned int    undefined4;

/* Boolean / null macros */
#define TRUE    1
#define FALSE   0
#define NULL    ((void*)0)

/* Common sizes */
#define MAX_SHORT_VALUE 32767

/*
 * Pack two values that are already in the u16 range into one u32.
 * The first argument occupies bits 15:0 and the second occupies bits 31:16.
 */
#define PACK_U16_PAIR(low, high) ((u32)(low) + ((u32)(high) << 16))

/* Round x up to the nearest multiple of 64 (PSX texture page width alignment) */
#define ALIGN64(x) (((x) + 0x3F) & 0xFFC0)

#endif
