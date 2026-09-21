#include "common.h"

extern s32 D_801B25C0;

/**
 * @brief Increment a world-map state counter.
 */
void func_80076830(void)
{
    D_801B25C0 += 1;
}
