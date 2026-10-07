/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Write a null-terminated uppercase hex string without leading zeros.
 * @param destination Buffer for the output digits and a null terminator.
 * @param value Value whose 32-bit representation is formatted.
 * @param max_digits Nonnegative digit limit, excluding the null terminator.
 * @note A zero limit produces an empty string; a short limit keeps the most
 *       significant digits. Zero is written as "0" when the limit is positive.
 */
void format_hex(s8* destination, s32 value, s32 max_digits)
{
    s32 nibble;
    s32 nibble_index;
    s32 emit_zero_digits;

    nibble_index = SAVE_HEX_MAX_DIGITS - 1;
    emit_zero_digits = 0;
    while (max_digits != 0)
    {
        nibble = (value >> (nibble_index * SAVE_HEX_DIGIT_BITS)) & SAVE_HEX_DIGIT_MASK;
        if (nibble != 0 || emit_zero_digits != 0)
        {
            hex_nibble_to_ascii(destination, nibble);
            destination++;
            max_digits--;
            emit_zero_digits = 1;
            value -= nibble << (nibble_index * SAVE_HEX_DIGIT_BITS);
        }
        nibble_index--;
        if (nibble_index == -1)
        {
            break;
        }
        if (nibble_index == 0)
        {
            /* Always emit the final digit so zero formats as "0". */
            emit_zero_digits = 1;
        }
    }
    *destination = 0;
}
