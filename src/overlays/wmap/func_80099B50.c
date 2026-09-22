/* Partial WMAP decompilation: 78.951220% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_8006CBD8(void *, s32 *, s32, s32);  /* extern */
extern s32 D_800D9164;
extern s32 D_800DCED8;
extern s32 D_800DCEE4;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_8019D240;
extern u8 func_80099E84;
extern u8 func_80099F98;

s32 func_80099B50(s32 arg3)
{
    void *var_a0_3;
    s32 *var_a1;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_t1;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_a3;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v1;

    var_a3 = arg3;
    D_8019D240 = -1;
    D_800D9164 = 0;
    if (D_800DCEF8 == D_800DCED8)
    {
        if (D_800DCF00 == D_800DCEE4)
        {
            goto block_12;
        }
        goto block_4;
    }
    if (D_800DCF00 == D_800DCEE4)
    {
block_4:
        var_a1 = &D_800DCF00;
        temp_a2 = D_800DCED8 - D_800DCEF8;
        var_a0 = D_800DCEE4 - D_800DCF00;
        if (temp_a2 != 0)
        {
            var_v0 = temp_a2;
            if (temp_a2 < 0)
            {
                var_v0 = -var_v0;
            }
            var_a2 = (temp_a2 / var_v0) + 1;
        }
        else
        {
            var_a2 = 1;
        }
        if (var_a0 == 0)
        {
            var_a0_2 = 1;
        }
        else
        {
            goto block_21;
        }
        goto block_25;
    }
block_12:
    var_a2 = D_800DCED8;
    temp_v1 = var_a2 - D_800DCEF8;
    var_a0 = D_800DCEE4 - D_800DCF00;
    var_a1 = (s32 *) temp_v1;
    if (temp_v1 < 0)
    {
        var_a1 = (s32 *) -(s32) var_a1;
    }
    var_v0_2 = var_a0;
    if (var_a0 < 0)
    {
        var_v0_2 = -var_v0_2;
    }
    if (var_a1 == var_v0_2)
    {
        if (temp_v1 != 0)
        {
            var_a2 = (temp_v1 / (s32) var_a1) + 1;
        }
        else
        {
            var_a2 = 1;
        }
        if (var_a0 != 0)
        {
block_21:
            var_v0_3 = var_a0;
            if (var_a0 < 0)
            {
                var_v0_3 = -var_v0_3;
            }
            var_a0_2 = (var_a0 / var_v0_3) + 1;
        }
        else
        {
            var_a0_2 = 1;
        }
block_25:
        D_8019D240 = var_a2 + (var_a0_2 * 3);
    }
    if (D_8019D240 != -1)
    {
        D_801398D0 = 2;
        var_a0_3 = &func_80099E84;
    }
    else
    {
        var_a2 = D_800DCED8 - D_800DCEF8;
        temp_t1 = D_800DCEE4 - D_800DCF00;
        var_v1 = var_a2;
        if (var_a2 < 0)
        {
            var_v1 = -var_v1;
        }
        var_v0_4 = temp_t1;
        if (temp_t1 < 0)
        {
            var_v0_4 = -var_v0_4;
        }
        temp_a3 = var_v1 - var_v0_4;
        if (temp_a3 > 0)
        {
            var_v0_5 = 0;
            if (var_a2 != 0)
            {
                var_v0_5 = -1;
                if (var_a2 > 0)
                {
                    var_v0_5 = 1;
                }
            }
            var_a1 = &D_80182D68;
            D_80182D78 = 0;
            var_a3 = temp_a3 * var_v0_5;
            D_80182D68 = var_a3 * 0x30;
            D_800DCEF8 += var_a3;
            if (var_a2 > 0)
            {
                var_v0_6 = 5;
            }
            else
            {
                var_v0_6 = 3;
            }
        }
        else
        {
            temp_v1_2 = D_800DCF00 - D_800DCEE4;
            var_v0_7 = 0;
            if (temp_v1_2 != 0)
            {
                var_v0_7 = -1;
                if (temp_v1_2 > 0)
                {
                    var_v0_7 = 1;
                }
            }
            var_a1 = &D_80182D78;
            D_80182D68 = 0;
            var_a3 = temp_a3 * var_v0_7;
            D_80182D78 = var_a3 * 0x30;
            D_800DCF00 += var_a3;
            if (temp_t1 > 0)
            {
                var_v0_6 = 7;
            }
            else
            {
                var_v0_6 = 1;
            }
        }
        D_8019D240 = var_v0_6;
        var_a0_3 = &func_80099F98;
    }
    func_8006CBD8(var_a0_3, var_a1, var_a2, var_a3);
    return 0;
}
