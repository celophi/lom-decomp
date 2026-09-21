#include "common.h"

extern void func_8008889C(void);
extern s32* D_80139280;
extern s32 D_801B2954;
extern s32 D_801B2950;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088854(void)
{
    D_801B2954 = 0x20;
    D_80139280[35] = -1;
    D_801B2950 += 1;
    func_8008889C();
}
