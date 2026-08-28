#include "common.h"

extern s32 D_80160940;
extern s32 D_801609AC;
extern s32 D_80160928;
extern s32 D_80160938;
extern s32 D_801609BC;

extern void func_80067F28(void);
extern void func_80141F00(void);

void func_80140C94(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    func_80067F28();
    var_a0 = &D_80160940;
    var_a1 = 0;
    do
    {
        temp_v1 = *var_a0;
        if (temp_v1 & 7)
        {
            temp = (temp_v1 & ~7) | 3;
            *var_a0 = (temp & ~0x78) | 0x40;
        }
        var_a1 += 1;
        var_a0 += 3;
    } while (var_a1 < 8);
}

void func_80140CFC(void)
{
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = D_801609AC;
    temp = (index << 3) - index;
    base = D_80160928;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B)
    {
        D_80160938 = pos - 0x46;
        D_801609BC = 4;
    }
    if (diff < 0)
    {
        D_80160938 = pos;
        D_801609BC = 4;
    }
}

void func_80140D60(void)
{
    func_80141F00();
}
