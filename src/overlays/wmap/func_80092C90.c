#include "common.h"

extern s32 D_801B2B08;

/**
 * @brief Increment a world-map state counter.
 */
void func_80092C90(void)
{
    D_801B2B08 += 1;
}
