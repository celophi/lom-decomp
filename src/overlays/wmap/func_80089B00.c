#include "common.h"

extern s32 D_801B2968;

/**
 * @brief Increment a world-map state counter.
 */
void func_80089B00(void)
{
    D_801B2968 += 1;
}
