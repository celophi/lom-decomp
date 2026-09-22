/* Partial WMAP decompilation: 74.519860% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_8005BD54(s32, u32, s32, void *);    /* extern */
M2C_UNK func_8005CE44(M2C_UNK, u32, s32, s32, s32 *, s32 *, s32, s32, void *); /* static */
extern u8 D_800CFDCC;
extern u8 D_800D8318;
extern u8 D_800D8B18;
extern s32 D_800D8B3C;
extern u8 g_saved_game;

void func_8005D018(s32 arg0, u32 arg1, s32 arg2, s32 *arg3, s32 arg4, s32 arg5)
{
    s32 *sp38;
    s32 sp34;
    s32 *sp30;
    u32 sp2C;
    s32 sp28;
    s32 *var_t1;
    s32 *var_t3;
    s32 temp_a0;
    s32 temp_a3;
    s32 temp_s3;
    s32 temp_s4;
    s32 temp_s7;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_a2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s0_3;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s2_3;
    s32 var_s6;
    s32 var_t2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1_3;
    u32 var_s1;
    u8 temp_v0;
    u8 temp_v0_2;
    u8 var_v1;
    u8 var_v1_2;

    var_t1 = arg3;
    var_s2 = 0;
    var_t2 = arg5;
    var_a2 = 0;
    sp28 = 0;
    *var_t1 = 0;
    do
    {
        var_s0 = 0;
        var_a0 = var_s2 << 5;
loop_2:
        temp_v0 = M2C_FIELD((var_s0 + var_a2 + (u8 *)&g_saved_game), u8 *, 0x2F4);
        var_s0 += 1;
        *(var_a0 + (u8 *)&D_800D8318) = (s32) temp_v0;
        var_a0 += 4;
        if (var_s0 < 8)
        {
            goto loop_2;
        }
        var_s2 += 1;
        var_a2 += 0xC;
    } while (var_s2 < 0x40);
    var_s0_2 = 0;
    var_a0_2 = var_t2 << 5;
    do
    {
        temp_v0_2 = *(var_s0_2 + (var_t2 * 0xC) + (u8 *)&D_800CFDCC);
        var_s0_2 += 1;
        *(var_a0_2 + (u8 *)&D_800D8318) = (s32) temp_v0_2;
        var_a0_2 += 4;
    } while (var_s0_2 < 8);
    if ((arg1 < 6U) && (arg2 >= 0) && (arg2 < 6))
    {
        var_v1 = *(arg1 + (arg2 * 6) + (u8 *)&D_800D8B18);
    }
    else
    {
        var_v1 = 0xFF;
    }
    if (var_v1 == 0xFF)
    {
        sp30 = var_t1;
        sp34 = var_t2;
        func_8005BD54(var_t2, arg1, arg2, &D_800D8318);
    }
    var_s2_2 = 0;
    var_t3 = &D_800D8B3C;
    var_s6 = 0;
    sp2C = arg1 - (arg0 % 3);
    temp_s7 = arg2 - (arg0 / 3);
    do
    {
        var_s0_3 = 0;
        temp_s3 = temp_s7 + var_s2_2;
        temp_s4 = temp_s3 * 6;
        var_s1 = sp2C;
        var_v0 = var_s6;
loop_15:
        temp_a3 = 1 << var_v0;
        if ((var_s1 < 6U) && (temp_s3 >= 0) && (temp_s3 < 6))
        {
            var_v1_2 = *(var_s1 + temp_s4 + (u8 *)&D_800D8B18);
        }
        else
        {
            var_v1_2 = 0xFF;
        }
        var_a0_3 = 0;
        if (var_v1_2 != 0xFF)
        {
            sp30 = var_t1;
            sp34 = var_t2;
            sp38 = var_t3;
            func_8005CE44(0, var_s1, temp_s7 + var_s2_2, temp_a3, &sp28, var_t1, arg4, var_t2, &D_800D8318);
            var_t1 = sp30;
            var_t2 = sp34;
            var_t3 = sp38;
            var_s1 += 1;
        }
        else
        {
            var_a1 = 0;
            temp_v1 = var_s1 + temp_s4;
            var_v0_2 = temp_v1 * 0xC;
            do
            {
                var_a1 += 1;
                var_a0_3 += M2C_FIELD((*var_t3 + var_v0_2), u8 *, 8);
                var_v0_2 = var_a1 + (temp_v1 * 0xC);
            } while (var_a1 < 8);
            if (var_a0_3 > 0)
            {
                var_a1_2 = 0;
                *var_t1 |= temp_a3;
                do
                {
                    temp_v0_3 = var_a1_2 + ((var_s1 + temp_s4) * 0xC);
                    var_a1_2 += 1;
                    *(s32 *)((sp28 * 4) + arg4) = M2C_FIELD((*var_t3 + temp_v0_3), u8 *, 8) + 0xD;
                    sp28 += 1;
                } while (var_a1_2 < 8);
                var_a1_3 = 0;
                do
                {
                    var_a1_3 += 1;
                    *(s32 *)((sp28 * 4) + arg4) = 0;
                    sp28 += 1;
                } while (var_a1_3 < 0x10);
            }
            var_s1 += 1;
        }
        var_s0_3 += 1;
        var_v0 = var_s6 + var_s0_3;
        if (var_s0_3 < 3)
        {
            goto loop_15;
        }
        var_s2_2 += 1;
        var_s6 += 3;
    } while (var_s2_2 < 3);
    var_s2_3 = 0;
    *var_t1 |= 0x200;
    do
    {
        temp_a0 = *((var_s2_3 * 4) + (var_t2 << 5) + (u8 *)&D_800D8318) - *(var_s2_3 + (var_t2 * 0xC) + (u8 *)&D_800CFDCC);
        if (temp_a0 == 0)
        {
            temp_v1_2 = sp28 + 1;
            *(s32 *)((sp28 * 4) + arg4) = 0;
            sp28 = temp_v1_2;
            *(s32 *)((temp_v1_2 * 4) + arg4) = 0;
        }
        else
        {
            var_v1_3 = 0x1B;
            if (temp_a0 < 0)
            {
                var_v1_3 = 0x1A;
            }
            *(s32 *)((sp28 * 4) + arg4) = var_v1_3;
            temp_v0_4 = sp28 + 1;
            sp28 = temp_v0_4;
            *(s32 *)((temp_v0_4 * 4) + arg4) = temp_a0 + 0x16;
        }
        var_s2_3 += 1;
        sp28 += 1;
    } while (var_s2_3 < 8);
}
