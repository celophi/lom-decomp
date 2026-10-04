/*
 * Shared Shift-JIS number formatting; see include/common/sjis.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/sjis.h"

/**
 * @brief Format @p value as full-width Shift-JIS digits without leading zeros and null-terminate it.
 * @param out Destination buffer.
 * @param value Value to format; 1000000 and up is written as g_decimal_overflow_text.
 * @return Pointer to the terminator.
 */
s8* format_decimal(s8* out, s32 value)
{
    s32 digit;
    s32 divisor;
    s32 started;
    s8* p;

    p = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(SjisDecimalOverflowText*)p = g_decimal_overflow_text;
        return p + sizeof(SjisDecimalOverflowText) - 1;
    }

    started = 0;
    for (;; divisor /= 10)
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *p++ = (digit + SJIS_DIGIT_ZERO) >> 8;
            *p++ = digit + (SJIS_DIGIT_ZERO & 0xFF);
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
    }
    *p = 0;
    return p;
}
