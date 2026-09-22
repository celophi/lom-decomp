#include "wmap_resource_support.h"
#include "common.h"

extern s16 D_800D930E;
extern s32 D_801B2E58;
extern s32 D_801B2E5C;

/** @brief Play sound 57, clear its field, and start a 30-tick delay. */
void func_800A8688(void)
{
    func_800652A8(0x39, 0x80);
    D_800D930E = 0;
    D_801B2E5C = 0x1E;
    D_801B2E58 += 1;
}
