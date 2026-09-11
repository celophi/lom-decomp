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
