#include "common.h"

extern s32 D_801B2B58;

/**
 * @brief Increment a world-map state counter.
 */
void func_80094E68(void)
{
    D_801B2B58 += 1;
}
