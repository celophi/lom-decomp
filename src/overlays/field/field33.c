#include "common.h"

extern s32 D_8010A018;

/**
 * @brief Look up a value from a self-relative offset table anchored at
 *        D_8010A018.
 * @param arg0 Index into the u16 offset table.
 * @return D_8010A018 plus the u16 offset at index arg0.
 */
s32 func_80087EF0(s32 arg0)
{
    return D_8010A018 + *(u16*)((arg0 * 2) + D_8010A018);
}
