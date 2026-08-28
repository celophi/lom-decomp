#include "common.h"

extern s32 D_80165F80;
extern s32 D_80165FF4;
extern s32 D_80166104;
extern s32 D_80165F38;
extern s32 D_80165FFC;

extern void func_80067F28(void);
extern void func_80142668(void);

void func_80141164(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    func_80067F28();
    var_a0 = &D_80165F80;
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

void func_801411CC(void)
{
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = D_80165FF4;
    temp = (index << 3) - index;
    base = D_80166104;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B)
    {
        D_80165F38 = pos - 0x46;
        D_80165FFC = 4;
    }
    if (diff < 0)
    {
        D_80165F38 = pos;
        D_80165FFC = 4;
    }
}

void func_80141230(void)
{
    func_80142668();
}
