#include "common.h"

extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DF0;
extern s32 D_801B2638;
extern s32 D_801B263C;
extern void func_8006A9C4(void *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);

/** @brief Update the actor effect and advance the sequence after its countdown. */
void func_80078678(void)
{
    func_8006A9C4(D_800D94D0, D_801399F8, 0xE, 0x22, D_80182DF0, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4);
    if (--D_801B263C == 0)
    {
        D_801B2638++;
    }
}
