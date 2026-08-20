#include "common.h"

/**
 * @brief Render @p value right-aligned into @p digits columns at the entry's
 *        0x54 field, blanking leading zeros with spaces.
 * @param arg0   Array index (low 16 bits used); selects the 0x98-stride entry.
 * @param value  Number to format.
 * @param digits Column count; per-digit values >= 10 are clamped to '9'.
 * @note Byte-matching only under the field3.c toolchain: gcc272_cdk without
 *       maspsx --expand-div. The two variable divides (`value / div`,
 *       `value % div`) must stay bare `divu` with no div-by-zero break check;
 *       --expand-div would inject six `bnez/nop/break 7` insns absent from the
 *       target. The `--digits` pre-decrement loop and the `blank` local are
 *       required to match - do not reshape them. See
 *       working/func_800675C8/STATUS.md.
 * @see decomp.me (100%) TODO
 */
void func_800675C8(s32 arg0, u32 value, u8 digits)
{
    u8* dst;
    u32 div;
    u32 digit;
    s32 leading;
    u32 blank;

    leading = 1;
    dst = (u8*)((u32)(arg0 & 0xFFFF) * 0x98 + 0x801ED054);
    div = 1;
    while (--digits != 0)
    {
        div = div * 10;
    }
    blank = 0x20;
    if (div != 1)
    {
        do
        {
            digit = value / div;
            if ((digit == 0) && (leading != 0))
            {
                *dst = blank;
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
