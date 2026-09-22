#include "common.h"

extern s32 D_801B28A8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80085424(void)
{
    D_801B28A8 += 1;
}
