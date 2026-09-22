#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B2558;
extern s32 D_801B255C;

/** @brief Draw the sequence effect and advance when the countdown expires. */
void func_8007421C(void)
{
    s32 value;

    func_8006CFE4((s32)D_800DB578, (s32)D_80139FE8, 0x18, 0x81, 0x81, 0x10);
    value = D_801B255C - 1;
    D_801B255C = value;
    if (value == 0)
    {
        D_801B2558 += 1;
    }
}
