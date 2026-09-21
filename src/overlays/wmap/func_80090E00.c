#include "common.h"

extern void func_80090E48(void);
extern s32* D_80139280;
extern s32 D_801B2A9C;
extern s32 D_801B2A98;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80090E00(void)
{
    D_801B2A9C = 0x20;
    D_80139280[25] = -1;
    D_801B2A98 += 1;
    func_80090E48();
}
