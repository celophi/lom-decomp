#include "common.h"

extern s32 D_801B2E68;
extern void func_800A89DC(s32 arg0);
extern void func_800591A8(s32 arg0);

/** @brief World-map step handler: kick two sub-tasks and expire the step counter. */
void func_800A89A8(void)
{
    func_800A89DC(0x16);
    func_800591A8(0x16);
    D_801B2E68 += 1;
}
