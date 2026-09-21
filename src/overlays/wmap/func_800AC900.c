#include "common.h"

extern s32 D_801B2E80;
extern s32 D_8013B20C;

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_800AC900(void)
{
    D_8013B20C = 0;
    D_801B2E80 += 1;
}
