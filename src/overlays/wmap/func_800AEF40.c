#include "common.h"

extern s32 D_801B2EFC;
extern s32 D_801B2EF8;
extern void func_8006CAC0(void (*step)(void));
extern void func_800AFDCC(void);
extern void func_800AFF20(void);
extern void func_800B0B74(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800AEF40(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800AEF74(void)
{
    func_8006CAC0(func_800AFDCC);
    func_8006CAC0(func_800AFF20);
    func_8006CAC0(func_800B0B74);
    D_801B2EFC = 0x7;
    D_801B2EF8 += 1;
}
