#include "common.h"

extern void func_800A0C44(void);
extern s32 D_801B2D8C;
extern s32 D_801B2D88;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2578(void)
{
    D_801B2D8C = 0x40;
    D_801B2D88 += 1;
    func_800A0C44();
}
