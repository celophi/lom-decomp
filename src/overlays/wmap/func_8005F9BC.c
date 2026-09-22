/* Partial WMAP decompilation: 38.630726% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_80060230();                            /* extern */
M2C_UNK func_8006534C(M2C_UNK, M2C_UNK, s32, void *); /* extern */
extern u8 D_800515F4;
extern u8 D_80051A58;
extern s32 D_800D921C;
extern u8 D_800DBE80;
extern s32 D_800DBE84;
extern s32 D_8011CF80;
extern u8 D_801391E8;
extern u8 D_801398D8;
extern u8 D_801398DC;
extern void *D_801398EC;
extern s32 D_8013B274;
extern u8 D_80182DA0;
extern s32 D_80182DA4;
extern s32 D_80182E34;

void func_8005F9BC(void)
{
    void *var_a2;
    void *var_a3;
    s32 *temp_a0;
    s32 *temp_a0_3;
    s32 *temp_a0_4;
    s32 *temp_a0_5;
    s32 *temp_a1;
    s32 *temp_a1_2;
    s32 *temp_a1_3;
    s32 *temp_a1_4;
    s32 *temp_a1_5;
    s32 var_t1;
    void *temp_a0_2;

    if (D_80182E34 != 3)
    {
        temp_a1 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
        M2C_FIELD(temp_a1, s32 *, 0) = M2C_FIELD(&D_80051A58, s32 *, 0);
        M2C_FIELD(temp_a1, s32 *, 4) = (s32) M2C_FIELD(&D_80051A58, s32 *, 4);
        M2C_FIELD(temp_a1, s32 *, 8) = (s32) M2C_FIELD(&D_80051A58, s32 *, 8);
        M2C_FIELD(temp_a1, s32 *, 0xC) = (s32) M2C_FIELD(&D_80051A58, s32 *, 0xC);
        M2C_FIELD(temp_a1, s32 *, 0x10) = (s32) M2C_FIELD(&D_80051A58, s32 *, 0x10);
        M2C_FIELD(temp_a1, s32 *, 0) = (M2C_FIELD(temp_a1, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32 *, 0x8C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFF000000) | ((s32) temp_a1 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x14);
        }
        if (D_8013B274 != D_8011CF80)
        {
            D_8011CF80 = D_8013B274;
            M2C_FIELD(&D_801391E8, s32 *, 0) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 0);
            M2C_FIELD(&D_801391E8, s32 *, 4) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 4);
            M2C_FIELD(&D_801391E8, s32 *, 8) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 8);
            M2C_FIELD(&D_801391E8, s32 *, 0xC) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 0xC);
            M2C_FIELD(&D_801391E8, s32 *, 0x10) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 0x10);
            M2C_FIELD(&D_80182DA0, s32 *, 4) = 0;
            M2C_FIELD(&D_800DBE80, s32 *, 0) = (s32) M2C_FIELD(&D_801398D8, s32 *, 0);
            M2C_FIELD(&D_800DBE80, s32 *, 4) = (s32) M2C_FIELD(&D_801398D8, s32 *, 4);
            M2C_FIELD(&D_800DBE80, s32 *, 8) = (s32) M2C_FIELD(&D_801398D8, s32 *, 8);
            M2C_FIELD(&D_800DBE80, s32 *, 0xC) = (s32) M2C_FIELD(&D_801398D8, s32 *, 0xC);
            M2C_FIELD(&D_800DBE80, s32 *, 0x10) = (s32) M2C_FIELD(&D_801398D8, s32 *, 0x10);
            M2C_FIELD(&D_801398D8, s32 *, 0xC) = 0;
            M2C_FIELD(&D_801391E8, u8 *, 7) = (u8) (M2C_FIELD(&D_801391E8, u8 *, 7) | 2);
            M2C_FIELD(&D_80182DA0, s32 *, 0xC) = (s8) (((D_8013B274 & 1) * 0x30) + 8);
            M2C_FIELD(&D_80182DA0, s8 *, 0xD) = (s8) ((((s32) D_8013B274 / 2) * 0x38) + 0xE);
            M2C_FIELD(&D_800DBE80, u8 *, 7) = (u8) (M2C_FIELD(&D_800DBE80, u8 *, 7) | 2);
            M2C_FIELD(&D_801398D8, s8 *, 0xD) = (s8) (D_8013B274 << 5);
        }
        var_a2 = &D_80182DA0;
        M2C_FIELD(&D_80182DA0, u8 *, 5) = (u8) M2C_FIELD(&D_80182DA0, s32 *, 4);
        M2C_FIELD(&D_80182DA0, u8 *, 6) = (u8) M2C_FIELD(&D_80182DA0, s32 *, 4);
        temp_a1_2 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
        M2C_FIELD(temp_a1_2, s32 *, 0) = M2C_FIELD(&D_80182DA0, s32 *, 0);
        M2C_FIELD(temp_a1_2, s32 *, 4) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 4);
        M2C_FIELD(temp_a1_2, s32 *, 8) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 8);
        M2C_FIELD(temp_a1_2, s32 *, 0xC) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 0xC);
        M2C_FIELD(temp_a1_2, s32 *, 0x10) = (s32) M2C_FIELD(&D_80182DA0, s32 *, 0x10);
        if ((s8) M2C_FIELD(&D_80182DA0, s32 *, 4) >= 0)
        {
            M2C_FIELD(&D_80182DA0, s32 *, 4) = (s8) ((u8) M2C_FIELD(&D_80182DA0, s32 *, 4) + 8);
            M2C_FIELD(temp_a1_2, u8 *, 7) = (u8) (M2C_FIELD(temp_a1_2, u8 *, 7) | 2);
        }
        temp_a0 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
        *temp_a0 = (*temp_a0 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32 *, 0x8C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x14);
        }
        if ((u8) M2C_FIELD(&D_801391E8, s32 *, 4) != 0)
        {
            M2C_FIELD(&D_801391E8, u8 *, 5) = (u8) M2C_FIELD(&D_801391E8, s32 *, 4);
            M2C_FIELD(&D_801391E8, u8 *, 6) = (u8) M2C_FIELD(&D_801391E8, s32 *, 4);
            var_a2 = D_801398EC;
            temp_a1_3 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            M2C_FIELD(temp_a1_3, s32 *, 0) = M2C_FIELD(&D_801391E8, s32 *, 0);
            M2C_FIELD(temp_a1_3, s32 *, 4) = (s32) M2C_FIELD(&D_801391E8, s32 *, 4);
            M2C_FIELD(temp_a1_3, s32 *, 8) = (s32) M2C_FIELD(&D_801391E8, s32 *, 8);
            M2C_FIELD(temp_a1_3, s32 *, 0xC) = (s32) M2C_FIELD(&D_801391E8, s32 *, 0xC);
            M2C_FIELD(temp_a1_3, s32 *, 0x10) = (s32) M2C_FIELD(&D_801391E8, s32 *, 0x10);
            M2C_FIELD(temp_a1_3, s32 *, 0) = (M2C_FIELD(temp_a1_3, s32 *, 0) & 0xFF000000) | (M2C_FIELD(var_a2, s32 *, 0x8C) & 0xFFFFFF);
            D_800DBE84 = M2C_FIELD(&D_801391E8, s32 *, 4);
            M2C_FIELD(var_a2, s32 *, 0x8C) = (s32) ((M2C_FIELD(var_a2, s32 *, 0x8C) & 0xFF000000) | ((s32) temp_a1_3 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(var_a2, s32 **, 0x33C) = (s32 *) (M2C_FIELD(var_a2, s32 **, 0x33C) + 0x14);
            }
            M2C_FIELD(&D_801391E8, s32 *, 4) = (s8) ((u8) M2C_FIELD(&D_801391E8, s32 *, 4) - 8);
        }
        func_8006534C(0xBC, 7, (s32) var_a2, &D_801391E8);
        func_80060230();
        temp_a1_4 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
        M2C_FIELD(&D_801398DC, s32 *, 0) = (s32) D_80182DA4;
        temp_a0_2 = &D_801398DC - 4;
        M2C_FIELD(temp_a0_2, u8 *, 7) = (u8) (M2C_FIELD(temp_a0_2, u8 *, 7) | 2);
        M2C_FIELD(temp_a1_4, s32 *, 0) = M2C_FIELD(&D_801398DC, s32 *, -4);
        M2C_FIELD(temp_a1_4, s32 *, 4) = (s32) M2C_FIELD(&D_801398DC, s32 *, 0);
        M2C_FIELD(temp_a1_4, s32 *, 8) = (s32) M2C_FIELD(&D_801398DC, s32 *, 4);
        M2C_FIELD(temp_a1_4, s32 *, 0xC) = (s32) M2C_FIELD(&D_801398DC, s32 *, 8);
        M2C_FIELD(temp_a1_4, s32 *, 0x10) = (s32) M2C_FIELD(&D_801398DC, s32 *, 0xC);
        temp_a0_3 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
        *temp_a0_3 = (*temp_a0_3 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32 *, 0x8C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x14);
        }
        if ((u8) M2C_FIELD(&D_800DBE80, s32 *, 4) >= 9U)
        {
            temp_a1_5 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            M2C_FIELD(temp_a1_5, s32 *, 0) = M2C_FIELD(&D_800DBE80, s32 *, 0);
            M2C_FIELD(temp_a1_5, s32 *, 4) = (s32) M2C_FIELD(&D_800DBE80, s32 *, 4);
            M2C_FIELD(temp_a1_5, s32 *, 8) = (s32) M2C_FIELD(&D_800DBE80, s32 *, 8);
            M2C_FIELD(temp_a1_5, s32 *, 0xC) = (s32) M2C_FIELD(&D_800DBE80, s32 *, 0xC);
            M2C_FIELD(temp_a1_5, s32 *, 0x10) = (s32) M2C_FIELD(&D_800DBE80, s32 *, 0x10);
            temp_a0_4 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            *temp_a0_4 = (*temp_a0_4 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32 *, 0x8C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x8C) & 0xFF000000) | ((s32) M2C_FIELD(D_801398EC, s32 **, 0x33C) & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x14);
            }
        }
        func_8006534C(0x3E, 7, (s32) D_801398EC, (void *)0xFFFFFF);
        var_t1 = 0;
        var_a3 = &D_800515F4;
        do
        {
            temp_a0_5 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
            M2C_FIELD(temp_a0_5, s32 *, 0) = M2C_FIELD(var_a3, s32 *, 0);
            M2C_FIELD(temp_a0_5, s32 *, 4) = (s32) M2C_FIELD(var_a3, s32 *, 4);
            M2C_FIELD(temp_a0_5, s32 *, 8) = (s32) M2C_FIELD(var_a3, s32 *, 8);
            M2C_FIELD(temp_a0_5, s32 *, 0xC) = (s32) M2C_FIELD(var_a3, s32 *, 0xC);
            M2C_FIELD(temp_a0_5, s32 *, 0x10) = (s32) M2C_FIELD(var_a3, s32 *, 0x10);
            M2C_FIELD(temp_a0_5, s32 *, 0x14) = (s32) M2C_FIELD(var_a3, s32 *, 0x14);
            M2C_FIELD(temp_a0_5, s32 *, 0x18) = (s32) M2C_FIELD(var_a3, s32 *, 0x18);
            M2C_FIELD(temp_a0_5, s8 *, 3) = 6;
            M2C_FIELD(temp_a0_5, s8 *, 7) = 0x32;
            M2C_FIELD(temp_a0_5, s32 *, 0) = (M2C_FIELD(temp_a0_5, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x90) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32 *, 0x90) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x90) & 0xFF000000) | ((s32) temp_a0_5 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x1C;
                M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) (M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x1C);
            }
            var_t1 += 1;
            var_a3 += 0x1C;
        } while (var_t1 < 0x24);
        func_8006534C(0x20, 8, D_800D921C, var_a3);
    }
}
