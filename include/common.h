#ifndef _COMMON_H
#define _COMMON_H

#include "include_asm.h"
#include "common/memory_map.h"

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
typedef signed short    s16;

/*
 * Integers wide enough to hold an address (uintptr_t, and intptr_t for signed address
 * compares). The PS1 toolchains (GCC 2.x for the R3000) predate C99 and have no
 * <stdint.h>, so they are defined here for them.
 */
#if defined(__mips__) && defined(__GNUC__) && __GNUC__ < 3
typedef u32 uintptr_t;
typedef s32 intptr_t;
#else
#include <stdint.h>
#endif

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

/** @brief Byte offset of @p member within @p type, as a u32 (the PS1 toolchain has no stddef.h). */
#define OFFSETOF(type, member) ((u32)((u8*)&((type*)0)->member - (u8*)0))

#endif
