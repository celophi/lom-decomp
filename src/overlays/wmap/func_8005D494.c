#include "common.h"

extern u16 D_800432BE;

/**
 * @brief Read the low seven bits of the saved world-map value.
 * @return Value masked to seven bits.
 */
s32 func_8005D494(void)
{
    return D_800432BE & 0x7F;
}
