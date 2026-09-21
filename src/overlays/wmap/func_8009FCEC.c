#include "common.h"

extern void func_8009FD34(void);
extern s32* D_80139280;
extern s32 D_801B2D2C;
extern s32 D_801B2D28;

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009FCEC(void)
{
    D_801B2D2C = 0x20;
    D_80139280[25] = -1;
    D_801B2D28 += 1;
    func_8009FD34();
}
