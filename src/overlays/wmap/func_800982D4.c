#include "common.h"

extern s32 D_801B2BEC;
extern s32 D_801B2BE8;
extern void func_8006CAC0(void (*step)(void));
extern void func_80098AF0(void);
extern void func_80098E28(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800982D4(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80098308(void)
{
    func_8006CAC0(func_80098AF0);
    func_8006CAC0(func_80098E28);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}
