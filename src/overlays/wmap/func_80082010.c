#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B27F4;
extern s32 D_801B27F0;
extern void func_800827F8(void);
extern void func_800838D8(void);
extern void func_80083CBC(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80082010(void)
{
    if (--D_801B27F4 == 0)
    {
        D_801B27F0 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_80082044(void)
{
    func_8006CAC0(func_800827F8);
    func_8006CAC0(func_800838D8);
    func_8006CAC0(func_80083CBC);
    D_801B27F4 = 0x38;
    D_801B27F0 += 1;
}
