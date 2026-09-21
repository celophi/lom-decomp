#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2AD0;
extern void func_80091D1C(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_80091E08(void);
extern void func_80091D60(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80091CE0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2AD0 += 1;
        func_80091D1C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80091D1C(void)
{
    func_8006CAC0(func_80091E08);
    D_8013B20C = 1;
    D_801B2AD0 += 1;
    func_80091D60();
}
