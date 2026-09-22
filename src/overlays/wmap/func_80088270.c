#include "common.h"

extern void func_800882B8(void);
extern s32* D_80139280;
extern s32 D_801B293C;
extern s32 D_801B2938;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088270(void)
{
    D_801B293C = 0x20;
    D_80139280[5] = -1;
    D_801B2938 += 1;
    func_800882B8();
}
