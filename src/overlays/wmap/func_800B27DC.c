#include "common.h"

extern s32 D_801B2F98;
extern s32 D_8013B20C;

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_800B27DC(void)
{
    D_8013B20C = 0;
    D_801B2F98 += 1;
}
