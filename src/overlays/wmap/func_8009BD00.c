#include "common.h"

extern s32 D_80139244;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8009BD00(void)
{
    D_80139244 = 1;
    D_801B2C6C = 0x14;
    D_801B2C68 += 1;
}
