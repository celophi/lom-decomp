/*
 * Shared save-file function; see include/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "save_file.h"

/**
 * @brief Write one nibble as its hex digit ('0'-'9', 'A'-'F'), or '_' when it is out of range.
 * @param out Destination byte.
 * @param value Nibble to convert.
 */
void hex_nibble_to_ascii(s8* out, s32 value)
{
    if (value < 10)
    {
        *out = value + '0';
    }
    else if (value < 16)
    {
        *out = value + ('A' - 10);
    }
    else
    {
        *out = '_';
    }
}
