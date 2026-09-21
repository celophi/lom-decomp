#include "common.h"

extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8006CAC0(void (*step)(void));
extern void func_8009C7F0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BB64(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009BB98(void)
{
    func_8006CAC0(func_8009C7F0);
    D_801B2C6C = 0xF;
    D_801B2C68 += 1;
}
