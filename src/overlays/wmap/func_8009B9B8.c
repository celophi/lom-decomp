#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B2C60;
extern void func_8009B9F4(void);
extern void func_8009BA8C(void);
extern void func_8009BA38(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009B9B8(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2C60 += 1;
        func_8009B9F4();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8009B9F4(void)
{
    func_8006CAC0(func_8009BA8C);
    D_8013B20C = 1;
    D_801B2C60 += 1;
    func_8009BA38();
}
