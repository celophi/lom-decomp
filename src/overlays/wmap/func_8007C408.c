#include "common.h"

extern s32 D_801B2700;
extern s32 D_801B2704;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007C408(void)
{
    D_801B2700 = 1;
    D_801B2704 = 1;
}
