#include "common.h"

extern s16 D_800D933A;
extern s32 D_801B2528;
extern s32 D_801B252C;
extern void func_800735FC(void);

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_800735B8(void)
{
    D_800D933A = 1;
    D_801B252C = 0x10;
    D_801B2528 += 1;
    func_800735FC();
}
