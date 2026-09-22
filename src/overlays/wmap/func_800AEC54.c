#include "common.h"

extern s32 D_801B2EF0;
extern s32 D_800D923C;

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_800AEC54(void)
{
    D_800D923C = 0;
    D_801B2EF0 += 1;
}
