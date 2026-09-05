#include "common.h"

/**
 * @param arg0 Byte stream to read a little-endian 16-bit value from.
 * @param arg1 Destination for the unpacked 16-bit value.
 * @return arg0 advanced past the two bytes read.
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
u8* field_script_read_u16(u8* data, u16* value)
{
    *value = data[0] + (data[1] << 8);
    return data + 2;
}
