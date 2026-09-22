#include "common.h"

extern s32 D_801B24C0;
extern s32 D_8013B20C;

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_8007198C(void)
{
    D_8013B20C = 0;
    D_801B24C0 += 1;
}
