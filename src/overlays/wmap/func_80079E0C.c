#include "common.h"

extern s32 D_801B268C;
extern s32 D_801B2688;
extern void func_8006CAC0(void (*step)(void));
extern void func_8007A910(void);
extern void func_8007AD48(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80079E0C(void)
{
    if (--D_801B268C == 0)
    {
        D_801B2688 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80079E40(void)
{
    func_8006CAC0(func_8007A910);
    func_8006CAC0(func_8007AD48);
    D_801B268C = 0x42;
    D_801B2688 += 1;
}
