#include "common.h"

extern u32 D_800DC7A8[];
extern u8 *D_801058D4;
extern u32 *D_801058D8;

/**
 * @brief Reset the field resource cursor pair to the base blob and its end.
 * @note D_801058D8 points at the blob base; D_801058D4 points one past the
 *       payload, using the length word stored at D_800DC7A8[1].
 */
void func_80083948(void)
{
    D_801058D8 = D_800DC7A8;
    D_801058D4 = (u8 *)D_800DC7A8 + D_800DC7A8[1];
}
