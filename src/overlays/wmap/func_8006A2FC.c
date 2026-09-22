/* Partial WMAP decompilation: 79.917050% (gcc280_g0). */
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
extern s32 D_8013B278;
extern u16 D_8013B280;
extern s32 D_8013B284;
extern u8 D_801AFBD0;

void func_8006A2FC(void *arg0, s32 arg1, s32 arg2, s32 arg3, u16 arg4, u16 arg5, u32 arg6, void *arg7)
{
    SVECTOR position;
    s32 sp20;
    M2C_UNK var_s4;
    s16 *temp_s2_2;
    s16 temp_v0_6;
    s32 temp_a1;
    s32 temp_lo;
    s32 temp_s0;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1_2;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_v0;
    u16 temp_v0;
    u16 temp_v0_2;
    u16 temp_v0_5;
    u8 var_v1;
    void *temp_s2;
    void *temp_v0_7;
    void *temp_v0_8;
    void *temp_v1;
    void *var_a0;
    void *var_s0;

    var_s4 = 4;
    var_s1 = 0;
    if (arg2 > 0)
    {
        var_s0 = arg0;
        do
        {
            temp_s2 = ((var_s1 + M2C_FIELD(arg7, s32 *, 0x1C)) * 0x14) + (u8 *)&D_801AFBD0;
            if (M2C_FIELD(temp_s2, s16 *, 0) != 0)
            {
                position.vx = (s16) ((s32) (((s32) M2C_FIELD(temp_s2, s32 *, 8) >> 3) * (ccos(M2C_FIELD(temp_s2, s16 *, 2)) >> 6)) >> 0xC);
                position.vy = (s16) ((s32) (((s32) M2C_FIELD(temp_s2, s32 *, 8) >> 3) * (csin(M2C_FIELD(temp_s2, s16 *, 2)) >> 6)) >> 0xC);
                position.vz = M2C_FIELD(temp_s2, u16 *, 0xE);
                gte_ldv0(&position);
                gte_rtps();
                switch (arg6)
                {
                case 5:
                    if (M2C_FIELD(var_s0, s16 *, 0x24) >= arg3)
                    {
                        M2C_FIELD(var_s0, s16 *, 0x22) = 0;
                    }
                    if ((M2C_FIELD(var_s0, s16 *, 0x22) == 0) && (M2C_FIELD(var_s0, s16 *, 0x24) < 4))
                    {
                        M2C_FIELD(temp_s2, s16 *, 0) = 0;
                    }
                    /* fallthrough */
                case 0:
                    M2C_FIELD(temp_s2, u16 *, 0xE) = (u16) (M2C_FIELD(temp_s2, u16 *, 0xE) + M2C_FIELD(temp_s2, u16 *, 4));
                    temp_v0 = M2C_FIELD(temp_s2, u16 *, 0xC) - 1;
                    M2C_FIELD(temp_s2, u16 *, 0xC) = temp_v0;
                    M2C_FIELD(temp_s2, s32 *, 8) = (s32) (M2C_FIELD(temp_s2, s32 *, 8) + M2C_FIELD(arg7, s32 *, 0x18));
                    if ((temp_v0 << 0x10) == 0)
                    {
                        M2C_FIELD(temp_s2, s16 *, 0) = 0;
                    }
                default:
block_28:
                    var_v0 = var_s1 * 8;
                    break;
                case 1:
                    if (*(s32 *)0x8013B264 < (s16) M2C_FIELD(temp_s2, u16 *, 0xC))
                    {
                        M2C_FIELD(temp_s2, u16 *, 0xE) = (u16) (M2C_FIELD(temp_s2, u16 *, 0xE) + (*(u16 *)0x8013B270 + ((s32) (rand() * D_8013B278) >> 0xF)));
                        M2C_FIELD(temp_s2, s32 *, 8) = (s32) (M2C_FIELD(temp_s2, s32 *, 8) + M2C_FIELD(arg7, s32 *, 0x18));
                    }
                    else
                    {
                        temp_v0_2 = M2C_FIELD(temp_s2, u16 *, 0xE) - (D_8013B280 + ((s32) (rand() * D_8013B284) >> 0xF));
                        M2C_FIELD(temp_s2, u16 *, 0xE) = temp_v0_2;
                        if (temp_v0_2 & 0x8000)
                        {
                            M2C_FIELD(temp_s2, u16 *, 0xE) = 0U;
                            M2C_FIELD(var_s0, s16 *, 0x22) = 0;
                            M2C_FIELD(var_s0, s16 *, 0x26) = 8;
                            if (M2C_FIELD(var_s0, s16 *, 0x24) < 4)
                            {
                                M2C_FIELD(temp_s2, s16 *, 0) = 0;
                            }
                        }
                    }
                    var_s4 = 0xC;
                    if ((u32) ((u16) M2C_FIELD(temp_s2, s16 *, 2) - 1) < 0x7FFU)
                    {
                        var_s4 = 0xA;
                    }
                    M2C_FIELD(temp_s2, u16 *, 0xC) = (u16) (M2C_FIELD(temp_s2, u16 *, 0xC) - 1);
                    goto block_28;
                case 2:
                    temp_v0_3 = (s16) M2C_FIELD(temp_s2, u16 *, 0xC) - *(s32 *)0x8013B264;
                    M2C_FIELD(temp_s2, u16 *, 0xE) = (u16) (D_8013B280 + (*(u16 *)0x8013B270 - ((s32) (temp_v0_3 * temp_v0_3) / (s32) D_8013B278)));
                    if ((s16) M2C_FIELD(temp_s2, u16 *, 0xC) != 0)
                    {
                        M2C_FIELD(temp_s2, u16 *, 0xC) = (u16) (M2C_FIELD(temp_s2, u16 *, 0xC) - 1);
                        M2C_FIELD(temp_s2, s32 *, 8) = (s32) (M2C_FIELD(temp_s2, s32 *, 8) + M2C_FIELD(arg7, s32 *, 0x18));
                    }
                    goto block_28;
                case 3:
                    temp_v0_4 = *(s32 *)0x8013B264 - (s16) M2C_FIELD(temp_s2, u16 *, 0xC);
                    M2C_FIELD(temp_s2, u16 *, 0xE) = (u16) ((s32) (temp_v0_4 * temp_v0_4) / (s32) *(u16 *)0x8013B270);
                    goto block_28;
                case 4:
                    M2C_FIELD(temp_s2, u16 *, 0xE) = (u16) (M2C_FIELD(temp_s2, u16 *, 0xE) + M2C_FIELD(temp_s2, u16 *, 4));
                    temp_v0_5 = M2C_FIELD(temp_s2, u16 *, 0xC) - 1;
                    M2C_FIELD(temp_s2, u16 *, 0xC) = temp_v0_5;
                    M2C_FIELD(temp_s2, s32 *, 8) = (s32) (M2C_FIELD(temp_s2, s32 *, 8) + M2C_FIELD(arg7, s32 *, 0x18));
                    if ((temp_v0_5 << 0x10) == 0)
                    {
                        M2C_FIELD(temp_s2, s16 *, 0) = 0;
                    }
                    goto block_28;
                case 7:
                    M2C_FIELD(temp_s2, u16 *, 0xE) = (u16) (M2C_FIELD(temp_s2, u16 *, 0xE) + M2C_FIELD(temp_s2, u16 *, 4));
                    M2C_FIELD(temp_s2, s16 *, 2) = (s16) ((u16) M2C_FIELD(temp_s2, s16 *, 2) + M2C_FIELD(arg7, u16 *, 0x10));
                    M2C_FIELD(temp_s2, s32 *, 8) = (s32) (M2C_FIELD(temp_s2, s32 *, 8) + M2C_FIELD(arg7, s32 *, 0x18));
                    /* fallthrough */
                case 6:
                    var_v0 = var_s1 * 8;
                    if ((s16) M2C_FIELD(temp_s2, u16 *, 0xE) < 0)
                    {
                        M2C_FIELD(temp_s2, s16 *, 0) = 0;
                        goto block_28;
                    }
                    break;
                }
                temp_a1 = M2C_FIELD((var_v0 + arg1), s32 *, 4);
                temp_v0_6 = (s16) M2C_FIELD(var_s0, u16 *, 0xE);
                if (M2C_FIELD(var_s0, s16 *, 0x10) != temp_v0_6)
                {
                    M2C_FIELD(var_s0, s16 *, 0x10) = (s16) M2C_FIELD(var_s0, u16 *, 0xE);
                    M2C_FIELD(var_s0, s16 *, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16 *)((temp_v0_6 * 2) + temp_a1);
                    M2C_FIELD(var_s0, void **, 0x18) = temp_v1;
                    M2C_FIELD(var_s0, void **, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0, s16 *, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(var_s0, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0, s16 *, 0x20) == 0)
                {
                    temp_v0_7 = M2C_FIELD(var_s0, void **, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_7, u8 *, 0);
                    M2C_FIELD(var_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_7, u8 *, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_8 = M2C_FIELD(var_s0, void **, 0x18);
                        M2C_FIELD(var_s0, void **, 0x14) = temp_v0_8;
                        var_v1 = M2C_FIELD(temp_v0_8, u8 *, 0);
                        M2C_FIELD(var_s0, s16 *, 0x20) = (s16) M2C_FIELD(temp_v0_8, u8 *, 1);
                    }
                    M2C_FIELD(var_s0, void **, 0x14) = (void *) (M2C_FIELD(var_s0, void **, 0x14) + 4);
                    M2C_FIELD(var_s0, s32 *, 0x1C) = (s32) (temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16 *, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(var_s0, sp20, M2C_FIELD(arg7, s32 *, 0x20), var_s4, 0);
            }
            var_s1 += 1;
            var_s0 += 0x2C;
        } while (var_s1 < arg2);
    }
    temp_v1_2 = M2C_FIELD(arg7, s32 *, 0x14);
    if ((temp_v1_2 != -1) && (((s32) D_8011CF74 % temp_v1_2) == 0) && (var_s1_2 = 0, (arg2 > 0)))
    {
        var_a0 = arg0;
loop_43:
        temp_s2_2 = ((var_s1_2 + M2C_FIELD(arg7, s32 *, 0x1C)) * 0x14) + (u8 *)&D_801AFBD0;
        var_s1_2 += 1;
        if (M2C_FIELD(temp_s2_2, s16 *, 0) == 0)
        {
            M2C_FIELD(var_a0, s16 *, 2) = 0;
            M2C_FIELD(var_a0, s8 *, 6) = 0xF;
            M2C_FIELD(var_a0, s16 *, 0x10) = -1;
            M2C_FIELD(var_a0, u16 *, 0x22) = (u16) arg3;
            M2C_FIELD(var_a0, u16 *, 0x24) = arg4;
            M2C_FIELD(var_a0, u16 *, 0xE) = (u16) M2C_FIELD(arg7, u16 *, 0x24);
            M2C_FIELD(var_a0, u16 *, 0x26) = arg5;
            M2C_FIELD(temp_s2_2, s16 *, 0) = 1;
            M2C_FIELD(temp_s2_2, s16 *, 2) = rand();
            temp_s0 = M2C_FIELD(arg7, s32 *, 0x28);
            if (temp_s0 < 0)
            {
                M2C_FIELD(temp_s2_2, s32 *, 8) = (s32) (((s32) (rand() * ((temp_s0 & 0xFFFF) << 5)) >> 0xF) + ((s32) (temp_s0 & 0x7FFF0000) >> 0xE));
            }
            else
            {
                M2C_FIELD(temp_s2_2, s32 *, 8) = temp_s0;
            }
            M2C_FIELD(temp_s2_2, s32 *, 4) = (s32) (((s32) (rand() * M2C_FIELD(arg7, s32 *, 8)) >> 0xF) + M2C_FIELD(arg7, s32 *, 4));
            temp_lo = rand() * (s32) M2C_FIELD(arg7, u16 *, 0x10);
            M2C_FIELD(temp_s2_2, s16 *, 0xE) = 0;
            M2C_FIELD(temp_s2_2, s16 *, 0xC) = (s16) (M2C_FIELD(arg7, u16 *, 0xC) + (temp_lo >> 0xF));
            return;
        }
        var_a0 += 0x2C;
        if (var_s1_2 >= arg2)
        {

        }
        else
        {
            goto loop_43;
        }
    }
}
