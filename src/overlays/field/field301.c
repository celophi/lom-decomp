#include "common.h"

extern u8 *D_80122B74;

void func_800C1154(u32 arg0)
{
    u32 temp_v0;
    u32 temp_v1;
    u32 var_a2;
    u8 *base;

    arg0 = arg0 >> 3;
    var_a2 = 0;
    base = D_80122B74;
    do
    {
        if ((base[(var_a2 * 0x60) + 0x2EF4] != 0) &&
            (((u32) *(u32 *) (base + (var_a2 * 0x60) + 0x2F38) >> 0x1E) & 1))
        {
            temp_v0 = *(u32 *) (base + (var_a2 * 0x60) + 0x2F0C);
            temp_v1 = (temp_v0 & 0xFF) | (((temp_v0 >> 8) + arg0) << 8);
            *(u32 *) (base + (var_a2 * 0x60) + 0x2F0C) = temp_v1;
            if ((s32) (temp_v1 >> 8) > 0x98967F)
            {
                *(u32 *) (base + (var_a2 * 0x60) + 0x2F0C) =
                    (temp_v1 & 0xFF) | 0x98967F00;
            }
        }
        var_a2 += 1;
    } while (var_a2 < 5U);
}
