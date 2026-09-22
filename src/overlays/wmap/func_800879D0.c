#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2914;
extern s32 D_801B2910;
extern void func_80087ED0(void);

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_800879D0(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_80087ED0);
    D_801B2914 = 0x3C;
    D_801B2910 += 1;
}
