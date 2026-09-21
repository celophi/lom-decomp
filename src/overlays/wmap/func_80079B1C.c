#include "common.h"

extern s32 D_801B2680;

/**
 * @brief Increment a world-map state counter.
 */
void func_80079B1C(void)
{
    D_801B2680 += 1;
}
