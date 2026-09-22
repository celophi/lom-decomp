#include "common.h"

extern void func_8007C0E0(void);
extern s32 D_801B26F4;
extern s32 D_801B26F0;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007C0A8(void)
{
    D_801B26F4 = 0x28;
    D_801B26F0 += 1;
    func_8007C0E0();
}
