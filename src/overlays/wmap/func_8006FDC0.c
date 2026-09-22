#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F22C(void);
extern void func_8006EF20(void);
extern void func_80070408(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FDC0(void)
{
    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_8006FDF4(void)
{
    func_8006CAC0(func_8006F22C);
    func_8006CAC0(func_8006EF20);
    func_8006CAC0(func_80070408);
    D_801B2404 = 0x10;
    D_801B2400 += 1;
}
