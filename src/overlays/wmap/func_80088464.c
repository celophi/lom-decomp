#include "common.h"

extern void func_800884AC(void);
extern s32* D_80139280;
extern s32 D_801B2944;
extern s32 D_801B2940;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088464(void)
{
    D_801B2944 = 0x20;
    D_80139280[15] = -1;
    D_801B2940 += 1;
    func_800884AC();
}
