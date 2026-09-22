/* Partial WMAP decompilation: 79.354164% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

s32 func_8005D670(s32, s32);
extern u8 D_800432BD;
extern u8 D_800CFDCC;
extern s32 D_800D8B3C;
extern u8 g_saved_game;

s32 func_8005B8C8(u32 arg0, s32 arg1, s32 arg2)
{
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 var_v0;
    s32 var_v0_2;
    u32 temp_s0_4;
    u32 temp_s0_5;

    temp_s0 = arg0 + (arg1 * 6);
    if ((arg0 < 6U) && (arg1 >= 0) && (arg1 < 6))
    {
        var_v0 = 0;
        if (func_8005D670(arg0, arg1) == 0xFF)
        {
            if ((((u32) M2C_FIELD(((arg2 * 0xC) + (u8 *)&D_800CFDCC), u32 *, 8) >> 0xA) & 1) == 1)
            {
                if (!(M2C_FIELD((D_800D8B3C + (temp_s0 * 0xC)), u16 *, 0x10) & 4))
                {
                    return 0;
                }
                goto block_11;
            }
            if ((u32) (arg2 - 0x18) >= 2U)
            {
                if ((arg2 == 0x1F) || (var_v0 = 0, ((M2C_FIELD((D_800D8B3C + (temp_s0 * 0xC)), u16 *, 0x10) & 1) == 0)))
                {
                    goto block_11;
                }
                /* Duplicate return node #29. Try simplifying control flow for better match */
                return var_v0;
            }
block_11:
            if (D_800432BD != 0)
            {
                if (((M2C_FIELD((D_800D8B3C + (temp_s0 * 0xC)), u16 *, 0x10) & 2) || (var_v0 = 0, ((((u32) M2C_FIELD(((arg2 * 0xC) + (u8 *)&D_800CFDCC), u32 *, 8) >> 8) & 1) != 1))) && ((arg1 <= 0) || (temp_s0_2 = arg1 - 1, (func_8005D670(arg0, temp_s0_2) == 0xFF)) || (var_v0 = 1, ((((u8) M2C_FIELD(((func_8005D670(arg0, temp_s0_2) * 0xC) + (u8 *)&g_saved_game), u8 *, 0x2F0) >> 2) & 1) != 1))) && ((arg1 >= 5) || (temp_s0_3 = arg1 + 1, (func_8005D670(arg0, temp_s0_3) == 0xFF)) || (var_v0 = 1, ((((u8) M2C_FIELD(((func_8005D670(arg0, temp_s0_3) * 0xC) + (u8 *)&g_saved_game), u8 *, 0x2F0) >> 2) & 1) != 1))))
                {
                    temp_s0_4 = arg0 - 1;
                    if (((s32) arg0 > 0) && (func_8005D670(temp_s0_4, arg1) != 0xFF))
                    {
                        var_v0_2 = (s32) arg0 < 5;
                        if ((((u8) M2C_FIELD(((func_8005D670(temp_s0_4, arg1) * 0xC) + (u8 *)&g_saved_game), u8 *, 0x2F0) >> 2) & 1) == 1)
                        {
                            goto block_23;
                        }
                        goto block_25;
                    }
                    var_v0_2 = (s32) arg0 < 5;
block_25:
                    temp_s0_5 = arg0 + 1;
                    if ((var_v0_2 == 0) || (func_8005D670(temp_s0_5, arg1) == 0xFF) || (var_v0 = 1, ((((u8) M2C_FIELD(((func_8005D670(temp_s0_5, arg1) * 0xC) + (u8 *)&g_saved_game), u8 *, 0x2F0) >> 2) & 1) != 1)))
                    {
                        goto block_28;
                    }
                    /* Duplicate return node #29. Try simplifying control flow for better match */
                    return var_v0;
                }
                /* Duplicate return node #29. Try simplifying control flow for better match */
                return var_v0;
            }
block_23:
            return 1;
        }
        /* Duplicate return node #29. Try simplifying control flow for better match */
        return var_v0;
    }
block_28:
    var_v0 = 0;
    return var_v0;
}
