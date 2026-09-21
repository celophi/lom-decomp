#include "common.h"

extern u16 D_800DBE5E;
extern s32 D_801B2550;
extern s32 D_801B2554;
extern void func_80074100(void);

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_800740C0(void)
{
    D_800DBE5E = 0;
    D_801B2554 = 0x10;
    D_801B2550 += 1;
    func_80074100();
}
