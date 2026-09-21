#include "common.h"

extern u16 D_801AFBD0;
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_8009AE08(void);

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_8009ADC8(void)
{
    D_801AFBD0 = 0;
    D_801B2C58 = 0x78;
    D_801B2C54 += 1;
    func_8009AE08();
}
