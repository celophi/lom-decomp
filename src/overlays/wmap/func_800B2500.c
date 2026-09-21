#include "common.h"

extern s32 D_801B2F9C;
extern s32 D_801B2F98;
extern void func_8006CAC0(void (*step)(void));
extern void func_800B2CE8(void);
extern void func_800B317C(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2500(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800B2534(void)
{
    func_8006CAC0(func_800B2CE8);
    func_8006CAC0(func_800B317C);
    D_801B2F9C = 0x3C;
    D_801B2F98 += 1;
}
