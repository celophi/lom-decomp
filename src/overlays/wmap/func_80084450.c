#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2868;
extern void func_8008448C(void);
extern void func_8006CAC0(void (*step)(void));
extern void func_80084524(void);
extern void func_800844D0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80084450(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        func_8008448C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008448C(void)
{
    func_8006CAC0(func_80084524);
    D_8013B20C = 1;
    D_801B2868 += 1;
    func_800844D0();
}
