#include "common.h"

/** @brief Header containing the world-map value read at offset six. */
typedef struct
{
    u8 unknown_0[6];
    u16 value;
} WmapValueHeader;

extern WmapValueHeader *D_800D0454;

/**
 * @brief Combine the header value shifted right and left by one byte.
 * @return The combined value, including the upper bits of the left shift.
 */
s32 func_80065428(void)
{
    s32 value = D_800D0454->value;
    return ((s32)value >> 8) | (value << 8);
}
