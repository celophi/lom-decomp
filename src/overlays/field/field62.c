#include "common.h"

/**
 * @brief Copy a run of bytes from one buffer to another.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param count Number of bytes to copy.
 */
void func_800A4320(u8 *dst, u8 *src, s32 count)
{
    u8 temp;

    if (count > 0)
    {
        do
        {
            temp = *src;
            src += 1;
            count -= 1;
            *dst = temp;
            dst += 1;
        } while (count != 0);
    }
}
