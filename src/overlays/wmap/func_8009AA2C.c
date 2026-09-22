#include "common.h"

extern u16 D_801AFBD0;
extern s32 D_801B2C4C;
extern s32 D_801B2C50;
extern void func_8009AA6C(void);

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_8009AA2C(void)
{
    D_801AFBD0 = 0;
    D_801B2C50 = 0x78;
    D_801B2C4C += 1;
    func_8009AA6C();
}
