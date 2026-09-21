#include "common.h"

extern s32 D_801B24C8;
extern s32 D_801B24CC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80071A20(void)
{
    D_801B24C8 = 1;
    D_801B24CC = 1;
}
