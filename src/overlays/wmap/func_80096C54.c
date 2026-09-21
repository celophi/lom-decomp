#include "common.h"

extern void func_80096C9C(void);
extern s32* D_80139280;
extern s32 D_801B2BBC;
extern s32 D_801B2BB8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80096C54(void)
{
    D_801B2BBC = 0x40;
    D_80139280[5] = -1;
    D_801B2BB8 += 1;
    func_80096C9C();
}
