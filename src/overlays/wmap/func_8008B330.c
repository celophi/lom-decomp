#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B29A8;
extern void func_8008B36C(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_8008B404(void);
extern void func_8008B3B0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008B330(void)
{
    if (D_8013B20C == 0)
    {
        D_801B29A8 += 1;
        func_8008B36C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008B36C(void)
{
    func_8006CAC0(func_8008B404);
    D_8013B20C = 1;
    D_801B29A8 += 1;
    func_8008B3B0();
}
