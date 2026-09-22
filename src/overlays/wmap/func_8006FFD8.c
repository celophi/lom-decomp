#include "common.h"

extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2400;
extern s32 D_801B2404;

/** @brief World-map step handler: seed timers and advance the counter. */
void func_8006FFD8(void)
{
    D_80139244 = 0;
    D_801ADAF4 = 0x10;
    D_801B2404 = 0x28;
    D_801B2400 += 1;
}
