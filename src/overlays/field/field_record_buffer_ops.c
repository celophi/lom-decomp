/** @file field_record_buffer_ops.c
 * @brief Copy or clear record buffers and sort packed value/key lists.
 */

#include "common.h"

/**
 * @brief Copy or clear a buffer in 32-bit words.
 * @param src Source buffer, or NULL to clear the destination.
 * @param dest Destination buffer.
 * @param n Byte count; only complete 32-bit words are processed.
 * @return Pointer to the first destination word after the processed range.
 */
s32* func_800C1EC8(s32* src, s32* dest, s32 n)
{
    s32* result;

    if (n < 0)
    {
        n += 3;
    }
    n >>= 2;

    if (src != NULL)
    {
        result = dest;
        if (n)
        {
            do
            {
                *dest++ = *src++;
                n--;
            } while (n);
            return dest;
        }
        return result;
    }

    result = dest;
    if (n)
    {
        do
        {
            *dest++ = 0;
            n--;
        } while (n);
        result = dest;
    }
    return result;
}


/**
 * @brief Sort a (value, key) pair list in place by ascending key.
 * @param arg0 Word 0 holds the pair count; pairs follow as (value, key) from word 1.
 */
void func_800C1F28(u32 *arg0)
{
    u32 i;
    u32 j;
    u32 key;
    u32 data;

    for (i = 0; i < arg0[0]; i++)
    {
        for (j = 1; j < arg0[0]; j++)
        {
            key = arg0[2 * i + 2];
            if (arg0[2 * j + 2] < key)
            {
                data = arg0[2 * i + 1];
                arg0[2 * i + 1] = arg0[2 * j + 1];
                arg0[2 * i + 2] = arg0[2 * j + 2];
                arg0[2 * j + 1] = data;
                arg0[2 * j + 2] = key;
            }
        }
    }
}
