#include "common.h"

extern s32 D_800DBE70;
extern s32 D_80139978;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

/** @brief World-map step handler: seed timers and advance the counter. */
void func_8009E6B8(void)
{
    D_800DBE70 = 0;
    D_80139978 = -1;
    D_801B2CDC = 0x2D;
    D_801B2CD8 += 1;
}
