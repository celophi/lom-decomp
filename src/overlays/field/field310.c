#include "common.h"

extern u8 *D_80122B78;

/**
 * @see decomp.me (100%) TODO
 */
void func_800B0E80(void)
{
    s32 var_a0;
    s32 var_a3;
    s32 var_v1;
    s32 off;
    u8 *base;

    var_a3 = 0;
    do
    {
        off = var_a3 * 0x94;
        (D_80122B78 + off)[0xD70] = var_a3 - 0x80;
        var_a0 = 0;
        (D_80122B78 + off)[0xD71] = 0xFF;
        var_v1 = off;
        (D_80122B78 + off)[0x434] = 0xFF;
        base = D_80122B78;
    loop_2:
        *(u16 *)(base + var_v1 + 0xD78) = 0xFFFF;
        var_a0 += 1;
        var_v1 += 2;
        if (var_a0 < 0x10)
        {
            goto loop_2;
        }
        var_a3 += 1;
    } while (var_a3 < 2);
}
