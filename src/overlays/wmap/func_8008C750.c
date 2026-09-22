#include "common.h"

extern u8 D_800E4F18[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;
extern void func_8006B998(s32, s32, void *, s32, s32);

/** @brief Draw the particle range and advance when its countdown expires. */
void func_8008C750(void)
{
    s32 remaining;

    func_8006B998(100, 124, D_800E4F18, 8, 10);
    remaining = D_801B2A04 - 1;
    D_801B2A04 = remaining;
    if (remaining == 0)
    {
        D_801B2A00++;
    }
}
