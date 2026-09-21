#include "common.h"

extern void func_8007C05C(void);
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_801B26F0;
extern s32 D_801B26F4;

/** @brief Initialize effect values and an eight-tick countdown, then run its first update. */
void func_8007C00C(void)
{
    D_80139234 = 0x10;
    D_8013923C = 0x80;
    D_801B26F4 = 8;
    D_801B26F0 += 1;
    func_8007C05C();
}
