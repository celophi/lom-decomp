#include "common.h"

/**
 * @brief Copy a fixed 0x40-byte record from src to dst.
 * @param dst Destination buffer.
 * @param src Source buffer.
 */
void func_800A8F8C(u8 *dst, u8 *src)
{
    u32 i;

    i = 0;
    do
    {
        i++;
        *dst++ = *src++;
    } while (i < 0x40);
}
