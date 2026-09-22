#include "common.h"

extern u8 D_800CC0C4[];
extern s8 D_800D7CD0[];
extern u8 D_800D7CD8[];

/** @brief Enable eight slots and copy their initial table values. */
void func_800582E8(void)
{
    s32 index;
    for (index = 0; index < 8; index++)
    {
        D_800D7CD0[index] = 1;
        D_800D7CD8[index] = D_800CC0C4[index + 1];
    }
}
