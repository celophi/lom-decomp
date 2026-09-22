/* Partial WMAP decompilation: 75.209880% (gcc280_g0). */
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
M2C_UNK func_80064F14(s16);                         /* extern */
void *memset(void *, int, unsigned int);
extern u8 D_800CBDC4;
extern u8 D_800D0A6C;
extern s32 D_800D921C;
extern void *D_801398EC;

void func_80066F9C(void *arg0, s32 arg1, s32 arg2, void *arg3, s32 arg4)
{
    SVECTOR position;
    s16 *sp7C;
    u16 *sp78;
    s16 sp6C;
    void *sp68;
    void *sp64;
    s32 sp60;
    MATRIX sp40;
    SVECTOR sp38;
    VECTOR sp28;
    VECTOR transformed;
    s16 temp_a0;
    s16 temp_a0_2;
    s16 temp_a1;
    s16 temp_a2;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v1_2;
    s16 var_s5;
    s32 var_v0;
    s16 var_v0_3;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a2_2;
    s32 temp_s4;
    s32 temp_v1_3;
    s8 *temp_s3;
    s8 temp_v0_6;
    s8 temp_v0_7;
    s8 var_s6;
    u16 temp_v1;
    u32 var_v0_2;
    u8 *var_s3;
    u8 temp_v0_4;
    u8 temp_v0_5;
    u8 temp_v1_4;
    u8 temp_v1_5;
    u8 var_v0_4;
    void *temp_a0_5;
    void *temp_a1_2;
    void *temp_s1;
    void *var_a3;
    void *var_s2;

    var_a3 = arg3;
    if (arg4 >= 0x100)
    {
        sp60 = ((arg4 >> 8) - 1) << 6;
        arg4 &= 0xFF;
    }
    else
    {
        sp60 = 0;
    }
    temp_s3 = M2C_FIELD(arg0, s8 **, 0x1C);
    temp_a0 = M2C_FIELD(arg0, s16 *, 0x24);
    temp_v0 = M2C_FIELD(arg0, s16 *, 0x22);
    temp_v1 = (u16) M2C_FIELD(arg0, s16 *, 0x24);
    var_s6 = *temp_s3;
    var_s3 = temp_s3 + 1;
    if (temp_a0 != temp_v0)
    {
        if (temp_v0 < temp_a0)
        {
            temp_v0_2 = temp_v1 - M2C_FIELD(arg0, u16 *, 0x26);
            M2C_FIELD(arg0, s16 *, 0x24) = temp_v0_2;
            var_v0 = temp_v0_2;
        }
        else
        {
            temp_v0_3 = temp_v1 + M2C_FIELD(arg0, u16 *, 0x26);
            M2C_FIELD(arg0, s16 *, 0x24) = temp_v0_3;
            if (temp_v0_3 >= 0x100)
            {
                M2C_FIELD(arg0, s16 *, 0x24) = 0xFF;
            }
            var_v0 = M2C_FIELD(arg0, s16 *, 0x24);
        }
        var_v0_2 = var_s6 - 1;
        if (var_v0 < 0)
        {
            M2C_FIELD(arg0, s16 *, 0x24) = 0;
            goto block_11;
        }
    }
    else
    {
block_11:
        var_v0_2 = var_s6 - 1;
    }
    if (var_v0_2 >= 0x20U)
    {
        func_80064F14(temp_a0);
        return;
    }
    temp_v1_2 = M2C_FIELD(arg0, s16 *, 0x24);
    var_s5 = temp_v1_2;
    if (temp_v1_2 >= 0x80)
    {
        var_s5 = 0x100 - temp_v1_2;
    }
    sp64 = &sp28;
    temp_s4 = arg1 >> 0x10;
    sp68 = &sp38;
    sp7C = &position.vx;
    var_s2 = var_s3 + 9;
    sp78 = (u16 *)&transformed;
    do
    {
        temp_s1 = M2C_FIELD(D_801398EC, void **, 0x33C);
        if (M2C_FIELD(var_s2, s8 *, -1) != 0)
        {
            memset(sp64, 0, 0x10);
            memset(sp68, 0, 8);
            sp38.vz = (s16) ((s32) ((u8) M2C_FIELD(var_s2, s8 *, -1) << 0x18) >> 0x14);
            TransMatrix(&sp40, sp64);
            RotMatrix(sp68, &sp40);
            SetRotMatrix(&sp40);
            SetTransMatrix(&sp40);
            temp_a0_2 = *var_s3 | (M2C_FIELD(var_s2, u8 *, 1) << 8);
            temp_a2 = temp_a0_2 + arg1;
            M2C_FIELD(temp_s1, s16 *, 8) = temp_a2;
            temp_a1 = (M2C_FIELD(var_s2, u8 *, -8) | (M2C_FIELD(var_s2, u8 *, 2) << 8)) + temp_s4;
            M2C_FIELD(temp_s1, s16 *, 0xA) = temp_a1;
            position.vy = 0;
            position.vz = 0;
            sp6C = temp_a0_2;
            position.vx = (s16) M2C_FIELD(var_s2, u8 *, -5);
            gte_ldv0(&position);
    gte_rtv0tr();
            gte_stlvnl(&transformed);
            M2C_FIELD(temp_s1, s16 *, 0x10) = (s16) (transformed.vx + temp_a2);
            M2C_FIELD(temp_s1, s16 *, 0x12) = (s16) (transformed.vy + temp_a1);
            position.vx = 0;
            position.vz = 0;
            position.vy = (s16) M2C_FIELD(var_s2, u8 *, -4);
            gte_ldv0(&position);
    gte_rtv0tr();
            gte_stlvnl(&transformed);
            M2C_FIELD(temp_s1, s16 *, 0x18) = (s16) (transformed.vx + temp_a2);
            M2C_FIELD(temp_s1, s16 *, 0x1A) = (s16) (transformed.vy + temp_a1);
            position.vx = (s16) M2C_FIELD(var_s2, u8 *, -5);
            position.vz = 0;
            position.vy = (s16) M2C_FIELD(var_s2, u8 *, -4);
            gte_ldv0(&position);
    gte_rtv0tr();
            gte_stlvnl(&transformed);
            M2C_FIELD(temp_s1, s16 *, 0x20) = (s16) (transformed.vx + temp_a2);
            var_v0_3 = transformed.vy + temp_a1;
        }
        else
        {
            temp_a1_2 = (arg4 << 8) + ((M2C_FIELD(arg0, s8 *, 6) * 0x10) + (u8 *)&D_800CBDC4);
            temp_v1_3 = (s8) *var_s3 | ((s8) M2C_FIELD(var_s2, u8 *, 1) << 8);
            M2C_FIELD(temp_s1, s16 *, 8) = (s16) (arg1 + ((s32) (temp_v1_3 * M2C_FIELD(temp_a1_2, s16 *, 0)) >> 8));
            temp_a2_2 = temp_v1_3 + M2C_FIELD(var_s2, u8 *, -5);
            M2C_FIELD(temp_s1, s16 *, 0x10) = (s16) (arg1 + ((s32) (temp_a2_2 * M2C_FIELD(temp_a1_2, s16 *, 4)) >> 8));
            M2C_FIELD(temp_s1, s16 *, 0x18) = (s16) (arg1 + ((s32) (temp_v1_3 * M2C_FIELD(temp_a1_2, s16 *, 8)) >> 8));
            M2C_FIELD(temp_s1, s16 *, 0x20) = (s16) (arg1 + ((s32) (temp_a2_2 * M2C_FIELD(temp_a1_2, s16 *, 0xC)) >> 8));
            temp_a0_3 = (s8) M2C_FIELD(var_s2, u8 *, -8) | ((s8) M2C_FIELD(var_s2, u8 *, 2) << 8);
            M2C_FIELD(temp_s1, s16 *, 0xA) = (s16) (temp_s4 + ((s32) (temp_a0_3 * M2C_FIELD(temp_a1_2, s16 *, 2)) >> 8));
            M2C_FIELD(temp_s1, s16 *, 0x12) = (s16) (temp_s4 + ((s32) (temp_a0_3 * M2C_FIELD(temp_a1_2, s16 *, 6)) >> 8));
            temp_a0_4 = temp_a0_3 + M2C_FIELD(var_s2, u8 *, -4);
            M2C_FIELD(temp_s1, s16 *, 0x1A) = (s16) (temp_s4 + ((s32) (temp_a0_4 * M2C_FIELD(temp_a1_2, s16 *, 0xA)) >> 8));
            var_v0_3 = temp_s4 + ((s32) (temp_a0_4 * M2C_FIELD(temp_a1_2, s16 *, 0xE)) >> 8);
        }
        M2C_FIELD(temp_s1, s16 *, 0x22) = var_v0_3;
        if (M2C_FIELD(var_s2, s8 *, -3) & 0x80)
        {
            temp_v0_4 = M2C_FIELD(var_s2, u8 *, -7);
            M2C_FIELD(temp_s1, u8 *, 0x24) = temp_v0_4;
            M2C_FIELD(temp_s1, u8 *, 0x14) = temp_v0_4;
            temp_v1_4 = M2C_FIELD(var_s2, u8 *, -7) + M2C_FIELD(var_s2, u8 *, -5);
            M2C_FIELD(temp_s1, u8 *, 0x1C) = temp_v1_4;
            M2C_FIELD(temp_s1, u8 *, 0xC) = temp_v1_4;
        }
        else
        {
            temp_v0_5 = M2C_FIELD(var_s2, u8 *, -7);
            M2C_FIELD(temp_s1, u8 *, 0x1C) = temp_v0_5;
            M2C_FIELD(temp_s1, u8 *, 0xC) = temp_v0_5;
            temp_v1_5 = M2C_FIELD(var_s2, u8 *, -7) + M2C_FIELD(var_s2, u8 *, -5);
            M2C_FIELD(temp_s1, u8 *, 0x24) = temp_v1_5;
            M2C_FIELD(temp_s1, u8 *, 0x14) = temp_v1_5;
        }
        if ((u8) M2C_FIELD(var_s2, s8 *, -3) & 0x40)
        {
            temp_v0_6 = M2C_FIELD(var_s2, u8 *, -6) + M2C_FIELD(var_s2, u8 *, -4);
            M2C_FIELD(temp_s1, s8 *, 0x15) = temp_v0_6;
            M2C_FIELD(temp_s1, s8 *, 0xD) = temp_v0_6;
            var_v0_4 = M2C_FIELD(var_s2, u8 *, -6);
            M2C_FIELD(temp_s1, u8 *, 0x25) = var_v0_4;
        }
        else
        {
            temp_v0_7 = M2C_FIELD(var_s2, u8 *, -6) + sp60;
            M2C_FIELD(temp_s1, s8 *, 0x15) = temp_v0_7;
            M2C_FIELD(temp_s1, s8 *, 0xD) = temp_v0_7;
            var_v0_4 = M2C_FIELD(var_s2, u8 *, -6) + M2C_FIELD(var_s2, u8 *, -4) + sp60;
            M2C_FIELD(temp_s1, u8 *, 0x25) = var_v0_4;
        }
        M2C_FIELD(temp_s1, u8 *, 0x1D) = var_v0_4;
        M2C_FIELD(temp_s1, s32 *, 4) = (s32) (var_s5 | (var_s5 << 8) | (var_s5 << 0x10));
        M2C_FIELD(temp_s1, u16 *, 0x16) = (u16) M2C_FIELD((((arg2 + M2C_FIELD(var_s2, s8 *, -2)) * 0x1C) + (u8 *)&D_800D0A6C), u16 *, 0x18);
        M2C_FIELD(temp_s1, s8 *, 3) = 9;
        M2C_FIELD(temp_s1, s8 *, 7) = 0x2C;
        M2C_FIELD(temp_s1, u16 *, 0xE) = (u16) M2C_FIELD(((((u8) M2C_FIELD(var_s2, s8 *, -3) & 0x3F) * 2) + (arg2 * 0x1C) + (u8 *)&D_800D0A6C), u16 *, 8);
        if ((var_s5 != 0x80) || (M2C_FIELD(var_s2, s8 *, 0) != 0))
        {
            M2C_FIELD(temp_s1, s8 *, 7) = 0x2E;
        }
        var_a3 = D_801398EC;
        temp_a0_5 = ((s32) arg3 * 4) + var_a3;
        M2C_FIELD(temp_s1, s32 *, 0) = (s32) ((M2C_FIELD(temp_s1, s32 *, 0) & 0xFF000000) | (M2C_FIELD(temp_a0_5, s32 *, 0x70) & 0xFFFFFF));
        M2C_FIELD(temp_a0_5, s32 *, 0x70) = (s32) ((M2C_FIELD(temp_a0_5, s32 *, 0x70) & 0xFF000000) | ((s32) temp_s1 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x28;
            M2C_FIELD(var_a3, void **, 0x33C) = (void *) (M2C_FIELD(var_a3, void **, 0x33C) + 0x28);
        }
        var_s2 += 0xC;
        var_s6 -= 1;
        var_s3 += 0xC;
    } while (var_s6 != 0);
}
