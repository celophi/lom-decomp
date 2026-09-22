/* Partial WMAP decompilation: 66.979866% (gcc280_g0). */
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
extern u8 D_800D9268;
extern u8 D_80139988;
extern u8 D_801AFBD0;

void func_8006BC44(s32 arg0, s32 arg1, void *arg2, s32 arg3)
{
    SVECTOR position;
    u32 sp20;
    s16 temp_v0_4;
    s16 temp_v1_2;
    s32 temp_a1_2;
    s32 temp_v0_3;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s4_2;
    u8 var_v1;
    void *temp_a1;
    void *temp_s1;
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v0_5;
    void *temp_v0_6;
    void *temp_v1;
    void *var_a0;
    void *var_s0;
    void *var_s0_2;
    void *var_s4;
    void *var_s5;
    void *var_v0;

    M2C_FIELD(arg2, s32 *, 8) = (s32) (M2C_FIELD(arg2, s32 *, 8) - 1);
    var_s2 = arg0;
    if (var_s2 < (var_s2 + arg1))
    {
        var_s5 = (var_s2 * 8) + (u8 *)&D_80139988;
        var_s4 = (var_s2 * 0x14) + (u8 *)&D_801AFBD0;
        var_s0 = (var_s2 * 0x2C) + (u8 *)&D_800D9268;
        do
        {
            position.vx = (s16) ((s32) (((s32) M2C_FIELD(var_s4, s32 *, 8) >> 3) * (ccos(M2C_FIELD(var_s4, s16 *, 2)) >> 6)) >> 0xC);
            position.vy = (s16) ((s32) (((s32) M2C_FIELD(var_s4, s32 *, 8) >> 3) * (csin(M2C_FIELD(var_s4, s16 *, 2)) >> 6)) >> 0xC);
            position.vz = M2C_FIELD(var_s4, u16 *, 0xE);
            gte_ldv0(&position);
                gte_rtps();
            M2C_FIELD(var_s4, s32 *, 8) = (s32) (M2C_FIELD(var_s4, s32 *, 8) - M2C_FIELD(arg2, s32 *, 0x10));
            M2C_FIELD(var_s4, s16 *, 2) = (s16) ((u16) M2C_FIELD(var_s4, s16 *, 2) + M2C_FIELD(arg2, u16 *, 0x14));
            gte_stsxy(&sp20);
            M2C_FIELD(var_s4, u16 *, 0x10) = sp20;
            M2C_FIELD(var_s4, u16 *, 0x12) = (u16) M2C_FIELD(&sp20, u16 *, 2);
            if (M2C_FIELD(arg2, s32 *, 8) == 0)
            {
                var_a0 = var_s0;
                if (M2C_FIELD(arg2, s32 *, 0) != -1)
                {
                    temp_a1 = ((var_s2 + M2C_FIELD(arg2, s32 *, 4)) * 0x2C) + (u8 *)&D_800D9268;
                    var_v0 = temp_a1;
                    do
                    {
                        M2C_FIELD(var_v0, s32 *, 0) = (s32) M2C_FIELD(var_a0, s32 *, 0);
                        M2C_FIELD(var_v0, s32 *, 4) = (s32) M2C_FIELD(var_a0, s32 *, 4);
                        M2C_FIELD(var_v0, s32 *, 8) = (s32) M2C_FIELD(var_a0, s32 *, 8);
                        M2C_FIELD(var_v0, s32 *, 0xC) = (s32) M2C_FIELD(var_a0, s32 *, 0xC);
                        var_a0 += 0x10;
                        var_v0 += 0x10;
                    } while (var_a0 != (var_s0 + 0x20));
                    M2C_FIELD(var_v0, s32 *, 0) = (s32) M2C_FIELD(var_a0, s32 *, 0);
                    M2C_FIELD(var_v0, s32 *, 4) = (s32) M2C_FIELD(var_a0, s32 *, 4);
                    M2C_FIELD(var_v0, s32 *, 8) = (s32) M2C_FIELD(var_a0, s32 *, 8);
                    temp_v0 = ((var_s2 + M2C_FIELD(arg2, s32 *, 4)) * 0x14) + (u8 *)&D_801AFBD0;
                    M2C_FIELD(temp_v0, s32 *, 0) = (s32) M2C_FIELD(var_s4, s32 *, 0);
                    M2C_FIELD(temp_v0, s32 *, 4) = (s32) M2C_FIELD(var_s4, s32 *, 4);
                    M2C_FIELD(temp_v0, s32 *, 8) = (s32) M2C_FIELD(var_s4, s32 *, 8);
                    M2C_FIELD(temp_v0, s32 *, 0xC) = (s32) M2C_FIELD(var_s4, s32 *, 0xC);
                    M2C_FIELD(temp_v0, s32 *, 0x10) = (s32) M2C_FIELD(var_s4, u16 *, 0x10);
                    temp_v0_2 = ((var_s2 + M2C_FIELD(arg2, s32 *, 4)) * 8) + (u8 *)&D_80139988;
                    M2C_FIELD(temp_v0_2, s32 *, 0) = (s32) M2C_FIELD(var_s5, s32 *, 0);
                    M2C_FIELD(temp_v0_2, s32 *, 4) = (s32) M2C_FIELD(var_s5, s32 *, 4);
                    M2C_FIELD(temp_a1, u16 *, 0x26) = (u16) M2C_FIELD(arg2, u16 *, 0x18);
                    M2C_FIELD(temp_a1, u16 *, 0x22) = (u16) M2C_FIELD(arg2, u16 *, 0x1C);
                    if (arg3 == 0)
                    {
                        M2C_FIELD(temp_a1, s16 *, 0xE) = 1;
                    }
                }
            }
            var_s5 += 0x78;
            var_s4 += 0x12C;
            var_s2 += 0xF;
            var_s0 += 0x294;
        } while (var_s2 < (arg0 + arg1));
    }
    if (M2C_FIELD(arg2, s32 *, 8) == 0)
    {
        temp_v0_3 = M2C_FIELD(arg2, s32 *, 4) + 1;
        M2C_FIELD(arg2, s32 *, 4) = temp_v0_3;
        M2C_FIELD(arg2, s32 *, 8) = (s32) M2C_FIELD(arg2, s32 *, 0);
        if (temp_v0_3 >= 0xF)
        {
            M2C_FIELD(arg2, s32 *, 4) = 1;
        }
    }
    var_s2_2 = arg0;
    if (var_s2_2 < (var_s2_2 + arg1))
    {
        var_s4_2 = var_s2_2 * 0x14;
        var_s0_2 = (var_s2_2 * 0x2C) + (u8 *)&D_800D9268;
        do
        {
            temp_s1 = var_s4_2 + (u8 *)&D_801AFBD0;
            if (M2C_FIELD(temp_s1, s16 *, 0) != 0)
            {
                temp_a1_2 = M2C_FIELD(((var_s2_2 * 8) + (u8 *)&D_80139988), s32 *, 4);
                temp_v0_4 = M2C_FIELD(var_s0_2, s16 *, 0xE);
                if (M2C_FIELD(var_s0_2, s16 *, 0x10) != temp_v0_4)
                {
                    M2C_FIELD(var_s0_2, s16 *, 0x10) = (s16) (u16) M2C_FIELD(var_s0_2, s16 *, 0xE);
                    M2C_FIELD(var_s0_2, s16 *, 0x20) = 1;
                    temp_v1 = temp_a1_2 + *(s16 *)((temp_v0_4 * 2) + temp_a1_2);
                    M2C_FIELD(var_s0_2, void **, 0x18) = temp_v1;
                    M2C_FIELD(var_s0_2, void **, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0_2, s16 *, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0_2, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(var_s0_2, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0_2, s16 *, 0x20) == 0)
                {
                    temp_v0_5 = M2C_FIELD(var_s0_2, void **, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_5, u8 *, 0);
                    M2C_FIELD(var_s0_2, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_5, u8 *, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_6 = M2C_FIELD(var_s0_2, void **, 0x18);
                        M2C_FIELD(var_s0_2, void **, 0x14) = temp_v0_6;
                        var_v1 = M2C_FIELD(temp_v0_6, u8 *, 0);
                        M2C_FIELD(var_s0_2, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_6, u8 *, 1);
                    }
                    M2C_FIELD(var_s0_2, void **, 0x14) = (void *) (M2C_FIELD(var_s0_2, void **, 0x14) + 4);
                    M2C_FIELD(var_s0_2, s32 *, 0x1C) = (s32) (temp_a1_2 + M2C_FIELD(((var_v1 * 2) + temp_a1_2), s16 *, 0x40));
                }
                func_80066F9C(var_s0_2, M2C_FIELD(temp_s1, s32 *, 0x10), M2C_FIELD(arg2, s32 *, 0x24), M2C_FIELD(arg2, s32 *, 0x20), 0);
                if ((arg3 != 0) && ((temp_v1_2 = M2C_FIELD(var_s0_2, s16 *, 0x24), ((temp_v1_2 < 0xFD) == 0)) || (temp_v1_2 < 4)))
                {
                    M2C_FIELD(temp_s1, s16 *, 0) = 0;
                }
            }
            var_s4_2 += 0x14;
            var_s2_2 += 1;
            var_s0_2 += 0x2C;
        } while (var_s2_2 < (arg0 + arg1));
    }
}
