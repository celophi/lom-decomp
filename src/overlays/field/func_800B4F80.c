#include "common.h"

s32 func_8008ADB4(u8 arg0);
s32 func_800B2D34(u8 *arg0, s32 arg1);
u32 func_800B4CE4(void *arg0, s32 arg1);
void saturating_counter_add(void *counter, s32 delta);

extern u8 *D_80122B78;

/**
 * @brief Update the record's saturating counters when its growth interval elapses.
 * @param arg0 Record containing the growth state and linked counter.
 */
void func_800B4F80(u8 *arg0)
{
    u16 flags;
    s32 multiplier;
    s32 scaled_remaining;
    s32 divisor;
    s32 classification;

    flags = *(u16 *)(arg0 + 0xA);
    if (flags & 1)
    {
        return;
    }
    if (*(s32 *)(*(u8 **)(arg0 + 0x10) + 0xC) & 0x391)
    {
        return;
    }
    if ((flags & 8) || func_800B4CE4(arg0, 2) != 0)
    {
        multiplier = 8;
    }
    else
    {
        classification = func_8008ADB4(arg0[4]);
        if (classification < 0)
        {
            return;
        }
        if (classification < 2)
        {
            multiplier = 4;
        }
        else if (classification != 0x31)
        {
            return;
        }
        else
        {
            multiplier = 2;
        }
    }

    scaled_remaining = (0x64 - func_800B2D34(arg0, 4)) * multiplier;
    if (scaled_remaining < 0)
    {
        scaled_remaining += 0xF;
    }
    divisor = scaled_remaining >> 4;
    if (divisor <= 0)
    {
        divisor = 1;
    }
    if ((u32)(*(s32 *)(D_80122B78 + 0xBC)) % (u32)divisor == 0)
    {
        saturating_counter_add(*(u8 **)(arg0 + 0x10), 1);
        if (arg0[4] < 3)
        {
            saturating_counter_add(*(u8 **)(arg0 + 0x10), func_800B4CE4(arg0, 1));
        }
    }
}
