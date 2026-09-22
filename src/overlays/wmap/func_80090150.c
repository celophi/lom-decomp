#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2A74;
extern s32 D_801B2A70;
extern void func_80091538(void);

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80090150(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_80091538);
    D_801B2A74 = 0x3C;
    D_801B2A70 += 1;
}
