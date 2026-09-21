#include "common.h"

extern s32 D_801B24B8;
extern s32 D_801B24BC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80071338(void)
{
    D_801B24B8 = 1;
    D_801B24BC = 1;
}
