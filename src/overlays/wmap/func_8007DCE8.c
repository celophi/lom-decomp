#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2740;
extern void func_8007DD24(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8007DDBC(void);
extern void func_8007DD68(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007DCE8(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2740 += 1;
        func_8007DD24();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007DD24(void)
{
    func_8006CAC0(func_8007DDBC);
    D_8013B20C = 1;
    D_801B2740 += 1;
    func_8007DD68();
}
