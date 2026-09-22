#include "wmap_effect_primitives.h"
#include "common.h"

extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32* D_80139280;
extern s32 D_801B2D04;
extern s32 D_801B2D00;

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F380(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2D04 == 0)
    {
        D_801B2D00 += 1;
    }
}
