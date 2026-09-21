#include "common.h"

extern void func_8006CAC0(void (*fn)(void));
extern void func_8009A3E0(void);
extern void func_8009A9F0(void);
extern s32 D_800D9164;
extern s32 D_801B2C4C;

/** @brief World-map step handler: install a callback, advance the step counter, chain to the next step. */
void func_8009A9A8(void)
{
    D_800D9164 = 1;
    func_8006CAC0(func_8009A3E0);
    D_801B2C4C += 1;
    func_8009A9F0();
}
