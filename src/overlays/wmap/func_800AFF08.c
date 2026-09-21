#include "common.h"

extern s32 D_801B2F28;

/**
 * @brief Increment a world-map state counter.
 */
void func_800AFF08(void)
{
    D_801B2F28 += 1;
}
