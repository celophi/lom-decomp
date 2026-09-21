#include "common.h"

extern void func_80096EB8(void);
extern s32 D_801B2BC4;
extern s32 D_801B2BC0;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80096E80(void)
{
    D_801B2BC4 = 0x18;
    D_801B2BC0 += 1;
    func_80096EB8();
}
