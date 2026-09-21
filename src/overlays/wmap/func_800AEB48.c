#include "common.h"

extern s32 D_800D923C;
extern s32 D_8013B20C;
extern s32 D_801B2EF0;
extern void func_8006C81C(void);
extern void func_8006CAC0(void (*fn)(void));
extern void func_800AEB98(void);

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void func_800AEB48(void)
{
    D_800D923C = 1;
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2EF0 += 1;
    func_800AEB98();
}
