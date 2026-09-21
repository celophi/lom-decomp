#include "common.h"

extern s32 D_801B2A40;

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E570(void)
{
    D_801B2A40 += 1;
}
