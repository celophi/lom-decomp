#include "common.h"

extern void func_800652A8(s32, s32);
extern s32 D_801B2510;
extern s32 D_801B2514;

/** @brief Play sound 19, start a 54-tick delay, and advance the state. */
void func_8007335C(void)
{
    func_800652A8(0x13, 0x80);
    D_801B2514 = 54;
    D_801B2510 += 1;
}
