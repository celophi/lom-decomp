/*
 * Shared save-file function; see include/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "save_file.h"

/**
 * @brief Format @p value as uppercase hex without leading zeros and null-terminate it.
 * @param out Destination buffer.
 * @param value Value to format.
 * @param max_chars Most digits to write.
 */
void format_hex(s8* out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 started;

    shift_index = 7;
    started = 0;
    while (max_chars != 0)
    {
        nibble = (value >> (shift_index * 4)) & 0xF;
        if (nibble != 0 || started != 0)
        {
            hex_nibble_to_ascii(out, nibble);
            out++;
            max_chars--;
            started = 1;
            value -= nibble << (shift_index * 4);
        }
        shift_index--;
        if (shift_index == -1)
        {
            break;
        }
        if (shift_index == 0)
        {
            started = 1;
        }
    }
    *out = 0;
}
