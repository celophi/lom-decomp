#include "common.h"

/**
 * @brief Extract a masked bitfield from an 8-, 16-, or 32-bit element.
 *
 * @param arg0 Element width selector: 0 = byte, 1 = halfword, 2 = word.
 * @param arg1 Base address of the element array.
 * @param arg2 Element index.
 * @param arg3 Right-shift amount.
 * @param arg4 Number of low bits to retain; values >= 32 retain all bits.
 * @return The selected element shifted right by @p arg3 and masked to @p arg4 bits.
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 */
s32 func_800BD650(s32 arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 mask;
    s32 value;

    if (arg4 < 0x20)
    {
        mask = (1 << arg4) - 1;
    }
    else
    {
        mask = -1;
    }

    switch (arg0)
    {
    case 0:
        value = arg1[arg2] >> arg3;
        return value & mask;
    case 1:
        value = (*(u16 *)(arg1 + arg2 * 2)) >> arg3;
        return value & mask;
    case 2:
        return (*(u32 *)(arg1 + arg2 * 4) >> arg3) & mask;
    }
}
