#include "common.h"

extern void func_800A5758(void);
extern s32 D_801B2E2C;
extern s32 D_801B2E28;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A5720(void)
{
    D_801B2E2C = 0x30;
    D_801B2E28 += 1;
    func_800A5758();
}
