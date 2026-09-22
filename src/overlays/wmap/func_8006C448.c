/* Partial WMAP decompilation: 79.933334% (gcc280_g0). */
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
extern u8 D_800D9268;
extern u8 D_80139988;
extern u8 D_801AFBD0;

void func_8006C448(void *arg0)
{
    SVECTOR position;
    s32 sp20;
    s16 temp_v0_2;
    s32 temp_a0;
    s32 temp_a2;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_s2;
    s32 var_s2_2;
    u8 var_v1;
    void *temp_a0_2;
    void *temp_s0;
    void *temp_s1;
    void *temp_v0_3;
    void *temp_v0_4;
    void *temp_v1_2;
    void *temp_v1_3;

    var_s2 = 0;
    if (M2C_FIELD(arg0, s32 *, 0) > 0)
    {
loop_2:
        temp_v1 = M2C_FIELD(arg0, s32 *, 4) + var_s2;
        temp_a0 = temp_v1 * 0x2C;
        temp_s1 = (temp_v1 * 0x14) + (u8 *)&D_801AFBD0;
        temp_s0 = temp_a0 + (u8 *)&D_800D9268;
        if (M2C_FIELD(temp_s1, s16 *, 0) == 0)
        {
            M2C_FIELD(temp_s1, s16 *, 0) = 1;
            M2C_FIELD(temp_s1, s16 *, 0x10) = (s16) (M2C_FIELD(arg0, u16 *, 8) + ((s32) (rand() * M2C_FIELD(arg0, s32 *, 0xC)) >> 0xF));
            M2C_FIELD(temp_s1, s16 *, 0xE) = (s16) (M2C_FIELD(arg0, u16 *, 0x10) + ((s32) (rand() * M2C_FIELD(arg0, s32 *, 0x14)) >> 0xF));
            M2C_FIELD(temp_s1, s16 *, 0x12) = (s16) (M2C_FIELD(arg0, u16 *, 0x18) + ((s32) (rand() * M2C_FIELD(arg0, s32 *, 0x1C)) >> 0xF));
            M2C_FIELD(temp_s0, s16 *, 2) = 0;
            M2C_FIELD(temp_s0, s8 *, 6) = 0xF;
            M2C_FIELD(temp_s0, s16 *, 0x10) = -1;
            M2C_FIELD(temp_s0, u16 *, 0xE) = (u16) M2C_FIELD(arg0, u16 *, 0x34);
            M2C_FIELD(temp_s0, u16 *, 0x22) = (u16) M2C_FIELD(arg0, u16 *, 0x20);
            M2C_FIELD(temp_s0, u16 *, 0x24) = (u16) M2C_FIELD(arg0, u16 *, 0x24);
            M2C_FIELD(temp_s0, u16 *, 0x26) = (u16) M2C_FIELD(arg0, u16 *, 0x28);
        }
        else
        {
            var_s2 += 1;
            if (var_s2 < M2C_FIELD(arg0, s32 *, 0))
            {
                goto loop_2;
            }
        }
    }
    var_s2_2 = 0;
    if (M2C_FIELD(arg0, s32 *, 0) > 0)
    {
        do
        {
            temp_v0 = M2C_FIELD(arg0, s32 *, 4) + var_s2_2;
            temp_v1_2 = (temp_v0 * 0x14) + (u8 *)&D_801AFBD0;
            temp_a0_2 = (temp_v0 * 0x2C) + (u8 *)&D_800D9268;
            if (M2C_FIELD(temp_v1_2, s16 *, 0) != 0)
            {
                position.vx = M2C_FIELD(temp_v1_2, u16 *, 0x10);
                position.vy = M2C_FIELD(temp_v1_2, u16 *, 0xE);
                position.vz = M2C_FIELD(temp_v1_2, u16 *, 0x12);
                gte_ldv0(&position);
                gte_rtps();
                temp_a2 = M2C_FIELD((((var_s2_2 + M2C_FIELD(arg0, s32 *, 4)) * 8) + (u8 *)&D_80139988), s32 *, 4);
                temp_v0_2 = (s16) M2C_FIELD(temp_a0_2, u16 *, 0xE);
                if (M2C_FIELD(temp_a0_2, s16 *, 0x10) != temp_v0_2)
                {
                    M2C_FIELD(temp_a0_2, s16 *, 0x10) = (s16) M2C_FIELD(temp_a0_2, u16 *, 0xE);
                    M2C_FIELD(temp_a0_2, s16 *, 0x20) = 1;
                    temp_v1_3 = temp_a2 + *(s16 *)((temp_v0_2 * 2) + temp_a2);
                    M2C_FIELD(temp_a0_2, void **, 0x18) = temp_v1_3;
                    M2C_FIELD(temp_a0_2, void **, 0x14) = temp_v1_3;
                }
                if (M2C_FIELD(temp_a0_2, s16 *, 0x20) != 0xFF)
                {
                    M2C_FIELD(temp_a0_2, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(temp_a0_2, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(temp_a0_2, s16 *, 0x20) == 0)
                {
                    temp_v0_3 = M2C_FIELD(temp_a0_2, void **, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_3, u8 *, 0);
                    M2C_FIELD(temp_a0_2, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_3, u8 *, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_4 = M2C_FIELD(temp_a0_2, void **, 0x18);
                        M2C_FIELD(temp_a0_2, void **, 0x14) = temp_v0_4;
                        var_v1 = M2C_FIELD(temp_v0_4, u8 *, 0);
                        M2C_FIELD(temp_a0_2, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_4, u8 *, 1);
                    }
                    M2C_FIELD(temp_a0_2, void **, 0x14) = (void *) (M2C_FIELD(temp_a0_2, void **, 0x14) + 4);
                    M2C_FIELD(temp_a0_2, s32 *, 0x1C) = (s32) (temp_a2 + M2C_FIELD(((var_v1 * 2) + temp_a2), s16 *, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(temp_a0_2, sp20, M2C_FIELD(arg0, s32 *, 0x30), M2C_FIELD(arg0, s32 *, 0x2C), 0);
            }
            var_s2_2 += 1;
        } while (var_s2_2 < M2C_FIELD(arg0, s32 *, 0));
    }
}
