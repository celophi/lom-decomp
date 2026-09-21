#include "common.h"

extern s32 D_801B2C70;

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C228(void)
{
    D_801B2C70 += 1;
}
