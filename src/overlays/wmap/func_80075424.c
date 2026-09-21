#include "common.h"

extern s32 D_801B2574;
extern s32 D_801B2570;
extern void func_8006CAC0(void (*step)(void));
extern void func_800765A4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075424(void)
{
    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80075458(void)
{
    func_8006CAC0(func_800765A4);
    D_801B2574 = 0x4;
    D_801B2570 += 1;
}
