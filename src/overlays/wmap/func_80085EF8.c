#include "common.h"

extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800861B0(void);
extern void func_80086CDC(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085EF8(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80085F2C(void)
{
    func_8006CAC0(func_800861B0);
    func_8006CAC0(func_80086CDC);
    D_801B28BC = 0xBF;
    D_801B28B8 += 1;
}
