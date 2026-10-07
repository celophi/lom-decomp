/*
 * Shared Shift-JIS number formatting; see include/common/sjis.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/sjis.h"

/**
 * @brief Format @p value as full-width Shift-JIS digits without leading zeros and null-terminate it.
 * @param destination Buffer with room for up to six two-byte digits and a null byte.
 * @param value Nonnegative value to format; 1000000 and up becomes full-width "MAX".
 * @return Pointer to the terminator.
 */
s8* format_decimal(s8* destination, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 emit_zero_digits;
    s8* cursor;

    cursor = destination;
    divisor = SJIS_DECIMAL_FIRST_DIVISOR;
    if (value >= divisor * SJIS_DECIMAL_RADIX)
    {
        *(SjisDecimalOverflowText*)cursor = g_decimal_overflow_text;
        return cursor + sizeof(SjisDecimalOverflowText) - 1;
    }

    emit_zero_digits = 0;
    for (;; divisor /= SJIS_DECIMAL_RADIX)
    {
        digit = value / divisor;
        if (digit != 0 || emit_zero_digits != 0)
        {
            *cursor++ = (digit + SJIS_DIGIT_ZERO) >> SJIS_CODE_LEAD_SHIFT;
            *cursor++ = digit + (SJIS_DIGIT_ZERO & SJIS_CODE_TRAIL_MASK);
            emit_zero_digits = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        /* Always emit the units digit, even when the value is zero. */
        if (divisor == SJIS_DECIMAL_RADIX)
        {
            emit_zero_digits = 1;
        }
        value -= digit * divisor;
    }
    *cursor = 0;
    return cursor;
}
