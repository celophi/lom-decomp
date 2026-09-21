#include "common.h"

extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006CAC0(void (*step)(void));
extern void func_8006F3E8(void);
extern void func_80070114(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FE48(void)
{
    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8006FE7C(void)
{
    func_8006CAC0(func_8006F3E8);
    func_8006CAC0(func_80070114);
    D_801B2404 = 0x18;
    D_801B2400 += 1;
}
