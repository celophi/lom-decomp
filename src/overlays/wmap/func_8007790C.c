#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_800783C8(void);
extern void func_8006CAC0(void (*callback)(void));

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_8007790C(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_800783C8);
    D_801B25F4 = 0x8;
    D_801B25F0 += 1;
}
