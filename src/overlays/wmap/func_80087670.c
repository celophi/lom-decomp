#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2908;
extern void func_800876AC(void);
extern void func_80087744(void);
extern void func_800876F0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80087670(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2908 += 1;
        func_800876AC();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800876AC(void)
{
    func_8006CAC0(func_80087744);
    D_8013B20C = 1;
    D_801B2908 += 1;
    func_800876F0();
}
