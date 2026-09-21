#include "common.h"

extern void func_800A053C(void);
extern s32 D_801B2D44;
extern s32 D_801B2D40;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A0504(void)
{
    D_801B2D44 = 0x28;
    D_801B2D40 += 1;
    func_800A053C();
}
