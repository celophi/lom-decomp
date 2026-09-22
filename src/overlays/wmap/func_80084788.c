#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084A3C(void);
extern void func_80085270(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80084788(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800847BC(void)
{
    func_8006CAC0(func_80084A3C);
    func_8006CAC0(func_80085270);
    D_801B2874 = 0x78;
    D_801B2870 += 1;
}
