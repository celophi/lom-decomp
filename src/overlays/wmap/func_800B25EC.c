#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2F9C;
extern s32 D_801B2F98;
extern void func_800B299C(void);
extern void func_800B32D8(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B25EC(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800B2620(void)
{
    func_8006CAC0(func_800B299C);
    func_8006CAC0(func_800B32D8);
    D_801B2F9C = 0x5A;
    D_801B2F98 += 1;
}
