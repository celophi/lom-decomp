#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2568;
extern void func_800750D0(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_80075168(void);
extern void func_80075114(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80075094(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2568 += 1;
        func_800750D0();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800750D0(void)
{
    func_8006CAC0(func_80075168);
    D_8013B20C = 1;
    D_801B2568 += 1;
    func_80075114();
}
