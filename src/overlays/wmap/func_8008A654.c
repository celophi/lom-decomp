#include "common.h"

extern void func_8008A69C(void);
extern s32* D_80139280;
extern s32 D_801B299C;
extern s32 D_801B2998;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008A654(void)
{
    D_801B299C = 0x20;
    D_80139280[35] = -1;
    D_801B2998 += 1;
    func_8008A69C();
}
