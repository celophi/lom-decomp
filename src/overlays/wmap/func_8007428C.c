#include "common.h"

extern void func_800742C4(void);
extern s32 D_801B255C;
extern s32 D_801B2558;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007428C(void)
{
    D_801B255C = 0x20;
    D_801B2558 += 1;
    func_800742C4();
}
