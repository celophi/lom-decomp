#include "wmap_land_layout.h"
#include "saved_game.h"
#include "cdrom.h"

void func_8005B58C(s32 unused0, s32 unused1, s32 unused2, s32 arg3)
{
/* Partial WMAP decompilation: 87.594200% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

extern u16 D_800432BE;
extern u8 g_saved_game__for_func_8005B58C __asm__("g_saved_game");

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
        if ((M2C_FIELD(((temp_v0 * 4) + (u8 *)&g_saved_game__for_func_8005B58C), s32 *, 0x2E8) & (1 << (var_a0 - (temp_v0 << 5)))) && (((u32) (var_a0 - 9) < 6U) || (var_a0 == 0x18) || (var_a0 == 0) || (var_a0 == 0x1B) || (var_a0 == 0x1C)))
        {
            var_a1 = 1;
        }
        var_a0 += 1;
    } while (var_a0 < 0x40);
    if (var_a1 == 0)
    {
        temp_v1 = (M2C_FIELD(&g_saved_game__for_func_8005B58C, u32 *, 0x2E4) & 0xFF80FFFF) | ((((((u32) M2C_FIELD(&g_saved_game__for_func_8005B58C, u32 *, 0x2E4) >> 0x10) & 0x7F) + 1) & 0x7F) << 0x10);
        M2C_FIELD(&g_saved_game__for_func_8005B58C, u32 *, 0x2E4) = temp_v1;
        if ((u32) ((temp_v1 >> 0x10) & 0x7F) >= 6U)
        {
            M2C_FIELD(&g_saved_game__for_func_8005B58C, u32 *, 0x2E4) = (u32) (temp_v1 & 0xFF80FFFF);
        }
        var_t0 = 0;
        var_v1 = &g_saved_game__for_func_8005B58C;
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
            temp_v1_2 = var_a2 + (u8 *)&g_saved_game__for_func_8005B58C;
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
            temp_a0_2 = M2C_FIELD((var_a1_2 + var_t1 + (u8 *)&g_saved_game__for_func_8005B58C), u8 *, 0x26F8) + *(s32 *)(sp + ((var_a2_2 * 4) + (var_t0_2 << 5)));
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
            M2C_FIELD((var_a1_2 + var_t1 + (u8 *)&g_saved_game__for_func_8005B58C), s8 *, 0x26F8) = var_v1_2;
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
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

s32 func_8005B8C8(u32 arg0, s32 arg1, s32 arg2)
{
/* Partial WMAP decompilation: 79.354164% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

s32 func_8005D670__for_func_8005B8C8(s32, s32) __asm__("func_8005D670");
extern u8 D_800432BD;
extern u8 D_800CFDCC;
extern s32 D_800D8B3C;
extern u8 g_saved_game__for_func_8005B8C8 __asm__("g_saved_game");

    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 var_v0;
    s32 var_v0_2;
    u32 temp_s0_4;
    u32 temp_s0_5;

    temp_s0 = arg0 + (arg1 * 6);
    if ((arg0 < 6U) && (arg1 >= 0) && (arg1 < 6))
    {
        var_v0 = 0;
        if (func_8005D670__for_func_8005B8C8(arg0, arg1) == 0xFF)
        {
            if ((((u32) M2C_FIELD(((arg2 * 0xC) + (u8 *)&D_800CFDCC), u32 *, 8) >> 0xA) & 1) == 1)
            {
                if (!(M2C_FIELD((D_800D8B3C + (temp_s0 * 0xC)), u16 *, 0x10) & 4))
                {
                    return 0;
                }
                goto block_11;
            }
            if ((u32) (arg2 - 0x18) >= 2U)
            {
                if ((arg2 == 0x1F) || (var_v0 = 0, ((M2C_FIELD((D_800D8B3C + (temp_s0 * 0xC)), u16 *, 0x10) & 1) == 0)))
                {
                    goto block_11;
                }
                /* Duplicate return node #29. Try simplifying control flow for better match */
                return var_v0;
            }
block_11:
            if (D_800432BD != 0)
            {
                if (((M2C_FIELD((D_800D8B3C + (temp_s0 * 0xC)), u16 *, 0x10) & 2) || (var_v0 = 0, ((((u32) M2C_FIELD(((arg2 * 0xC) + (u8 *)&D_800CFDCC), u32 *, 8) >> 8) & 1) != 1))) && ((arg1 <= 0) || (temp_s0_2 = arg1 - 1, (func_8005D670__for_func_8005B8C8(arg0, temp_s0_2) == 0xFF)) || (var_v0 = 1, ((((u8) M2C_FIELD(((func_8005D670__for_func_8005B8C8(arg0, temp_s0_2) * 0xC) + (u8 *)&g_saved_game__for_func_8005B8C8), u8 *, 0x2F0) >> 2) & 1) != 1))) && ((arg1 >= 5) || (temp_s0_3 = arg1 + 1, (func_8005D670__for_func_8005B8C8(arg0, temp_s0_3) == 0xFF)) || (var_v0 = 1, ((((u8) M2C_FIELD(((func_8005D670__for_func_8005B8C8(arg0, temp_s0_3) * 0xC) + (u8 *)&g_saved_game__for_func_8005B8C8), u8 *, 0x2F0) >> 2) & 1) != 1))))
                {
                    temp_s0_4 = arg0 - 1;
                    if (((s32) arg0 > 0) && (func_8005D670__for_func_8005B8C8(temp_s0_4, arg1) != 0xFF))
                    {
                        var_v0_2 = (s32) arg0 < 5;
                        if ((((u8) M2C_FIELD(((func_8005D670__for_func_8005B8C8(temp_s0_4, arg1) * 0xC) + (u8 *)&g_saved_game__for_func_8005B8C8), u8 *, 0x2F0) >> 2) & 1) == 1)
                        {
                            goto block_23;
                        }
                        goto block_25;
                    }
                    var_v0_2 = (s32) arg0 < 5;
block_25:
                    temp_s0_5 = arg0 + 1;
                    if ((var_v0_2 == 0) || (func_8005D670__for_func_8005B8C8(temp_s0_5, arg1) == 0xFF) || (var_v0 = 1, ((((u8) M2C_FIELD(((func_8005D670__for_func_8005B8C8(temp_s0_5, arg1) * 0xC) + (u8 *)&g_saved_game__for_func_8005B8C8), u8 *, 0x2F0) >> 2) & 1) != 1)))
                    {
                        goto block_28;
                    }
                    /* Duplicate return node #29. Try simplifying control flow for better match */
                    return var_v0;
                }
                /* Duplicate return node #29. Try simplifying control flow for better match */
                return var_v0;
            }
block_23:
            return 1;
        }
        /* Duplicate return node #29. Try simplifying control flow for better match */
        return var_v0;
    }
block_28:
    var_v0 = 0;
    return var_v0;
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/** @brief Place a map record and update its neighboring layout data.
 * @param x Grid column.
 * @param y Grid row.
 * @param index Save record index.
 */
