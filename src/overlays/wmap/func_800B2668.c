#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2F9C;
extern s32 D_801B2F98;
extern void func_800B2B40(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2668(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800B269C(void)
{
    func_8006CAC0(func_800B2B40);
    D_801B2F9C = 0x4;
    D_801B2F98 += 1;
}
