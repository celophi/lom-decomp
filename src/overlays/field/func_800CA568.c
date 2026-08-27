#include "common.h"

extern u8 g_menuLayoutBuffer[];

typedef struct MenuLayout
{
    u8 pad[0xCF6];
    u16 value;
} MenuLayout;

/**
 * @brief Combines the masked values for two menu-layout records.
 *
 * Each input index selects a 0x40-byte record. The function sums the low six
 * bits of each record's halfword at offset 0xCF6 and returns the sum modulo 11.
 *
 * @param indices Pointer to the two record indices.
 * @return The combined value in the range 0 through 10.
 */
s32 func_800CA568(s32 *indices)
{
    u8 *base;
    MenuLayout *first;
    MenuLayout *second;

    base = g_menuLayoutBuffer;
    first = (MenuLayout *)&base[indices[0] << 6];
    second = (MenuLayout *)&base[indices[1] << 6];
    return ((first->value & 0x3F) + (second->value & 0x3F)) % 11;
}
