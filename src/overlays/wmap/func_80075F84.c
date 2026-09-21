#include "common.h"

extern void func_80075FBC(void);
extern s32 D_801B259C;
extern s32 D_801B2598;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80075F84(void)
{
    D_801B259C = 0x24;
    D_801B2598 += 1;
    func_80075FBC();
}
