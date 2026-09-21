#include "common.h"

extern void func_8009F584(void);
extern s32* D_80139280;
extern s32 D_801B2D0C;
extern s32 D_801B2D08;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F53C(void)
{
    D_801B2D0C = 0x40;
    D_80139280[15] = -1;
    D_801B2D08 += 1;
    func_8009F584();
}
