#include "common.h"

extern void func_800B1318(void);
extern s32 D_801B2F84;
extern s32 D_801B2F80;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B12E0(void)
{
    D_801B2F84 = 0x5A;
    D_801B2F80 += 1;
    func_800B1318();
}
