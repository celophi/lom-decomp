#include "common.h"

extern u8 D_800D8B18[];

/**
 * @brief Read a byte from the six-by-six world-map table.
 * @param column Column index.
 * @param row Row index.
 * @return Table entry, or 0xFF when an index is outside the table.
 */
u8 func_8005D670(u32 column, s32 row)
{
    if (column < 6U && row >= 0 && row < 6)
    {
        return D_800D8B18[column + row * 6];
    }
    return 0xFF;
}
