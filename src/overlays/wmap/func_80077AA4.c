#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80078278(void);

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077AA4(void)
{
    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80077AD8(void)
{
    func_8006CAC0(func_80078278);
    D_801B25F4 = 0x10;
    D_801B25F0 += 1;
}
