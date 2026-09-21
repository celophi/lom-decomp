#include "common.h"

extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_8006CAC0(void (*step)(void));
extern void func_80086A38(void);
extern void func_800866E8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085E7C(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80085EB0(void)
{
    func_8006CAC0(func_80086A38);
    func_8006CAC0(func_800866E8);
    D_801B28BC = 0x2;
    D_801B28B8 += 1;
}
