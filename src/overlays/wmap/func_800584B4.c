/* Partial WMAP decompilation: 83.588780% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK akao_cmd_f1();                              /* extern */
M2C_UNK func_80059070();                            /* extern */
void func_800652A8(s32, s32);
extern u8 D_800D9268;
extern s32 D_800DCEE8;
extern s32 D_8011CF44;
extern s32 D_80139230;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182DF8;
extern u8 D_8019D248;
extern s32 D_801ADB00;

void func_800584B4(void)
{
    void *var_a1;
    void *var_a3;
    s16 temp_a0;
    s16 temp_a0_2;
    s16 temp_a0_3;
    s16 temp_a0_6;
    s16 temp_a0_7;
    s16 temp_t0;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 var_a2;
    s16 var_v0;
    s16 var_v0_2;
    s32 temp_a0_4;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 var_t1;
    s32 var_t2;
    u16 temp_v0_4;
    u16 temp_v1;
    u16 temp_v1_2;
    u16 temp_v1_3;
    void *temp_a0_5;
    void *temp_t6;

    var_t1 = 0;
    D_80139230 = 0;
    temp_t6 = (u8 *)&D_8019D248 + 0x36C;
    var_a3 = &D_800D9268;
    var_a1 = &D_8019D248;
    var_t2 = 0;
    do
    {
        temp_a0 = M2C_FIELD(var_a1, s16 *, 0xC);
        temp_v0 = M2C_FIELD(var_a1, s16 *, 0x10);
        temp_v1 = (u16) M2C_FIELD(var_a1, s16 *, 0xC);
        var_a2 = 0;
        if (temp_a0 != temp_v0)
        {
            var_a2 = 1;
            if (temp_v0 < temp_a0)
            {
                var_v0 = temp_v1 - 4;
            }
            else
            {
                var_v0 = temp_v1 + 4;
            }
            M2C_FIELD(var_a1, s16 *, 0xC) = var_v0;
        }
        temp_a0_2 = M2C_FIELD(var_a1, s16 *, 0xE);
        temp_v0_2 = M2C_FIELD(var_a1, s16 *, 0x12);
        temp_v1_2 = (u16) M2C_FIELD(var_a1, s16 *, 0xE);
        if (temp_a0_2 != temp_v0_2)
        {
            var_a2 = 1;
            if (temp_v0_2 < temp_a0_2)
            {
                var_v0_2 = temp_v1_2 - 4;
            }
            else
            {
                var_v0_2 = temp_v1_2 + 4;
            }
            M2C_FIELD(var_a1, s16 *, 0xE) = var_v0_2;
        }
        if (var_a2 == 0)
        {
            if (((M2C_FIELD(var_a1, s32 *, 0) != M2C_FIELD(var_a1, s16 *, 8)) || (M2C_FIELD(var_a1, s32 *, 4) != M2C_FIELD(var_a1, s16 *, 0xA))) && (var_t1 == 0))
            {
                D_800DCEE8 = 1;
            }
            temp_v0_3 = M2C_FIELD(var_a1, s16 *, 8);
            temp_a0_3 = M2C_FIELD(var_a1, s16 *, 0xA);
            M2C_FIELD(var_a1, s32 *, 0) = (s32) temp_v0_3;
            M2C_FIELD(var_a1, s32 *, 4) = (s32) temp_a0_3;
            if ((M2C_FIELD(var_a1, s32 *, 0x18) != temp_v0_3) || (M2C_FIELD(var_a1, s32 *, 0x1C) != temp_a0_3))
            {
                temp_t0 = M2C_FIELD(var_a1, s16 *, 8);
                var_a2 = M2C_FIELD(var_a1, s16 *, 0xA);
                temp_a0_4 = M2C_FIELD(var_a1, s32 *, 0x20) + 1;
                M2C_FIELD(var_a1, s32 *, 0x20) = temp_a0_4;
                temp_a0_5 = (temp_a0_4 * 4) + var_t2 + (u8 *)&D_8019D248;
                temp_v1_3 = M2C_FIELD(temp_a0_5, u16 *, 0x24);
                M2C_FIELD(var_a1, s16 *, 8) = (s16) temp_v1_3;
                M2C_FIELD(var_a1, s16 *, 0x10) = (s16) (((s16) temp_v1_3 - 1) * 0xA0);
                temp_v0_4 = M2C_FIELD(temp_a0_5, u16 *, 0xA4);
                M2C_FIELD(var_a1, s16 *, 0xA) = (s16) temp_v0_4;
                M2C_FIELD(var_a1, s16 *, 0x12) = (s16) (((s16) temp_v0_4 - 1) * 0xA0);
                if ((D_80182DF8 != 0) && (var_t1 == 3))
                {
                    D_801398D0 = 2;
                    D_80182D68 = (M2C_FIELD(temp_t6, s16 *, 8) - temp_t0) * 0x30;
                    D_80182D78 = (M2C_FIELD(temp_t6, s16 *, 0xA) - var_a2) * 0x30;
                }
                M2C_FIELD(var_a1, s32 *, 0x14) = 1;
            }
            else
            {
                M2C_FIELD(var_a1, s32 *, 0x14) = 0;
                M2C_FIELD(var_a3, s16 *, 0xE) = 0;
                if ((var_t1 == 3) && (D_80182DF8 != 0))
                {
                    D_80182DF8 = 0;
                }
            }
        }
        if (M2C_FIELD(var_a1, s32 *, 0x14) != 0)
        {
            if (M2C_FIELD(var_a1, s16 *, 0x10) == M2C_FIELD(var_a1, s16 *, 0xC))
            {
                temp_a0_6 = M2C_FIELD(var_a1, s16 *, 0x12);
                temp_v1_4 = M2C_FIELD(var_a1, s16 *, 0xE);
                if (temp_v1_4 < temp_a0_6)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 1;
                }
                else if (temp_a0_6 < temp_v1_4)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 4;
                }
            }
            if (M2C_FIELD(var_a1, s16 *, 0x12) == M2C_FIELD(var_a1, s16 *, 0xE))
            {
                temp_a0_7 = M2C_FIELD(var_a1, s16 *, 0x10);
                temp_v1_5 = M2C_FIELD(var_a1, s16 *, 0xC);
                if (temp_v1_5 < temp_a0_7)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 2;
                }
                else if (temp_a0_7 < temp_v1_5)
                {
                    M2C_FIELD(var_a3, s16 *, 0xE) = 3;
                }
            }
        }
        var_a3 += 0x2C;
        temp_v1_6 = M2C_FIELD(var_a1, s32 *, 0x14);
        var_a1 += 0x124;
        var_t2 += 0x124;
        var_t1 += 1;
        temp_v1_7 = D_80139230 | temp_v1_6;
        D_80139230 = temp_v1_7;
    } while (var_t1 < 4);
    if (temp_v1_7 != D_801ADB00)
    {
        if ((temp_v1_7 != 0) && (D_8011CF44 == 0))
        {
            func_800652A8(0x15, 0x80);
            D_801ADB00 = D_80139230;
        }
        if ((D_80139230 != D_801ADB00) && (D_80139230 == 0))
        {
            akao_cmd_f1();
            D_801ADB00 = D_80139230;
        }
    }
    if (D_800DCEE8 != 0)
    {
        func_80059070();
        D_800DCEE8 = 0;
    }
}
