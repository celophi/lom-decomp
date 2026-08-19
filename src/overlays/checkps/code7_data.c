#include "checkps.h"

s32 g_checkpsVsyncTimestamp = 0;
s32 g_cdLastTrackBcd = 0;
u8 g_cdSeekPositionBcd[8] = {0};

/*
 * GCC 2.7.2 normally emits const data with the MIPS .rdata pseudo-op, which
 * GNU as 2.7 gives 16-byte section alignment.  Explicit .rodata uses the
 * generic ELF section directive and keeps this four-byte object four bytes.
 * No recovered CHECKPS code references this value; its runtime purpose is unknown.
 */
const s32 g_checkpsUnusedConstant17 __attribute__((section(".rodata"))) = 0x11;
