#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009FDD8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E850(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E884(void)
{
    func_8006CAC0(func_8009FDD8);
    D_801B2CDC = 0x50;
    D_801B2CD8 += 1;
}
