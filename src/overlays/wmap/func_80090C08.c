#include "common.h"

extern void func_80090C50(void);
extern s32* D_80139280;
extern s32 D_801B2A94;
extern s32 D_801B2A90;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80090C08(void)
{
    D_801B2A94 = 0x20;
    D_80139280[15] = -1;
    D_801B2A90 += 1;
    func_80090C50();
}
