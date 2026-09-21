#include "common.h"

extern s32 D_801B25D0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80076C54(void)
{
    D_801B25D0 += 1;
}
