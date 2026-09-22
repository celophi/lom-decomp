#include "common.h"

extern void func_80095000(void);
extern s32 D_801B2B64;
extern s32 D_801B2B60;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80094FC8(void)
{
    D_801B2B64 = 0x20;
    D_801B2B60 += 1;
    func_80095000();
}
