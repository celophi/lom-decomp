#include "common.h"

extern void func_8009E114(void);
extern s32 D_801B2D4C;
extern s32 D_801B2D48;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A07CC(void)
{
    D_801B2D4C = 0x40;
    D_801B2D48 += 1;
    func_8009E114();
}
