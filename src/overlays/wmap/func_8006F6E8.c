#include "common.h"

extern void func_8006F720(void);
extern s32 D_801B2434;
extern s32 D_801B2430;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F6E8(void)
{
    D_801B2434 = 0x40;
    D_801B2430 += 1;
    func_8006F720();
}
