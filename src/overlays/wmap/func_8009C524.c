#include "common.h"

extern s32 D_801B2C80;

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C524(void)
{
    D_801B2C80 += 1;
}
