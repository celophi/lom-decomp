#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E10C(void);
extern void func_8008E268(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D8A4(void)
{
    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8008D8D8(void)
{
    func_8006CAC0(func_8008E10C);
    func_8006CAC0(func_8008E268);
    D_801B2A14 = 0x4;
    D_801B2A10 += 1;
}
