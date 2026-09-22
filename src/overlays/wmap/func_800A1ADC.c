#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2BE4(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1ADC(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1B10(void)
{
    func_8006CAC0(func_800A2BE4);
    D_801B2D5C = 0x78;
    D_801B2D58 += 1;
}
