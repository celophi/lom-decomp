#include "common.h"

extern s32 D_801B2410;

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F054(void)
{
    D_801B2410 += 1;
}
