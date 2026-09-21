#include "common.h"

extern s32 D_801B2940;

/**
 * @brief Increment a world-map state counter.
 */
void func_80088530(void)
{
    D_801B2940 += 1;
}
