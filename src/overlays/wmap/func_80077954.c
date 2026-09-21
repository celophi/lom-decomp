#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_8006CAC0(void (*step)(void));
extern void func_8007855C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077954(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80077988(void)
{
    func_8006CAC0(func_8007855C);
    D_801B25F4 = 0x14;
    D_801B25F0 += 1;
}
