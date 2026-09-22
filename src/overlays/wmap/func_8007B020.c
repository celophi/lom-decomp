#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B25D8;
extern s32 D_801B26D8;
extern s32 D_801B26DC;

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void func_8007B020(void)
{
    s32 value;
    s32 remaining_ticks;

    if (D_801B25D8 != 0)
    {
        func_8006D014((s32)D_800DB578, (s32)D_80139FE8, 0x28, 1, D_801B25D8, 8, 1);
        value = D_801B25D8 - 8;
        D_801B25D8 = value;
        if (value < 0)
        {
            D_801B25D8 = 0;
        }
    }
    remaining_ticks = D_801B26DC - 1;
    D_801B26DC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26D8 += 1;
    }
}
