#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008972C(void);
extern void func_8006CAC0(void (*callback)(void));

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80089474(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_8008972C);
    D_801B2964 = 0x34;
    D_801B2960 += 1;
}
