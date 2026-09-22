#include "common.h"

extern void func_80097588(void);
extern s32* D_80139280;
extern s32 D_801B2BDC;
extern s32 D_801B2BD8;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80097540(void)
{
    D_801B2BDC = 0x20;
    D_80139280[15] = -1;
    D_801B2BD8 += 1;
    func_80097588();
}
