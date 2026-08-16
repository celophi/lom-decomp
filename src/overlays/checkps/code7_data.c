#include "checkps.h"

s32 g_vsyncTimestamp = 0;
s32 g_displayMode = 0;
u8 g_timeBuffer[8] = {0};

/*
 * GCC 2.7.2 normally emits const data with the MIPS .rdata pseudo-op, which
 * GNU as 2.7 gives 16-byte section alignment.  Explicit .rodata uses the
 * generic ELF section directive and keeps this four-byte object four bytes.
 */
const s32 D_8004FC70 __attribute__((section(".rodata"))) = 0x11;
