#include "common.h"

extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8006CAC0(void (*step)(void));
extern void func_8008E870(void);
extern void func_8008DDA0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DA70(void)
{
    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8008DAA4(void)
{
    func_8006CAC0(func_8008E870);
    func_8006CAC0(func_8008DDA0);
    D_801B2A14 = 0x2;
    D_801B2A10 += 1;
}
