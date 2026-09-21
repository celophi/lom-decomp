#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2DD0;
extern void func_800A3DD8(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_800A3E70(void);
extern void func_800A3E1C(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A3D9C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2DD0 += 1;
        func_800A3DD8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800A3DD8(void)
{
    func_8006CAC0(func_800A3E70);
    D_8013B20C = 1;
    D_801B2DD0 += 1;
    func_800A3E1C();
}
