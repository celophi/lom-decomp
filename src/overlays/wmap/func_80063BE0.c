/* Partial WMAP decompilation: 85.911220% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern void *D_800D0454;
extern s32 D_800D9220;
extern s32 D_800DCE98;
extern s32 D_800DCE9C;
extern s32 D_8011CF50;
extern s32 D_8011CF74;
extern s32 D_8013922C;
extern s32 D_80139868;
extern s32 D_801398C0;
extern s32 D_801398D4;
extern s32 D_8013B26C;
extern s32 D_8019D6D8;

void func_80063BE0(void)
{
    s16 temp_v1_2;
    s16 temp_v1_3;
    s16 var_a0;
    s16 var_a0_2;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    u16 temp_a0;
    u16 temp_v1;
    u8 temp_v0;

    if ((D_8013B26C != 0) || (D_8011CF50 != 0))
    {
        D_8013922C = 0;
        D_801398C0 = 0;
        return;
    }
    temp_v1 = M2C_FIELD(D_800D0454, u16 *, 2);
    temp_a0 = M2C_FIELD(D_800D0454, u16 *, 6);
    D_801398C0 = (s32) temp_v1;
    D_801398C0 = (temp_v1 >> 8) | (temp_v1 << 8);
    D_8013922C = (s32) temp_a0;
    temp_a1 = (temp_a0 >> 8) | (temp_a0 << 8);
    D_8013922C = temp_a1;
    temp_v0 = M2C_FIELD(D_800D0454, u8 *, 0);
    if (temp_v0 == 0)
    {
        D_800DCE9C = 0;
        D_800DCE98 = 0;
    }
    else if ((s32) temp_v0 >= 0)
    {
        if ((s32) temp_v0 < 3)
        {
            temp_v1_2 = M2C_FIELD(D_800D0454, s16 *, 0xC);
            var_a0 = temp_v1_2;
            if (temp_v1_2 < 0)
            {
                var_a0 = -var_a0;
            }
            if (var_a0 >= 8)
            {
                var_a0 = 7;
            }
            if (temp_v1_2 < 0)
            {
                var_v1 = temp_a1;
                if (((s32) D_8011CF74 % (s32) (0x200 >> var_a0)) == 0)
                {
                    var_v1 |= 0x8000;
                }
                D_8013922C = var_v1;
            }
            if (M2C_FIELD(D_800D0454, s16 *, 0xC) > 0)
            {
                var_v1_2 = D_8013922C;
                if (((s32) D_8011CF74 % (s32) (0x200 >> var_a0)) == 0)
                {
                    var_v1_2 |= 0x2000;
                }
                D_8013922C = var_v1_2;
            }
            temp_v1_3 = M2C_FIELD(D_800D0454, s16 *, 0xE);
            var_a0_2 = temp_v1_3;
            if (temp_v1_3 < 0)
            {
                var_a0_2 = -var_a0_2;
            }
            if (var_a0_2 >= 8)
            {
                var_a0_2 = 7;
            }
            if (temp_v1_3 < 0)
            {
                var_v1_3 = D_8013922C;
                if (((s32) D_8011CF74 % (s32) (0x200 >> var_a0_2)) == 0)
                {
                    var_v1_3 |= 0x1000;
                }
                D_8013922C = var_v1_3;
            }
            if (M2C_FIELD(D_800D0454, s16 *, 0xE) > 0)
            {
                var_v1_4 = D_8013922C;
                if (((s32) D_8011CF74 % (s32) (0x200 >> var_a0_2)) == 0)
                {
                    var_v1_4 |= 0x4000;
                }
                D_8013922C = var_v1_4;
            }
        }
        else
        {
            goto block_32;
        }
    }
    else
    {
block_32:
        D_8013922C = 0;
        D_801398C0 = 0;
    }
    if (D_801398C0 & 0x200)
    {
        D_801398C0 |= 0x40;
    }
    if (D_8013922C & 0x200)
    {
        D_8013922C |= 0x40;
    }
    temp_a1_2 = D_801398C0 & D_80139868;
    D_801398C0 = temp_a1_2;
    temp_a0_2 = D_8013922C & D_80139868;
    D_8013922C = temp_a0_2;
    if (D_801398D4 != 0)
    {
        D_801398C0 = temp_a1_2 & D_800D9220;
        D_8013922C = temp_a0_2 & D_800D9220;
    }
    M2C_FIELD(D_800D0454, u8 *, 0x91) = (u8) D_800DCE98;
    M2C_FIELD(D_800D0454, u8 *, 0x92) = (u8) D_800DCE9C;
    D_8019D6D8 = 0;
}
