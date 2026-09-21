#include "common.h"

extern s32 D_801B28C8;
extern s32 D_801B28CC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80086228(void)
{
    D_801B28C8 = 1;
    D_801B28CC = 1;
}
