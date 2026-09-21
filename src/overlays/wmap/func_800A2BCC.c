#include "common.h"

extern s32 D_801B2DA8;
extern void func_800A10B4(void);
extern s32 D_801B2DAC;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2B94(void)
{
    D_801B2DAC = 0x10;
    D_801B2DA8 += 1;
    func_800A10B4();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2BCC(void)
{
    D_801B2DA8 += 1;
}
