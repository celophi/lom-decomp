#include "common.h"

extern void func_8009F380(void);
extern s32* D_80139280;
extern s32 D_801B2D04;
extern s32 D_801B2D00;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F338(void)
{
    D_801B2D04 = 0x20;
    D_80139280[5] = -1;
    D_801B2D00 += 1;
    func_8009F380();
}
