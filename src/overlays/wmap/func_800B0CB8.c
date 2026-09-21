#include "common.h"

extern s32 D_80139280;
extern s32 D_801B2F68;
extern s32 D_801B2F6C;
extern void func_8006C448(s32 arg);

/** @brief Kick a world-map sub-handler off the shared context, then step counters. */
void func_800B0CB8(void)
{
    func_8006C448(D_80139280 + 0xA0);
    if (--D_801B2F6C == 0)
    {
        D_801B2F68 += 1;
    }
}
