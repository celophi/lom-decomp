/*
 * Shared encoded-text helper; see include/encoded_text.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "encoded_text.h"

/**
 * @brief Measure an encoded string, counting a two-byte code as two bytes.
 * @param text Null-terminated encoded string.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%, GNAME copy) https://decomp.me/scratch/2QgjW
 */
s32 encoded_text_byte_length(u8* text)
{
    u8* cursor;
    u8 code;
    s32 byte_length;

    cursor = text;
    code = *cursor;
    byte_length = 0;
    while (code != 0)
    {
        if (ENCODED_TEXT_IS_DOUBLE_BYTE_LEAD(code))
        {
            cursor += 2;
            byte_length += 2;
        }
        else
        {
            cursor++;
            byte_length++;
        }
        code = *cursor;
    }
    return byte_length;
}
