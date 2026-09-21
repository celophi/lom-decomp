#include "common.h"

extern s32 D_801B2C28;

/**
 * @brief Increment a world-map state counter.
 */
void func_80099108(void)
{
    D_801B2C28 += 1;
}
