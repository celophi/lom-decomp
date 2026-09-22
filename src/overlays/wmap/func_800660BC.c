/* Partial WMAP decompilation: 82.125490% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern s32 D_801398EC;
extern u8 D_80139950;

void func_800660BC(void)
{
    s32 temp_v0;
    s32 temp_v0_10;
    s32 temp_v0_2;
    s32 temp_v0_5;
    s32 temp_v0_7;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_a3_3;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_t0_3;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t2;
    s32 var_t3;
    s32 var_t3_2;
    s32 var_t5;
    s32 var_t6;
    s8 temp_v0_11;
    s8 temp_v0_3;
    s8 temp_v0_4;
    s8 temp_v0_6;
    s8 temp_v0_8;
    s8 temp_v0_9;
    s8 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s8 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    u16 temp_a0_5;
    void *temp_a0;
    void *temp_a0_2;
    void *temp_a0_3;
    void *temp_a0_4;
    void *temp_a1;
    void *temp_a1_2;
    void *temp_a1_3;
    void *temp_a1_4;
    void *temp_a1_5;

    var_t3 = 0;
    var_t0 = 0;
    var_t1 = 0;
    var_t5 = 0x340;
    do
    {
        var_a1 = M2C_FIELD(&D_80139950, s32 *, 8);
        temp_a0 = D_801398EC + var_t5;
        if (var_a1 < 0)
        {
            var_a1 += 0xFFF;
        }
        var_a3 = 0;
        temp_v1 = M2C_FIELD(&D_80139950, u8 *, 0) + (var_a1 >> 0xC);
        M2C_FIELD(temp_a0, s8 *, 0x1C) = temp_v1;
        M2C_FIELD(temp_a0, s8 *, 0xC) = temp_v1;
loop_4:
        temp_v0 = (var_t1 + var_a3) * 0x28;
        temp_a1 = D_801398EC + (temp_v0 + 0x340);
        var_a2 = var_a3 * M2C_FIELD(&D_80139950, s32 *, 8);
        temp_a0_2 = D_801398EC + (temp_v0 + 0x368);
        if (var_a2 < 0)
        {
            var_a2 += 0xFFF;
        }
        temp_v0_2 = (var_a2 >> 0xC) + (s32) M2C_FIELD(&D_80139950, u8 *, 0);
        temp_v1_2 = temp_v0_2 + 0x30;
        if (var_t3 != 0)
        {
            var_t3 = 0;
            temp_v0_3 = temp_v0_2 + 0x10;
            M2C_FIELD(temp_a1, s8 *, 0x24) = temp_v0_3;
            M2C_FIELD(temp_a1, s8 *, 0x14) = temp_v0_3;
            M2C_FIELD(temp_a1, s16 *, 0x26) = 0x280;
        }
        else
        {
            if (temp_v1_2 < 0x100)
            {
                M2C_FIELD(temp_a1, s16 *, 0x26) = 0x240;
            }
            else
            {
                M2C_FIELD(temp_a1, s16 *, 0x26) = 0x280;
            }
            M2C_FIELD(temp_a1, s8 *, 0x24) = temp_v1_2;
            M2C_FIELD(temp_a1, s8 *, 0x14) = temp_v1_2;
        }
        if (temp_v1_2 < 0xF0)
        {
            M2C_FIELD(temp_a0_2, s16 *, 0x26) = 0x240;
            goto block_15;
        }
        M2C_FIELD(temp_a0_2, s16 *, 0x26) = 0x280;
        if (temp_v1_2 >= 0x100)
        {
block_15:
            M2C_FIELD(temp_a0_2, s8 *, 0x1C) = temp_v1_2;
            M2C_FIELD(temp_a0_2, s8 *, 0xC) = temp_v1_2;
        }
        else
        {
            var_t3 = 1;
            temp_v0_4 = temp_v1_2 - 0x20;
            M2C_FIELD(temp_a0_2, s8 *, 0x1C) = temp_v0_4;
            M2C_FIELD(temp_a0_2, s8 *, 0xC) = temp_v0_4;
        }
        var_a3 += 1;
        if (var_a3 < 0x19)
        {
            goto loop_4;
        }
        var_a0 = var_a3 * M2C_FIELD(&D_80139950, s32 *, 8);
        temp_a1_2 = D_801398EC + (((var_t1 + var_a3) * 0x28) + 0x340);
        if (var_a0 < 0)
        {
            var_a0 += 0xFFF;
        }
        temp_v0_5 = (var_a0 >> 0xC) + (s32) M2C_FIELD(&D_80139950, u8 *, 0);
        temp_v1_3 = temp_v0_5 + 0x30;
        if (var_t3 != 0)
        {
            var_t3 = 0;
            temp_v0_6 = temp_v0_5 + 0x10;
            M2C_FIELD(temp_a1_2, s8 *, 0x24) = temp_v0_6;
            M2C_FIELD(temp_a1_2, s8 *, 0x14) = temp_v0_6;
        }
        else
        {
            if (temp_v1_3 < 0x100)
            {
                M2C_FIELD(temp_a1_2, s16 *, 0x26) = 0x240;
            }
            else
            {
                M2C_FIELD(temp_a1_2, s16 *, 0x26) = 0x280;
            }
            M2C_FIELD(temp_a1_2, s8 *, 0x24) = temp_v1_3;
            M2C_FIELD(temp_a1_2, s8 *, 0x14) = temp_v1_3;
        }
        var_t1 += 0x1A;
        var_t0 += 1;
        var_t5 += 0x410;
    } while (var_t0 < 0x19);
    var_t3_2 = 0;
    var_a3_2 = 0;
    var_t6 = 0x340;
    do
    {
        var_a1_2 = M2C_FIELD(&D_80139950, s32 *, 8);
        temp_a0_3 = D_801398EC + var_t6;
        if (var_a1_2 < 0)
        {
            var_a1_2 += 0xFFF;
        }
        var_t0_2 = 0;
        var_t2 = 0x1A;
        var_t1_2 = var_a3_2;
        temp_v1_4 = M2C_FIELD(&D_80139950, u8 *, 4) + (var_a1_2 >> 0xC);
        M2C_FIELD(temp_a0_3, s8 *, 0x15) = temp_v1_4;
        M2C_FIELD(temp_a0_3, s8 *, 0xD) = temp_v1_4;
loop_31:
        temp_a1_3 = D_801398EC + ((var_t1_2 * 0x28) + 0x340);
        var_a2_2 = var_t0_2 * M2C_FIELD(&D_80139950, s32 *, 8);
        temp_a0_4 = D_801398EC + (((var_t2 + var_a3_2) * 0x28) + 0x340);
        if (var_a2_2 < 0)
        {
            var_a2_2 += 0xFFF;
        }
        temp_v0_7 = (var_a2_2 >> 0xC) + (s32) M2C_FIELD(&D_80139950, u8 *, 4);
        temp_v1_5 = temp_v0_7 + 0x30;
        if (var_t3_2 != 0)
        {
            var_t3_2 = 0;
            temp_v0_8 = temp_v0_7 + 0x10;
            M2C_FIELD(temp_a1_3, s8 *, 0x25) = temp_v0_8;
            M2C_FIELD(temp_a1_3, s8 *, 0x1D) = temp_v0_8;
            M2C_FIELD(temp_a1_3, s16 *, 0x1E) = 0x100;
        }
        else
        {
            if (temp_v1_5 < 0x100)
            {
                M2C_FIELD(temp_a1_3, s16 *, 0x1E) = 0;
            }
            else
            {
                M2C_FIELD(temp_a1_3, s16 *, 0x1E) = 0x100;
            }
            M2C_FIELD(temp_a1_3, s8 *, 0x25) = temp_v1_5;
            M2C_FIELD(temp_a1_3, s8 *, 0x1D) = temp_v1_5;
        }
        if (temp_v1_5 < 0xF0)
        {
            M2C_FIELD(temp_a0_4, s16 *, 0x1E) = 0x100;
            goto block_42;
        }
        M2C_FIELD(temp_a0_4, s16 *, 0x1E) = 0x100;
        if (temp_v1_5 >= 0x100)
        {
block_42:
            M2C_FIELD(temp_a0_4, s8 *, 0x15) = temp_v1_5;
            M2C_FIELD(temp_a0_4, s8 *, 0xD) = temp_v1_5;
        }
        else
        {
            var_t3_2 = 1;
            temp_v0_9 = temp_v1_5 - 0x20;
            M2C_FIELD(temp_a0_4, s8 *, 0x15) = temp_v0_9;
            M2C_FIELD(temp_a0_4, s8 *, 0xD) = temp_v0_9;
        }
        var_t2 += 0x1A;
        var_t0_2 += 1;
        var_t1_2 += 0x1A;
        if (var_t0_2 < 0x19)
        {
            goto loop_31;
        }
        var_a0_2 = var_t0_2 * M2C_FIELD(&D_80139950, s32 *, 8);
        temp_a1_4 = D_801398EC + ((((var_t0_2 * 0x1A) + var_a3_2) * 0x28) + 0x340);
        if (var_a0_2 < 0)
        {
            var_a0_2 += 0xFFF;
        }
        temp_v0_10 = (var_a0_2 >> 0xC) + (s32) M2C_FIELD(&D_80139950, u8 *, 4);
        temp_v1_6 = temp_v0_10 + 0x30;
        if (var_t3_2 != 0)
        {
            var_t3_2 = 0;
            temp_v0_11 = temp_v0_10 + 0x10;
            M2C_FIELD(temp_a1_4, s8 *, 0x25) = temp_v0_11;
            M2C_FIELD(temp_a1_4, s8 *, 0x1D) = temp_v0_11;
        }
        else
        {
            if (temp_v1_6 < 0x100)
            {
                M2C_FIELD(temp_a1_4, s16 *, 0x1E) = 0;
            }
            else
            {
                M2C_FIELD(temp_a1_4, s16 *, 0x1E) = 0x100;
            }
            M2C_FIELD(temp_a1_4, s8 *, 0x25) = temp_v1_6;
            M2C_FIELD(temp_a1_4, s8 *, 0x1D) = temp_v1_6;
        }
        var_a3_2 += 1;
        var_t6 += 0x410;
    } while (var_a3_2 < 0x19);
    var_t0_3 = 1;
    var_a2_3 = 0x1A;
    do
    {
        var_a3_3 = 1;
loop_56:
        temp_a1_5 = D_801398EC + ((var_a2_3 + var_a3_3) * 0x28);
        var_a3_3 += 1;
        temp_a0_5 = M2C_FIELD(temp_a1_5, u16 *, 0x35E);
        M2C_FIELD(temp_a1_5, s16 *, 0x356) = (s16) (((u32) (temp_a0_5 & 0x100) >> 4) | ((u32) (M2C_FIELD(temp_a1_5, u16 *, 0x366) & 0x3FF) >> 6) | ((temp_a0_5 & 0x200) * 4));
        if (var_a3_3 < 0x19)
        {
            goto loop_56;
        }
        var_t0_3 += 1;
        var_a2_3 += 0x1A;
    } while (var_t0_3 < 0x19);
}
