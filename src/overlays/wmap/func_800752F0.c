#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2574;
extern s32 D_801B2570;
extern void func_80074E20(void);

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_800752F0(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_80074E20);
    D_801B2574 = 0x24;
    D_801B2570 += 1;
}
