#include "common.h"

extern s32 D_801B2818;

/**
 * @brief Increment a world-map state counter.
 */
void func_80082AEC(void)
{
    D_801B2818 += 1;
}
