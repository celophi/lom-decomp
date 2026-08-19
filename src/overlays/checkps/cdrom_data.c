#include "checkps.h"

s32 g_checkps_vsync_timestamp = 0;
s32 g_cd_last_track_bcd = 0;
u8 g_cd_seek_position_bcd[8] = {0};

/*
 * GCC 2.7.2 normally emits const data with the MIPS .rdata pseudo-op, which
 * GNU as 2.7 gives 16-byte section alignment.  Explicit .rodata uses the
 * generic ELF section directive and keeps this four-byte object four bytes.
 * No recovered CHECKPS code references this value; its runtime purpose is unknown.
 */
const s32 g_checkps_unused_constant17 __attribute__((section(".rodata"))) = 0x11;
