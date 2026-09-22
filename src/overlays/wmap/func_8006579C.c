#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"
/* Partial WMAP decompilation: 91.988010% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_80065E20();                            /* static */
M2C_UNK func_800660BC();                            /* static */
extern u8 D_80054934;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCF04;
extern s32 D_8011CF18;
extern s32 D_8011CF44;
extern s32 D_8011CF50;
extern s32 D_8011CF7C;
extern s32 D_8013922C;
extern s32 D_8013986C;
extern s32 D_801398C0;
extern s32 D_801398D0;
extern void *D_801398EC;
extern u8 D_80139950;
extern s32 D_80139954;
extern s32 D_80182D68;
extern s32 D_80182D78;

s32 func_8006579C(void)
{
    s32 *temp_a0;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_t1;
    s32 var_t3;
    s32 var_v0;
    void *temp_a1_3;

    if (D_8013986C == -1)
    {
        func_8006AEE0();
        return 1;
    }
    if ((D_8013986C == 0) && (D_801398D0 == 0) && (D_8011CF18 == 0))
    {
        if (D_8013922C & 0x8000)
        {
            temp_a1 = ((D_800DCEEC - 2) * 0x18) + 0x80;
            D_800DCEEC -= 1;
            func_800652A8(3, temp_a1);
            if (D_800DCEEC < 0)
            {
                D_800DCEEC = 0;
                if (M2C_FIELD(&D_80139950, s32 *, 0) > 0)
                {
                    D_80182D68 = -0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
        if (D_8013922C & 0x2000)
        {
            temp_a1_2 = (D_800DCEEC * 0x18) + 0x80;
            D_800DCEEC += 1;
            func_800652A8(3, temp_a1_2);
            temp_v1 = *((D_8013986C * 4) + (u8 *)&D_80054934);
            if (temp_v1 < D_800DCEEC)
            {
                D_800DCEEC = temp_v1;
                if (M2C_FIELD(&D_80139950, s32 *, 0) < 0x90)
                {
                    D_80182D68 = 0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
        if (D_8013922C & 0x1000)
        {
            func_800652A8(3, ((D_800DCEEC - 1) * 0x18) + 0x80);
            temp_v0 = D_800DCEF0 - 1;
            D_800DCEF0 = temp_v0;
            if (temp_v0 < 0)
            {
                D_800DCEF0 = 0;
                if (D_80139954 > 0)
                {
                    D_80182D78 = -0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
        if (D_8013922C & 0x4000)
        {
            func_800652A8(3, ((D_800DCEEC - 1) * 0x18) + 0x80);
            temp_v1_2 = *((D_8013986C * 4) + (u8 *)&D_80054934);
            temp_v0_2 = D_800DCEF0 + 1;
            D_800DCEF0 = temp_v0_2;
            if (temp_v1_2 < temp_v0_2)
            {
                D_800DCEF0 = temp_v1_2;
                if (D_80139954 < 0x90)
                {
                    D_80182D78 = 0x30;
                    if (D_8011CF7C != 0)
                    {
                        D_801398D0 = 1;
                    }
                }
            }
        }
    }
    if (D_8013986C == 1)
    {
        if (D_8013922C & 0x8000)
        {
            temp_v0_3 = D_800DCF04 - 1;
            D_800DCF04 = temp_v0_3;
            if (temp_v0_3 < 0)
            {
                D_800DCF04 = 8;
            }
        }
        if (D_8013922C & 0x2000)
        {
            D_800DCF04 = (D_800DCF04 + 1) % 9;
        }
    }
    if (D_801398D0 != 0)
    {
        if (D_8013986C == 1)
        {
            D_80182D78 = 0;
            D_801398D0 = 0;
            D_80182D68 = 0;
        }
        else
        {
            D_8013922C = 0;
            if ((D_80182D78 | D_80182D68) == 0)
            {
                D_801398D0 = 0;
                if (D_8011CF44 == 0)
                {
                    D_8011CF50 = 0;
                }
            }
            if (D_80182D78 != 0)
            {
                var_a0 = -4;
                if (D_8011CF7C != 0)
                {
                    D_8011CF50 = 1;
                    D_801398C0 = 0;
                    D_8013922C = 0;
                    if (D_80182D78 > 0)
                    {
                        var_a0 = 4;
                    }
                    D_80182D78 -= var_a0;
                    temp_v0_4 = M2C_FIELD(&D_80139950, s32 *, 4) + var_a0;
                    M2C_FIELD(&D_80139950, s32 *, 4) = temp_v0_4;
                    if (D_801398D0 == 1)
                    {
                        if (temp_v0_4 < 0)
                        {
                            M2C_FIELD(&D_80139950, s32 *, 4) = 0;
                            D_80182D78 = 0;
                            D_8013922C = 0;
                            D_801398D0 = 0;
                        }
                        if (M2C_FIELD(&D_80139950, s32 *, 4) >= 0x91)
                        {
                            M2C_FIELD(&D_80139950, s32 *, 4) = 0x90;
                            D_80182D78 = 0;
                            D_801398D0 = 0;
                            D_8013922C &= ~0x5000;
                        }
                    }
                }
            }
            if (D_80182D68 != 0)
            {
                var_a0_2 = -4;
                if (D_8011CF7C != 0)
                {
                    D_8011CF50 = 1;
                    D_801398C0 = 0;
                    D_8013922C = 0;
                    if (D_80182D68 > 0)
                    {
                        var_a0_2 = 4;
                    }
                    D_80182D68 -= var_a0_2;
                    temp_v0_5 = M2C_FIELD(&D_80139950, s32 *, 0) + var_a0_2;
                    M2C_FIELD(&D_80139950, s32 *, 0) = temp_v0_5;
                    if (D_801398D0 == 1)
                    {
                        if (temp_v0_5 < 0)
                        {
                            M2C_FIELD(&D_80139950, s32 *, 0) = 0;
                            D_80182D68 = 0;
                            D_8013922C = 0;
                            D_801398D0 = 0;
                        }
                        if (M2C_FIELD(&D_80139950, s32 *, 0) >= 0x91)
                        {
                            M2C_FIELD(&D_80139950, s32 *, 0) = 0x90;
                            D_80182D68 = 0;
                            D_801398D0 = 0;
                            D_8013922C &= 0xFFFF5FFF;
                        }
                    }
                }
            }
        }
    }
    func_800660BC();
    func_80065E20();
    var_t3 = 1;
    var_t1 = 0x1A;
    do
    {
        var_a3 = 1;
        var_v0 = var_t1 + 1;
loop_60:
        temp_v1_3 = var_v0 * 0x28;
        temp_a1_3 = temp_v1_3 + D_801398EC;
        var_a3 += 1;
        M2C_FIELD(temp_a1_3, s32 *, 0x340) = (s32) ((M2C_FIELD(temp_a1_3, s32 *, 0x340) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x32C) & 0xFFFFFF));
        M2C_FIELD(D_801398EC, s32 *, 0x32C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x32C) & 0xFF000000) | ((s32) (D_801398EC + (temp_v1_3 + 0x340)) & 0xFFFFFF));
        var_v0 = var_t1 + var_a3;
        if (var_a3 < 0x19)
        {
            goto loop_60;
        }
        var_t3 += 1;
        var_t1 += 0x1A;
    } while (var_t3 < 0x19);
    var_a3_2 = 0;
    var_a2 = 0x6CE0;
    do
    {
        temp_a0 = D_801398EC + var_a2;
        var_a3_2 += 1;
        *temp_a0 = (*temp_a0 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x32C) & 0xFFFFFF);
        M2C_FIELD(D_801398EC, s32 *, 0x32C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x32C) & 0xFF000000) | ((s32) temp_a0 & 0xFFFFFF));
        var_a2 += 0x18;
    } while (var_a3_2 < 0xB8);
    M2C_FIELD(D_801398EC, s32 *, 0x7E20) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x7E20) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x32C) & 0xFFFFFF));
    M2C_FIELD(D_801398EC, s32 *, 0x32C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x32C) & 0xFF000000) | ((s32) (D_801398EC + 0x7E20) & 0xFFFFFF));
    return 1;
}
