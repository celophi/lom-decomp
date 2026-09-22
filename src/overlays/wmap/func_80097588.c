#include "wmap_effect_primitives.h"
#include "common.h"

extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32* D_80139280;
extern s32 D_801B2BD8;
extern s32 D_801B2BDC;

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void func_80097588(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2BDC == 0)
    {
        D_801B2BD8 += 1;
    }
}
