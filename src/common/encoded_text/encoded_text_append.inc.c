/*
 * Shared encoded-text function; see include/common/encoded_text.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/encoded_text.h"

/**
 * @brief Append an encoded string to another and null-terminate the result.
 * @param dst Null-terminated encoded string to extend; must have room for @p src.
 * @param src Null-terminated encoded string to append.
 */
void encoded_text_append(u8* dst, u8* src)
{
    s32 dst_length;
    s32 src_length;
    s32 i;

    dst_length = encoded_text_byte_length(dst);
    src_length = encoded_text_byte_length(src);
    for (i = 0; i < src_length; i++)
    {
        dst[dst_length + i] = src[i];
    }
    dst[dst_length + i] = 0;
}
