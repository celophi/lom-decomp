#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2E84;
extern s32 D_801B2E80;
extern void func_800AD23C(void);
extern void func_800ACB64(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800ABEEC(void)
{
    if (--D_801B2E84 == 0)
    {
        D_801B2E80 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800ABF20(void)
{
    func_8006CAC0(func_800AD23C);
    func_8006CAC0(func_800ACB64);
    D_801B2E84 = 0x8;
    D_801B2E80 += 1;
}
