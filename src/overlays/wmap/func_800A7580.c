#include "common.h"

extern u16 D_801AFBD0;
extern s32 D_801B2E70;
extern s32 D_801B2E74;
extern void func_800A75C0(void);

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_800A7580(void)
{
    D_801AFBD0 = 0;
    D_801B2E74 = 0x78;
    D_801B2E70 += 1;
    func_800A75C0();
}
