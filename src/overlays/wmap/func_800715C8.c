#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072034(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800715C8(void)
{
    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800715FC(void)
{
    func_8006CAC0(func_80072034);
    D_801B24C4 = 0x1E;
    D_801B24C0 += 1;
}
