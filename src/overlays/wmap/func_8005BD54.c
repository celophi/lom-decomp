/* Partial WMAP decompilation: 81.913550% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

s32 func_8005D670(s32, s32);
s32 func_8005B8C8(u32, s32, s32);                   /* static */
extern u8 D_800CFDCC;
extern s32 D_800D8B3C;
extern u8 g_saved_game;

void func_8005BD54(s32 arg0, u32 arg1, s32 arg2, s32 arg3)
{
    s32 sp2C;
    u32 sp10;
    s32 *var_a0_6;
    s32 *var_a1;
    s32 *var_a1_2;
    s32 *var_a1_3;
    s32 *var_a1_5;
    s32 *var_a3_2;
    s32 *var_v0;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_12;
    s32 temp_v1_13;
    s32 temp_v1_18;
    s32 temp_v1_21;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 temp_v1_9;
    s32 var_a0_15;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_a1_10;
    s32 var_a1_11;
    s32 var_a1_12;
    s32 var_a1_13;
    s32 var_a1_14;
    s32 var_a1_4;
    s32 var_a1_6;
    s32 var_a1_7;
    s32 var_a1_8;
    s32 var_a1_9;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_a2_4;
    s32 var_a3;
    u32 *var_a0_10;
    u32 *var_a0_11;
    u32 *var_a0_12;
    u32 *var_a0_13;
    u32 *var_a0_14;
    u32 *var_a0_7;
    u32 *var_a0_8;
    u32 *var_a0_9;
    u32 *var_a2_5;
    u32 temp_v0_5;
    u32 temp_v1_14;
    u32 temp_v1_16;
    u32 var_a0;
    u8 temp_v1_15;
    u8 temp_v1_17;
    u8 temp_v1_19;
    u8 temp_v1_20;
    u8 temp_v1_5;

    if ((arg1 < 6U) && (arg2 >= 0) && (arg2 < 6) && (func_8005B8C8(arg1, arg2, arg0) != 0))
    {
        temp_v0 = func_8005D670(arg1 - 1, arg2);
        var_a0 = arg1 + 1;
        if (temp_v0 != 0xFF)
        {
            var_a2 = 0;
            if (arg0 != 0xFF)
            {
                var_a1 = (temp_v0 << 5) + arg3;
                do
                {
                    temp_v1 = *var_a1 + (*(var_a2 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1 = temp_v1;
                    if (temp_v1 >= 0)
                    {
                        var_a0_2 = 6;
                        if (temp_v1 < 7)
                        {
                            var_a0_2 = temp_v1;
                        }
                    }
                    else
                    {
                        var_a0_2 = 0;
                    }
                    *var_a1 = var_a0_2;
                    var_a2 += 1;
                    var_a1 = (s32 *)((u8 *)var_a1 + 4);
                } while (var_a2 < 8);
                var_a0 = arg1 + 1;
            }
        }
        temp_v0_2 = func_8005D670(var_a0, arg2);
        if (temp_v0_2 != 0xFF)
        {
            var_a2_2 = 0;
            if (arg0 != 0xFF)
            {
                var_a1_2 = (temp_v0_2 << 5) + arg3;
                do
                {
                    temp_v1_2 = *var_a1_2 + (*(var_a2_2 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1_2 = temp_v1_2;
                    if (temp_v1_2 >= 0)
                    {
                        var_a0_3 = 6;
                        if (temp_v1_2 < 7)
                        {
                            var_a0_3 = temp_v1_2;
                        }
                    }
                    else
                    {
                        var_a0_3 = 0;
                    }
                    *var_a1_2 = var_a0_3;
                    var_a2_2 += 1;
                    var_a1_2 = (s32 *)((u8 *)var_a1_2 + 4);
                } while (var_a2_2 < 8);
            }
        }
        temp_v0_3 = func_8005D670(arg1, arg2 - 1);
        if (temp_v0_3 != 0xFF)
        {
            var_a2_3 = 0;
            if (arg0 != 0xFF)
            {
                var_a1_3 = (temp_v0_3 << 5) + arg3;
                do
                {
                    temp_v1_3 = *var_a1_3 + (*(var_a2_3 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1_3 = temp_v1_3;
                    if (temp_v1_3 >= 0)
                    {
                        var_a0_4 = 6;
                        if (temp_v1_3 < 7)
                        {
                            var_a0_4 = temp_v1_3;
                        }
                    }
                    else
                    {
                        var_a0_4 = 0;
                    }
                    *var_a1_3 = var_a0_4;
                    var_a2_3 += 1;
                    var_a1_3 = (s32 *)((u8 *)var_a1_3 + 4);
                } while (var_a2_3 < 8);
            }
        }
        temp_v0_4 = func_8005D670(arg1, arg2 + 1);
        var_a1_4 = 0;
        if (temp_v0_4 != 0xFF)
        {
            var_a2_4 = 0;
            if (arg0 != 0xFF)
            {
                var_a1_5 = (temp_v0_4 << 5) + arg3;
                do
                {
                    temp_v1_4 = *var_a1_5 + (*(var_a2_4 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1_5 = temp_v1_4;
                    if (temp_v1_4 >= 0)
                    {
                        var_a0_5 = 6;
                        if (temp_v1_4 < 7)
                        {
                            var_a0_5 = temp_v1_4;
                        }
                    }
                    else
                    {
                        var_a0_5 = 0;
                    }
                    *var_a1_5 = var_a0_5;
                    var_a2_4 += 1;
                    var_a1_5 = (s32 *)((u8 *)var_a1_5 + 4);
                } while (var_a2_4 < 8);
                var_a1_4 = 0;
            }
        }
        temp_a2 = (arg1 + (arg2 * 6)) * 0xC;
        var_a0_6 = (arg0 << 5) + arg3;
        do
        {
            temp_v1_5 = M2C_FIELD((D_800D8B3C + (var_a1_4 + temp_a2)), u8 *, 8);
            var_a1_4 += 1;
            *var_a0_6 += temp_v1_5;
            var_a0_6 = (s32 *)((u8 *)var_a0_6 + 4);
        } while (var_a1_4 < 8);
        var_a1_6 = 7;
        var_v0 = &sp2C;
        do
        {
            *var_v0 = 0;
            var_a1_6 -= 1;
            var_v0 = (s32 *)((u8 *)var_v0 - 4);
        } while (var_a1_6 >= 0);
        temp_v1_6 = func_8005D670(arg1 - 1, arg2);
        var_a0_7 = &sp10;
        if (arg0 != 0xFF)
        {
            var_a1_7 = 0;
            if (temp_v1_6 != 0xFF)
            {
                do
                {
                    temp_v1_7 = var_a1_7 + (temp_v1_6 * 0xC);
                    var_a1_7 += 1;
                    *var_a0_7 = *var_a0_7 - 3 + M2C_FIELD((temp_v1_7 + (u8 *)&g_saved_game), u8 *, 0x2F4);
                    var_a0_7 = (u32 *)((u8 *)var_a0_7 + 4);
                } while (var_a1_7 < 8);
            }
        }
        temp_v1_8 = func_8005D670(arg1 + 1, arg2);
        var_a0_8 = &sp10;
        if (arg0 != 0xFF)
        {
            var_a1_8 = 0;
            if (temp_v1_8 != 0xFF)
            {
                do
                {
                    temp_v1_9 = var_a1_8 + (temp_v1_8 * 0xC);
                    var_a1_8 += 1;
                    *var_a0_8 = *var_a0_8 - 3 + M2C_FIELD((temp_v1_9 + (u8 *)&g_saved_game), u8 *, 0x2F4);
                    var_a0_8 = (u32 *)((u8 *)var_a0_8 + 4);
                } while (var_a1_8 < 8);
            }
        }
        temp_v1_10 = func_8005D670(arg1, arg2 - 1);
        var_a0_9 = &sp10;
        if (arg0 != 0xFF)
        {
            var_a1_9 = 0;
            if (temp_v1_10 != 0xFF)
            {
                do
                {
                    temp_v1_11 = var_a1_9 + (temp_v1_10 * 0xC);
                    var_a1_9 += 1;
                    *var_a0_9 = *var_a0_9 - 3 + M2C_FIELD((temp_v1_11 + (u8 *)&g_saved_game), u8 *, 0x2F4);
                    var_a0_9 = (u32 *)((u8 *)var_a0_9 + 4);
                } while (var_a1_9 < 8);
            }
        }
        temp_v1_12 = func_8005D670(arg1, arg2 + 1);
        var_a0_10 = &sp10;
        if ((arg0 != 0xFF) && (temp_v1_12 != 0xFF))
        {
            var_a1_10 = 0;
            do
            {
                temp_v1_13 = var_a1_10 + (temp_v1_12 * 0xC);
                var_a1_10 += 1;
                *var_a0_10 = *var_a0_10 - 3 + M2C_FIELD((temp_v1_13 + (u8 *)&g_saved_game), u8 *, 0x2F4);
                var_a0_10 = (u32 *)((u8 *)var_a0_10 + 4);
            } while (var_a1_10 < 8);
        }
        temp_v1_14 = arg1 - 1;
        var_a0_11 = &sp10;
        if ((temp_v1_14 < 6U) && (arg2 >= 0) && (arg2 < 6))
        {
            var_a1_11 = 0;
            do
            {
                temp_v1_15 = M2C_FIELD((D_800D8B3C + (var_a1_11 + ((temp_v1_14 + (arg2 * 6)) * 0xC))), u8 *, 8);
                var_a1_11 += 1;
                *var_a0_11 += temp_v1_15;
                var_a0_11 = (u32 *)((u8 *)var_a0_11 + 4);
            } while (var_a1_11 < 8);
        }
        temp_v1_16 = arg1 + 1;
        var_a0_12 = &sp10;
        if ((temp_v1_16 < 6U) && (arg2 >= 0) && (arg2 < 6))
        {
            var_a1_12 = 0;
            do
            {
                temp_v1_17 = M2C_FIELD((D_800D8B3C + (var_a1_12 + ((temp_v1_16 + (arg2 * 6)) * 0xC))), u8 *, 8);
                var_a1_12 += 1;
                *var_a0_12 += temp_v1_17;
                var_a0_12 = (u32 *)((u8 *)var_a0_12 + 4);
            } while (var_a1_12 < 8);
        }
        temp_v1_18 = arg2 - 1;
        var_a0_13 = &sp10;
        if ((arg1 < 6U) && (temp_v1_18 >= 0))
        {
            var_a1_13 = 0;
            if (temp_v1_18 < 6)
            {
                do
                {
                    temp_v1_19 = M2C_FIELD((D_800D8B3C + (var_a1_13 + ((arg1 + (temp_v1_18 * 6)) * 0xC))), u8 *, 8);
                    var_a1_13 += 1;
                    *var_a0_13 += temp_v1_19;
                    var_a0_13 = (u32 *)((u8 *)var_a0_13 + 4);
                } while (var_a1_13 < 8);
            }
        }
        temp_a1 = arg2 + 1;
        var_a0_14 = &sp10;
        if ((arg1 < 6U) && (temp_a1 >= 0))
        {
            var_a3 = 0;
            if (temp_a1 < 6)
            {
                do
                {
                    temp_v1_20 = M2C_FIELD((D_800D8B3C + (var_a3 + ((arg1 + (temp_a1 * 6)) * 0xC))), u8 *, 8);
                    var_a3 += 1;
                    *var_a0_14 += temp_v1_20;
                    var_a0_14 = (u32 *)((u8 *)var_a0_14 + 4);
                } while (var_a3 < 8);
            }
        }
        var_a1_14 = 0;
        var_a3_2 = (arg0 << 5) + arg3;
        var_a2_5 = &sp10;
        do
        {
            temp_v0_5 = *var_a2_5;
            temp_v1_21 = *var_a3_2 + ((s32) (temp_v0_5 + (temp_v0_5 >> 0x1F)) >> 1);
            *var_a3_2 = temp_v1_21;
            if (temp_v1_21 >= 0)
            {
                var_a0_15 = 6;
                if (temp_v1_21 < 7)
                {
                    var_a0_15 = temp_v1_21;
                }
            }
            else
            {
                var_a0_15 = 0;
            }
            *var_a3_2 = var_a0_15;
            var_a3_2 = (s32 *)((u8 *)var_a3_2 + 4);
            var_a1_14 += 1;
            var_a2_5 = (u32 *)((u8 *)var_a2_5 + 4);
        } while (var_a1_14 < 8);
    }
}
