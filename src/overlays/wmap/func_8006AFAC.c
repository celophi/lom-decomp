/* Partial WMAP decompilation: 71.771300% (gcc280_g0). */
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

M2C_UNK func_80066F9C(void *, s32, M2C_UNK, s32, s32); /* extern */
s32 rand(void);
extern u8 D_800D9268;
extern u16 D_80139980;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern s32 D_801B0FD0;

void func_8006AFAC(s32 arg0, s32 arg1, M2C_UNK arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u16 arg7)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0;
    s32 temp_a1;
    s32 temp_lo;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s3;
    s32 var_s4;
    u16 temp_v0_4;
    u8 var_v1;
    void *temp_s0;
    void *temp_v0_2;
    void *temp_v0_3;
    void *temp_v1;
    void *var_s0;
    void *var_s1;
    void *var_s1_2;

    var_s3 = 0;
    var_s2 = arg0;
    if (var_s2 < arg1)
    {
        var_s1 = (var_s2 * 0x14) + (u8 *)&D_801AFBD0;
        var_s4 = var_s2 * 0x2C;
        do
        {
            if (M2C_FIELD(var_s1, s16 *, 0) != 0)
            {
                temp_s0 = var_s4 + (u8 *)&D_800D9268;
                position.vx = (s16) ((s32) (((s32) M2C_FIELD(var_s1, s32 *, 8) >> 6) * (ccos(M2C_FIELD(var_s1, s16 *, 2)) >> 6)) >> 0xC);
                temp_lo = ((s32) M2C_FIELD(var_s1, s32 *, 8) >> 6) * (csin(M2C_FIELD(var_s1, s16 *, 2)) >> 6);
                position.vz = 0;
                position.vy = (s16) (temp_lo >> 0xC);
                gte_ldv0(&position);
                gte_rtps();
                M2C_FIELD(var_s1, s32 *, 8) = (s32) (M2C_FIELD(var_s1, s32 *, 8) + M2C_FIELD(var_s1, s32 *, 4));
                M2C_FIELD(temp_s0, u16 *, 0x22) = (u16) D_80139980;
                M2C_FIELD(temp_s0, u16 *, 0x24) = (u16) D_80139980;
                gte_stsxy(&sp20);
                temp_a1 = M2C_FIELD(((var_s2 * 8) + (u8 *)&D_80139988), s32 *, 4);
                temp_v0 = M2C_FIELD(temp_s0, s16 *, 0xE);
                if (M2C_FIELD(temp_s0, s16 *, 0x10) != temp_v0)
                {
                    M2C_FIELD(temp_s0, s16 *, 0x10) = (s16) (u16) M2C_FIELD(temp_s0, s16 *, 0xE);
                    M2C_FIELD(temp_s0, s16 *, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16 *)((temp_v0 * 2) + temp_a1);
                    M2C_FIELD(temp_s0, void **, 0x18) = temp_v1;
                    M2C_FIELD(temp_s0, void **, 0x14) = temp_v1;
                }
                if (M2C_FIELD(temp_s0, s16 *, 0x20) != 0xFF)
                {
                    M2C_FIELD(temp_s0, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(temp_s0, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(temp_s0, s16 *, 0x20) == 0)
                {
                    temp_v0_2 = M2C_FIELD(temp_s0, void **, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_2, u8 *, 0);
                    M2C_FIELD(temp_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_2, u8 *, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_3 = M2C_FIELD(temp_s0, void **, 0x18);
                        M2C_FIELD(temp_s0, void **, 0x14) = temp_v0_3;
                        var_v1 = M2C_FIELD(temp_v0_3, u8 *, 0);
                        M2C_FIELD(temp_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_3, u8 *, 1);
                    }
                    M2C_FIELD(temp_s0, void **, 0x14) = (void *) (M2C_FIELD(temp_s0, void **, 0x14) + 4);
                    M2C_FIELD(temp_s0, s32 *, 0x1C) = (s32) (temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16 *, 0x40));
                }
                func_80066F9C(temp_s0, sp20, arg2, arg3, 0);
                temp_v0_4 = M2C_FIELD(var_s1, u16 *, 0xC) - 1;
                M2C_FIELD(var_s1, u16 *, 0xC) = temp_v0_4;
                if ((temp_v0_4 << 0x10) == 0)
                {
                    M2C_FIELD(var_s1, s16 *, 0) = 0;
                }
                var_s3 += 1;
            }
            var_s1 += 0x14;
            var_s2 += 1;
            var_s4 += 0x2C;
        } while (var_s2 < arg1);
    }
    var_s2_2 = arg0;
    if (var_s2_2 < arg1)
    {
        var_s0 = (var_s2_2 * 0x14) + (u8 *)&D_801AFBD0;
        var_s1_2 = (var_s2_2 * 0x2C) + (u8 *)&D_800D9268;
loop_17:
        if (M2C_FIELD(var_s0, s16 *, 0) == 0)
        {
            var_s3 += 1;
            if (D_801B0FD0 >= var_s3)
            {
                M2C_FIELD(var_s1_2, s16 *, 2) = 0;
                M2C_FIELD(var_s1_2, s8 *, 6) = 0xF;
                M2C_FIELD(var_s1_2, s16 *, 0x10) = -1;
                M2C_FIELD(var_s1_2, u16 *, 0xE) = arg7;
                M2C_FIELD(var_s0, s16 *, 0) = 1;
                M2C_FIELD(var_s0, s16 *, 2) = (s16) (rand() >> 3);
                M2C_FIELD(var_s0, s32 *, 8) = 0;
                M2C_FIELD(var_s0, s32 *, 4) = (s32) (arg4 + (rand() / arg5));
                M2C_FIELD(var_s0, s16 *, 0xC) = (s16) (arg6 + (rand() & 0x3C));
                goto block_20;
            }
        }
        else
        {
block_20:
            var_s0 += 0x14;
            var_s2_2 += 1;
            var_s1_2 += 0x2C;
            if (var_s2_2 < arg1)
            {
                goto loop_17;
            }
        }
    }
}
