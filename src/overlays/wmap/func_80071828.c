#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80071E94(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80071828(void)
{
    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007185C(void)
{
    func_8006CAC0(func_80071E94);
    D_801B24C4 = 0x3C;
    D_801B24C0 += 1;
}
