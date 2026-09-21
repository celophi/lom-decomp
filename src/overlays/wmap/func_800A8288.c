#include "common.h"

extern s16 D_800D930E;
extern s32 D_801B2E50;
extern s32 D_801B2E54;

/** @brief World-map step handler: clear a flag and advance the step. */
void func_800A8288(void)
{
    D_800D930E = 0;
    D_801B2E54 = 0x1E;
    D_801B2E50 += 1;
}
