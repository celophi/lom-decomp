#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;
extern s32 D_801B296C;
extern s32 D_801B2968;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800898FC(void)
{
    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}
