#include "common.h"

extern void func_8009CDC4(void);
extern s32* D_80139280;
extern s32 D_801B2CAC;
extern s32 D_801B2CA8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009CD7C(void)
{
    D_801B2CAC = 0x20;
    D_80139280[5] = -1;
    D_801B2CA8 += 1;
    func_8009CDC4();
}
