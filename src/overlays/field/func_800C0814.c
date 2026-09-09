#include "common.h"

s32 rand(void); /* extern */

/**
 * @brief Apply packed stat adjustments and effect entries from a selected record.
 * @param arg0 Destination record containing stats and effect flags.
 * @param arg1 Record selector; values outside 0x60 through 0x87 are ignored.
 * @param arg2 Packed adjustment and effect table.
 */
void func_800C0814(u8 *arg0, s32 arg1, u8 *arg2)
{
    s32 temp_a0;
    s32 offset;
    s32 one;
    s32 var_a0;
    s32 var_s2;
    s32 var_s4;
    s32 var_v1;
    s32 var_v1_2;
    u32 temp_v1;
    s32 index;
    s32 temp_s0;
    s32 temp_s1;
    u8 temp_v1_2;
    u8 temp_v1_3;
    u8 *var_a1;
    u8 *var_a1_2;
    u8 *var_v0;

    temp_v1 = arg1 - 0x60;
    if (temp_v1 < 0x28U)
    {
        var_s2 = 0;
        index = temp_v1;
        offset = index * 0x14;
        var_a1 = arg0 + var_s2;
        do
        {
            temp_a0 = var_a1[0x4C];
            temp_v1_2 = *(arg2 + (var_s2 + offset) + 4);
            temp_a0 = ((temp_a0 >> 4) + (temp_v1_2 & 0xF)) - (temp_v1_2 >> 4);
            if (temp_a0 >= 0)
            {
                var_v1 = 0xF;
                if (temp_a0 < 0x10)
                {
                    var_v1 = temp_a0;
                }
            }
            else
            {
                var_v1 = 0;
            }
            var_s2 += 1;
            var_a1[0x4C] = (u8)((var_a1[0x4C] & 0xF) | (var_v1 * 0x10));
            var_a1 = arg0 + var_s2;
        } while (var_s2 < 8);
        var_s2 = 0;
        offset = index * 0x14;
        var_a1_2 = arg0 + var_s2;
        do
        {
            var_a0 = var_a1_2[0x54];
            temp_v1_3 = *(arg2 + (var_s2 + offset) + 0xC);
            var_a0 = ((var_a0 >> 4) + (temp_v1_3 & 0xF)) - (temp_v1_3 >> 4);
            if (var_a0 >= 0)
            {
                var_v1_2 = 0xF;
                if (var_a0 < 0x10)
                {
                    var_v1_2 = var_a0;
                }
            }
            else
            {
                var_v1_2 = 0;
            }
            var_s2 += 1;
            var_a1_2[0x54] = (u8)((var_a1_2[0x54] & 0xF) | (var_v1_2 * 0x10));
            var_a1_2 = arg0 + var_s2;
        } while (var_s2 < 4);
        var_s2 = 0;
        one = 1;
        var_s4 = index * 0x14;
        var_v0 = arg2 + var_s4;
        do
        {
            temp_s1 = var_v0[0x10];
            temp_s0 = var_v0[0x11];
            if (index < 8)
            {
                var_a0 = rand() & 0xFF;
                if (var_a0 < (s32)temp_s0)
                {
                    arg0[0x48] = (u8)(arg0[0x48] | (one << temp_s1));
                }
                goto block_25;
            }
            if (index < 0x10)
            {
                var_a0 = rand() & 0xFF;
                if (var_a0 < (s32)temp_s0)
                {
                    arg0[0x48] = (u8)(arg0[0x48] & ~(one << (temp_s1 - 8)));
                }
                goto block_25;
            }
            switch (temp_s1)
            {
            case 0xF0:
                arg0[0x3C] = temp_s0;
                break;
            case 0xF1:
                arg0[0x3D] = temp_s0;
                break;
            }
        block_25:
            var_s4 += 2;
            var_s2 += 1;
            var_v0 = arg2 + var_s4;
        } while (var_s2 < 4);
    }
}
