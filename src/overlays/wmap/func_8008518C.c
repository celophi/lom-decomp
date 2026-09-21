#include "common.h"

extern void func_800851D4(void);
extern s32* D_80139280;
extern s32 D_801B28A4;
extern s32 D_801B28A0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008518C(void)
{
    D_801B28A4 = 0x20;
    D_80139280[15] = -1;
    D_801B28A0 += 1;
    func_800851D4();
}
