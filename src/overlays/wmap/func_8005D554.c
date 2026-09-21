#include "common.h"

extern u32 D_80043454;
extern u8 D_800D8B18[];

/**
 * @brief Test whether a different grid location has a valid table entry.
 * @param x Grid column.
 * @param y Grid row.
 * @return Nonzero if the location differs from the current location and is valid.
 */
s32 func_8005D554(u32 x, s32 y)
{
    s32 value;
    if (((D_80043454 >> 8) & 15) == x && ((D_80043454 >> 12) & 15) == y)
   
   {
        return 0;
    }
    if (x < 6 && y >= 0 && y < 6)
   
   {
        value = D_800D8B18[x + y * 6];
    }
    else
   
   {
        value = 255;
    }
    return value != 255;
}
