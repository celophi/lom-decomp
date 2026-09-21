#include "common.h"

extern s32 D_801398EC;

/**
 * @brief Fill the leading value of each interior world-map grid cell.
 * @param value Value assigned to the 24-by-24 interior.
 */
void func_800667E8(s32 value)
{
    s32 row;
    s32 column;
    for (row = 1; row < 25; row++)
    {
        for (column = 1; column < 25; column++)
        {
            *(s32 *)(D_801398EC + (row * 26 + column) * 0x28 + 0x344) = value;
        }
    }
}
