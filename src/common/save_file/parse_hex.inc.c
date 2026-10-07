/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Parse leading hex digits, accepting uppercase and lowercase letters.
 * @param cursor Text to parse, starting at its first digit.
 * @param digits_left Nonnegative limit on the number of digits to read.
 * @return The parsed value; 0 for a zero limit or no leading hex digit.
 * @note Parsing stops at the first non-hex byte; overflow retains the low 32 bits.
 */
u32 parse_hex(const u8* cursor, s32 digits_left)
{
    u32 parsed_value;

    parsed_value = 0;
    while ((*cursor >= '0' && *cursor <= '9') ||
           (*cursor >= 'a' && *cursor <= 'f') ||
           (*cursor >= 'A' && *cursor <= 'F'))
    {
        if (digits_left == 0)
        {
            break;
        }
        parsed_value <<= SAVE_HEX_DIGIT_BITS;
        if (*cursor >= '0' && *cursor <= '9')
        {
            u32 decimal_base;

            decimal_base = parsed_value - '0';
            parsed_value = decimal_base + *cursor;
        }
        else if (*cursor >= 'A' && *cursor <= 'F')
        {
            u32 uppercase_base;

            uppercase_base = parsed_value - ('A' - SAVE_HEX_DECIMAL_DIGITS);
            parsed_value = uppercase_base + *cursor;
        }
        else if (*cursor >= 'a' && *cursor <= 'f')
        {
            u32 lowercase_base;

            lowercase_base = parsed_value - ('a' - SAVE_HEX_DECIMAL_DIGITS);
            parsed_value = lowercase_base + *cursor;
        }
        cursor++;
        digits_left--;
    }
    return parsed_value;
}
