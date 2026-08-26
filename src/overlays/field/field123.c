#include "common.h"

/**
 * @param arg0 Byte stream to read a little-endian 16-bit value from.
 * @param arg1 Destination for the unpacked 16-bit value.
 * @return arg0 advanced past the two bytes read.
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
u8 *func_800BD2FC(u8 *arg0, s16 *arg1)
{
    *arg1 = arg0[0] + (arg0[1] << 8);
    return arg0 + 2;
}
