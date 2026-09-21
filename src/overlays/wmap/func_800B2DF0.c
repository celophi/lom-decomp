#include "common.h"

extern void func_800B2E38(void);
extern s32* D_80139280;
extern s32 D_801B2FBC;
extern s32 D_801B2FB8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800B2DF0(void)
{
    D_801B2FBC = 0x20;
    D_80139280[30] = 9999;
    D_801B2FB8 += 1;
    func_800B2E38();
}
