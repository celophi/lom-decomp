#include "common.h"

extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern s32 D_80139280;
extern s32 D_801B2898;
extern s32 D_801B289C;

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80084FE0(void)
{
    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, D_80139280);
    if (--D_801B289C == 0)
    {
        D_801B2898 += 1;
    }
}
