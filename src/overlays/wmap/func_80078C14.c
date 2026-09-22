#include "common.h"

extern s32 D_801B2640;

/**
 * @brief Increment a world-map state counter.
 */
void func_80078C14(void)
{
    D_801B2640 += 1;
}
