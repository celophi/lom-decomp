#include "common.h"

extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_8006CAC0(void (*step)(void));
extern void func_800A21B0(void);
extern void func_800A2308(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1BE8(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800A1C1C(void)
{
    func_8006CAC0(func_800A21B0);
    func_8006CAC0(func_800A2308);
    D_801B2D5C = 0x5;
    D_801B2D58 += 1;
}
