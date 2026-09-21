#include "common.h"

extern s32 D_801B28A0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80085258(void)
{
    D_801B28A0 += 1;
}
