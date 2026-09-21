#include "common.h"

extern void func_8006CAC0(void (*callback)(void));
extern s32 D_801B2570;
extern s32 D_801B2574;
extern void func_80076074(void);
extern void func_800761C0(void);

/** @brief Register two sequence callbacks and advance to a one-tick delay. */
void func_800755A8(void)
{
    func_8006CAC0(&func_80076074);
    func_8006CAC0(&func_800761C0);
    D_801B2574 = 1;
    D_801B2570 += 1;
}