void func_8005BBC8(s32 x, s32 y, s32 index)
{
/* Partial WMAP decompilation: 93.676765% (gcc280_g0). */

extern u8 D_800CFDCC[64][12];
extern s32 D_800D8318[64][8];
extern s16 g_music_track_index;
extern void func_8005BD54__for_func_8005BBC8(s32, s32, s32, s32 (*)[8]) __asm__("func_8005BD54");
extern void func_8005C998__for_func_8005BBC8(void) __asm__("func_8005C998");

    s32 record;
    s32 field;
    SavedGame *entry;

    entry = (SavedGame *)(g_saved_game.bytes + index * 12);
    entry->bytes[0x2F1] = (x & 15) | (y * 16);
    entry->bytes[0x2F0] |= 2;
    g_saved_game.bytes[0x2E5]++;
    entry->bytes[0x2F2] = g_saved_game.bytes[0x2E5];
    if (g_saved_game.bytes[0x2E5] == 1)
    {
        g_music_track_index = index;
    }
    for (record = 0; record < 64; record++)
    {
        for (field = 0; field < 8; field++)
        {
            D_800D8318[record][field] = ((SavedGame *)(g_saved_game.bytes + record * 12 + field))->bytes[0x2F4];
        }
    }
    for (field = 0; field < 8; field++)
    {
        D_800D8318[index][field] = D_800CFDCC[index][field];
    }
    func_8005BD54__for_func_8005BBC8(index, x, y, D_800D8318);
    for (record = 0; record < 64; record++)
    {
        for (field = 0; field < 8; field++)
        {
            ((SavedGame *)(g_saved_game.bytes + record * 12 + field))->bytes[0x2F4] = D_800D8318[record][field];
        }
    }
    func_8005C998__for_func_8005BBC8();
}

