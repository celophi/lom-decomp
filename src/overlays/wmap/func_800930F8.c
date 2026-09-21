#include "common.h"

extern void func_80093140(void);
extern s32* D_80139280;
extern s32 D_801B2B1C;
extern s32 D_801B2B18;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800930F8(void)
{
    D_801B2B1C = 0x20;
    D_80139280[35] = -1;
    D_801B2B18 += 1;
    func_80093140();
}
