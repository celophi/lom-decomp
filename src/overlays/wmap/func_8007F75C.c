#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_801ADAE0;
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_8008049C(void);

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_8007F75C(void)
{
    D_801ADAE0 = 1;
    func_8006CAC0(func_8008049C);
    D_801B278C = 0x14;
    D_801B2788 += 1;
}
