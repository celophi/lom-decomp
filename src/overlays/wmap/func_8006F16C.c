#include "common.h"

extern void func_8006DC98(void);
extern s32 D_801B241C;
extern s32 D_801B2418;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F16C(void)
{
    D_801B241C = 0x9;
    D_801B2418 += 1;
    func_8006DC98();
}
