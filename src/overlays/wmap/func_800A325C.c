#include "common.h"

extern void func_800A32A4(void);
extern s32* D_80139280;
extern s32 D_801B2DCC;
extern s32 D_801B2DC8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A325C(void)
{
    D_801B2DCC = 0x20;
    D_80139280[35] = -1;
    D_801B2DC8 += 1;
    func_800A32A4();
}
