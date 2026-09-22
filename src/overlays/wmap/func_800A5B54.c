#include "common.h"

extern void func_800A5B94(void);
extern s32 D_800DCEB4;
extern s32 D_801B2E34;
extern s32 D_801B2E30;

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A5B54(void)
{
    D_800DCEB4 = 0;
    D_801B2E34 = 0xA0;
    D_801B2E30 += 1;
    func_800A5B94();
}
