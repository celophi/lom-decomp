#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_80139898[];
extern u8 D_801B2670[];
extern s32 D_801B25D8;
extern s32 D_8011CF2C;
extern s32 D_801B2A40;
extern s32 D_801B2A44;

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void func_8008CF04(void)
{
    u8 *p;
    u8 *q;

    p = D_80139898;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = D_801B2670;
    func_8006CFA8(p, q);
    if (D_801B25D8 != 0)
    {
        func_8006CD98(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
        D_801B25D8 -= 1;
        if (D_801B25D8 < 0)
        {
            D_801B25D8 = 0;
        }
        *(s16 *)(q + 4) += -3;
    }
    PopMatrix();
    if (--D_801B2A44 == 0)
    {
        D_801B2A40 += 1;
    }
}
