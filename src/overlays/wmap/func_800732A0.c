#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073F48(void);
extern void func_80074194(void);
extern void func_80073DB0(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800732A0(void)
{
    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800732D4(void)
{
    func_8006CAC0(func_80073F48);
    func_8006CAC0(func_80074194);
    func_8006CAC0(func_80073DB0);
    D_801B2514 = 0x60;
    D_801B2510 += 1;
}
