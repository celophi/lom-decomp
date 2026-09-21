#include "common.h"

extern s32 D_801B2900;

/**
 * @brief Increment a world-map state counter.
 */
void func_80086F30(void)
{
    D_801B2900 += 1;
}
