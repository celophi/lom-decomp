/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Advance past leading hex digits ('0'-'9', 'a'-'f', 'A'-'F').
 * @param cursor Start of the text to scan.
 * @return Pointer to the first non-hex byte, unchanged if no digits are present.
 */
u8* skip_hex_digits(u8* cursor)
{
    while ((*cursor >= '0' && *cursor <= '9') ||
           (*cursor >= 'a' && *cursor <= 'f') ||
           (*cursor >= 'A' && *cursor <= 'F'))
    {
        cursor++;
    }
    return cursor;
}
