#include "common.h"

extern s16 D_800D930E;
extern s32 D_801B2E48;
extern s32 D_801B2E4C;

/** @brief World-map step handler: clear a flag and advance the step. */
void func_800A7EA0(void)
{
    D_800D930E = 0;
    D_801B2E4C = 0x1E;
    D_801B2E48 += 1;
}
