#include "common.h"

extern void func_80086844(void);
extern s32* D_80139280;
extern s32 D_801B28E4;
extern s32 D_801B28E0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800867FC(void)
{
    D_801B28E4 = 0x20;
    D_80139280[15] = -1;
    D_801B28E0 += 1;
    func_80086844();
}
