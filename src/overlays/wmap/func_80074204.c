#include "common.h"

extern s32 D_801B2558;
extern s32 D_801B255C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80074204(void)
{
    D_801B2558 = 1;
    D_801B255C = 1;
}
