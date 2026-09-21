#include "common.h"

extern s32 D_801B2574;
extern s32 D_801B2570;
extern void func_8006CAC0(void (*step)(void));
extern void func_80075B34(void);
extern void func_800759B4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075338(void)
{
    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007536C(void)
{
    func_8006CAC0(func_80075B34);
    func_8006CAC0(func_800759B4);
    D_801B2574 = 0x20;
    D_801B2570 += 1;
}
