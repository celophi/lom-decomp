/* Partial WMAP decompilation: 99.062500% (gcc280_g0). */
#include "common.h"

/**
 * @brief Copy whole words from source to destination.
 * @param source Source words.
 * @param destination Destination words.
 * @param byte_count Nonnegative byte count; trailing partial words are ignored.
 */
void func_80064F1C(s32 *source, s32 *destination, s32 byte_count)
{
    byte_count /= 4;
    while (--byte_count != -1)
    {
        *destination++ = *source++;
    }
}
