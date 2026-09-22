#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 67.098595% (gcc280_g0). */
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

extern u8 D_800D9268;
extern s32 D_80139234;
extern s32 D_8013923C;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern s32 D_801B2770;
extern s32 D_801B2774;

void func_8007D7FC(void)
{
    SVECTOR position;
    u32 sp20;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_s2;
    s32 var_s2_2;
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v1;
    void *var_a0;
    void *var_s0;
    void *var_s0_2;
    void *var_s1;
    void *var_s1_2;
    void *var_s3;
    void *var_s3_2;
    void *var_v0;

    var_s2 = 0x14;
    var_s3 = (u8 *)&D_80139988 + 0xA0;
    var_s0 = (u8 *)&D_801AFBD0 + 0x190;
    var_s1 = (u8 *)&D_800D9268 + 0x370;
    D_8013923C -= 1;
    do
    {
        position.vx = (s16) ((s32) (((s32) M2C_FIELD(var_s0, s32 *, 8) >> 3) * (ccos(M2C_FIELD(var_s0, s16 *, 2)) >> 6)) >> 0xC);
        position.vy = (s16) ((s32) (((s32) M2C_FIELD(var_s0, s32 *, 8) >> 3) * (csin(M2C_FIELD(var_s0, s16 *, 2)) >> 6)) >> 0xC);
        position.vz = M2C_FIELD(var_s0, u16 *, 0xE);
        gte_ldv0(&position);
                gte_rtps();
        M2C_FIELD(var_s0, s16 *, 2) = (s16) ((u16) M2C_FIELD(var_s0, s16 *, 2) + 0x60);
        M2C_FIELD(var_s0, s32 *, 8) = (s32) (M2C_FIELD(var_s0, s32 *, 8) - 0xBB8);
        gte_stsxy(&sp20);
        M2C_FIELD(var_s0, u16 *, 0x10) = sp20;
        M2C_FIELD(var_s0, u16 *, 0x12) = (u16) M2C_FIELD(&sp20, u16 *, 2);
        if (D_8013923C == 0)
        {
            var_a0 = var_s1;
            var_v0 = ((var_s2 + D_80139234) * 0x2C) + (u8 *)&D_800D9268;
            do
            {
                M2C_FIELD(var_v0, s32 *, 0) = (s32) M2C_FIELD(var_a0, s32 *, 0);
                M2C_FIELD(var_v0, s32 *, 4) = (s32) M2C_FIELD(var_a0, s32 *, 4);
                M2C_FIELD(var_v0, s32 *, 8) = (s32) M2C_FIELD(var_a0, s32 *, 8);
                M2C_FIELD(var_v0, s32 *, 0xC) = (s32) M2C_FIELD(var_a0, s32 *, 0xC);
                var_a0 += 0x10;
                var_v0 += 0x10;
            } while (var_a0 != (var_s1 + 0x20));
            M2C_FIELD(var_v0, s32 *, 0) = (s32) M2C_FIELD(var_a0, s32 *, 0);
            M2C_FIELD(var_v0, s32 *, 4) = (s32) M2C_FIELD(var_a0, s32 *, 4);
            M2C_FIELD(var_v0, s32 *, 8) = (s32) M2C_FIELD(var_a0, s32 *, 8);
            temp_v0 = ((var_s2 + D_80139234) * 0x14) + (u8 *)&D_801AFBD0;
            M2C_FIELD(temp_v0, s32 *, 0) = (s32) M2C_FIELD(var_s0, s32 *, 0);
            M2C_FIELD(temp_v0, s32 *, 4) = (s32) M2C_FIELD(var_s0, s32 *, 4);
            M2C_FIELD(temp_v0, s32 *, 8) = (s32) M2C_FIELD(var_s0, s32 *, 8);
            M2C_FIELD(temp_v0, s32 *, 0xC) = (s32) M2C_FIELD(var_s0, s32 *, 0xC);
            M2C_FIELD(temp_v0, s32 *, 0x10) = (s32) M2C_FIELD(var_s0, s32 *, 0x10);
            temp_v0_2 = ((var_s2 + D_80139234) * 0x2C) + (u8 *)&D_800D9268;
            temp_v1 = ((var_s2 + D_80139234) * 8) + (u8 *)&D_80139988;
            M2C_FIELD(temp_v1, s32 *, 0) = (s32) M2C_FIELD(var_s3, s32 *, 0);
            M2C_FIELD(temp_v1, s32 *, 4) = (s32) M2C_FIELD(var_s3, s32 *, 4);
            M2C_FIELD(temp_v0_2, s16 *, 0x26) = 8;
            M2C_FIELD(temp_v0_2, s16 *, 0x22) = 1;
        }
        var_s3 += 0x78;
        var_s0 += 0x12C;
        var_s2 += 0xF;
        var_s1 += 0x294;
    } while (var_s2 < 0x50);
    var_s2_2 = 0x14;
    if (D_8013923C == 0)
    {
        D_8013923C = 2;
        temp_v0_3 = D_80139234 + 1;
        D_80139234 = temp_v0_3;
        if (temp_v0_3 >= 0xF)
        {
            D_80139234 = 1;
            var_s2_2 = 0x14;
        }
    }
    var_s1_2 = (u8 *)&D_801AFBD0 + 0x190;
    var_s3_2 = (u8 *)&D_80139988 + 0xA0;
    var_s0_2 = (u8 *)&D_800D9268 + 0x370;
    do
    {
        if (M2C_FIELD(var_s1_2, s16 *, 0) != 0)
        {
            func_8006CC4C(var_s0_2, var_s3_2);
            func_80066F9C(var_s0_2, M2C_FIELD(var_s1_2, s32 *, 0x10), 8, 8, 0);
            if (M2C_FIELD(var_s0_2, s16 *, 0x24) < 5)
            {
                M2C_FIELD(var_s1_2, s16 *, 0) = 0;
            }
        }
        var_s1_2 += 0x14;
        var_s3_2 += 8;
        var_s2_2 += 1;
        var_s0_2 += 0x2C;
    } while (var_s2_2 < 0x50);
    temp_v0_4 = D_801B2774 - 1;
    D_801B2774 = temp_v0_4;
    if (temp_v0_4 == 0)
    {
        D_801B2770 += 1;
    }
}
