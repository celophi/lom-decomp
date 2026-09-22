/* Partial WMAP decompilation: 78.884170% (gcc280_g0). */
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

M2C_UNK func_80066F9C(void *, s32, s32, M2C_UNK, s32); /* extern */
s32 rand(void);
extern s32 D_8011CF74;
extern u8 D_801AFBD0;
extern s32 D_801B0FD0;

void func_8006A9C4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, u16 arg9, s32 arg10, s32 arg11)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0;
    s32 temp_a1;
    s32 temp_lo;
    s32 var_a1;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s4;
    u16 temp_v0_4;
    u8 var_v1;
    void *temp_v0_2;
    void *temp_v0_3;
    void *temp_v1;
    void *var_a0;
    void *var_s0;
    void *var_s0_2;
    void *var_s1;
    void *var_s3;

    var_s4 = 0;
    var_s2 = arg2;
    if (var_s2 < arg3)
    {
        var_s1 = (var_s2 * 0x14) + (u8 *)&D_801AFBD0;
        var_s0 = (var_s2 * 0x2C) + arg0;
        var_s3 = (var_s2 * 8) + arg1;
        do
        {
            if (M2C_FIELD(var_s1, s16 *, 0) != 0)
            {
                position.vx = (s16) ((s32) (((s32) M2C_FIELD(var_s1, s32 *, 8) >> 6) * (ccos(M2C_FIELD(var_s1, s16 *, 2)) >> 6)) >> 0xC);
                position.vy = (s16) ((s32) (((s32) M2C_FIELD(var_s1, s32 *, 8) >> 6) * (csin(M2C_FIELD(var_s1, s16 *, 2)) >> 6)) >> 0xC);
                position.vz = (s16) (0xC8 / (s16) M2C_FIELD(var_s1, s16 *, 0xE));
                gte_ldv0(&position);
                gte_rtps();
                var_a1 = arg4 * 0x81;
                M2C_FIELD(var_s1, s16 *, 0xE) = (s16) ((u16) M2C_FIELD(var_s1, s16 *, 0xE) + 1);
                M2C_FIELD(var_s1, s32 *, 8) = (s32) (M2C_FIELD(var_s1, s32 *, 8) + M2C_FIELD(var_s1, s32 *, 4));
                M2C_FIELD(var_s0, s16 *, 0x22) = 0;
                if (var_a1 < 0)
                {
                    var_a1 += 0xFF;
                }
                M2C_FIELD(var_s0, s16 *, 0x24) = (s16) (var_a1 >> 8);
                gte_stsxy(&sp20);
                temp_a1 = M2C_FIELD(var_s3, s32 *, 4);
                temp_v0 = M2C_FIELD(var_s0, s16 *, 0xE);
                if (M2C_FIELD(var_s0, s16 *, 0x10) != temp_v0)
                {
                    M2C_FIELD(var_s0, s16 *, 0x10) = (s16) (u16) M2C_FIELD(var_s0, s16 *, 0xE);
                    M2C_FIELD(var_s0, s16 *, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16 *)((temp_v0 * 2) + temp_a1);
                    M2C_FIELD(var_s0, void **, 0x18) = temp_v1;
                    M2C_FIELD(var_s0, void **, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0, s16 *, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(var_s0, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0, s16 *, 0x20) == 0)
                {
                    temp_v0_2 = M2C_FIELD(var_s0, void **, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_2, u8 *, 0);
                    M2C_FIELD(var_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_2, u8 *, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_3 = M2C_FIELD(var_s0, void **, 0x18);
                        M2C_FIELD(var_s0, void **, 0x14) = temp_v0_3;
                        var_v1 = M2C_FIELD(temp_v0_3, u8 *, 0);
                        M2C_FIELD(var_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_3, u8 *, 1);
                    }
                    M2C_FIELD(var_s0, void **, 0x14) = (void *) (M2C_FIELD(var_s0, void **, 0x14) + 4);
                    M2C_FIELD(var_s0, s32 *, 0x1C) = (s32) (temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16 *, 0x40));
                }
                func_80066F9C(var_s0, sp20, arg10, 0xA, 0);
                temp_v0_4 = M2C_FIELD(var_s1, u16 *, 0xC) - 1;
                M2C_FIELD(var_s1, u16 *, 0xC) = temp_v0_4;
                if ((temp_v0_4 << 0x10) == 0)
                {
                    M2C_FIELD(var_s1, s16 *, 0) = 0;
                }
                var_s4 += 1;
            }
            var_s1 += 0x14;
            var_s0 += 0x2C;
            var_s2 += 1;
            var_s3 += 8;
        } while (var_s2 < arg3);
    }
    if ((((s32) D_8011CF74 % arg11) == 0) && (var_s2_2 = arg2, ((var_s2_2 < arg3) != 0)))
    {
        var_s0_2 = (var_s2_2 * 0x14) + (u8 *)&D_801AFBD0;
        var_a0 = (var_s2_2 * 0x2C) + arg0;
loop_20:
        var_s2_2 += 1;
        if (M2C_FIELD(var_s0_2, s16 *, 0) == 0)
        {
            if (D_801B0FD0 >= var_s4)
            {
                M2C_FIELD(var_a0, s8 *, 6) = 0xF;
                M2C_FIELD(var_a0, s16 *, 2) = 0;
                M2C_FIELD(var_a0, s16 *, 0xE) = 1;
                M2C_FIELD(var_a0, s16 *, 0x10) = -1;
                M2C_FIELD(var_s0_2, s16 *, 0) = 1;
                M2C_FIELD(var_s0_2, s16 *, 2) = (s16) (rand() >> 3);
                M2C_FIELD(var_s0_2, s32 *, 8) = 0;
                M2C_FIELD(var_s0_2, s32 *, 4) = (s32) (((s32) (rand() * arg6) >> 0xF) + arg5);
                temp_lo = rand() * arg8;
                M2C_FIELD(var_s0_2, u16 *, 0xE) = arg9;
                M2C_FIELD(var_s0_2, s16 *, 0xC) = (s16) ((temp_lo >> 0xF) + arg7);
            }
        }
        else
        {
            var_s0_2 += 0x14;
            var_a0 += 0x2C;
            if (var_s2_2 >= arg3)
            {

            }
            else
            {
                goto loop_20;
            }
        }
    }
}
