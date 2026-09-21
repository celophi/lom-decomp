#include "common.h"

extern void func_800A2E9C(void);
extern s32* D_80139280;
extern s32 D_801B2DBC;
extern s32 D_801B2DB8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A2E54(void)
{
    D_801B2DBC = 0x40;
    D_80139280[15] = -1;
    D_801B2DB8 += 1;
    func_800A2E9C();
}
