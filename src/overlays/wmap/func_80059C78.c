/* Partial WMAP decompilation: 71.188680% (gcc280_g0). */
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
void func_8006534C(s32, s32);
extern u16 D_80050170;
extern u8 D_80050AA0;
extern u8 D_80050DA8;
extern s32 D_800CC130;
extern u8 D_800CC134;
extern u8 D_800CC164;
extern u8 D_800CC764;
extern u8 D_800CC774;
extern s32 D_800D921C;
extern s32 D_800DBE78;
extern u8 D_80139838;
extern void *D_801398EC;
extern s32 D_801ADAFC;

void func_80059C78(void)
{
    SVECTOR position;
    u32 sp3C;
    s32 sp38;
    MATRIX sp10;
    s32 *temp_a1;
    s32 *temp_a1_2;
    s32 *var_a2;
    s32 temp_a0;
    s32 var_a3;
    s32 var_s0;
    s32 var_t2;
    s32 var_t5;
    s32 var_t6;
    s32 var_t7;
    s32 var_t8;
    s32 var_t9;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    u16 *var_t4;
    void *temp_v0;

    if (D_800DBE78 != 3)
    {
        if (D_800DBE78 == 2)
        {
            if (M2C_FIELD(&D_800CC764, s32 *, 0) >= 0x65)
            {
                M2C_FIELD(&D_800CC764, s32 *, 0) = (s32) (M2C_FIELD(&D_800CC764, s32 *, 0) - 6);
                var_v0 = M2C_FIELD(&D_800CC764, s32 *, 4) - 6;
                goto block_8;
            }
            D_800DBE78 = 0;
            goto block_9;
        }
        if (D_800DBE78 == 1)
        {
            if (M2C_FIELD(&D_800CC764, s32 *, 0) < 0x94)
            {
                M2C_FIELD(&D_800CC764, s32 *, 0) = (s32) (M2C_FIELD(&D_800CC764, s32 *, 0) + 6);
                var_v0 = M2C_FIELD(&D_800CC764, s32 *, 4) + 6;
block_8:
                M2C_FIELD(&D_800CC764, s32 *, 4) = var_v0;
block_9:
                goto block_10;
            }
        }
        else
        {
block_10:
            TransMatrix(&sp10, &D_800CC764);
            RotMatrix(&D_800CC774, &sp10);
            SetRotMatrix(&sp10);
            SetTransMatrix(&sp10);
            var_a3 = 0;
            var_s0 = 0xC;
            var_t9 = 0xA;
            var_t8 = 8;
            var_t7 = 6;
            var_t6 = 4;
            var_t5 = 2;
            var_t4 = &D_80050170;
            do
            {
                var_a2 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                gte_ldv3((u8 *)&D_80050AA0 + M2C_FIELD(&D_80050DA8, s16 *, var_a3 * 6) * 8,
                    (u8 *)&D_80050AA0 + M2C_FIELD(&D_80050DA8, s16 *, var_a3 * 6 + 2) * 8,
                    (u8 *)&D_80050AA0 + M2C_FIELD(&D_80050DA8, s16 *, var_a3 * 6 + 4) * 8);
                gte_rtpt();
                M2C_FIELD(var_a2, u16 *, 0xE) = (u16) *var_t4;
                M2C_FIELD(var_a2, u8 *, 0xD) = M2C_FIELD(&D_80050170, u8 *, var_t6);
                M2C_FIELD(var_a2, u8 *, 0xC) = M2C_FIELD(&D_80050170, u8 *, var_t5);
                M2C_FIELD(var_a2, s32 *, 4) = 0x24808080;
                gte_stsxy3((u8 *)var_a2 + 8, (u8 *)var_a2 + 16, (u8 *)var_a2 + 24);
                gte_nclip();
                M2C_FIELD(var_a2, u8 *, 0x14) = M2C_FIELD(&D_80050170, u8 *, var_t7);
                M2C_FIELD(var_a2, u8 *, 0x15) = M2C_FIELD(&D_80050170, u8 *, var_t8);
                gte_stopz(&sp38);
                if (sp38 > 0)
                {
                    M2C_FIELD(var_a2, s32 *, 0) = 0x07000000;
                    M2C_FIELD(var_a2, s16 *, 0x16) = 0x1B;
                    M2C_FIELD(var_a2, u8 *, 0x1C) = M2C_FIELD(&D_80050170, u8 *, var_t9);
                    M2C_FIELD(var_a2, u8 *, 0x1D) = M2C_FIELD(&D_80050170, u8 *, var_s0);
                    M2C_FIELD(var_a2, s32 *, 0) = (M2C_FIELD(var_a2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0xE8) & 0xFFFFFF);
                    M2C_FIELD(D_801398EC, s32 *, 0xE8) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0xE8) & 0xFF000000) | ((s32) var_a2 & 0xFFFFFF));
                    if (D_800D921C < 0x7D00)
                    {
                        D_800D921C += 0x20;
                        M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) ((u8 *)M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x20);
                    }
                }
                var_s0 += 0xE;
                var_t9 += 0xE;
                var_t8 += 0xE;
                var_t7 += 0xE;
                var_t6 += 0xE;
                var_t5 += 0xE;
                var_t4 = (u16 *)((u8 *)var_t4 + 0xE);
                var_a3 += 1;
            } while (var_a3 < 0xA8);
            var_v0_2 = D_800CC130;
            if (var_v0_2 < 0)
            {
                var_v0_2 += 3;
            }
            var_t2 = (var_v0_2 >> 2) + 0xB;
