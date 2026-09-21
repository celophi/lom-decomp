#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_8006CAC0(void (*step)(void));
extern void func_800ACFF4(void);
extern void func_800ACED0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABDE8(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABE1C(void)
{
    func_8006CAC0(func_800ACFF4);
    func_8006CAC0(func_800ACED0);
    D_801B2E84 = 0x10;
    D_801B2E80 += 1;
}
