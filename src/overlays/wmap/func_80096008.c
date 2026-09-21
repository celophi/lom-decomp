#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2B88;
extern void func_80096044(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_800960DC(void);
extern void func_80096088(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80096008(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2B88 += 1;
        func_80096044();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80096044(void)
{
    func_8006CAC0(func_800960DC);
    D_8013B20C = 1;
    D_801B2B88 += 1;
    func_80096088();
}
