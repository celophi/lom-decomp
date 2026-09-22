#include "common.h"

extern s32 D_801B2810;

/**
 * @brief Increment a world-map state counter.
 */
void func_80082938(void)
{
    D_801B2810 += 1;
}
