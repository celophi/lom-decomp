#include "common.h"

extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D58;
extern s32 D_80139250;
extern s32 D_801B2D30;
extern s32 D_801B2D34;
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void func_8009FF00(void)
{
    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_80182D58, 0x8, 0x2, 0);
    *(s16*)((u8*)&D_80182D58 + 0x2) = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--D_801B2D34 == 0)
    {
        D_801B2D30 += 1;
    }
}
