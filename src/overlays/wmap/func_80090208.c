#include "common.h"

extern s32 D_801B2A74;
extern s32 D_801B2A70;
extern void func_8006CAC0(void (*step)(void));
extern void func_80091490(void);
extern void func_80090888(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80090208(void)
{
    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009023C(void)
{
    func_8006CAC0(func_80091490);
    func_8006CAC0(func_80090888);
    D_801B2A74 = 0x28;
    D_801B2A70 += 1;
}
