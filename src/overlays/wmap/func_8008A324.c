#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D58;
extern s32 D_801B2994;
extern s32 D_801B2990;

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008A324(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D58, 0xF, 0x2, 0);
    if (--D_801B2994 == 0)
    {
        D_801B2990 += 1;
    }
}
