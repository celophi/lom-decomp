#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B27E8;
extern void func_80081CB0(void);
extern void func_80081D48(void);
extern void func_80081CF4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80081C74(void)
{
    if (D_8013B20C == 0)
    {
        D_801B27E8 += 1;
        func_80081CB0();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80081CB0(void)
{
    func_8006CAC0(func_80081D48);
    D_8013B20C = 1;
    D_801B27E8 += 1;
    func_80081CF4();
}
