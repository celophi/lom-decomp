#include "common.h"

extern void func_80097390(void);
extern s32* D_80139280;
extern s32 D_801B2BD4;
extern s32 D_801B2BD0;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80097348(void)
{
    D_801B2BD4 = 0x20;
    D_80139280[35] = -1;
    D_801B2BD0 += 1;
    func_80097390();
}