loop_18:
            var_v0_3 = D_800CC130;
            if (var_v0_3 < 0)
            {
                var_v0_3 += 3;
            }
            if (((var_v0_3 >> 2) - 1) < var_t2)
            {
                var_a3 = (var_t2 + 0xC) % 12;
                temp_a0 = var_a3 * 4;
                var_a2 = temp_a0 + (u8 *)&D_80139838;
                if (*var_a2 != -1)
                {
                    temp_v0 = temp_a0 + (u8 *)&D_800CC134;
                    temp_a1 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                    position.vx = M2C_FIELD(temp_v0, u16 *, 0);
                    position.vy = 0;
                    position.vz = M2C_FIELD(temp_v0, u16 *, 2);
                    gte_ldv0(&position);
    gte_rtps();
                    M2C_FIELD(temp_a1, s32 *, 4) = 0x80808080;
                    var_a2 = (*var_a2 * 0x18) + (u8 *)&D_800CC164;
                    gte_stsxy(&sp3C);
                    M2C_FIELD(temp_a1, s16 *, 8) = (s16) (sp3C + M2C_FIELD(var_a2, u16 *, 8));
                    M2C_FIELD(temp_a1, s16 *, 0xA) = (s16) (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xA));
                    M2C_FIELD(temp_a1, u16 *, 0x10) = (u16) M2C_FIELD(var_a2, u16 *, 4);
                    M2C_FIELD(temp_a1, u16 *, 0x12) = (u16) M2C_FIELD(var_a2, u16 *, 6);
                    M2C_FIELD(temp_a1, u8 *, 0xC) = (u8) M2C_FIELD(var_a2, u8 *, 0);
                    M2C_FIELD(temp_a1, s16 *, 0xE) = 0x7FEC;
                    M2C_FIELD(temp_a1, s8 *, 3) = 4;
                    M2C_FIELD(temp_a1, s8 *, 7) = 0x66;
                    M2C_FIELD(temp_a1, u8 *, 0xD) = (u8) M2C_FIELD(var_a2, u8 *, 2);
                    var_a3 = D_800D921C;
                    M2C_FIELD(temp_a1, s32 *, 0) = (M2C_FIELD(temp_a1, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x9C) & 0xFFFFFF);
                    M2C_FIELD(D_801398EC, s32 *, 0x9C) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x9C) & 0xFF000000) | ((s32) temp_a1 & 0xFFFFFF));
                    if (var_a3 < 0x7D00)
                    {
                        D_800D921C = var_a3 + 0x14;
                        M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) ((u8 *)M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x14);
                    }
                    temp_a1_2 = M2C_FIELD(D_801398EC, s32 **, 0x33C);
                    M2C_FIELD(temp_a1_2, s32 *, 4) = 0x404040;
                    M2C_FIELD(temp_a1_2, s16 *, 8) = (s16) (sp3C + M2C_FIELD(var_a2, u16 *, 0xC) + 0xA);
                    M2C_FIELD(temp_a1_2, s16 *, 0xA) = (s16) (M2C_FIELD(var_a2, u16 *, 6) + (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE)));
                    M2C_FIELD(temp_a1_2, s16 *, 0x10) = (s16) (M2C_FIELD(var_a2, u16 *, 4) + (sp3C + M2C_FIELD(var_a2, u16 *, 0xC)) + 0xA);
                    M2C_FIELD(temp_a1_2, s16 *, 0x12) = (s16) (M2C_FIELD(var_a2, u16 *, 6) + (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE)));
                    M2C_FIELD(temp_a1_2, s16 *, 0x18) = (s16) (sp3C + M2C_FIELD(var_a2, u16 *, 0xC));
                    M2C_FIELD(temp_a1_2, s16 *, 0x1A) = (s16) (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE));
                    M2C_FIELD(temp_a1_2, s16 *, 0x20) = (s16) (M2C_FIELD(var_a2, u16 *, 4) + (sp3C + M2C_FIELD(var_a2, u16 *, 0xC)));
                    M2C_FIELD(temp_a1_2, s16 *, 0x22) = (s16) (M2C_FIELD(&sp3C, u16 *, 2) + M2C_FIELD(var_a2, u16 *, 0xE));
                    M2C_FIELD(temp_a1_2, u8 *, 0xC) = (u8) M2C_FIELD(var_a2, u8 *, 0);
                    M2C_FIELD(temp_a1_2, u8 *, 0xD) = (u8) M2C_FIELD(var_a2, u8 *, 2);
                    M2C_FIELD(temp_a1_2, s8 *, 0x14) = (s8) (M2C_FIELD(var_a2, u8 *, 0) + (u8) M2C_FIELD(var_a2, u16 *, 4));
                    M2C_FIELD(temp_a1_2, u8 *, 0x15) = (u8) M2C_FIELD(var_a2, u8 *, 2);
                    M2C_FIELD(temp_a1_2, u8 *, 0x1C) = (u8) M2C_FIELD(var_a2, u8 *, 0);
                    M2C_FIELD(temp_a1_2, s8 *, 0x1D) = (s8) (M2C_FIELD(var_a2, u8 *, 2) + (u8) M2C_FIELD(var_a2, u16 *, 6));
                    M2C_FIELD(temp_a1_2, s8 *, 0x24) = (s8) (M2C_FIELD(var_a2, u8 *, 0) + (u8) M2C_FIELD(var_a2, u16 *, 4));
                    M2C_FIELD(temp_a1_2, s16 *, 0xE) = 0x7FC0;
                    M2C_FIELD(temp_a1_2, s8 *, 3) = 9;
                    M2C_FIELD(temp_a1_2, s8 *, 7) = 0x2E;
                    M2C_FIELD(temp_a1_2, s8 *, 0x25) = (s8) (M2C_FIELD(var_a2, u8 *, 2) + (u8) M2C_FIELD(var_a2, u16 *, 6));
                    M2C_FIELD(temp_a1_2, s16 *, 0x16) = 0xAE;
                    if (D_801ADAFC != 0)
                    {
                        M2C_FIELD(temp_a1_2, s32 *, 0) = (M2C_FIELD(temp_a1_2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0xE4) & 0xFFFFFF);
                        M2C_FIELD(D_801398EC, s32 *, 0xE4) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0xE4) & 0xFF000000) | ((s32) temp_a1_2 & 0xFFFFFF));
                        if (D_800D921C < 0x7D00)
                        {
                            D_800D921C += 0x28;
                            M2C_FIELD(D_801398EC, s32 **, 0x33C) = (s32 *) ((u8 *)M2C_FIELD(D_801398EC, s32 **, 0x33C) + 0x28);
                        }
                    }
                }
                var_t2 -= 1;
                goto loop_18;
            }
            func_8006534C(0xAE, 0x1D);
            func_8006534C(0xAE, 0xB);
        }
    }
}
