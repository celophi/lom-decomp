#include "common.h"

extern void func_8009CFC8(void);
extern s32* D_80139280;
extern s32 D_801B2CB4;
extern s32 D_801B2CB0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009CF80(void)
{
    D_801B2CB4 = 0x80;
    D_80139280[15] = -1;
    D_801B2CB0 += 1;
    func_8009CFC8();
}
