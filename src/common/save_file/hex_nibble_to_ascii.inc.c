/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Write one uppercase hex digit, or '_' for values of 16 or greater.
 * @param destination Byte to receive the digit; no null terminator is appended.
 * @param nibble Value to convert, normally in the range 0-15.
 * @note Negative values also take the numeric-digit branch.
 */
void hex_nibble_to_ascii(s8* destination, s32 nibble)
{
    if (nibble < SAVE_HEX_DECIMAL_DIGITS)
    {
        *destination = nibble + '0';
    }
    else if (nibble < SAVE_HEX_RADIX)
    {
        *destination = (nibble - SAVE_HEX_DECIMAL_DIGITS) + 'A';
    }
    else
    {
        *destination = SAVE_HEX_INVALID_DIGIT;
    }
}
