#include "common.h"

extern void func_8009D1D0(void);
extern s32* D_80139280;
extern s32 D_801B2CBC;
extern s32 D_801B2CB8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009D188(void)
{
    D_801B2CBC = 0x20;
    D_80139280[25] = -1;
    D_801B2CB8 += 1;
    func_8009D1D0();
}
