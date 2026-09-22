#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B2698;
extern s32 D_801B269C;

/** @brief World-map step handler: spawn a sub-object and expire the step counter. */
void func_8007A32C(void)
{
    func_8006CFE4(D_800DA448, D_80139CC8, 0x18, D_801B24B4, D_801B24B4, 0);
    if (--D_801B269C == 0)
    {
        D_801B2698 += 1;
    }
}
