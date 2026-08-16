#include "checkps.h"

s32 g_vsyncTimestamp = 0;
s32 g_displayMode = 0;
u8 g_timeBuffer[8] = {0};

/* Avoid GNU as 2.7's 16-byte tail padding on a four-byte .rodata section. */
const s32 D_8004FC70 __attribute__((section(".rodata.code7_data"))) = 0x11;
