/* Partial WMAP decompilation: 87.746890% (gcc280_g0). */
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
extern u8 D_800D9150;
extern u8 D_800D9268;
extern u8 D_800DCEA8;
extern u8 D_80139988;
extern u8 D_801AFBD0;

void func_8006B328(s32 arg0, s32 arg1, s32 arg2, s16 arg3, s32 arg4, s32 arg5, u16 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11, s32 arg12, u16 arg13, u16 arg14, u16 arg15, s32 arg16)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0_2;
    s32 *temp_a0;
    s32 *temp_a1_3;
    s32 *temp_v0;
    s32 *temp_v1_3;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_v1_2;
    s32 var_s2;
    s32 var_s3;
    s32 var_s4;
    u16 temp_v0_5;
    u8 var_v1;
    void *temp_s0;
    void *temp_s1;
    void *temp_v0_3;
    void *temp_v0_4;
    void *temp_v1;

    temp_a1 = arg16 * 4;
    temp_v0 = temp_a1 + (u8 *)&D_800D9150;
    var_s2 = arg0;
    *temp_v0 -= 1;
    if (var_s2 < arg1)
    {
        var_s4 = var_s2 * 0x2C;
        var_s3 = var_s2 * 0x14;
        do
        {
            temp_s1 = var_s3 + (u8 *)&D_801AFBD0;
            temp_s0 = var_s4 + (u8 *)&D_800D9268;
            if (M2C_FIELD(temp_s1, s16 *, 0) != 0)
            {
                if (arg3 != -1)
                {
                    M2C_FIELD(temp_s0, s16 *, 0x24) = arg3;
                    M2C_FIELD(temp_s0, s16 *, 0x22) = arg3;
                }
                position.vx = M2C_FIELD(temp_s1, u16 *, 0x10);
                position.vy = M2C_FIELD(temp_s1, u16 *, 0x12);
                position.vz = M2C_FIELD(temp_s1, u16 *, 0xE);
                gte_ldv0(&position);
                gte_rtps();
                temp_a1_2 = M2C_FIELD(((var_s2 * 8) + (u8 *)&D_80139988), s32 *, 4);
                temp_v0_2 = M2C_FIELD(temp_s0, s16 *, 0xE);
                if (M2C_FIELD(temp_s0, s16 *, 0x10) != temp_v0_2)
                {
                    M2C_FIELD(temp_s0, s16 *, 0x10) = (s16) (u16) M2C_FIELD(temp_s0, s16 *, 0xE);
                    M2C_FIELD(temp_s0, s16 *, 0x20) = 1;
                    temp_v1 = temp_a1_2 + *(s16 *)((temp_v0_2 * 2) + temp_a1_2);
                    M2C_FIELD(temp_s0, void **, 0x18) = temp_v1;
                    M2C_FIELD(temp_s0, void **, 0x14) = temp_v1;
                }
                if (M2C_FIELD(temp_s0, s16 *, 0x20) != 0xFF)
                {
                    M2C_FIELD(temp_s0, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(temp_s0, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(temp_s0, s16 *, 0x20) == 0)
                {
                    temp_v0_3 = M2C_FIELD(temp_s0, void **, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_3, u8 *, 0);
                    M2C_FIELD(temp_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_3, u8 *, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_4 = M2C_FIELD(temp_s0, void **, 0x18);
                        M2C_FIELD(temp_s0, void **, 0x14) = temp_v0_4;
                        var_v1 = M2C_FIELD(temp_v0_4, u8 *, 0);
                        M2C_FIELD(temp_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_4, u8 *, 1);
                    }
                    M2C_FIELD(temp_s0, void **, 0x14) = (void *) (M2C_FIELD(temp_s0, void **, 0x14) + 4);
                    M2C_FIELD(temp_s0, s32 *, 0x1C) = (s32) (temp_a1_2 + M2C_FIELD(((var_v1 * 2) + temp_a1_2), s16 *, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(temp_s0, sp20, arg7, 4, 0);
                temp_v0_5 = M2C_FIELD(temp_s1, u16 *, 0xE) - M2C_FIELD(temp_s1, u16 *, 4);
                M2C_FIELD(temp_s1, u16 *, 0xE) = temp_v0_5;
                if ((s16) temp_v0_5 < arg12)
                {
                    M2C_FIELD(temp_s0, s16 *, 0x22) = 0;
                }
                if ((s16) M2C_FIELD(temp_s1, u16 *, 0xE) < 0)
                {
                    M2C_FIELD(temp_s1, u16 *, 0xE) = 0U;
                }
                if ((M2C_FIELD(temp_s0, s16 *, 0x22) == 0) && (M2C_FIELD(temp_s0, s16 *, 0x24) < 5))
                {
                    M2C_FIELD(temp_s1, s16 *, 0) = 0;
                }
            }
            else
            {
                temp_a1_3 = temp_a1 + (u8 *)&D_800D9150;
                if (*temp_a1_3 == 0)
                {
                    temp_a0 = temp_a1 + (u8 *)&D_800DCEA8;
                    temp_v1_2 = *temp_a0;
                    if (temp_v1_2 != 0)
                    {
                        if (temp_v1_2 == 1)
                        {
                            *temp_a1_3 = arg2;
                        }
                        else
                        {
                            *temp_a0 = temp_v1_2 - 1;
                        }
                        M2C_FIELD(temp_s1, s16 *, 0) = 1;
                        M2C_FIELD(temp_s1, u16 *, 0xE) = arg6;
                        M2C_FIELD(temp_s1, u16 *, 0x10) = (u16) (((s32) (rand() * arg9) >> 0xF) + arg8);
                        M2C_FIELD(temp_s1, u16 *, 0x12) = (u16) (((s32) (rand() * arg11) >> 0xF) + arg10);
                        M2C_FIELD(temp_s1, u16 *, 4) = (s32) (((s32) (rand() * arg5) >> 0xF) + arg4);
                        M2C_FIELD(temp_s0, s16 *, 0x22) = (s16) arg13;
                        M2C_FIELD(temp_s0, s16 *, 0x24) = (s16) arg14;
                        M2C_FIELD(temp_s0, u16 *, 0x26) = arg15;
                    }
                }
            }
            var_s4 += 0x2C;
            var_s2 += 1;
            var_s3 += 0x14;
        } while (var_s2 < arg1);
    }
    temp_v1_3 = (arg16 * 4) + (u8 *)&D_800D9150;
    if (*temp_v1_3 == 0)
    {
        *temp_v1_3 = arg2;
    }
}
