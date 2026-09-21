#include "common.h"

extern s32 D_801B25B0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80076440(void)
{
    D_801B25B0 += 1;
}
