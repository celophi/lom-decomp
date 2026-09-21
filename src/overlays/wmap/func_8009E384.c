#include "common.h"

extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2CD0;

/** @brief World-map trigger: set two flags and bump a counter. */
void func_8009E384(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2CD0 += 1;
}
