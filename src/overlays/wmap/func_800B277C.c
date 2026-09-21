#include "common.h"

extern s32 D_8011D500;
extern s32 D_801B2F98;
extern s32 D_801B2F9C;

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_800B277C(void)
{
    D_8011D500 = 0xFF;
    D_801B2F9C = 0x40;
    D_801B2F98 += 1;
}
