#include "common.h"

extern void func_8007CC44(void);
extern s32 D_801B2724;
extern s32 D_801B2720;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007CC0C(void)
{
    D_801B2724 = 0x14;
    D_801B2720 += 1;
    func_8007CC44();
}
