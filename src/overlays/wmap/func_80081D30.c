#include "common.h"

extern s32 D_801B27E8;

/**
 * @brief Increment a world-map state counter.
 */
void func_80081D30(void)
{
    D_801B27E8 += 1;
}
