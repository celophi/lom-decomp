#include "common.h"

extern void func_80083B04(void);
extern s32 D_801B285C;
extern s32 D_801B2858;

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80083ACC(void)
{
    D_801B285C = 0x18;
    D_801B2858 += 1;
    func_80083B04();
}
