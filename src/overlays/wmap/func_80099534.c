#include "common.h"

extern s32 D_801B2C38;

/**
 * @brief Increment a world-map state counter.
 */
void func_80099534(void)
{
    D_801B2C38 += 1;
}
