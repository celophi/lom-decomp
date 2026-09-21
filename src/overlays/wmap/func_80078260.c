#include "common.h"

extern s32 D_801B2618;

/**
 * @brief Increment a world-map state counter.
 */
void func_80078260(void)
{
    D_801B2618 += 1;
}
