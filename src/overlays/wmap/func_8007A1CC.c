#include "common.h"

extern s16 D_800D9366;
extern s32 D_801B2690;
extern s32 D_801B2694;
extern void func_8007A210(void);

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_8007A1CC(void)
{
    D_800D9366 = 1;
    D_801B2694 = 0x10;
    D_801B2690 += 1;
    func_8007A210();
}
