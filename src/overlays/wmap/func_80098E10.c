#include "common.h"

extern s32 D_801B2C18;

/**
 * @brief Increment a world-map state counter.
 */
void func_80098E10(void)
{
    D_801B2C18 += 1;
}
