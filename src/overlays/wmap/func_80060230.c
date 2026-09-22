/* Partial WMAP decompilation: 56.248890% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_8006534C(u16, M2C_UNK, void *, void *); /* extern */
extern u8 D_80051A6C;
extern u16 D_800D0368;
extern u8 D_800D03EC;
extern u8 D_800D040C;
extern s32 D_800D921C;
extern u16 D_8011CF78;
extern u8 D_8011D518;
extern s32 D_801398BC;
extern void *D_801398EC;
extern s32 D_8013B258;
extern s32 D_8013B28C;
extern u16 D_80182DD0;
extern s32 D_80182E04;
extern u8 D_80182E08;

void func_80060230(void)
{
    s32 var_t0;
    u16 var_v0;
    u8 temp_v1;
    void *temp_a0;
    void *temp_a2;
    void *temp_a2_2;
    void *temp_v1_2;
    void *var_a3;

    if (M2C_FIELD(&D_8011D518, u8 *, 4) != 0)
    {
        M2C_FIELD(&D_8011D518, u8 *, 6) = (u8) M2C_FIELD(&D_8011D518, u8 *, 4);
        M2C_FIELD(&D_8011D518, u8 *, 5) = (u8) M2C_FIELD(&D_8011D518, u8 *, 4);
        temp_a2 = M2C_FIELD(D_801398EC, void **, 0x33C);
        if (!(M2C_FIELD(&D_8011D518, u8 *, 4) & 0x80))
        {
            var_v0 = D_800D0368;
            M2C_FIELD(&D_8011D518, u8 *, 4) = (u8) (M2C_FIELD(&D_8011D518, u8 *, 4) + 8);
        }
        else
        {
            var_v0 = D_800D0368 + 1;
        }
        M2C_FIELD(&D_8011D518, s16 *, 0xE) = (s16) ((var_v0 << 6) | 0x2F);
        M2C_FIELD(temp_a2, s32 *, 0) = (s32) M2C_FIELD(&D_8011D518, s32 *, 0);
        M2C_FIELD(temp_a2, s32 *, 4) = (s32) M2C_FIELD(&D_8011D518, u8 *, 4);
        M2C_FIELD(temp_a2, s32 *, 8) = (s32) M2C_FIELD(&D_8011D518, s32 *, 8);
        M2C_FIELD(temp_a2, s32 *, 0xC) = (s32) M2C_FIELD(&D_8011D518, s32 *, 0xC);
        M2C_FIELD(temp_a2, s32 *, 0x10) = (s32) M2C_FIELD(&D_8011D518, s32 *, 0x10);
        M2C_FIELD(temp_a2, u8 *, 7) = (u8) (M2C_FIELD(temp_a2, u8 *, 7) | 2);
        M2C_FIELD(temp_a2, s32 *, 0) = (s32) ((M2C_FIELD(temp_a2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x7C) & 0xFFFFFF));
        M2C_FIELD(D_801398EC, s32 *, 0x7C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x7C) & 0xFF000000) | ((s32) temp_a2 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x14);
        }
        func_8006534C(D_8011CF78, 3, temp_a2, D_801398EC);
    }
    temp_v1 = M2C_FIELD(&D_80182E08, u8 *, 4);
    if (temp_v1 != 0)
    {
        temp_a2_2 = M2C_FIELD(D_801398EC, void **, 0x33C);
        M2C_FIELD(&D_80182E08, u8 *, 4) = (u8) (temp_v1 - 8);
        M2C_FIELD(&D_80182E08, u8 *, 6) = temp_v1;
        M2C_FIELD(&D_80182E08, u8 *, 5) = temp_v1;
        M2C_FIELD(temp_a2_2, s32 *, 0) = (s32) M2C_FIELD(&D_80182E08, s32 *, 0);
        M2C_FIELD(temp_a2_2, s32 *, 4) = (s32) M2C_FIELD(&D_80182E08, u8 *, 4);
        M2C_FIELD(temp_a2_2, s32 *, 8) = (s32) M2C_FIELD(&D_80182E08, s32 *, 8);
        M2C_FIELD(temp_a2_2, s32 *, 0xC) = (s32) M2C_FIELD(&D_80182E08, s32 *, 0xC);
        M2C_FIELD(temp_a2_2, s32 *, 0x10) = (s32) M2C_FIELD(&D_80182E08, s32 *, 0x10);
        if (D_80182E04 != -1)
        {
            M2C_FIELD(temp_a2_2, s32 *, 0) = (s32) ((M2C_FIELD(temp_a2_2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x7C) & 0xFFFFFF));
            M2C_FIELD(D_801398EC, s32 *, 0x7C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x7C) & 0xFF000000) | ((s32) temp_a2_2 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x14);
            }
            func_8006534C(D_80182DD0, 3, temp_a2_2, D_801398EC);
        }
    }
    if (D_8013B28C != D_801398BC)
    {
        D_8013B28C = D_801398BC;
    }
    if (D_8013B258 == 0)
    {
        var_t0 = *((D_8013B28C * 4) + (u8 *)&D_80051A6C);
        var_a3 = (var_t0 * 6) + (u8 *)&D_800D040C;
        do
        {
            temp_v1_2 = (M2C_FIELD(var_a3, u8 *, 0) * 8) + (u8 *)&D_800D03EC;
            temp_a0 = M2C_FIELD(D_801398EC, void **, 0x33C);
            M2C_FIELD(temp_a0, u8 *, 0xC) = (u8) M2C_FIELD(temp_v1_2, u8 *, 0);
            M2C_FIELD(temp_a0, s8 *, 0xD) = (s8) (M2C_FIELD(temp_v1_2, u8 *, 2) - 0x20);
            M2C_FIELD(temp_a0, u16 *, 0x10) = (u16) M2C_FIELD(temp_v1_2, u16 *, 4);
            M2C_FIELD(temp_a0, u16 *, 0x12) = (u16) M2C_FIELD(temp_v1_2, u16 *, 6);
            M2C_FIELD(temp_a0, u16 *, 8) = (u16) M2C_FIELD(var_a3, u16 *, 2);
            M2C_FIELD(temp_a0, s16 *, 0xE) = 0x7F2E;
            M2C_FIELD(temp_a0, s32 *, 4) = 0x808080;
            M2C_FIELD(temp_a0, s8 *, 3) = 4;
            M2C_FIELD(temp_a0, s8 *, 7) = 0x64;
            M2C_FIELD(temp_a0, s16 *, 0xA) = (s16) M2C_FIELD(var_a3, u8 *, 4);
            M2C_FIELD(temp_a0, s32 *, 0) = (s32) ((M2C_FIELD(temp_a0, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFFFFFF));
            M2C_FIELD(D_801398EC, s32 *, 0x74) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFF000000) | ((s32) temp_a0 & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x14;
                M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x14);
            }
            var_t0 += 1;
            var_a3 += 6;
        } while (*(((D_8013B28C + 1) * 4) + (u8 *)&D_80051A6C) != var_t0);
        func_8006534C(0xBU, 1, (void *) D_800D921C, var_a3);
    }
}
