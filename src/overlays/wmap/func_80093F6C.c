#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2B28;
extern void func_80093FA8(void);
extern void func_80094040(void);
extern void func_80093FEC(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80093F6C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2B28 += 1;
        func_80093FA8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80093FA8(void)
{
    func_8006CAC0(func_80094040);
    D_8013B20C = 1;
    D_801B2B28 += 1;
    func_80093FEC();
}
