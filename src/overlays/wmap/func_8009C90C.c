#include "common.h"

extern void func_8009C954(void);
extern s32* D_80139280;
extern s32 D_801B2C9C;
extern s32 D_801B2C98;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009C90C(void)
{
    D_801B2C9C = 0x20;
    D_80139280[35] = -1;
    D_801B2C98 += 1;
    func_8009C954();
}
