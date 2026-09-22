#include "common.h"

extern void func_800864AC(void);
extern s32* D_80139280;
extern s32 D_801B28D4;
extern s32 D_801B28D0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80086464(void)
{
    D_801B28D4 = 0x40;
    D_80139280[5] = -1;
    D_801B28D0 += 1;
    func_800864AC();
}
