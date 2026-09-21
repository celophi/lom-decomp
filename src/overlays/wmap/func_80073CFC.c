#include "common.h"

extern void func_80073D34(void);
extern s32 D_801B2544;
extern s32 D_801B2540;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80073CFC(void)
{
    D_801B2544 = 0x20;
    D_801B2540 += 1;
    func_80073D34();
}
