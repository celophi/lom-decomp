#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B25D8;
extern s32 D_801B26D8;
extern s32 D_801B26DC;

/** @brief Draw the sequence effect and advance when its countdown expires. */
void func_8007AF6C(void)
{
    s32 value;

    func_8006D014((s32)D_800DB578, (s32)D_80139FE8, 0x28, 0, D_801B25D8, 8, 1);
    value = D_801B26DC - 1;
    D_801B26DC = value;
    if (value == 0)
    {
        D_801B26D8 += 1;
    }
}
