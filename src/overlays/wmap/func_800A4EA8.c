#include "common.h"

extern void func_800A4EE0(void);
extern s32 D_801B2E14;
extern s32 D_801B2E10;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4EA8(void)
{
    D_801B2E14 = 0x3C;
    D_801B2E10 += 1;
    func_800A4EE0();
}
