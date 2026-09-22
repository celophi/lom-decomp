#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B20C;
extern s32 D_801B25E8;
extern void func_80077670(void);
extern void func_80077708(void);
extern void func_800776B4(void);

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80077634(void)
{
    if (D_8013B20C == 0)
    {
        D_801B25E8 += 1;
        func_80077670();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80077670(void)
{
    func_8006CAC0(func_80077708);
    D_8013B20C = 1;
    D_801B25E8 += 1;
    func_800776B4();
}
