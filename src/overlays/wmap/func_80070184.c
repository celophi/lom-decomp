#include "common.h"

extern s32 D_801B2448;
extern s32 D_801B244C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80070184(void)
{
    D_801B2448 = 1;
    D_801B244C = 1;
}
