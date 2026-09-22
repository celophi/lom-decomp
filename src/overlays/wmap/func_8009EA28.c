#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F8E0(void);
extern void func_8009F784(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EA28(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009EA5C(void)
{
    func_8006CAC0(func_8009F8E0);
    func_8006CAC0(func_8009F784);
    D_801B2CDC = 0x48;
    D_801B2CD8 += 1;
}
