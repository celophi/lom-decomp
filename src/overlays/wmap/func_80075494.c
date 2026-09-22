#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2574;
extern s32 D_801B2570;
extern void func_80076848(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075494(void)
{
    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800754C8(void)
{
    func_8006CAC0(func_80076848);
    D_801B2574 = 0x26;
    D_801B2570 += 1;
}
