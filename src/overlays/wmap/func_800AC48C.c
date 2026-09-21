#include "common.h"

extern void func_8006CAC0(void (*fn)(void));
extern void func_800AC91C(void);
extern void func_800ACED0(void);
extern void func_800AD610(void);
extern void func_800AD768(void);
extern s32 D_801B2E80;
extern s32 D_801B2E84;

/** @brief World-map step: register the four draw callbacks and tick the frame counter. */
void func_800AC48C(void)
{
    func_8006CAC0(func_800AC91C);
    func_8006CAC0(func_800ACED0);
    func_8006CAC0(func_800AD610);
    func_8006CAC0(func_800AD768);
    D_801B2E84 = 4;
    D_801B2E80 += 1;
}
