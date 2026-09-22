#include "wmap_effect_primitives.h"
#include "common.h"

extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32* D_80139280;
extern s32 D_801B2E1C;
extern s32 D_801B2E18;

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A51E8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2E1C == 0)
    {
        D_801B2E18 += 1;
    }
}
