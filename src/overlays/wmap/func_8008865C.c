#include "common.h"

extern void func_800886A4(void);
extern s32* D_80139280;
extern s32 D_801B294C;
extern s32 D_801B2948;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008865C(void)
{
    D_801B294C = 0x20;
    D_80139280[25] = -1;
    D_801B2948 += 1;
    func_800886A4();
}
