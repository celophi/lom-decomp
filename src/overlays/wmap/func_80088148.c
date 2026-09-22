#include "common.h"

extern s32 D_801B2930;

/**
 * @brief Increment a world-map state counter.
 */
void func_80088148(void)
{
    D_801B2930 += 1;
}
