#include "common.h"

extern s32 D_801B2718;
extern s32 D_801B271C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007C910(void)
{
    D_801B2718 = 1;
    D_801B271C = 1;
}
