#include "common.h"

extern void func_800A309C(void);
extern s32* D_80139280;
extern s32 D_801B2DC4;
extern s32 D_801B2DC0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A3054(void)
{
    D_801B2DC4 = 0x20;
    D_80139280[25] = -1;
    D_801B2DC0 += 1;
    func_800A309C();
}
