#include "common.h"

extern void func_800B0E84(void);
extern s32* D_80139280;
extern s32 D_801B2F74;
extern s32 D_801B2F70;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800B0E3C(void)
{
    D_801B2F74 = 0x40;
    D_80139280[5] = -1;
    D_801B2F70 += 1;
    func_800B0E84();
}
