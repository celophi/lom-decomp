#include "common.h"

extern s32 D_801B28C0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80086198(void)
{
    D_801B28C0 += 1;
}
