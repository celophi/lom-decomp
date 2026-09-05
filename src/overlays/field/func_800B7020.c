#include "common.h"

extern u8 *D_80123FB0;

/**
 * @brief Roll a randomized scaled value into the 4-byte slot of the active resource.
 * @note Reads the packed word at *(D_80123FB0+0x1C)+4; if its top byte is zero it is
 *       forced to 1. A random roll modulo that byte is added to the middle byte, then
 *       multiplied by the slot's current value at *(D_80123FB0+0x24)+0x10, shifted right
 *       by 4, floored to 1, and written back.
 * @see decomp.me (100.00%)
 */
void func_800B7020(void)
{
    u32 packed;
    u8 *slot;
    u32 value;
    s32 roll;
    u32 scratch;

    packed = *(u32 *)(*(u8 **)(D_80123FB0 + 0x1C) + 4);
    if ((packed >> 24) == 0)
    {
        packed &= 0xFFFFFF;
        packed |= 0x1000000;
    }
    roll = rand();
    scratch = packed >> 24;
    roll %= (s32)scratch;
    scratch = *(u32 *)(D_80123FB0 + 0x24);
    slot = *(u8 **)(scratch + 0x10);
    value = ((packed >> 16) & 0xFF) + roll;
    {
        s32 scale;
        scale = *(s32 *)(slot + 4);
        scale *= value;
        value = (u32)scale >> 4;
    }
    if (value == 0)
    {
        value = 1;
    }
    *(s32 *)(slot + 4) = value;
}
