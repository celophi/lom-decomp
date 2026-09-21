#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2780;
extern void func_8007F4A8(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8007F540(void);
extern void func_8007F4EC(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007F46C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2780 += 1;
        func_8007F4A8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007F4A8(void)
{
    func_8006CAC0(func_8007F540);
    D_8013B20C = 1;
    D_801B2780 += 1;
    func_8007F4EC();
}
