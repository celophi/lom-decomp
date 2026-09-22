#include "common.h"

extern s32 D_801B2630;

/**
 * @brief Increment a world-map state counter.
 */
void func_80078A78(void)
{
    D_801B2630 += 1;
}
