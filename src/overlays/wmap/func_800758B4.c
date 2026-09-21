#include "common.h"

extern void func_800758EC(void);
extern s32 D_801B257C;
extern s32 D_801B2578;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800758B4(void)
{
    D_801B257C = 0x20;
    D_801B2578 += 1;
    func_800758EC();
}
