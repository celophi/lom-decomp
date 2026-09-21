#include "common.h"

extern void func_800B10FC(void);
extern s16 D_800D93F4[];
extern s32 D_801B2F7C;
extern s32 D_801B2F78;

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800B10B0(void)
{
    D_800D93F4[19] = 2;
    D_800D93F4[17] = 0;
    D_801B2F7C = 0x40;
    D_801B2F78 += 1;
    func_800B10FC();
}
