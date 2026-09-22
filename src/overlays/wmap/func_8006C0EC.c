/* Partial WMAP decompilation: 68.381390% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

M2C_UNK func_8006AEE0();                            /* extern */
extern u8 D_800DCEB8;
extern s16 D_800DCEBA;
extern s16 D_800DCEBC;
extern u8 D_8011CF4C;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_80139200;
extern s32 D_80139204;
extern u16 D_80139210;
extern u16 D_80139212;
extern u16 D_80139214;
extern u8 D_801398C8;
extern u8 D_80139950;
extern s32 D_80139968;
extern s32 D_8013996C;
extern s32 D_8013B29C;
extern u8 D_80182D48;

s32 func_8006C0EC(void)
{
    SVECTOR position;
    s16 var_v0;
    s16 var_v0_2;
    s16 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;

    func_8006AEE0();
    position.vz = 0;
    position.vx = (s16) ((s32) ((((D_8011D510 - 1) * 0xA0) - ((s32) (M2C_FIELD(&D_80139950, s32 *, 0) * 0x14000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8))) * 0x6000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8));
    position.vy = (s16) ((s32) ((((D_8011D530 - 1) * 0xA0) - ((s32) (M2C_FIELD(&D_80139950, s32 *, 4) * 0x14000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8))) * 0x6000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8));
    gte_ldv0(&position);
                gte_rtps();
    if (M2C_FIELD(&D_800DCEB8, s16 *, 0) != M2C_FIELD(&D_801398C8, s16 *, 0))
    {
        if (M2C_FIELD(&D_800DCEB8, s16 *, 0) < M2C_FIELD(&D_801398C8, s16 *, 0))
        {
            var_v0 = (u16) M2C_FIELD(&D_801398C8, s16 *, 0) - D_80139210;
        }
        else
        {
            var_v0 = (u16) M2C_FIELD(&D_801398C8, s16 *, 0) + D_80139210;
        }
        M2C_FIELD(&D_801398C8, s16 *, 0) = var_v0;
    }
    if (D_800DCEBA != M2C_FIELD(&D_801398C8, s16 *, 2))
    {
        if (D_800DCEBA < M2C_FIELD(&D_801398C8, s16 *, 2))
        {
            var_v0_2 = (u16) M2C_FIELD(&D_801398C8, s16 *, 2) - D_80139212;
        }
        else
        {
            var_v0_2 = (u16) M2C_FIELD(&D_801398C8, s16 *, 2) + D_80139212;
        }
        M2C_FIELD(&D_801398C8, s16 *, 2) = var_v0_2;
    }
    if (D_800DCEBC != M2C_FIELD(&D_801398C8, s16 *, 4))
    {
        if (D_800DCEBC < M2C_FIELD(&D_801398C8, s16 *, 4))
        {
            var_v0_3 = (u16) M2C_FIELD(&D_801398C8, s16 *, 4) - D_80139214;
        }
        else
        {
            var_v0_3 = (u16) M2C_FIELD(&D_801398C8, s16 *, 4) + D_80139214;
        }
        M2C_FIELD(&D_801398C8, s16 *, 4) = var_v0_3;
    }
    if (M2C_FIELD(&D_80139200, s32 *, 0) != M2C_FIELD(&D_80182D48, s32 *, 0))
    {
        if (M2C_FIELD(&D_80139200, s32 *, 0) < M2C_FIELD(&D_80182D48, s32 *, 0))
        {
            var_v0_4 = M2C_FIELD(&D_80182D48, s32 *, 0) - D_80139968;
        }
        else
        {
            var_v0_4 = M2C_FIELD(&D_80182D48, s32 *, 0) + D_80139968;
        }
        M2C_FIELD(&D_80182D48, s32 *, 0) = var_v0_4;
    }
    if (D_80139204 != M2C_FIELD(&D_80182D48, s32 *, 4))
    {
        if (D_80139204 < M2C_FIELD(&D_80182D48, s32 *, 4))
        {
            var_v0_5 = M2C_FIELD(&D_80182D48, s32 *, 4) - D_8013996C;
        }
        else
        {
            var_v0_5 = M2C_FIELD(&D_80182D48, s32 *, 4) + D_8013996C;
        }
        M2C_FIELD(&D_80182D48, s32 *, 4) = var_v0_5;
    }
    if (D_8013B29C != 0)
    {
        gte_stsxy(&D_8011CF4C);
        return D_8013B29C;
    }
    M2C_FIELD(&D_801398C8, s16 *, 0) = 0;
    M2C_FIELD(&D_801398C8, s16 *, 2) = 0;
    M2C_FIELD(&D_801398C8, s16 *, 4) = 0;
    M2C_FIELD(&D_80182D48, s32 *, 0) = 0;
    M2C_FIELD(&D_80182D48, s32 *, 4) = 0;
    M2C_FIELD(&D_80182D48, s32 *, 8) = 0;
    M2C_FIELD(&D_80139200, s32 *, 0) = 0;
    M2C_FIELD(&D_80139200, s32 *, 4) = 0;
    M2C_FIELD(&D_80139200, s32 *, 8) = 0;
    M2C_FIELD(&D_800DCEB8, s16 *, 0) = 0;
    M2C_FIELD(&D_800DCEB8, s16 *, 2) = 0;
    M2C_FIELD(&D_800DCEB8, s16 *, 4) = 0;
    M2C_FIELD(&D_8011CF4C, s16 *, 0) = 0xA4;
    M2C_FIELD(&D_8011CF4C, s16 *, 2) = 0x69;
    return 0;
}
