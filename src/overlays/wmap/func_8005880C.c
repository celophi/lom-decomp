#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 85.668540% (gcc280_g0). */
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
s32 func_800582A0(s32, s32, s32);                   /* extern */
M2C_UNK func_8005EB68(s32, s32, s32, s32, void *, void *); /* extern */
M2C_UNK func_800584B4();                            /* static */
extern u8 D_8004FD04;
extern u8 D_800D9268;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 D_80129550;
extern s32 D_8013922C;
extern s32 D_80139230;
extern u8 D_80139290;
extern s32 D_8013986C;
extern u8 D_80139950;
extern u8 D_80139988;
extern s32 D_8013B27C;
extern s32 D_8013B294;
extern u8 D_8019D248;
extern u8 func_8009A420;

void func_8005880C(void)
{
    SVECTOR position;
    s32 sp24;
    s32 sp20;
    void *var_s0;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_s1;
    s32 var_s3;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;
    s32 var_v0;
    s32 var_v1;
    void *temp_s2;

    if (D_8013B27C != 0)
    {
        var_s3 = 0;
        var_s4 = 0x100;
        var_s0 = &D_8019D248;
        var_s6 = 0;
        var_s5 = 0;
        do
        {
            temp_s2 = var_s5 + (u8 *)&D_800D9268;
            if (M2C_FIELD(temp_s2, s16 *, 2) != -1)
            {
                func_8006CC4C(temp_s2, var_s6 + (u8 *)&D_80139988);
                if (D_8013986C == 0)
                {
                    temp_v0 = (sp20 & 0xFFFF0000) | M2C_FIELD(var_s0, u16 *, 0xC);
                    sp20 = temp_v0;
                    sp20 = (temp_v0 & 0xFFFF) | (M2C_FIELD(var_s0, u16 *, 0xE) << 0x10);
                    position.vx = (s16) ((s32) (((s16) M2C_FIELD(var_s0, u16 *, 0xC) - (((s32) (M2C_FIELD(&D_80139950, s32 *, 0) * 0x14000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8)) - 0x14)) * 0x6000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8));
                    position.vz = 0;
                    position.vy = (s16) ((s32) (((s16) M2C_FIELD(var_s0, u16 *, 0xE) - (((s32) (M2C_FIELD(&D_80139950, s32 *, 4) * 0x14000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8)) - 0x14)) * 0x6000) / (s32) M2C_FIELD(&D_80139950, s32 *, 8));
                    gte_ldv0(&position);
    gte_rtps();
                    gte_stsxy(&sp20);
                    gte_stsz(&sp24);
                    sp24 >>= 2;
                    var_v0 = 0x1B91 - sp24;
                    if (var_v0 < 0)
                    {
                        var_v0 += 3;
                    }
                    temp_v0_2 = var_v0 >> 2;
                    var_s1 = temp_v0_2 + 0x2A;
                    if ((u32) (temp_v0_2 + 0xB) >= 0x90U)
                    {
                        var_s1 = 0x1F;
                    }
                    if (func_800582A0((((s16) M2C_FIELD(var_s0, u16 *, 0xC) * 0x30) / 160) + 0x30, (((s16) M2C_FIELD(var_s0, u16 *, 0xE) * 0x30) / 160) + 0x30, M2C_FIELD(&D_80139950, s32 *, 8)) != 0)
                    {
                        func_80066F9C(temp_s2, sp20, var_s3, var_s1, var_s4);
                    }
                }
                if (D_8013986C == 1)
                {
                    temp_v0_3 = *(s32 *)(((M2C_FIELD(var_s0, s32 *, 0) + (M2C_FIELD(var_s0, s32 *, 4) * 6)) * 4) + (u8 *)&D_8004FD04);
                    temp_v0_4 = (temp_v0_3 & 0xFFFF0000) | ((temp_v0_3 + 0xE) & 0xFFFF);
                    func_80066F9C(temp_s2, (temp_v0_4 & 0xFFFF) | (((temp_v0_4 >> 0x10) + 0x1C) << 0x10), var_s3, 0x1F, var_s4);
                }
            }
            var_s4 += 0x100;
            var_s0 += 0x124;
            var_s6 += 8;
            var_s3 += 1;
            var_s5 += 0x2C;
        } while (var_s3 < 4);
    }
    if ((D_8013922C & 0x40) && (D_80139230 == 0) && (D_80129550 == 0))
    {
        temp_a2 = (M2C_FIELD(&D_80139950, s32 *, 0) / 48) + D_800DCEEC;
        temp_a3 = (M2C_FIELD(&D_80139950, s32 *, 4) / 48) + D_800DCEF0;
        if ((M2C_FIELD(((temp_a3 * 0x28) + (temp_a2 * 0xF0) + (u8 *)&D_80139290), s16 *, 6) != 0) && (D_8013986C == 0) && (D_8011CF18 == 0))
        {
            if ((temp_a2 != M2C_FIELD(&D_8019D248, s32 *, 0)) || (var_v1 = 1, (temp_a3 != M2C_FIELD(&D_8019D248, s32 *, 4))))
            {
                var_v1 = 0;
            }
            D_8013B294 = var_v1;
            if (var_v1 == 0)
            {
                if (*((temp_a3 * 0x28) + (temp_a2 * 0xF0) + (u8 *)&D_80139290) == 0x18)
                {
                    func_8006CAC0(&func_8009A420);
                }
                else
                {
                    M2C_FIELD(&D_8019D248, s32 *, 0x18) = temp_a2;
                    M2C_FIELD(&D_8019D248, s32 *, 0x1C) = temp_a3;
                    if ((temp_a2 != M2C_FIELD(&D_8019D248, s32 *, 0)) || (temp_a3 != M2C_FIELD(&D_8019D248, s32 *, 4)))
                    {
                        func_8005EB68(M2C_FIELD(&D_8019D248, s32 *, 0), M2C_FIELD(&D_8019D248, s32 *, 4), temp_a2, temp_a3, (u8 *)&D_8019D248 + 0x24, (u8 *)&D_8019D248 + 0xA4);
                        M2C_FIELD(&D_8019D248, s32 *, 0x14) = 1;
                        M2C_FIELD(&D_8019D248, s32 *, 0x20) = 1;
                        M2C_FIELD(&D_8019D248, u16 *, 8) = (u16) M2C_FIELD(&D_8019D248, u16 *, 0x28);
                        M2C_FIELD(&D_8019D248, u16 *, 0xA) = (u16) M2C_FIELD(&D_8019D248, u16 *, 0xA8);
                        M2C_FIELD(&D_8019D248, s16 *, 0x10) = (s16) (((s16) M2C_FIELD(&D_8019D248, u16 *, 0x28) - 1) * 0xA0);
                        M2C_FIELD(&D_8019D248, s16 *, 0x12) = (s16) (((s16) M2C_FIELD(&D_8019D248, u16 *, 0xA8) - 1) * 0xA0);
                    }
                    else
                    {
                        M2C_FIELD(&D_8019D248, s32 *, 0x14) = 0;
                    }
                }
            }
        }
    }
    func_800584B4();
}
