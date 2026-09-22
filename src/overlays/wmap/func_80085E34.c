#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_80086B90(void);

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80085E34(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_80086B90);
    D_801B28BC = 0x3C;
    D_801B28B8 += 1;
}
