#include "common.h"

extern void func_800652A8(s32, s32);
extern void func_8006CAC0(void (*callback)(void));
extern s32 D_801B2E80;
extern s32 D_801B2E84;
extern void func_800ACC88(void);

/** @brief Register a sequence callback, play sound 45, and start a 20-tick delay. */
void func_800AC0A0(void)
{
    func_8006CAC0(&func_800ACC88);
    func_800652A8(0x2D, 0x80);
    D_801B2E84 = 0x14;
    D_801B2E80 += 1;
}
