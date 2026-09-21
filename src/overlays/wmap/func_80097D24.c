#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2BE0;
extern void func_80097D60(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_80097DF8(void);
extern void func_80097DA4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80097D24(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        func_80097D60();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80097D60(void)
{
    func_8006CAC0(func_80097DF8);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    func_80097DA4();
}
