#include "common.h"

extern s32 D_801B2890;

/**
 * @brief Increment a world-map state counter.
 */
void func_80084E70(void)
{
    D_801B2890 += 1;
}
