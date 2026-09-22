#include "common.h"

extern s32 D_801B2D00;

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F408(void)
{
    D_801B2D00 += 1;
}
