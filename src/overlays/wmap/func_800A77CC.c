#include "common.h"

extern u8 D_801AFBD0[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_8006AEE0(void);
extern void func_800A66C0(void);
extern void func_800A6800(void);

/** @brief Step the world-map particle set, decaying each slot's velocity field. */
void func_800A77CC(void)
{
    s32 i;
    u8* p;

    func_8006AEE0();
    func_800A66C0();
    func_800A6800();
    for (i = 0; i < 4; i++)
    {
        p = D_801AFBD0 + (0x14 + i) * 0x14;
        *(s32*)(p + 0x8) -= *(s32*)(p + 0x4);
    }
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}
