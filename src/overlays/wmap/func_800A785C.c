#include "common.h"

extern void func_8005FF88(s32);
extern void func_800A78B0(void);
extern s32 D_8018222C;
extern s32 D_801B2E40;
extern s32 D_801B2E44;

/** @brief Set up the effect, select its delay, and run the next sequence step. */
void func_800A785C(void)
{
    s32 delay;

    func_8005FF88(-1);
    delay = 0x46;
    if (D_8018222C != 0)
    {
        delay = 0x32;
    }
    D_801B2E44 = delay;
    D_801B2E40 += 1;
    func_800A78B0();
}
