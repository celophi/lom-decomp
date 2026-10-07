/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Read up to two hex digits after a serial and its separator.
 * @param field_text Hex serial, one non-hex separator byte, then a null-terminated suffix.
 * @return Parsed suffix value from 0 to 255, or zero if no suffix digit is present.
 * @note Accepts either letter case. The separator must be present and is
 *       skipped without validation.
 */
s32 parse_hex_suffix_byte(const char* field_text)
{
    s32 digits_left;
    u32 suffix_value;
    const u8* cursor;

    /* Decode filename characters as unsigned bytes. */
    cursor = (const u8*)field_text;
    while ((*cursor >= '0' && *cursor <= '9') ||
           (*cursor >= 'a' && *cursor <= 'f') ||
           (*cursor >= 'A' && *cursor <= 'F'))
    {
        cursor++;
    }
    /* The first non-hex byte separates the serial from the suffix. */
    cursor++;
    digits_left = SAVE_HEX_SUFFIX_DIGITS;
    suffix_value = 0;
    while ((*cursor >= '0' && *cursor <= '9') ||
           (*cursor >= 'a' && *cursor <= 'f') ||
           (*cursor >= 'A' && *cursor <= 'F'))
    {
        if (digits_left == 0)
        {
            break;
        }
        suffix_value <<= SAVE_HEX_DIGIT_BITS;
        if (*cursor >= '0' && *cursor <= '9')
        {
            u32 decimal_base;

            decimal_base = suffix_value - '0';
            suffix_value = decimal_base + *cursor;
        }
        else if (*cursor >= 'A' && *cursor <= 'F')
        {
            u32 uppercase_base;

            uppercase_base = suffix_value - ('A' - SAVE_HEX_DECIMAL_DIGITS);
            suffix_value = uppercase_base + *cursor;
        }
        else if (*cursor >= 'a' && *cursor <= 'f')
        {
            u32 lowercase_base;

            lowercase_base = suffix_value - ('a' - SAVE_HEX_DECIMAL_DIGITS);
            suffix_value = lowercase_base + *cursor;
        }
        cursor++;
        digits_left--;
    }
    return suffix_value;
}
