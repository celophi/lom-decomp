#include "common.h"

extern void func_800A5410(void);
extern s32 D_801B2E24;
extern s32 D_801B2E20;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A53D8(void)
{
    D_801B2E24 = 0x18;
    D_801B2E20 += 1;
    func_800A5410();
}
