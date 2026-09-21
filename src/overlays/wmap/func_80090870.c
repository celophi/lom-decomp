#include "common.h"

extern s32 D_801B2A80;

/**
 * @brief Increment a world-map state counter.
 */
void func_80090870(void)
{
    D_801B2A80 += 1;
}
