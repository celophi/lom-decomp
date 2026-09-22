#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 59.689026% (gcc280_g0). */
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

M2C_UNK func_80066F9C(void *, s32, s32, s32, s32);  /* extern */
s32 rand(void);
extern s32 D_800D921C;
extern s32 D_800D9230;
extern u8 D_800D9268;
extern s32 D_8011CF74;
extern void *D_801398EC;
extern u8 D_80139988;
extern u8 D_801AFBD0;

void func_8008ECF8(s32 arg0, s32 arg1, s32 arg2, void *arg3)
{
    SVECTOR position;
    s32 sp24;
    s32 sp20;
    s16 *var_v1;
    s16 temp_t0;
    s16 var_v0;
    s16 var_v0_2;
    s16 var_v0_3;
    s32 *var_s6;
    s32 temp_v0;
    s32 var_s2;
    s32 var_s3;
    s32 var_s3_2;
    s32 var_s5;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    u16 temp_v0_2;
    void *temp_a0;
    void *temp_s0;
    void *temp_s0_2;
    void *temp_v1;
    void *temp_v1_2;
    void *var_s0;
    void *var_s1;
    void *var_s2_2;

    var_s2 = 3;
    if (((s32) D_8011CF74 % (s32) M2C_FIELD(arg3, s32 *, 0)) == 0)
    {
        var_s3 = arg0;
        if (var_s3 < arg1)
        {
            var_s0 = (var_s3 * 0x10) + arg2;
            var_v1 = (var_s3 * 0x14) + (u8 *)&D_801AFBD0;
loop_3:
            if ((*var_v1 != 0) || (*var_v1 = 1, M2C_FIELD(var_s0, s16 *, 0) = (s16) (M2C_FIELD(arg3, u16 *, 8) + ((s32) (rand() * M2C_FIELD(arg3, s32 *, 0xC)) >> 0xF)), M2C_FIELD(var_s0, s16 *, 2) = (s16) (M2C_FIELD(arg3, u16 *, 8) + ((s32) (rand() * M2C_FIELD(arg3, s32 *, 0xC)) >> 0xF)), M2C_FIELD(var_s0, u16 *, 4) = (u16) M2C_FIELD(arg3, u16 *, 0x10), M2C_FIELD(var_s0, u16 *, 8) = (u16) M2C_FIELD(arg3, u16 *, 0x18), temp_v1 = var_s0 + 8, M2C_FIELD(temp_v1, u16 *, 2) = (u16) M2C_FIELD(arg3, u16 *, 0x1C), var_s2 -= 1, M2C_FIELD(temp_v1, u16 *, 4) = (u16) M2C_FIELD(arg3, u16 *, 0x20), (var_s2 != 0)))
            {
                var_s0 += 0x10;
                var_s3 += 1;
                var_v1 = (s16 *)((u8 *)var_v1 + 0x14);
                if (var_s3 < arg1)
                {
                    goto loop_3;
                }
            }
        }
    }
    D_800D9230 = 0;
    var_s3_2 = arg0;
    if (var_s3_2 < arg1)
    {
        temp_v0 = var_s3_2 * 0x14;
        var_s2_2 = temp_v0 + (u8 *)&D_801AFBD0;
        var_s6 = temp_v0 + ((u8 *)&D_801AFBD0 + 0x10);
        var_s5 = var_s3_2 * 0x2C;
        var_s1 = (var_s3_2 * 0x10) + arg2;
        do
        {
            temp_t0 = M2C_FIELD(var_s2_2, s16 *, 0);
            if (temp_t0 != 1)
            {
                if (temp_t0 != 2)
                {
                    var_s2_2 += 0x14;
                }
                else
                {
                    if ((M2C_FIELD(arg3, s32 *, 0x28) == -1) || (temp_s0 = var_s5 + (u8 *)&D_800D9268, func_8006CC4C(temp_s0, (var_s3_2 * 8) + (u8 *)&D_80139988), func_80066F9C(temp_s0, *var_s6, M2C_FIELD(arg3, s32 *, 0x28), M2C_FIELD(arg3, s32 *, 4), 0), temp_v0_2 = M2C_FIELD(var_s2_2, u16 *, 0xC) - 1, M2C_FIELD(var_s2_2, u16 *, 0xC) = temp_v0_2, (temp_v0_2 & 0x8000)))
                    {
                        M2C_FIELD(var_s2_2, s16 *, 0) = 0;
                    }
                    goto block_30;
                }
            }
            else
            {
                var_v0 = M2C_FIELD(var_s1, s16 *, 0);
                if (var_v0 < 0)
                {
                    var_v0 += 0xF;
                }
                position.vx = (s16) (var_v0 >> 4);
                var_v0_2 = M2C_FIELD(var_s1, s16 *, 2);
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 0xF;
                }
                position.vy = (s16) (var_v0_2 >> 4);
                var_v0_3 = M2C_FIELD(var_s1, s16 *, 4);
                if (var_v0_3 < 0)
                {
                    var_v0_3 += 0xF;
                }
                position.vz = (s16) (var_v0_3 >> 4);
                gte_ldv0(&position);
                gte_rtps();
                var_v0_4 = M2C_FIELD(var_s1, s16 *, 0) - (M2C_FIELD(var_s1, s16 *, 8) * M2C_FIELD(arg3, s32 *, 0x14));
                if (var_v0_4 < 0)
                {
                    var_v0_4 += 0xF;
                }
                position.vx = (s16) (var_v0_4 >> 4);
                var_v0_5 = M2C_FIELD(var_s1, s16 *, 2) - (M2C_FIELD(var_s1, s16 *, 0xA) * M2C_FIELD(arg3, s32 *, 0x14));
                if (var_v0_5 < 0)
                {
                    var_v0_5 += 0xF;
                }
                position.vy = (s16) (var_v0_5 >> 4);
                var_v0_6 = M2C_FIELD(var_s1, s16 *, 4) - (M2C_FIELD(var_s1, s16 *, 0xC) * M2C_FIELD(arg3, s32 *, 0x14));
                if (var_v0_6 < 0)
                {
                    var_v0_6 += 0xF;
                }
                position.vz = (s16) (var_v0_6 >> 4);
                gte_stsxy(&sp20);
                gte_ldv0(&position);
                gte_rtps();
                gte_stsxy(&sp24);
                temp_a0 = M2C_FIELD(D_801398EC, void **, 0x33C);
                M2C_FIELD(temp_a0, s32 *, 4) = 0x808080;
                M2C_FIELD(temp_a0, s32 *, 0xC) = 0;
                M2C_FIELD(temp_a0, s32 *, 8) = sp20;
                M2C_FIELD(temp_a0, s8 *, 3) = 4;
                M2C_FIELD(temp_a0, s8 *, 7) = 0x52;
                M2C_FIELD(temp_a0, s32 *, 0x10) = sp24;
                M2C_FIELD(temp_a0, s32 *, 0) = (s32) ((M2C_FIELD(temp_a0, s32 *, 0) & 0xFF000000) | (M2C_FIELD(((M2C_FIELD(arg3, s32 *, 4) * 4) + D_801398EC), s32 *, 0x70) & 0xFFFFFF));
                temp_v1_2 = (M2C_FIELD(arg3, s32 *, 4) * 4) + D_801398EC;
                M2C_FIELD(temp_v1_2, s32 *, 0x70) = (s32) ((M2C_FIELD(temp_v1_2, s32 *, 0x70) & 0xFF000000) | ((s32) temp_a0 & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x14;
                    M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x14);
                }
                M2C_FIELD(var_s1, s16 *, 0) = (s16) ((u16) M2C_FIELD(var_s1, s16 *, 0) + (u16) M2C_FIELD(var_s1, s16 *, 8));
                M2C_FIELD(var_s1, s16 *, 4) = (s16) ((u16) M2C_FIELD(var_s1, s16 *, 4) + (u16) M2C_FIELD(var_s1, s16 *, 0xC));
                M2C_FIELD(var_s1, s16 *, 2) = (s16) ((u16) M2C_FIELD(var_s1, s16 *, 2) + (u16) M2C_FIELD(var_s1, s16 *, 0xA));
                D_800D9230 += 1;
                if (M2C_FIELD(var_s1, s16 *, 4) < 0)
                {
                    temp_s0_2 = var_s5 + (u8 *)&D_800D9268;
                    M2C_FIELD(var_s2_2, s32 *, 0x10) = sp20;
                    M2C_FIELD(var_s2_2, s16 *, 0) = 2;
                    M2C_FIELD(var_s2_2, u16 *, 0xC) = (u16) M2C_FIELD(arg3, u16 *, 0x24);
                    M2C_FIELD(temp_s0_2, s16 *, 2) = 0;
                    M2C_FIELD(temp_s0_2, s8 *, 6) = 0xF;
                    M2C_FIELD(temp_s0_2, s16 *, 0x10) = -1;
                    M2C_FIELD(temp_s0_2, u16 *, 0xE) = (u16) M2C_FIELD(arg3, u16 *, 0x2C);
                    M2C_FIELD(temp_s0_2, s16 *, 0x22) = temp_t0;
                    M2C_FIELD(temp_s0_2, s16 *, 0x24) = 0x81;
                    M2C_FIELD(temp_s0_2, s16 *, 0x26) = (s16) (0x80 / (s32) M2C_FIELD(arg3, u16 *, 0x24));
                }
block_30:
                var_s2_2 += 0x14;
            }
            var_s6 = (s32 *)((u8 *)var_s6 + 0x14);
            var_s5 += 0x2C;
            var_s3_2 += 1;
            var_s1 += 0x10;
        } while (var_s3_2 < arg1);
    }
}
