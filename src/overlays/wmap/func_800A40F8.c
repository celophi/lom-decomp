#include "common.h"

extern s32 D_801B2DDC;
extern s32 D_801B2DD8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800A55D0(void);
extern void func_800A5918(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A40F8(void)
{
    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800A412C(void)
{
    func_8006CAC0(func_800A55D0);
    func_8006CAC0(func_800A5918);
    D_801B2DDC = 0x44;
    D_801B2DD8 += 1;
}
