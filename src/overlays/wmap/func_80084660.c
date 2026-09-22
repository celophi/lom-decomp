#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084E88(void);

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80084660(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_80084E88);
    D_801B2874 = 0x18;
    D_801B2870 += 1;
}
