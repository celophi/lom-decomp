#include "common.h"

extern void func_80078754(void);
extern s32 D_801B263C;
extern s32 D_801B2638;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007871C(void)
{
    D_801B263C = 0x40;
    D_801B2638 += 1;
    func_80078754();
}
