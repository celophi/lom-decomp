#include "common.h"

extern void func_8007A150(void);
extern s32 D_801B2694;
extern s32 D_801B2690;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007A118(void)
{
    D_801B2694 = 0x8;
    D_801B2690 += 1;
    func_8007A150();
}
