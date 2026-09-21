#include "common.h"

extern s32 D_801B2914;
extern s32 D_801B2910;
extern void func_8006CAC0(void (*step)(void));
extern void func_80088740(void);
extern void func_80088548(void);
extern void func_80087B90(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087834(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_80087868(void)
{
    func_8006CAC0(func_80088740);
    func_8006CAC0(func_80088548);
    func_8006CAC0(func_80087B90);
    D_801B2914 = 0x18;
    D_801B2910 += 1;
}
