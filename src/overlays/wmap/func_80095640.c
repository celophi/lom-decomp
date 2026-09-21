#include "common.h"

extern void func_80095688(void);
extern s32* D_80139280;
extern s32 D_801B2B7C;
extern s32 D_801B2B78;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80095640(void)
{
    D_801B2B7C = 0x40;
    D_80139280[15] = -1;
    D_801B2B78 += 1;
    func_80095688();
}
