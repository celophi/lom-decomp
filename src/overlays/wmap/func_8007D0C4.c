#include "common.h"

extern void func_8007D0FC(void);
extern s32 D_801B2734;
extern s32 D_801B2730;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007D0C4(void)
{
    D_801B2734 = 0x40;
    D_801B2730 += 1;
    func_8007D0FC();
}
