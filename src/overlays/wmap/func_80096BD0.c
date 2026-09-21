#include "common.h"

extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;
extern s32 D_801B2BBC;
extern s32 D_801B2BB8;

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void func_80096BD0(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--D_801B2BBC == 0)
    {
        D_801B2BB8 += 1;
    }
}
