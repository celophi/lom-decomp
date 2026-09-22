#include "common.h"

extern s32 D_801B2A50;

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E858(void)
{
    D_801B2A50 += 1;
}