void func_8005BD54(s32 arg0, u32 arg1, s32 arg2, s32 arg3)
{
/* Partial WMAP decompilation: 81.913550% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

s32 func_8005D670__for_func_8005BD54(s32, s32) __asm__("func_8005D670");
s32 func_8005B8C8__for_func_8005BD54(u32, s32, s32) __asm__("func_8005B8C8");                   /* static */
extern u8 D_800CFDCC;
extern s32 D_800D8B3C;
extern u8 g_saved_game__for_func_8005BD54 __asm__("g_saved_game");

    s32 sp2C;
    u32 sp10;
    s32 *var_a0_6;
    s32 *var_a1;
    s32 *var_a1_2;
    s32 *var_a1_3;
    s32 *var_a1_5;
    s32 *var_a3_2;
    s32 *var_v0;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_12;
    s32 temp_v1_13;
    s32 temp_v1_18;
    s32 temp_v1_21;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 temp_v1_9;
    s32 var_a0_15;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_a1_10;
    s32 var_a1_11;
    s32 var_a1_12;
    s32 var_a1_13;
    s32 var_a1_14;
    s32 var_a1_4;
    s32 var_a1_6;
    s32 var_a1_7;
    s32 var_a1_8;
    s32 var_a1_9;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_a2_4;
    s32 var_a3;
    u32 *var_a0_10;
    u32 *var_a0_11;
    u32 *var_a0_12;
    u32 *var_a0_13;
    u32 *var_a0_14;
    u32 *var_a0_7;
    u32 *var_a0_8;
    u32 *var_a0_9;
    u32 *var_a2_5;
    u32 temp_v0_5;
    u32 temp_v1_14;
    u32 temp_v1_16;
    u32 var_a0;
    u8 temp_v1_15;
    u8 temp_v1_17;
    u8 temp_v1_19;
    u8 temp_v1_20;
    u8 temp_v1_5;

    if ((arg1 < 6U) && (arg2 >= 0) && (arg2 < 6) && (func_8005B8C8__for_func_8005BD54(arg1, arg2, arg0) != 0))
    {
        temp_v0 = func_8005D670__for_func_8005BD54(arg1 - 1, arg2);
        var_a0 = arg1 + 1;
        if (temp_v0 != 0xFF)
        {
            var_a2 = 0;
            if (arg0 != 0xFF)
            {
                var_a1 = (temp_v0 << 5) + arg3;
                do
                {
                    temp_v1 = *var_a1 + (*(var_a2 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1 = temp_v1;
                    if (temp_v1 >= 0)
                    {
                        var_a0_2 = 6;
                        if (temp_v1 < 7)
                        {
                            var_a0_2 = temp_v1;
                        }
                    }
                    else
                    {
                        var_a0_2 = 0;
                    }
                    *var_a1 = var_a0_2;
                    var_a2 += 1;
                    var_a1 = (s32 *)((u8 *)var_a1 + 4);
                } while (var_a2 < 8);
                var_a0 = arg1 + 1;
            }
        }
        temp_v0_2 = func_8005D670__for_func_8005BD54(var_a0, arg2);
        if (temp_v0_2 != 0xFF)
        {
            var_a2_2 = 0;
            if (arg0 != 0xFF)
            {
                var_a1_2 = (temp_v0_2 << 5) + arg3;
                do
                {
                    temp_v1_2 = *var_a1_2 + (*(var_a2_2 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1_2 = temp_v1_2;
                    if (temp_v1_2 >= 0)
                    {
                        var_a0_3 = 6;
                        if (temp_v1_2 < 7)
                        {
                            var_a0_3 = temp_v1_2;
                        }
                    }
                    else
                    {
                        var_a0_3 = 0;
                    }
                    *var_a1_2 = var_a0_3;
                    var_a2_2 += 1;
                    var_a1_2 = (s32 *)((u8 *)var_a1_2 + 4);
                } while (var_a2_2 < 8);
            }
        }
        temp_v0_3 = func_8005D670__for_func_8005BD54(arg1, arg2 - 1);
        if (temp_v0_3 != 0xFF)
        {
            var_a2_3 = 0;
            if (arg0 != 0xFF)
            {
                var_a1_3 = (temp_v0_3 << 5) + arg3;
                do
                {
                    temp_v1_3 = *var_a1_3 + (*(var_a2_3 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1_3 = temp_v1_3;
                    if (temp_v1_3 >= 0)
                    {
                        var_a0_4 = 6;
                        if (temp_v1_3 < 7)
                        {
                            var_a0_4 = temp_v1_3;
                        }
                    }
                    else
                    {
                        var_a0_4 = 0;
                    }
                    *var_a1_3 = var_a0_4;
                    var_a2_3 += 1;
                    var_a1_3 = (s32 *)((u8 *)var_a1_3 + 4);
                } while (var_a2_3 < 8);
            }
        }
        temp_v0_4 = func_8005D670__for_func_8005BD54(arg1, arg2 + 1);
        var_a1_4 = 0;
        if (temp_v0_4 != 0xFF)
        {
            var_a2_4 = 0;
            if (arg0 != 0xFF)
            {
                var_a1_5 = (temp_v0_4 << 5) + arg3;
                do
                {
                    temp_v1_4 = *var_a1_5 + (*(var_a2_4 + (arg0 * 0xC) + (u8 *)&D_800CFDCC) - 3);
                    *var_a1_5 = temp_v1_4;
                    if (temp_v1_4 >= 0)
                    {
                        var_a0_5 = 6;
                        if (temp_v1_4 < 7)
                        {
                            var_a0_5 = temp_v1_4;
                        }
                    }
                    else
                    {
                        var_a0_5 = 0;
                    }
                    *var_a1_5 = var_a0_5;
                    var_a2_4 += 1;
                    var_a1_5 = (s32 *)((u8 *)var_a1_5 + 4);
                } while (var_a2_4 < 8);
                var_a1_4 = 0;
            }
        }
        temp_a2 = (arg1 + (arg2 * 6)) * 0xC;
        var_a0_6 = (arg0 << 5) + arg3;
        do
        {
            temp_v1_5 = M2C_FIELD((D_800D8B3C + (var_a1_4 + temp_a2)), u8 *, 8);
            var_a1_4 += 1;
            *var_a0_6 += temp_v1_5;
            var_a0_6 = (s32 *)((u8 *)var_a0_6 + 4);
        } while (var_a1_4 < 8);
        var_a1_6 = 7;
        var_v0 = &sp2C;
        do
        {
            *var_v0 = 0;
            var_a1_6 -= 1;
            var_v0 = (s32 *)((u8 *)var_v0 - 4);
        } while (var_a1_6 >= 0);
        temp_v1_6 = func_8005D670__for_func_8005BD54(arg1 - 1, arg2);
        var_a0_7 = &sp10;
        if (arg0 != 0xFF)
        {
            var_a1_7 = 0;
            if (temp_v1_6 != 0xFF)
            {
                do
                {
                    temp_v1_7 = var_a1_7 + (temp_v1_6 * 0xC);
                    var_a1_7 += 1;
                    *var_a0_7 = *var_a0_7 - 3 + M2C_FIELD((temp_v1_7 + (u8 *)&g_saved_game__for_func_8005BD54), u8 *, 0x2F4);
                    var_a0_7 = (u32 *)((u8 *)var_a0_7 + 4);
                } while (var_a1_7 < 8);
            }
        }
        temp_v1_8 = func_8005D670__for_func_8005BD54(arg1 + 1, arg2);
        var_a0_8 = &sp10;
        if (arg0 != 0xFF)
        {
            var_a1_8 = 0;
            if (temp_v1_8 != 0xFF)
            {
                do
                {
                    temp_v1_9 = var_a1_8 + (temp_v1_8 * 0xC);
                    var_a1_8 += 1;
                    *var_a0_8 = *var_a0_8 - 3 + M2C_FIELD((temp_v1_9 + (u8 *)&g_saved_game__for_func_8005BD54), u8 *, 0x2F4);
                    var_a0_8 = (u32 *)((u8 *)var_a0_8 + 4);
                } while (var_a1_8 < 8);
            }
        }
        temp_v1_10 = func_8005D670__for_func_8005BD54(arg1, arg2 - 1);
        var_a0_9 = &sp10;
        if (arg0 != 0xFF)
        {
            var_a1_9 = 0;
            if (temp_v1_10 != 0xFF)
            {
                do
                {
                    temp_v1_11 = var_a1_9 + (temp_v1_10 * 0xC);
                    var_a1_9 += 1;
                    *var_a0_9 = *var_a0_9 - 3 + M2C_FIELD((temp_v1_11 + (u8 *)&g_saved_game__for_func_8005BD54), u8 *, 0x2F4);
                    var_a0_9 = (u32 *)((u8 *)var_a0_9 + 4);
                } while (var_a1_9 < 8);
            }
        }
        temp_v1_12 = func_8005D670__for_func_8005BD54(arg1, arg2 + 1);
        var_a0_10 = &sp10;
        if ((arg0 != 0xFF) && (temp_v1_12 != 0xFF))
        {
            var_a1_10 = 0;
            do
            {
                temp_v1_13 = var_a1_10 + (temp_v1_12 * 0xC);
                var_a1_10 += 1;
                *var_a0_10 = *var_a0_10 - 3 + M2C_FIELD((temp_v1_13 + (u8 *)&g_saved_game__for_func_8005BD54), u8 *, 0x2F4);
                var_a0_10 = (u32 *)((u8 *)var_a0_10 + 4);
            } while (var_a1_10 < 8);
        }
        temp_v1_14 = arg1 - 1;
        var_a0_11 = &sp10;
        if ((temp_v1_14 < 6U) && (arg2 >= 0) && (arg2 < 6))
        {
            var_a1_11 = 0;
            do
            {
                temp_v1_15 = M2C_FIELD((D_800D8B3C + (var_a1_11 + ((temp_v1_14 + (arg2 * 6)) * 0xC))), u8 *, 8);
                var_a1_11 += 1;
                *var_a0_11 += temp_v1_15;
                var_a0_11 = (u32 *)((u8 *)var_a0_11 + 4);
            } while (var_a1_11 < 8);
        }
        temp_v1_16 = arg1 + 1;
        var_a0_12 = &sp10;
        if ((temp_v1_16 < 6U) && (arg2 >= 0) && (arg2 < 6))
        {
            var_a1_12 = 0;
            do
            {
                temp_v1_17 = M2C_FIELD((D_800D8B3C + (var_a1_12 + ((temp_v1_16 + (arg2 * 6)) * 0xC))), u8 *, 8);
                var_a1_12 += 1;
                *var_a0_12 += temp_v1_17;
                var_a0_12 = (u32 *)((u8 *)var_a0_12 + 4);
            } while (var_a1_12 < 8);
        }
        temp_v1_18 = arg2 - 1;
        var_a0_13 = &sp10;
        if ((arg1 < 6U) && (temp_v1_18 >= 0))
        {
            var_a1_13 = 0;
            if (temp_v1_18 < 6)
            {
                do
                {
                    temp_v1_19 = M2C_FIELD((D_800D8B3C + (var_a1_13 + ((arg1 + (temp_v1_18 * 6)) * 0xC))), u8 *, 8);
                    var_a1_13 += 1;
                    *var_a0_13 += temp_v1_19;
                    var_a0_13 = (u32 *)((u8 *)var_a0_13 + 4);
                } while (var_a1_13 < 8);
            }
        }
        temp_a1 = arg2 + 1;
        var_a0_14 = &sp10;
        if ((arg1 < 6U) && (temp_a1 >= 0))
        {
            var_a3 = 0;
            if (temp_a1 < 6)
            {
                do
                {
                    temp_v1_20 = M2C_FIELD((D_800D8B3C + (var_a3 + ((arg1 + (temp_a1 * 6)) * 0xC))), u8 *, 8);
                    var_a3 += 1;
                    *var_a0_14 += temp_v1_20;
                    var_a0_14 = (u32 *)((u8 *)var_a0_14 + 4);
                } while (var_a3 < 8);
            }
        }
        var_a1_14 = 0;
        var_a3_2 = (arg0 << 5) + arg3;
        var_a2_5 = &sp10;
        do
        {
            temp_v0_5 = *var_a2_5;
            temp_v1_21 = *var_a3_2 + ((s32) (temp_v0_5 + (temp_v0_5 >> 0x1F)) >> 1);
            *var_a3_2 = temp_v1_21;
            if (temp_v1_21 >= 0)
            {
                var_a0_15 = 6;
                if (temp_v1_21 < 7)
                {
                    var_a0_15 = temp_v1_21;
                }
            }
            else
            {
                var_a0_15 = 0;
            }
            *var_a3_2 = var_a0_15;
            var_a3_2 = (s32 *)((u8 *)var_a3_2 + 4);
            var_a1_14 += 1;
            var_a2_5 = (u32 *)((u8 *)var_a2_5 + 4);
        } while (var_a1_14 < 8);
    }
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/** @brief Calculate eight map attributes after placing a proposed land. */
void func_8005C404(u32 x, s32 y, u32 land, u32 proposed_x, s32 proposed_y, s32 *output)
{
/* Partial WMAP decompilation: 94.825584% (gcc280_g0). */

extern u8 D_800CFDCC[][12];
extern s32 D_800D00CC[];
extern s32 D_800D8318[][8];
extern u8 *D_800D8B3C;
extern s32 func_8005D670__for_func_8005C404(void) __asm__("func_8005D670");
extern void func_8005BD54__for_func_8005C404(s32, s32, s32, s32 (*)[8]) __asm__("func_8005BD54");

    s32 current_land;
    s32 cell;
    s32 i;
    s32 j;
    s32 component;
    s32 level;
    s32 row;
    s32 value;
    s32 fallback;

    if (x < 6U && y >= 0 && y < 6 && land < 64U)
    {
        if (proposed_x < 6U && proposed_y >= 0 && proposed_y < 6)
        {
            cell = x + y * 6;
            current_land = func_8005D670__for_func_8005C404();
            for (i = 0; i < 64; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    D_800D8318[i][j] = ((SavedGame *)(g_saved_game.bytes + (j + i * 12)))->bytes[0x2F4];
                }
            }
            for (j = 0; j < 8; j++)
            {
                D_800D8318[land][j] = D_800CFDCC[land][j];
            }
            func_8005BD54__for_func_8005C404(land, proposed_x, proposed_y, D_800D8318);
            for (i = 0; i < 8; i++)
            {
                value = 0;
                if (current_land == 255)
                {
                    if (proposed_x == x && proposed_y == y)
                    {
                        component = D_800CFDCC[land][i];
                        row = land * 32;
                        goto selected_land;
                    }
                    component = (D_800D8B3C + (i + cell * 12))[8];
                    fallback = 0;
                    if (component >= 0)
                    {
                        fallback = D_800D00CC[component * 8];
                    }
                    output[i] = fallback;
                }
                else
                {
                    component = ((SavedGame *)(g_saved_game.bytes + (i + current_land * 12)))->bytes[0x2F4];
                    row = current_land * 32;
selected_land:
                    level = *(s32 *)((u8 *)D_800D8318 + (i * 4 + row));
                    if (component >= 0 && level >= 0)
                    {
                        value = D_800D00CC[component * 7 + level];
                    }
                    output[i] = value;
                }
            }
        }
        else
        {
            for (i = 0; i < 8; i++)
            {
                output[i] = D_800D00CC[0];
            }
        }
    }
}

/** @brief Load the map layout and reconcile saved placement data.
 * @return Current layout status.
 */
s32 func_8005C6B4(void)

{
/* Partial WMAP decompilation: 94.203540% (gcc280_g0). */

extern void func_8005C998__for_func_8005C6B4(void) __asm__("func_8005C998");
extern void func_8005CC50__for_func_8005C6B4(s32) __asm__("func_8005CC50");
extern s32 D_800430B8;
extern u8 D_800D8B18[];
extern u8 *D_800D8B3C;
extern u8 D_800D8B40[];
extern u8 *D_800D8FF8;
extern u8 *D_800D8FFC;
extern s32 D_800D9130;

    struct SaveMapHeader
    {
        u8 pad_00[0x28];
        unsigned int unused_flags : 2;
        unsigned int option : 1;
        unsigned int other_flags : 29;
        u8 pad_2C[0xE0 - 0x2C];
        u16 layout_id;
    } *header;
    s32 index;
    u8 *cell;
    u8 saved_second;
    u8 saved_first;

    D_800D9130 = -1;
    cdrom_wait_queue_empty();
    header = (struct SaveMapHeader *)&g_saved_game;
    cdrom_queue_read((header->layout_id + 0x12EA) & 0xFFFF, D_800D8B40);
    cdrom_wait_queue_empty();
    D_800D8B3C = D_800D8B40;
    D_800D8FF8 = D_800D8B40 + 0x1B8;
    D_800D8FFC = D_800D8B40 + 0x3B8;
    if ((header->option != 1) && (g_saved_game.bytes[0x2E5] == 1))
    {
        g_saved_game.words[0x2E8 / 4] = (s32) (g_saved_game.words[0x2E8 / 4] | 2);
    }
    if ((g_saved_game.words[0x410 / 4] & 4) && !(g_saved_game.words[0x47C / 4] & 4))
    {
        g_saved_game.words[0x47C / 4] = (s32) (((((g_saved_game.words[0x47C / 4] | 7) & ~0xF00) | (g_saved_game.words[0x410 / 4] & 0xF00)) & 0xFFFF0FFF) | (g_saved_game.words[0x410 / 4] & 0xF000));
        g_saved_game.bytes[0x480] = 3;
        g_saved_game.bytes[0x481] = 3;
        g_saved_game.bytes[0x482] = 3;
        g_saved_game.bytes[0x483] = 3;
        g_saved_game.bytes[0x484] = 3;
        g_saved_game.bytes[0x485] = 3;
        g_saved_game.bytes[0x486] = 3;
        g_saved_game.bytes[0x487] = 3;
        saved_first = g_saved_game.bytes[0x412];
        saved_second = g_saved_game.bytes[0x413];
        g_saved_game.words[0x410 / 4] &= ~2;
        g_saved_game.words[0x410 / 4] &= ~1;
        g_saved_game.words[0x410 / 4] |= 0xFF00;
        g_saved_game.bytes[0x412] = 0U;
        g_saved_game.bytes[0x413] = 0U;
        g_saved_game.bytes[0x414] = 0;
        g_saved_game.bytes[0x415] = 0;
        g_saved_game.bytes[0x416] = 0;
        g_saved_game.bytes[0x417] = 0;
        g_saved_game.bytes[0x418] = 0;
        g_saved_game.bytes[0x419] = 0;
        g_saved_game.bytes[0x41A] = 0;
        g_saved_game.bytes[0x41B] = 0;
        g_saved_game.bytes[0x47E] = saved_first;
        g_saved_game.bytes[0x47F] = saved_second;
    }
    index = 0x23;
    cell = D_800D8B18 + index;
    do
    {
        *cell = 0xFF;
        index -= 1;
        cell -= 1;
    } while (index >= 0);
    func_8005C998__for_func_8005C6B4();
    func_8005CC50__for_func_8005C6B4(0);
    return D_800430B8;
}

/**
 * @brief Find the next pending world-map flag and consume nonpersistent entries.
 * @return The flag index, or -1 when no pending entry remains.
 */
s32 func_8005C878(void)
{
/* Partial WMAP decompilation: 97.222220% (gcc280_g0). */

extern s32 D_800D9130;
extern s32 g_layout_sub_mode;

    s32 bank;
    s32 mask;
    SavedGame *flags;

    D_800D9130++;
    while (D_800D9130 < 64)
    {
        bank = D_800D9130 / 32;
        flags = (SavedGame *)(g_saved_game.words + bank);
        mask = 1 << (D_800D9130 - (bank << 5));
        if (flags->words[0x2E8 / 4] & mask)
        {
            if (D_800D9130 == 0)
            {
                g_layout_sub_mode = 31;
            }
            if ((u32)(D_800D9130 - 2) >= 2U &&
                D_800D9130 != 22 && D_800D9130 != 23 &&
                D_800D9130 != 9 && D_800D9130 != 10 &&
                D_800D9130 != 11 && D_800D9130 != 12 &&
                D_800D9130 != 13 && D_800D9130 != 14 &&
                D_800D9130 != 24 && D_800D9130 != 0 &&
                D_800D9130 != 27 && D_800D9130 != 28)
            {
                flags->words[0x2E8 / 4] &= ~(1 << (D_800D9130 % 32));
            }
            return D_800D9130;
        }
        D_800D9130++;
    }
    return -1;
}

/** @brief Rebuild the world-map lookup from placed save records. */
void func_8005C998(void)
{
/* Partial WMAP decompilation: 88.707310% (gcc280_g0). */

extern s8 D_800D8B18[];

    s32 index;
    SavedGame *record;
    s32 row;
    u32 column;
    s32 doubled_column;
    u8 position;

    index = 0;
    record = &g_saved_game;
    do
    {
        if ((((u32)record->bytes[0x2F0] >> 1) & 1) == 1)
        {
            if (index == 0x18)
            {
                doubled_column = column * 2;
                if (!(g_saved_game.words[0x47C / 4] & 4))
                {
                    position = ((SavedGame *)(g_saved_game.bytes + 0x120))->bytes[0x2F1];
                    row = position & 0xF;
                    goto decode_column;
                }
            }
            else
            {
                position = record->bytes[0x2F1];
                row = position & 0xF;
decode_column:
                column = position >> 4;
                doubled_column = column * 2;
            }
            D_800D8B18[row + ((doubled_column + column) * 2)] = index;
        }
        index++;
        record = (SavedGame *)((u8 *)record + 12);
    } while (index < 0x40);
}

/** @brief Scroll the save list and rebuild its twelve visible entries. */
void func_8005CA3C(s32 forward, s32 *visible_entries)
{
extern u32 D_800D82F8;
extern s32 D_800D82FC;
extern s32 D_800D8300;
extern s32 D_800D8304;
extern s32 D_800D8308;
extern s32 D_800D9000[];
extern s32 func_8005D948__for_func_8005CA3C(s32, s32, s32) __asm__("func_8005D948");

    s32 row;
    s32 entry;
    s32 i;

    if (forward != 0)
    {
        if (D_800D9000[D_800D82F8] == -1)
        {
            if (D_800D82F8 >= 12U)
            {
                if (D_800D9000[D_800D82FC] == -1)
                {
                    D_800D82F8--;
                    D_800D82FC--;
                }
            }
        }
        D_800D8300++;
        D_800D82FC++;
    }
    else
    {
        if (D_800D9000[D_800D82F8] == -1 && D_800D82F8 >= 12U && D_800D9000[D_800D8304] == -1)
        {
            D_800D82F8--;
        }
        D_800D8300--;
        D_800D82FC--;
    }
    D_800D8300 = func_8005D948__for_func_8005CA3C(D_800D8300, 0, 11);
    D_800D82FC = func_8005D948__for_func_8005CA3C(D_800D82FC, 0, D_800D82F8);
    D_800D8308 = func_8005D948__for_func_8005CA3C(D_800D8300 + 11, 0, 11);
    D_800D8304 = func_8005D948__for_func_8005CA3C(D_800D82FC + 11, 0, D_800D82F8);
    for (i = 0; i < 12; i++)
    {
        row = func_8005D948__for_func_8005CA3C(D_800D8300 + i, 0, 11);
        entry = func_8005D948__for_func_8005CA3C(D_800D82FC + i, 0, D_800D82F8);
        visible_entries[row] = D_800D9000[entry];
    }
}

/** @brief Sort visible map entries by priority and initialize the selection range.
 * @param selection Requested selection position.
 * @return Number of visible map entries.
 */
s32 func_8005CC50(s32 selection)
{
/* Partial WMAP decompilation: 95.224000% (gcc280_g0). */

/** @brief Saved map entry status at its original byte offsets. */
typedef struct
{
    u8 pad_000[0x2F0];
    union
    {
        struct
        {
            unsigned int flag0 : 1;
            unsigned int hidden : 1;
            unsigned int other_flags : 6;
        } flags;
        struct
        {
            u8 flags;
            u8 pad[2];
            u8 priority;
        } data;
    } entry;
} WmapSavedEntry;
extern s32 D_800D82F8;
extern s32 D_800D82FC;
extern s32 D_800D8300;
extern s32 D_800D8304;
extern s32 D_800D8308;
extern s32 D_800D9000[];
extern s32 func_8005D948__for_func_8005CC50(s32, s32, s32) __asm__("func_8005D948");

    s32 i;
    s32 j;
    s32 current;
    s32 previous;
    s32 previous_index;
    s32 *current_slot;
    s32 *previous_slot;
    u32 end;
    WmapSavedEntry *entry;

    D_800D82F8 = 0;
    for (i = 63; i >= 0; i--)
    {
        D_800D9000[i] = -1;
    }
    for (i = 0; i < 64; i++)
    {
        entry = (WmapSavedEntry *)((u8 *)&g_saved_game + i * 12);
        if (entry->entry.data.priority != 0 && entry->entry.flags.hidden != 1)
        {
            j = D_800D82F8;
            D_800D82F8 = j + 1;
            D_800D9000[j] = i;
            while (j > 0)
            {
                current_slot = &D_800D9000[j];
                previous_index = j - 1;
                previous_slot = &D_800D9000[previous_index];
                current = *current_slot;
                previous = *previous_slot;
                if (((WmapSavedEntry *)((u8 *)&g_saved_game + previous * 12))->entry.data.priority <
                    ((WmapSavedEntry *)((u8 *)&g_saved_game + current * 12))->entry.data.priority)
                {
                    *current_slot = previous;
                    *previous_slot = current;
                }
                j = previous_index;
            }
        }
    }
    i = D_800D82F8;
    end = i + 12;
    for (; (u32)i < end; i++)
    {
        D_800D9000[i] = -1;
    }
    D_800D82F8 += 11;
    D_800D8300 = func_8005D948__for_func_8005CC50(selection + 3, 0, 11);
    D_800D82FC = func_8005D948__for_func_8005CC50(D_800D82F8 - 11, 0, D_800D82F8);
    D_800D8308 = func_8005D948__for_func_8005CC50(D_800D8300 + 11, 0, 11);
    D_800D8304 = func_8005D948__for_func_8005CC50(D_800D82FC + 11, 0, D_800D82F8);
    return D_800D82F8 - 11;
}

/** @brief Append a map cell's saved values and optional change indicators.
 * @param mode Nonzero to suppress change indicators.
 * @param x Map column.
 * @param y Map row.
 * @param mask Flag to set when the cell is occupied.
 * @param count Current output length, updated for each appended value.
 * @param flags Output flags.
 * @param output Destination values.
 * @param unused Unused argument slot.
 * @param comparison Per-cell comparison values.
 */
void func_8005CE44(s32 mode, u32 x, s32 y, s32 mask, s32 *count,
                  s32 *flags, s32 *output, s32 unused, s32 *comparison)
{
/* Partial WMAP decompilation: 87.153850% (gcc280_g0). */

extern u8 D_800D8B18[];

    s32 i;
    s32 difference;
    s32 marker;
    s32 *values;
    u8 cell = 255;

    if (x < 6U && y >= 0 && y < 6)
    {
        cell = D_800D8B18[x + y * 6];
    }
    if (cell != 255)
    {
        *flags |= mask;
        for (i = 0; i < 8; i++)
        {
            output[*count] = g_saved_game.bytes[0x2F4 + i + cell * 12] + 10;
            (*count)++;
        }
        if (mode == 0)
        {
            values = comparison + cell * 8;
            for (i = 0; i < 8; i++)
            {
                difference = *values - g_saved_game.bytes[0x2F4 + i + cell * 12];
                if (difference == 0)
                {
                    output[*count] = 0;
                    (*count)++;
                    output[*count] = 0;
                }
                else
                {
                    marker = 18;
                    if (difference < 0)
                    {
                        marker = 17;
                    }
                    output[*count] = marker;
                    (*count)++;
                    output[*count] = difference + 13;
                }
                (*count)++;
                values++;
            }
        }
        else
        {
            for (i = 0; i < 16; i++)
            {
                output[*count] = 0;
                (*count)++;
            }
        }
    }
}

void func_8005D018(s32 arg0, u32 arg1, s32 arg2, s32 *arg3, s32 arg4, s32 arg5)
{
/* Partial WMAP decompilation: 74.519860% (gcc280_g0). */

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

M2C_UNK func_8005BD54__for_func_8005D018(s32, u32, s32, void *) __asm__("func_8005BD54");    /* extern */
M2C_UNK func_8005CE44__for_func_8005D018(M2C_UNK, u32, s32, s32, s32 *, s32 *, s32, s32, void *) __asm__("func_8005CE44"); /* static */
extern u8 D_800CFDCC;
extern u8 D_800D8318;
extern u8 D_800D8B18;
extern s32 D_800D8B3C;
extern u8 g_saved_game__for_func_8005D018 __asm__("g_saved_game");

    s32 *sp38;
    s32 sp34;
    s32 *sp30;
    u32 sp2C;
    s32 sp28;
    s32 *var_t1;
    s32 *var_t3;
    s32 temp_a0;
    s32 temp_a3;
    s32 temp_s3;
    s32 temp_s4;
    s32 temp_s7;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_a2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s0_3;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s2_3;
    s32 var_s6;
    s32 var_t2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1_3;
    u32 var_s1;
    u8 temp_v0;
    u8 temp_v0_2;
    u8 var_v1;
    u8 var_v1_2;

    var_t1 = arg3;
    var_s2 = 0;
    var_t2 = arg5;
    var_a2 = 0;
    sp28 = 0;
    *var_t1 = 0;
    do
    {
        var_s0 = 0;
        var_a0 = var_s2 << 5;
loop_2:
        temp_v0 = M2C_FIELD((var_s0 + var_a2 + (u8 *)&g_saved_game__for_func_8005D018), u8 *, 0x2F4);
        var_s0 += 1;
        *(var_a0 + (u8 *)&D_800D8318) = (s32) temp_v0;
        var_a0 += 4;
        if (var_s0 < 8)
        {
            goto loop_2;
        }
        var_s2 += 1;
        var_a2 += 0xC;
    } while (var_s2 < 0x40);
    var_s0_2 = 0;
    var_a0_2 = var_t2 << 5;
    do
    {
        temp_v0_2 = *(var_s0_2 + (var_t2 * 0xC) + (u8 *)&D_800CFDCC);
        var_s0_2 += 1;
        *(var_a0_2 + (u8 *)&D_800D8318) = (s32) temp_v0_2;
        var_a0_2 += 4;
    } while (var_s0_2 < 8);
    if ((arg1 < 6U) && (arg2 >= 0) && (arg2 < 6))
    {
        var_v1 = *(arg1 + (arg2 * 6) + (u8 *)&D_800D8B18);
    }
    else
    {
        var_v1 = 0xFF;
    }
    if (var_v1 == 0xFF)
    {
        sp30 = var_t1;
        sp34 = var_t2;
        func_8005BD54__for_func_8005D018(var_t2, arg1, arg2, &D_800D8318);
    }
    var_s2_2 = 0;
    var_t3 = &D_800D8B3C;
    var_s6 = 0;
    sp2C = arg1 - (arg0 % 3);
    temp_s7 = arg2 - (arg0 / 3);
    do
    {
        var_s0_3 = 0;
        temp_s3 = temp_s7 + var_s2_2;
        temp_s4 = temp_s3 * 6;
        var_s1 = sp2C;
        var_v0 = var_s6;
loop_15:
        temp_a3 = 1 << var_v0;
        if ((var_s1 < 6U) && (temp_s3 >= 0) && (temp_s3 < 6))
        {
            var_v1_2 = *(var_s1 + temp_s4 + (u8 *)&D_800D8B18);
        }
        else
        {
            var_v1_2 = 0xFF;
        }
        var_a0_3 = 0;
        if (var_v1_2 != 0xFF)
        {
            sp30 = var_t1;
            sp34 = var_t2;
            sp38 = var_t3;
            func_8005CE44__for_func_8005D018(0, var_s1, temp_s7 + var_s2_2, temp_a3, &sp28, var_t1, arg4, var_t2, &D_800D8318);
            var_t1 = sp30;
            var_t2 = sp34;
            var_t3 = sp38;
            var_s1 += 1;
        }
        else
        {
            var_a1 = 0;
            temp_v1 = var_s1 + temp_s4;
            var_v0_2 = temp_v1 * 0xC;
            do
            {
                var_a1 += 1;
                var_a0_3 += M2C_FIELD((*var_t3 + var_v0_2), u8 *, 8);
                var_v0_2 = var_a1 + (temp_v1 * 0xC);
            } while (var_a1 < 8);
            if (var_a0_3 > 0)
            {
                var_a1_2 = 0;
                *var_t1 |= temp_a3;
                do
                {
                    temp_v0_3 = var_a1_2 + ((var_s1 + temp_s4) * 0xC);
                    var_a1_2 += 1;
                    *(s32 *)((sp28 * 4) + arg4) = M2C_FIELD((*var_t3 + temp_v0_3), u8 *, 8) + 0xD;
                    sp28 += 1;
                } while (var_a1_2 < 8);
                var_a1_3 = 0;
                do
                {
                    var_a1_3 += 1;
                    *(s32 *)((sp28 * 4) + arg4) = 0;
                    sp28 += 1;
                } while (var_a1_3 < 0x10);
            }
            var_s1 += 1;
        }
        var_s0_3 += 1;
        var_v0 = var_s6 + var_s0_3;
        if (var_s0_3 < 3)
        {
            goto loop_15;
        }
        var_s2_2 += 1;
        var_s6 += 3;
    } while (var_s2_2 < 3);
    var_s2_3 = 0;
    *var_t1 |= 0x200;
    do
    {
        temp_a0 = *((var_s2_3 * 4) + (var_t2 << 5) + (u8 *)&D_800D8318) - *(var_s2_3 + (var_t2 * 0xC) + (u8 *)&D_800CFDCC);
        if (temp_a0 == 0)
        {
            temp_v1_2 = sp28 + 1;
            *(s32 *)((sp28 * 4) + arg4) = 0;
            sp28 = temp_v1_2;
            *(s32 *)((temp_v1_2 * 4) + arg4) = 0;
        }
        else
        {
            var_v1_3 = 0x1B;
            if (temp_a0 < 0)
            {
                var_v1_3 = 0x1A;
            }
            *(s32 *)((sp28 * 4) + arg4) = var_v1_3;
            temp_v0_4 = sp28 + 1;
            sp28 = temp_v0_4;
            *(s32 *)((temp_v0_4 * 4) + arg4) = temp_a0 + 0x16;
        }
        var_s2_3 += 1;
        sp28 += 1;
    } while (var_s2_3 < 8);
}
#undef M2C_FIELD
#undef M2C_UNALIGNED32
#undef M2C_BITWISE

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D46C(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D474(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D47C(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D484(void)
{
}

/**
 * @brief Empty world-map handler (no operation).
 */
void func_8005D48C(void)
{
}

/**
 * @brief Read the low seven bits of the saved world-map value.
 * @return Value masked to seven bits.
 */
s32 func_8005D494(void)
{
extern u16 D_800432BE;

    return D_800432BE & 0x7F;
}

/**
 * @brief Read the current table value, clamped to the range zero through nine.
 * @return Clamped table value.
 */
s32 func_8005D4A4(void)
{
/** @brief Table of 64 world-map values copied to a local buffer. */
typedef struct
{
    s32 values[64];
} WmapValueTable;

extern u8 D_800432BD;
extern WmapValueTable D_800514F4;

    WmapValueTable table = D_800514F4;
    s32 value;
    s32 result;

    value = table.values[D_800432BD];
    if (value >= 0)
   
   {
        result = 9;
        if (value < 10)
       
       {
            result = value;
        }
    }
    else
   
   {
        result = 0;
    }
    return result;
}

/**
 * @brief Read bit two of a saved world-map record's flags.
 * @param record_index Index of the 12-byte record.
 * @return One when bit two is set, otherwise zero.
 */
s32 func_8005D528(s32 record_index)
{
    return (g_saved_game.bytes[record_index * 12 + 0x2F0] >> 2) & 1;
}

/**
 * @brief Test whether a different grid location has a valid table entry.
 * @param x Grid column.
 * @param y Grid row.
 * @return Nonzero if the location differs from the current location and is valid.
 */
s32 func_8005D554(u32 x, s32 y)
{
extern u32 D_80043454;
extern u8 D_800D8B18[];

    s32 value;
    if (((D_80043454 >> 8) & 15) == x && ((D_80043454 >> 12) & 15) == y)
   
   {
        return 0;
    }
    if (x < 6 && y >= 0 && y < 6)
   
   {
        value = D_800D8B18[x + y * 6];
    }
    else
   
   {
        value = 255;
    }
    return value != 255;
}

/**
 * @brief Compute the current world-map progress difference using the selected record flags.
 * @return Difference from the saved progress byte, or from the current progress byte.
 */
s32 func_8005D5D8(void)
{
extern u8 D_800432BD;
extern u8 *D_800D8B3C;

    u32 flags = g_saved_game.words[0x434 / 4];
    u8 *data;
    s32 column;
    s32 row;
    s32 cell;
    u8 *entry;

    if (flags & 2)
    {
        data = D_800D8B3C;
        column = (flags >> 8) & 15;
        row = (flags >> 12) & 15;
        cell = column + row * 6;
        entry = data + cell * 12;
        if (*(u16 *)(entry + 0x10) & 4)
        {
            return *(s32 *)(data + 4) - g_saved_game.bytes[0x2E5] + 1;
        }
    }
    return *(s32 *)(D_800D8B3C + 4) - D_800432BD;
}

/**
 * @brief Read a byte from the six-by-six world-map table.
 * @param column Column index.
 * @param row Row index.
 * @return Table entry, or 0xFF when an index is outside the table.
 */
u8 func_8005D670(u32 column, s32 row)
{
extern u8 D_800D8B18[];

    if (column < 6U && row >= 0 && row < 6)
    {
        return D_800D8B18[column + row * 6];
    }
    return 0xFF;
}

/**
 * @brief Resolve a map cell's eight entries into resource values.
 * @param column Map column.
 * @param row Map row.
 * @param output Destination for eight words.
 */
void func_8005D6B8(u32 column, s32 row, s32 *output)
{
/* Partial WMAP decompilation: 91.206894% (gcc280_g0). */

extern s32 func_8005D670__for_func_8005D6B8(u32, s32) __asm__("func_8005D670");
extern u8 *D_800D8B3C;
extern s32 D_800D00CC[][8];

    s32 *entry;
    s32 record;
    s32 i;
    s32 index;
    s32 value;
    s32 cell;
    u8 *map_data;

    if (column < 6U && row >= 0 && row < 6)
    {
        cell = column + row * 6;
        record = func_8005D670__for_func_8005D6B8(column, row);
        i = 0;
        map_data = D_800D8B3C;
        entry = output;
        do
        {
            if (record == 255)
            {
                index = *(map_data + (i + cell * 12) + 8) + 3;
            }
            else
            {
                index = g_saved_game.bytes[i + record * 12 + 0x2F4];
            }
            value = 0;
            if (index >= 0)
            {
                value = D_800D00CC[index][0];
            }
            *entry = value;
            i++;
            entry++;
        } while (i < 8);
    }
}

/**
 * @brief Resolve eight table entries into the output buffer.
 * @param table_index Table row, or -1 to use entry three for every output.
 * @param output Destination for eight words.
 */
void func_8005D7A0(s32 table_index, s32 *output)
{
extern u8 D_800CFDCC[];
extern s32 D_800D00CC[][8];

    s32 *entry;
    s32 i;
    s32 value;
    s32 index;

    entry = output;
    i = 0;
    do
    {
        index = 3;
        if (table_index != -1)
    {
            index = D_800CFDCC[i + table_index * 12];
        }
        value = 0;
        if ((s32) index >= 0)
    {
            value = D_800D00CC[index][0];
        }
        *entry = value;
        i += 1;
        entry += 1;
    } while (i < 8);
}

/**
 * @brief Test whether a saved world-map record has bit zero set and bit one clear.
 * @param record_index Index of the 12-byte record.
 * @return One when both flag conditions hold, otherwise zero.
 */
s32 func_8005D810(s32 record_index)
{
    u32 flags = g_saved_game.bytes[record_index * 12 + 0x2F0];
    if (flags & 1)
    {
        return ((flags >> 1) & 1) ^ 1;
    }
    return 0;
}

/**
 * @brief Read the active map coordinates, falling back to the default pair.
 * @param row Receives the low-nibble coordinate or default first coordinate.
 * @param column Receives the high-nibble coordinate or default second coordinate.
 * @return Low seven bits of the current map status.
 */
s32 func_8005D850(s32 *row, u32 *column)
{
extern u8 D_800435E0;
extern u16 *D_800D8B3C;
extern u16 g_music_track_index;

    if (!(g_saved_game.words[0x2F0 / 4] & 2))
    {
        *row = D_800D8B3C[0];
        *column = D_800D8B3C[1];
    }
    else
    {
        *row = ((SavedGame *)(g_music_track_index * 12 + g_saved_game.bytes))->bytes[0x2F1] & 15;
        *column = ((SavedGame *)(g_music_track_index * 12 + g_saved_game.bytes))->bytes[0x2F1] >> 4;
    }
    return D_800435E0 & 0x7F;
}

/**
 * @brief Look up the entry four positions ahead, wrapping at the table limit.
 * @return Value at the wrapped index.
 */
s32 func_8005D8FC(void)
{
extern s32 func_8005D948__for_func_8005D8FC(s32, s32, s32) __asm__("func_8005D948");
extern s32 D_800D82F8;
extern s32 D_800D82FC;
extern s32 D_800D9000[];

    return D_800D9000[func_8005D948__for_func_8005D8FC(D_800D82FC + 4, 0, D_800D82F8)];
}

/**
 * @brief Wrap a value into an inclusive range.
 * @param value Value to wrap.
 * @param minimum Lower endpoint.
 * @param maximum Upper endpoint; must not be less than minimum.
 * @return Wrapped value.
 */
s32 func_8005D948(s32 value, s32 minimum, s32 maximum)
{
    s32 adjusted;
    if (value < minimum)
    {
        adjusted = value - minimum + 1;
        return func_8005D948(adjusted + maximum, minimum, maximum);
    }
    if (maximum < value)
    {
        adjusted = value - 1;
        adjusted -= maximum;
        return func_8005D948(adjusted + minimum, minimum, maximum);
    }
    return value;
}

/**
 * @brief Look up a value in the seven-column world-map table.
 * @param row Table row.
 * @param column Table column.
 * @return Table value, or zero if either index is negative.
 */
s32 func_8005D980(s32 row, s32 column)
{
extern s32 D_800D00CC[];

    s32 value = 0;
    if (row >= 0 && column >= 0)
    {
        value = D_800D00CC[row * 7 + column];
    }
    return value;
}

/**
 * @brief Test whether saved flags 0, 9 through 14, 24, 27, or 28 are set.
 * @return One if any of the selected flags is set, otherwise zero.
 */
s32 func_8005D9B4(void)
{
    s32 index;
    s32 found;
    s32 word_index;
    u32 bits;
    u32 flags;
    SavedGame *word_base;

    found = 0;
    index = 0;
    do
    {
        word_index = index / 32;
        word_base = (SavedGame *)(word_index * 4 + g_saved_game.bytes);
        bits = 1U << (index - word_index * 32);
        flags = word_base->words[0x2E8 / 4];
        if ((flags & bits) &&
            ((u32)(index - 9) < 6 || index == 24 || index == 0 || index == 27 || index == 28))
        {
            found = 1;
        }
        index++;
    } while (index < 64);
    return found;
}

/**
 * @brief Test whether saved flags 2, 3, 22, or 23 are set.
 * @return One if any of the four flags is set, otherwise zero.
 */
s32 func_8005DA50(void)
{
    s32 index;
    s32 found;
    s32 word_index;
    u32 bits;
    u32 flags;
    SavedGame *word_base;

    found = 0;
    index = 0;
    do
    {
        word_index = index / 32;
        word_base = (SavedGame *)(word_index * 4 + g_saved_game.bytes);
        bits = 1U << (index - word_index * 32);
        flags = word_base->words[0x2E8 / 4];
        if ((flags & bits) &&
            ((u32)(index - 2) < 2 || index == 22 || index == 23))
        {
            found = 1;
        }
        index++;
    } while (index < 64);
    return found;
}

/**
 * @brief Apply eight table adjustments and clamp each result to zero through six.
 * @param table_row Adjustment row, or 255 to skip.
 * @param output_row Destination row, or 255 to skip.
 * @param output_address Address of the destination table, with eight words per row.
 */
void func_8005DAD8(s32 table_row, s32 output_row, s32 output_address)
{
extern u8 D_800CFDCC[];

    s32 *entry;
    u8 *table;
    s32 row_offset;
    s32 adjustment;
    s32 value;
    s32 clamped;
    s32 index;

    if (output_row != 0xFF)
    {
        index = 0;
        if (table_row != 0xFF)
        {
            table = D_800CFDCC;
            row_offset = table_row * 12;
            entry = (s32 *)((output_row << 5) + output_address);
            for (; index < 8; index++, entry++)
            {
                adjustment = *(u8 *)(index + row_offset + (s32)table) - 3;
                value = *entry + adjustment;
                *entry = value;
                if (value >= 0)
                {
                    clamped = 6;
                    if (value < 7)
                    {
                        clamped = value;
                    }
                }
                else
                {
                    clamped = 0;
                }
                *entry = clamped;
            }
        }
    }
}

/**
 * @brief Add eight saved-record adjustments, each biased by minus three.
 * @param record_index Record index; 0xFF disables the update.
 * @param other_index A second index; 0xFF disables the update.
 * @param unused Unused argument retained by the calling convention.
 * @param values Eight values updated in place.
 */
void func_8005DB5C(s32 record_index, s32 other_index, s32 unused, s32 *values)
{
    s32 index;
    s32 value;
    if (other_index != 0xFF)
    {
        index = 0;
        if (record_index != 0xFF)
        {
            do
            {
                value = *values - 3;
                *values = value + g_saved_game.bytes[index + record_index * 12 + 0x2F4];
                index++;
                values++;
            } while (index < 8);
        }
    }
}
