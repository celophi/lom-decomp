#include "common.h"

extern void func_800A51E8(void);
extern s32* D_80139280;
extern s32 D_801B2E1C;
extern s32 D_801B2E18;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A51A0(void)
{
    D_801B2E1C = 0x20;
    D_80139280[5] = -1;
    D_801B2E18 += 1;
    func_800A51E8();
}
