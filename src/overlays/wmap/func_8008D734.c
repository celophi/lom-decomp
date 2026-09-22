#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008DF40(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D734(void)
{
    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008D768(void)
{
    func_8006CAC0(func_8008DF40);
    D_801B2A14 = 0x1E;
    D_801B2A10 += 1;
}
