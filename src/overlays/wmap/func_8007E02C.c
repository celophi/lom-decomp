#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B274C;
extern s32 D_801B2748;
extern void func_8007E934(void);
extern void func_8007E140(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007E02C(void)
{
    if (--D_801B274C == 0)
    {
        D_801B2748 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007E060(void)
{
    func_8006CAC0(func_8007E934);
    func_8006CAC0(func_8007E140);
    D_801B274C = 0x7F;
    D_801B2748 += 1;
}
