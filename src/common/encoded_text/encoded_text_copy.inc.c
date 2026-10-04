/*
 * Shared encoded-text function; see include/common/encoded_text.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/encoded_text.h"

/**
 * @brief Copy an encoded string and null-terminate the copy.
 * @param dst Destination buffer with room for the string and its terminator.
 * @param src Null-terminated encoded string.
 * @note Measures @p src with its own loop instead of calling encoded_text_byte_length.
 * @see decomp.me (100%, GNAME copy) https://decomp.me/scratch/UeYRe
 */
void encoded_text_copy(u8* dst, u8* src)
{
    u8* cursor;
    s32 byte_length;
    s32 i;

    cursor = src;
    byte_length = 0;
    while (*cursor != 0)
    {
        if (ENCODED_TEXT_IS_DOUBLE_BYTE_LEAD(*cursor))
        {
            cursor += 2;
            byte_length += 2;
        }
        else
        {
            cursor++;
            byte_length++;
        }
    }
    for (i = 0; i < byte_length; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
}
