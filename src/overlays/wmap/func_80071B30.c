#include "common.h"

extern s32 D_801B24C8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80071B30(void)
{
    D_801B24C8 += 1;
}
