#include "common.h"

extern s32 D_801B24E8;
extern s32 D_801B24EC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800720AC(void)
{
    D_801B24E8 = 1;
    D_801B24EC = 1;
}
