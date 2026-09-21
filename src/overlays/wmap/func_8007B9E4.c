#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B26E0;
extern void func_8007BA20(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8007BAB8(void);
extern void func_8007BA64(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007B9E4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B26E0 += 1;
        func_8007BA20();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007BA20(void)
{
    func_8006CAC0(func_8007BAB8);
    D_8013B20C = 1;
    D_801B26E0 += 1;
    func_8007BA64();
}
