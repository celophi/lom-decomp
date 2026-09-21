#include "common.h"

extern s32 D_801B2580;

/**
 * @brief Increment a world-map state counter.
 */
void func_80074F9C(void)
{
    D_801B2580 += 1;
}
