#include "common.h"

extern void func_80076790(void);
extern s32 D_801B25C4;
extern s32 D_801B25C0;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076758(void)
{
    D_801B25C4 = 0x10;
    D_801B25C0 += 1;
    func_80076790();
}
