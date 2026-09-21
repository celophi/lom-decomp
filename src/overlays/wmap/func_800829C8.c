#include "common.h"

extern s32 D_801B2818;
extern s32 D_801B281C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800829C8(void)
{
    D_801B2818 = 1;
    D_801B281C = 1;
}
