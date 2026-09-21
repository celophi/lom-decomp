#include "common.h"

extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8006CAC0(void (*step)(void));
extern void func_8008E9C8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DA00(void)
{
    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008DA34(void)
{
    func_8006CAC0(func_8008E9C8);
    D_801B2A14 = 0x74;
    D_801B2A10 += 1;
}
