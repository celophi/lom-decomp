#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007BF7C(void);
extern void func_8007CF38(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BDEC(void)
{
    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007BE20(void)
{
    func_8006CAC0(func_8007BF7C);
    func_8006CAC0(func_8007CF38);
    D_801B26EC = 0x24;
    D_801B26E8 += 1;
}
