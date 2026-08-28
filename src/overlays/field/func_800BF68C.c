#include "common.h"

extern u8 *D_80123FC4;

s32 func_800BF9F0(s32 arg0);

/**
 * @brief Replaces a matching byte in one of the active field slots.
 *
 * @param arg0 Minimum active-slot state passed to func_800BF9F0.
 * @param arg1 Byte value to find in slots 4 through 2.
 * @param arg2 Replacement byte.
 * @return The replaced slot index, or 0xFF when no slot was replaced.
 */
s32 func_800BF68C(s32 arg0, s32 arg1, u8 arg2)
{
    s32 index;
    s32 match;
    u8 *entry;
    u8 value;

    match = arg1;
    value = arg2;

    if (func_800BF9F0(arg0) != 0)
    {
        index = 4;
        do
        {
            entry = D_80123FC4 + index;
            if (entry[0x28] == match)
            {
                entry[0x28] = value;
                return index;
            }
            index--;
        } while (index >= 2);
    }

    return 0xFF;
}
