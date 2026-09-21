#include "common.h"

extern u8 D_800D2B54[];
extern u8 D_800D3BD4[];
extern void *D_801B23F8;

/**
 * @brief Select one of the two world-map data buffers.
 * @param use_second Nonzero selects the second buffer.
 */
void func_8006D8F0(s32 use_second)
{
    if (use_second != 0)
    {
        D_801B23F8 = D_800D3BD4;
        return;
    }
    D_801B23F8 = D_800D2B54;
}
