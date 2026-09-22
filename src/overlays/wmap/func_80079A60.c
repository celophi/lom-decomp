#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2680;
extern void func_80079A9C(void);
extern void func_80079B34(void);
extern void func_80079AE0(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80079A60(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2680 += 1;
        func_80079A9C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80079A9C(void)
{
    func_8006CAC0(func_80079B34);
    D_8013B20C = 1;
    D_801B2680 += 1;
    func_80079AE0();
}
