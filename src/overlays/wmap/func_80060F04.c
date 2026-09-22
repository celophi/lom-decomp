/* Partial WMAP decompilation: 56.904133% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern u8 D_800D0554;
extern u8 D_800D05F4;
extern s32 D_800D06E0;
extern s32 D_800D921C;
extern s32 D_800D9240;
extern s32 D_8011CF44;
extern s32 D_8011CF74;
extern s32 D_801398D4;
extern void *D_801398EC;
extern s32 D_801398F0;
extern u8 D_80182D74;
extern u8 D_80182D75;
extern u8 D_80182D76;
extern u8 D_80182D80;
extern u8 D_80182D81;
extern u8 D_80182D82;
extern u8 D_80182D8C;
extern u8 D_80182D8D;
extern u8 D_80182D8E;
extern u8 D_80182D94;
extern u8 D_80182D95;
extern u8 D_80182D96;
extern s32 D_801ADAF4;
extern u8 D_801ADB04;

s32 func_80060F04(void)
{
    s8 sp3;
    s8 sp2;
    s8 sp1;
    s8 sp0;
    void *var_a2;
    void *var_a2_2;
    void *var_v1_2;
    void *var_v1_3;
    void *var_v1_4;
    void *var_v1_5;
    s16 temp_a0_11;
    s16 temp_a0_2;
    s16 temp_a0_5;
    s16 temp_a0_8;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s32 *temp_a0_12;
    s32 *temp_a0_3;
    s32 *temp_a0_6;
    s32 *temp_a0_9;
    s32 *temp_a1;
    s32 *temp_a1_2;
    s32 *temp_a1_3;
    s32 *temp_a1_4;
    s32 *temp_a1_5;
    s32 *var_a1_2;
    s32 *var_v0_3;
    s32 *var_v0_4;
    s32 *var_v0_5;
    s32 *var_v0_6;
    s32 *var_v1_6;
    s32 temp_a0;
    s32 temp_a0_10;
    s32 temp_a0_4;
    s32 temp_a0_7;
    s32 var_a0;
    s32 var_a1;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    u32 temp_v0_10;
    u32 temp_v0_11;
    u32 temp_v0_12;
    u32 temp_v0_13;
    u32 temp_v0_14;
    u32 temp_v0_15;
    u32 temp_v0_16;
    u32 temp_v0_5;
    u32 temp_v0_6;
    u32 temp_v0_7;
    u32 temp_v0_8;
    u32 temp_v0_9;
    u8 var_v0_10;
    u8 var_v0_11;
    u8 var_v0_12;
    u8 var_v0_13;
    u8 var_v0_14;
    u8 var_v0_15;
    u8 var_v0_16;
    u8 var_v0_17;
    u8 var_v0_18;
    u8 var_v0_7;
    u8 var_v0_8;
    u8 var_v0_9;

    if (D_801398F0 != D_801ADAF4)
    {
        var_v0 = D_801398F0 - 1;
        if (D_801ADAF4 >= D_801398F0)
        {
            var_v0 = D_801398F0 + 1;
        }
        D_801398F0 = var_v0;
    }
    if (D_801398F0 != 0)
    {
        if (D_801398D4 != 0)
        {
            var_a0 = D_801398F0 * M2C_FIELD(&D_801ADB04, u8 *, 0);
            if (var_a0 < 0)
            {
                var_a0 += 0xF;
            }
            var_a1 = D_801398F0 * M2C_FIELD(&D_801ADB04, u8 *, 1);
            sp0 = (s8) (var_a0 >> 4);
            if (var_a1 < 0)
            {
                var_a1 += 0xF;
            }
            var_v1 = D_801398F0 * M2C_FIELD(&D_801ADB04, u8 *, 2);
            sp1 = (s8) (var_a1 >> 4);
            if (var_v1 < 0)
            {
                var_v1 += 0xF;
            }
            var_v0_2 = var_v1 >> 4;
        }
        else
        {
            sp0 = (D_801398F0 * M2C_FIELD(&D_801ADB04, u8 *, 0)) / 24;
            sp1 = (D_801398F0 * M2C_FIELD(&D_801ADB04, u8 *, 1)) / 24;
            var_v0_2 = (D_801398F0 * M2C_FIELD(&D_801ADB04, u8 *, 2)) / 24;
        }
        sp2 = (s8) var_v0_2;
        sp3 = 0x2C;
        if ((D_8011CF44 == 0) && (D_801398D4 != 0))
        {
            var_a2 = &D_800D0554;
            do
            {
                var_v1_2 = var_a2;
                temp_a1 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                var_v0_3 = temp_a1;
loop_18:
                M2C_FIELD(var_v0_3, s32 *, 0) = M2C_FIELD(var_v1_2, s32 *, 0);
                M2C_FIELD(var_v0_3, s32 *, 4) = (s32) M2C_FIELD(var_v1_2, s32 *, 4);
                M2C_FIELD(var_v0_3, s32 *, 8) = (s32) M2C_FIELD(var_v1_2, s32 *, 8);
                M2C_FIELD(var_v0_3, s32 *, 0xC) = (s32) M2C_FIELD(var_v1_2, s32 *, 0xC);
                var_v1_2 += 0x10;
                var_v0_3 = (s32 *)((u8 *)var_v0_3 + 0x10);
                if (var_v1_2 != (var_a2 + 0x20))
                {
                    goto loop_18;
                }
                M2C_FIELD(var_v0_3, s32 *, 0) = M2C_FIELD(var_v1_2, s32 *, 0);
                M2C_FIELD(var_v0_3, s32 *, 4) = (s32) M2C_FIELD(var_v1_2, s32 *, 4);
                temp_a0 = (M2C_FIELD(temp_a1, s16 *, 8) + 0x140 + D_800D06E0) % 640;
                temp_v0 = temp_a0 - 0x140;
                temp_a0_2 = temp_a0 - 0xA0;
                M2C_FIELD(temp_a1, s16 *, 0x18) = temp_v0;
                M2C_FIELD(temp_a1, s16 *, 8) = temp_v0;
                M2C_FIELD(temp_a1, s16 *, 0x20) = temp_a0_2;
                M2C_FIELD(temp_a1, s16 *, 0x10) = temp_a0_2;
                M2C_FIELD(temp_a1, s32 *, 4) = (s32) sp0;
                M2C_FIELD(temp_a1, u8 *, 7) = (u8) (M2C_FIELD(temp_a1, u8 *, 7) | 2);
                temp_a0_3 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                *temp_a0_3 = (*temp_a0_3 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFFFFFF);
                M2C_FIELD(D_801398EC, s32 *, 0x334) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x28;
                    M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x28);
                }
                var_v1_3 = var_a2;
                temp_a1_2 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                var_v0_4 = temp_a1_2;
loop_22:
                M2C_FIELD(var_v0_4, s32 *, 0) = M2C_FIELD(var_v1_3, s32 *, 0);
                M2C_FIELD(var_v0_4, s32 *, 4) = (s32) M2C_FIELD(var_v1_3, s32 *, 4);
                M2C_FIELD(var_v0_4, s32 *, 8) = (s32) M2C_FIELD(var_v1_3, s32 *, 8);
                M2C_FIELD(var_v0_4, s32 *, 0xC) = (s32) M2C_FIELD(var_v1_3, s32 *, 0xC);
                var_v1_3 += 0x10;
                var_v0_4 = (s32 *)((u8 *)var_v0_4 + 0x10);
                if (var_v1_3 != (var_a2 + 0x20))
                {
                    goto loop_22;
                }
                M2C_FIELD(var_v0_4, s32 *, 0) = M2C_FIELD(var_v1_3, s32 *, 0);
                M2C_FIELD(var_v0_4, s32 *, 4) = (s32) M2C_FIELD(var_v1_3, s32 *, 4);
                temp_a0_4 = (M2C_FIELD(temp_a1_2, s16 *, 8) + D_800D06E0) % 640;
                temp_v0_2 = temp_a0_4 - 0x140;
                temp_a0_5 = temp_a0_4 - 0xA0;
                M2C_FIELD(temp_a1_2, s16 *, 0x18) = temp_v0_2;
                M2C_FIELD(temp_a1_2, s16 *, 8) = temp_v0_2;
                M2C_FIELD(temp_a1_2, s16 *, 0x20) = temp_a0_5;
                M2C_FIELD(temp_a1_2, s16 *, 0x10) = temp_a0_5;
                M2C_FIELD(temp_a1_2, s32 *, 4) = (s32) sp0;
                M2C_FIELD(temp_a1_2, u8 *, 7) = (u8) (M2C_FIELD(temp_a1_2, u8 *, 7) | 2);
                temp_a0_6 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                *temp_a0_6 = (*temp_a0_6 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFFFFFF);
                M2C_FIELD(D_801398EC, s32 *, 0x334) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x28;
                    M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x28);
                }
                var_a2 += 0x28;
            } while ((s32) var_a2 < (s32) ((u8 *)&D_800D0554 + 0xA0));
            if (!(D_8011CF74 & 3))
            {
                D_800D06E0 = (D_800D06E0 + 1) & 0x7FFF;
            }
        }
        var_a2_2 = &D_800D05F4;
        do
        {
            var_v1_4 = var_a2_2;
            temp_a1_3 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            var_v0_5 = temp_a1_3;
loop_31:
            M2C_FIELD(var_v0_5, s32 *, 0) = M2C_FIELD(var_v1_4, s32 *, 0);
            M2C_FIELD(var_v0_5, s32 *, 4) = (s32) M2C_FIELD(var_v1_4, s32 *, 4);
            M2C_FIELD(var_v0_5, s32 *, 8) = (s32) M2C_FIELD(var_v1_4, s32 *, 8);
            M2C_FIELD(var_v0_5, s32 *, 0xC) = (s32) M2C_FIELD(var_v1_4, s32 *, 0xC);
            var_v1_4 += 0x10;
            var_v0_5 = (s32 *)((u8 *)var_v0_5 + 0x10);
            if (var_v1_4 != (var_a2_2 + 0x20))
            {
                goto loop_31;
            }
            M2C_FIELD(var_v0_5, s32 *, 0) = M2C_FIELD(var_v1_4, s32 *, 0);
            M2C_FIELD(var_v0_5, s32 *, 4) = (s32) M2C_FIELD(var_v1_4, s32 *, 4);
            temp_a0_7 = ((M2C_FIELD(temp_a1_3, s16 *, 8) + 0x8140) - D_800D06E0) % 640;
            temp_v0_3 = temp_a0_7 - 0x140;
            temp_a0_8 = temp_a0_7 - 0xA0;
            M2C_FIELD(temp_a1_3, s16 *, 0x18) = temp_v0_3;
            M2C_FIELD(temp_a1_3, s16 *, 8) = temp_v0_3;
            M2C_FIELD(temp_a1_3, s16 *, 0x20) = temp_a0_8;
            M2C_FIELD(temp_a1_3, s16 *, 0x10) = temp_a0_8;
            M2C_FIELD(temp_a1_3, s32 *, 4) = (s32) sp0;
            temp_a0_9 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            *temp_a0_9 = (*temp_a0_9 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32 *, 0x334) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x28;
                M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x28);
            }
            var_v1_5 = var_a2_2;
            temp_a1_4 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            var_v0_6 = temp_a1_4;
loop_35:
            M2C_FIELD(var_v0_6, s32 *, 0) = M2C_FIELD(var_v1_5, s32 *, 0);
            M2C_FIELD(var_v0_6, s32 *, 4) = (s32) M2C_FIELD(var_v1_5, s32 *, 4);
            M2C_FIELD(var_v0_6, s32 *, 8) = (s32) M2C_FIELD(var_v1_5, s32 *, 8);
            M2C_FIELD(var_v0_6, s32 *, 0xC) = (s32) M2C_FIELD(var_v1_5, s32 *, 0xC);
            var_v1_5 += 0x10;
            var_v0_6 = (s32 *)((u8 *)var_v0_6 + 0x10);
            if (var_v1_5 != (var_a2_2 + 0x20))
            {
                goto loop_35;
            }
            M2C_FIELD(var_v0_6, s32 *, 0) = M2C_FIELD(var_v1_5, s32 *, 0);
            M2C_FIELD(var_v0_6, s32 *, 4) = (s32) M2C_FIELD(var_v1_5, s32 *, 4);
            temp_a0_10 = ((M2C_FIELD(temp_a1_4, s16 *, 8) + 0x8000) - D_800D06E0) % 640;
            temp_v0_4 = temp_a0_10 - 0x140;
            temp_a0_11 = temp_a0_10 - 0xA0;
            M2C_FIELD(temp_a1_4, s16 *, 0x18) = temp_v0_4;
            M2C_FIELD(temp_a1_4, s16 *, 8) = temp_v0_4;
            M2C_FIELD(temp_a1_4, s16 *, 0x20) = temp_a0_11;
            M2C_FIELD(temp_a1_4, s16 *, 0x10) = temp_a0_11;
            M2C_FIELD(temp_a1_4, s32 *, 4) = (s32) sp0;
            temp_a0_12 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            *temp_a0_12 = (*temp_a0_12 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32 *, 0x334) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x28;
                M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x28);
            }
            var_a2_2 += 0x28;
        } while ((s32) var_a2_2 < (s32) ((u8 *)&D_800D05F4 + 0xA0));
        return 1;
    }
    temp_v0_5 = M2C_FIELD(&D_800D9240, u8 *, 4) & 0xFF;
    if (D_80182D74 != temp_v0_5)
    {
        var_v0_7 = M2C_FIELD(&D_800D9240, u8 *, 4) + 8;
        if (temp_v0_5 >= (u8) D_80182D74)
        {
            var_v0_7 = M2C_FIELD(&D_800D9240, u8 *, 4) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 4) = var_v0_7;
    }
    temp_v0_6 = M2C_FIELD(&D_800D9240, u8 *, 5) & 0xFF;
    if (D_80182D75 != temp_v0_6)
    {
        var_v0_8 = M2C_FIELD(&D_800D9240, u8 *, 5) + 8;
        if (temp_v0_6 >= (u8) D_80182D75)
        {
            var_v0_8 = M2C_FIELD(&D_800D9240, u8 *, 5) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 5) = var_v0_8;
    }
    temp_v0_7 = M2C_FIELD(&D_800D9240, u8 *, 6) & 0xFF;
    if (D_80182D76 != temp_v0_7)
    {
        var_v0_9 = M2C_FIELD(&D_800D9240, u8 *, 6) + 8;
        if (temp_v0_7 >= (u8) D_80182D76)
        {
            var_v0_9 = M2C_FIELD(&D_800D9240, u8 *, 6) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 6) = var_v0_9;
    }
    temp_v0_8 = M2C_FIELD(&D_800D9240, u8 *, 0xC) & 0xFF;
    if (D_80182D80 != temp_v0_8)
    {
        var_v0_10 = M2C_FIELD(&D_800D9240, u8 *, 0xC) + 8;
        if (temp_v0_8 >= (u8) D_80182D80)
        {
            var_v0_10 = M2C_FIELD(&D_800D9240, u8 *, 0xC) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0xC) = var_v0_10;
    }
    temp_v0_9 = M2C_FIELD(&D_800D9240, u8 *, 0xD) & 0xFF;
    if (D_80182D81 != temp_v0_9)
    {
        var_v0_11 = M2C_FIELD(&D_800D9240, u8 *, 0xD) + 8;
        if (temp_v0_9 >= (u8) D_80182D81)
        {
            var_v0_11 = M2C_FIELD(&D_800D9240, u8 *, 0xD) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0xD) = var_v0_11;
    }
    temp_v0_10 = M2C_FIELD(&D_800D9240, u8 *, 0xE) & 0xFF;
    if (D_80182D82 != temp_v0_10)
    {
        var_v0_12 = M2C_FIELD(&D_800D9240, u8 *, 0xE) + 8;
        if (temp_v0_10 >= (u8) D_80182D82)
        {
            var_v0_12 = M2C_FIELD(&D_800D9240, u8 *, 0xE) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0xE) = var_v0_12;
    }
    temp_v0_11 = M2C_FIELD(&D_800D9240, u8 *, 0x14) & 0xFF;
    if (D_80182D8C != temp_v0_11)
    {
        var_v0_13 = M2C_FIELD(&D_800D9240, u8 *, 0x14) + 8;
        if (temp_v0_11 >= (u8) D_80182D8C)
        {
            var_v0_13 = M2C_FIELD(&D_800D9240, u8 *, 0x14) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0x14) = var_v0_13;
    }
    temp_v0_12 = M2C_FIELD(&D_800D9240, u8 *, 0x15) & 0xFF;
    if (D_80182D8D != temp_v0_12)
    {
        var_v0_14 = M2C_FIELD(&D_800D9240, u8 *, 0x15) + 8;
        if (temp_v0_12 >= (u8) D_80182D8D)
        {
            var_v0_14 = M2C_FIELD(&D_800D9240, u8 *, 0x15) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0x15) = var_v0_14;
    }
    temp_v0_13 = M2C_FIELD(&D_800D9240, u8 *, 0x16) & 0xFF;
    if (D_80182D8E != temp_v0_13)
    {
        var_v0_15 = M2C_FIELD(&D_800D9240, u8 *, 0x16) + 8;
        if (temp_v0_13 >= (u8) D_80182D8E)
        {
            var_v0_15 = M2C_FIELD(&D_800D9240, u8 *, 0x16) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0x16) = var_v0_15;
    }
    temp_v0_14 = M2C_FIELD(&D_800D9240, u8 *, 0x1C) & 0xFF;
    if (D_80182D94 != temp_v0_14)
    {
        var_v0_16 = M2C_FIELD(&D_800D9240, u8 *, 0x1C) + 8;
        if (temp_v0_14 >= (u8) D_80182D94)
        {
            var_v0_16 = M2C_FIELD(&D_800D9240, u8 *, 0x1C) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0x1C) = var_v0_16;
    }
    temp_v0_15 = M2C_FIELD(&D_800D9240, u8 *, 0x1D) & 0xFF;
    if (D_80182D95 != temp_v0_15)
    {
        var_v0_17 = M2C_FIELD(&D_800D9240, u8 *, 0x1D) + 8;
        if (temp_v0_15 >= (u8) D_80182D95)
        {
            var_v0_17 = M2C_FIELD(&D_800D9240, u8 *, 0x1D) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0x1D) = var_v0_17;
    }
    temp_v0_16 = M2C_FIELD(&D_800D9240, u8 *, 0x1E) & 0xFF;
    if (D_80182D96 != temp_v0_16)
    {
        var_v0_18 = M2C_FIELD(&D_800D9240, u8 *, 0x1E) + 8;
        if (temp_v0_16 >= (u8) D_80182D96)
        {
            var_v0_18 = M2C_FIELD(&D_800D9240, u8 *, 0x1E) - 8;
        }
        M2C_FIELD(&D_800D9240, u8 *, 0x1E) = var_v0_18;
    }
    var_a1_2 = &D_800D9240;
    var_v1_6 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
    do
    {
        M2C_FIELD(var_v1_6, s32 *, 0) = M2C_FIELD(var_a1_2, s32 *, 0);
        M2C_FIELD(var_v1_6, s32 *, 4) = (s32) M2C_FIELD(var_a1_2, s32 *, 4);
        M2C_FIELD(var_v1_6, s32 *, 8) = (s32) M2C_FIELD(var_a1_2, s32 *, 8);
        M2C_FIELD(var_v1_6, s32 *, 0xC) = (s32) M2C_FIELD(var_a1_2, s32 *, 0xC);
        var_a1_2 = (s32 *)((u8 *)var_a1_2 + 0x10);
        var_v1_6 = (s32 *)((u8 *)var_v1_6 + 0x10);
    } while (var_a1_2 != ((u8 *)&D_800D9240 + 0x20));
    *var_v1_6 = *var_a1_2;
    temp_a1_5 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
    *temp_a1_5 = (*temp_a1_5 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFFFFFF);
    M2C_FIELD(D_801398EC, s32 *, 0x334) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x334) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x24;
        M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x24);
    }
    return 1;
}
