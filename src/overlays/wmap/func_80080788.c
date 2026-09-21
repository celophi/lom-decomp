#include "common.h"

extern s32 D_801B27C8;
extern s32 D_801B27CC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80080788(void)
{
    D_801B27C8 = 1;
    D_801B27CC = 1;
}
