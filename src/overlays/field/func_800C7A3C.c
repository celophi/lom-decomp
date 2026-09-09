#include "common.h"
extern void func_800C0260(s32, s32);
extern void func_800C7C88(void);
extern s32 rand(void);
extern u8 D_80122C1F;
/** @brief Layout buffer view exposing record metadata, entry slots, and shared counters. */
typedef struct Layout
{
    u8 pad[0x25E0];
    u8 counters[0x104];
    u32 packed;
    u8 gap[4];
    u8 slots[8];
    u8 entry;
} Layout;
extern Layout g_menuLayoutBuffer;
/**
 * @brief Fill free record entries up to the clamped capacity and update linked counters.
 * @note Random slot selection retries at most 1000 times before scanning for a free slot.
 */
void func_800C7A3C(void)
{
    s32 temp_a0;
    s32 temp_s4;
    s32 temp_s6;
    s32 temp_s7;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a1;
    s32 var_s0;
    s32 var_s1;
    s32 var_v0;
    s32 var_v1;
    s32 var_v1_2;
    u32 temp_v1;
    s32 temp_s2;

    u8 *temp_v1_4;

    var_s0 = 0;
    var_s1 = var_s0;
    temp_s2 = D_80122C1F;
    temp_v0 = temp_s2 * 0x8C;
    temp_v1 = ((Layout *)((u8 *)&g_menuLayoutBuffer + temp_v0))->packed;
    var_a1 = temp_v1 & 0xF;
    var_a0 = var_s1 * 16 + temp_v0;
    temp_s7 = temp_v1 >> 8;
    temp_s7 &= 0xF;
    temp_s4 = temp_v1 >> 12;
    temp_s4 &= 0xF;
    temp_s4 += 2;
    do
    {
        if (((Layout *)((u8 *)&g_menuLayoutBuffer + (var_a0)))->entry != 0xFF)
        {
            var_s0 += 1;
        }
        var_s1 += 1;
        var_a0 = var_s1 * 16 + temp_v0;
    } while (var_s1 < 8);
    var_a1 -= var_s0;
    if (temp_s4 >= 0)
    {
        var_v1 = var_a1;
        if (var_v1 >= temp_s4)
        {
            var_v1 = temp_s4;
        }
    }
    else
    {
        var_v1 = 0;
    }
    temp_s4 = var_v1;
    var_v1 = 0;
    var_s1 = var_v1;
    if (temp_s4 > 0)
    {
        temp_s6 = temp_s2 * 0x24;
        do
        {
            for (var_s0 = 0; var_s0 < 1000; var_s0++)
            {
                var_a1 = rand() / 4096;
                if (((Layout *)((u8 *)&g_menuLayoutBuffer + var_a1 * 16 + temp_s2 * 0x8C))->entry ==
                    0xFF)
                {
                    break;
                }
            }
            if (((Layout *)((u8 *)&g_menuLayoutBuffer + var_a1 * 16 + temp_s2 * 0x8C))->entry ==
                0xFF)
            {
                func_800C0260(temp_s2, var_a1);
            }
            else
            {
                for (var_s0 = 0; var_s0 < 8; var_s0++)
                {
                    if (((Layout *)((u8 *)&g_menuLayoutBuffer + var_s0 * 16 + temp_s2 * 0x8C))
                            ->entry == 0xFF)
                    {
                        func_800C0260(temp_s2, var_s0);
                        break;
                    }
                }
            }
            var_s1++;
        } while (var_s1 < temp_s4);
    }
    var_s1 = 0;
    if (temp_s7 != 0)
    {
        temp_v1_4 = (u8 *)&g_menuLayoutBuffer;
        var_a1 = temp_s2 * 0x8C;
        do
        {
            var_v1 = ((Layout *)((var_s1 + var_a1) + (u32)temp_v1_4))->slots[0];
            if ((s32)var_v1 < 0xFF)
            {
                temp_v1_4[var_v1 + 0x25E0]--;
            }
            var_s1 += 1;

        } while (var_s1 < temp_s7);
    }
    func_800C7C88();
}
