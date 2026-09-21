#include "common.h"

extern s32 D_801B2780;

/**
 * @brief Increment a world-map state counter.
 */
void func_8007F528(void)
{
    D_801B2780 += 1;
}
