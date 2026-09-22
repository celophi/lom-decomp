#include "common.h"

extern void func_8007C180(void);
extern s32 D_801B26F4;
extern s32 D_801B26F0;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007C148(void)
{
    D_801B26F4 = 0x8;
    D_801B26F0 += 1;
    func_8007C180();
}
