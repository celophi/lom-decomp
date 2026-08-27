#include "common.h"

extern u8 *D_80122B74;

/**
 * @brief Computes a scaled progression value from a field record's state byte.
 *
 * Reads the state byte at offset 0x610 of the record selected by @p index
 * (records are 0x250 bytes apart in the table at *D_80122B74). Below the
 * threshold 0x63 the return is a scaled progression of that byte,
 * (state - 1) * (state * 20) + state * 10; at or above the threshold the packed
 * 32-bit field at the same offset is returned shifted right by 8 instead.
 *
 * @param index Record index into the table at *D_80122B74.
 * @return The scaled progression below the threshold, otherwise the packed
 *         counter bits of the 32-bit field at offset 0x610.
 */
s32 func_800B607C(s32 index)
{
    u8 *record = D_80122B74 + index * 0x250;
    s32 value = record[0x610];

    if (value < 0x63)
    {
        s32 previous = value - 1;
        s32 scaled = value * 5;
        s32 scaled_x4 = scaled * 4;
        return previous * scaled_x4 + scaled * 2;
    }

    return *(u32 *)(record + 0x610) >> 8;
}
