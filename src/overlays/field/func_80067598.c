#include "common.h"

typedef struct {
    u8 _pad[0x4B];
    u8 unk4B;
} ArrEntry2;

/**
 * @brief Read byte at offset 0x4B of the array entry at index arg0 in the
 *        array at 0x801ED000 (element stride 0x98 bytes).
 * @param arg0 Array index (low 16 bits used).
 * @return The u8 value at entry->unk4B.
 * @see decomp.me (100%) TODO
 */
u8 func_80067598(s32 arg0) {
    ArrEntry2 *entry = (ArrEntry2 *)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED000);
    return entry->unk4B;
}

/**
 * @brief Render value right-aligned into `digits` columns at the entry's
 *        0x54 field, blanking leading zeros with spaces.
 * @param arg0   Array index (low 16 bits used).
 * @param value  Number to format.
 * @param digits Column count; digits >= 10 are clamped to '9'.
 * @note WIP - not yet byte-matching. The power-of-ten loop loses its guard
 *       to loop rotation. See working/func_800675C8/STATUS.md.
 * @see decomp.me (90.07%) TODO
 */
void func_800675C8(s32 arg0, u32 value, u8 digits)
{
    u8* dst = (u8*)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED054);
    u32 div;
    u32 digit;
    s32 leading;

    leading = 1;
    div = 1;
    digits = digits - 1;
    if (digits != 0)
    {
        do
        {
            div = div * 10;
            digits -= 1;
        } while (digits != 0);
    }
    if (div != 1)
    {
        do
        {
            digit = value / div;
            if ((digit == 0) && (leading != 0))
            {
                *dst = 0x20;
                dst += 1;
            }
            else
            {
                if (digit >= 10)
                {
                    digit = 9;
                }
                *dst = digit + 0x30;
                dst += 1;
                leading = 0;
            }
            value = value % div;
            div = div / 10;
        } while (div != 1);
    }
    *dst = value + 0x30;
    dst[1] = 0;
}
