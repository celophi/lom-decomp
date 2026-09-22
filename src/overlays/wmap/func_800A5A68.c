#include "common.h"

extern void func_800A5AA0(void);
extern s32 D_801B2E34;
extern s32 D_801B2E30;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A5A68(void)
{
    D_801B2E34 = 0x28;
    D_801B2E30 += 1;
    func_800A5AA0();
}
