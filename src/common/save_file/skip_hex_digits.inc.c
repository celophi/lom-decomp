/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Advance past a run of hex digits ('0'-'9', 'a'-'f', 'A'-'F').
 * @param text Start of the text to scan.
 * @return Pointer to the first byte that is not a hex digit.
 */
u8* skip_hex_digits(u8* text)
{
    while ((*text >= '0' && *text <= '9') || (*text >= 'a' && *text <= 'f') || (*text >= 'A' && *text <= 'F'))
    {
        text++;
    }
    return text;
}
