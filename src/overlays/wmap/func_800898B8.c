#include "common.h"

extern s16 D_800D9326;
extern s32 D_801B2968;
extern s32 D_801B296C;
extern void func_800898FC(void);

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_800898B8(void)
{
    D_800D9326 = 1;
    D_801B296C = 0x20;
    D_801B2968 += 1;
    func_800898FC();
}
