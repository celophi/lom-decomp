#include "common.h"

extern s32 D_801B28B0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80085C00(void)
{
    D_801B28B0 += 1;
}
