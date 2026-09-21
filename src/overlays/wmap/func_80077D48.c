#include "common.h"

extern s32 D_801B25F8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80077D48(void)
{
    D_801B25F8 += 1;
}
