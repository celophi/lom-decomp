#include "common.h"

extern s32 D_801B2A74;
extern s32 D_801B2A70;
extern void func_8006CAC0(void (*step)(void));
extern void func_80090CEC(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008FFA8(void)
{
    if (--D_801B2A74 == 0)
    {
        D_801B2A70 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008FFDC(void)
{
    func_8006CAC0(func_80090CEC);
    D_801B2A74 = 0x2;
    D_801B2A70 += 1;
}
