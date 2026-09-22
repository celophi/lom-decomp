#include "common.h"

extern s32 D_8013B294;
extern s32 D_801B2C54;

/** @brief Set the world-map ready flag and bump the wave index. */
void func_8009AE9C(void)
{
    D_8013B294 = 1;
    D_801B2C54 += 1;
}
