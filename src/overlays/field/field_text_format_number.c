#include "common.h"

#define FIELD_TEXT_WINDOW_STRIDE 0x98
#define FIELD_TEXT_INLINE_TEXT_BASE 0x801ED054

/**
 * @brief Format an unsigned value into a field text window's inline text buffer.
 * @param window_index Text-window slot; only the low 16 bits are used.
 * @param value Value to format as decimal text.
 * @param digits Number of character columns to emit; leading zero columns become spaces.
 * @note @p digits must be at least 1.
 */
void field_text_format_number(s32 window_index, u32 value, u8 digits)
{
    u8* text;
    u32 place_value;
    u32 digit;
    s32 leading_zero;
    u32 space;

    leading_zero = 1;
    text = (u8*)((u32)(window_index & 0xFFFF) * FIELD_TEXT_WINDOW_STRIDE + FIELD_TEXT_INLINE_TEXT_BASE);
    place_value = 1;
    while (--digits != 0)
    {
        place_value = place_value * 10;
    }
    space = ' ';
    if (place_value != 1)
    {
        do
        {
            digit = value / place_value;
            if ((digit == 0) && (leading_zero != 0))
            {
                *text = space;
                text += 1;
            }
            else
            {
                if (digit >= 10)
                {
                    digit = 9;
                }
                *text = digit + '0';
                text += 1;
                leading_zero = 0;
            }
            value = value % place_value;
            place_value = place_value / 10;
        } while (place_value != 1);
    }
    *text = value + '0';
    text[1] = 0;
}
