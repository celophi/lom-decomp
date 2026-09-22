#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2EF0;
extern void func_800AEBD4(void);
extern void func_800AEC70(void);
extern void func_800AEC18(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800AEB98(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2EF0 += 1;
        func_800AEBD4();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800AEBD4(void)
{
    func_8006CAC0(func_800AEC70);
    D_8013B20C = 1;
    D_801B2EF0 += 1;
    func_800AEC18();
}
