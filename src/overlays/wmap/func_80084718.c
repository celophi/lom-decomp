#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80085078(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80084718(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008474C(void)
{
    func_8006CAC0(func_80085078);
    D_801B2874 = 0x1E;
    D_801B2870 += 1;
}
