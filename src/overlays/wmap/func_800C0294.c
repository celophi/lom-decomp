#include "common.h"

extern s32 D_801B3220;
extern void func_800C02C0(void);

/** @brief World-map step handler: bump the step counter and run the next step. */
void func_800C0294(void)
{
    D_801B3220 += 1;
    func_800C02C0();
}
