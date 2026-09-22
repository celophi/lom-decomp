/* Partial WMAP decompilation: 87.594200% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern u16 D_800432BE;
extern u8 g_saved_game;

void func_8005B58C(s32 unused0, s32 unused1, s32 unused2, s32 arg3)
{
    u8 sp[0x80];
    void *var_v1;
    s32 temp_v0;
    s32 var_a0;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_t0;
    s32 var_t0_2;
    s32 var_t1;
    s32 var_v0;
    s32 temp_a0_2;
    s32 var_v1_2;
    u16 temp_a0;
    u16 temp_v0_2;
    u32 temp_v1;
    u32 temp_v1_3;
    void *temp_v1_2;
    void *temp_v1_4;
    void *var_a0_2;

    var_a3 = arg3;
    var_a1 = 0;
    var_a0 = 0;
    do
    {
        var_v0 = var_a0;
        if (var_a0 < 0)
        {
            var_v0 = var_a0 + 0x1F;
        }
        temp_v0 = var_v0 >> 5;
        if ((M2C_FIELD(((temp_v0 * 4) + (u8 *)&g_saved_game), s32 *, 0x2E8) & (1 << (var_a0 - (temp_v0 << 5)))) && (((u32) (var_a0 - 9) < 6U) || (var_a0 == 0x18) || (var_a0 == 0) || (var_a0 == 0x1B) || (var_a0 == 0x1C)))
        {
            var_a1 = 1;
        }
        var_a0 += 1;
    } while (var_a0 < 0x40);
    if (var_a1 == 0)
    {
        temp_v1 = (M2C_FIELD(&g_saved_game, u32 *, 0x2E4) & 0xFF80FFFF) | ((((((u32) M2C_FIELD(&g_saved_game, u32 *, 0x2E4) >> 0x10) & 0x7F) + 1) & 0x7F) << 0x10);
        M2C_FIELD(&g_saved_game, u32 *, 0x2E4) = temp_v1;
        if ((u32) ((temp_v1 >> 0x10) & 0x7F) >= 6U)
        {
            M2C_FIELD(&g_saved_game, u32 *, 0x2E4) = (u32) (temp_v1 & 0xFF80FFFF);
        }
        var_t0 = 0;
        var_v1 = &g_saved_game;
        do
        {
            if (M2C_FIELD(var_v1, u8 *, 0x2EF4) != 0)
            {
                if (M2C_FIELD(var_v1, s32 *, 0x2F38) < 0)
                {
                    temp_v0_2 = M2C_FIELD(var_v1, u16 *, 0x2F36);
                    if (temp_v0_2 != 0)
                    {
                        M2C_FIELD(var_v1, u16 *, 0x2F36) = (u16) (temp_v0_2 - 1);
                    }
                }
                else
                {
                    temp_a0 = M2C_FIELD(var_v1, u16 *, 0x2F36);
                    if (temp_a0 <= 0xFFFEU)
                    {
                        M2C_FIELD(var_v1, u16 *, 0x2F36) = (u16) (temp_a0 + 1);
                    }
                }
            }
            var_t0 += 1;
            var_v1 += 0x60;
        } while (var_t0 < 5);
        var_a2 = 0;
        var_a0_2 = sp;
        do
        {
            temp_v1_2 = var_a2 + (u8 *)&g_saved_game;
            M2C_FIELD(var_a0_2, s32 *, 0) = (s32) (M2C_FIELD(temp_v1_2, u8 *, 0x2F4) - 3);
            var_a2 += 1;
            M2C_FIELD(var_a0_2, s32 *, 0x40) = 0;
            M2C_FIELD(var_a0_2, s32 *, 0x60) = 0;
            M2C_FIELD(var_a0_2, s32 *, 0x20) = (s32) (M2C_FIELD(temp_v1_2, u8 *, 0x474) - 3);
            var_a0_2 += 4;
        } while (var_a2 < 8);
        temp_v1_3 = D_800432BE & 0x7F;
        switch (temp_v1_3)
        {
        case 0:
            var_a3 = 4;
            break;
        case 1:
            var_a3 = 3;
            break;
        case 2:
            var_a3 = 5;
            break;
        case 3:
            var_a3 = 6;
            break;
        case 4:
            var_a3 = 2;
            break;
        case 5:
            var_a3 = 7;
            break;
        }
        var_t0_2 = 0;
        var_t1 = 0;
        temp_v1_4 = sp + (var_a3 * 4);
        M2C_FIELD(temp_v1_4, s32 *, 0) = (s32) (M2C_FIELD(temp_v1_4, s32 *, 0) + 1);
        M2C_FIELD(temp_v1_4, s32 *, 0x20) = (s32) (M2C_FIELD(temp_v1_4, s32 *, 0x20) + 1);
        M2C_FIELD(temp_v1_4, s32 *, 0x40) = (s32) (M2C_FIELD(temp_v1_4, s32 *, 0x40) + 1);
        M2C_FIELD(temp_v1_4, s32 *, 0x60) = (s32) (M2C_FIELD(temp_v1_4, s32 *, 0x60) + 1);
        M2C_FIELD(sp, s32 *, 0x0) += 1;
        M2C_FIELD(sp, s32 *, 0x4) += 1;
        M2C_FIELD(sp, s32 *, 0x20) += 1;
        M2C_FIELD(sp, s32 *, 0x24) += 1;
        M2C_FIELD(sp, s32 *, 0x40) += 1;
        M2C_FIELD(sp, s32 *, 0x44) += 1;
        M2C_FIELD(sp, s32 *, 0x60) += 1;
        M2C_FIELD(sp, s32 *, 0x64) += 1;
        do
        {
            var_a3_2 = 0;
loop_34:
            var_a2_2 = 0;
            var_a1_2 = var_a3_2 * 0x10;
loop_35:
            temp_a0_2 = M2C_FIELD((var_a1_2 + var_t1 + (u8 *)&g_saved_game), u8 *, 0x26F8) + *(s32 *)(sp + ((var_a2_2 * 4) + (var_t0_2 << 5)));
            if (temp_a0_2 >= 0)
            {
                var_v1_2 = -1;
                if (temp_a0_2 < 0x100)
                {
                    var_v1_2 = temp_a0_2;
                }
            }
            else
            {
                var_v1_2 = 0;
            }
            M2C_FIELD((var_a1_2 + var_t1 + (u8 *)&g_saved_game), s8 *, 0x26F8) = var_v1_2;
            var_a2_2 += 1;
            var_a1_2 += 1;
            if (var_a2_2 < 8)
            {
                goto loop_35;
            }
            var_a3_2 += 1;
            if (var_a3_2 < 8)
            {
                goto loop_34;
            }
            var_t0_2 += 1;
            var_t1 += 0x8C;
        } while (var_t0_2 < 4);
    }
}
