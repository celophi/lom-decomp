#include "common.h"

extern void func_8007EAB8(void);
extern s32 D_801B277C;
extern s32 D_801B2778;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007EA80(void)
{
    D_801B277C = 0x40;
    D_801B2778 += 1;
    func_8007EAB8();
}
