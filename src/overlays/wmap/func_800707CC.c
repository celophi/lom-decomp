#include "common.h"

extern void func_8006CAC0(void (*fn)(void));
extern void func_8006FB64(void);
extern void func_80070808(void);
extern s32 D_801B2408;

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void func_800707CC(void)
{
    func_8006CAC0(func_8006FB64);
    D_801B2408 += 1;
    func_80070808();
}
