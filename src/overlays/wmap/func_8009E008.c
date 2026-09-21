#include "common.h"

extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139264;
extern s32 D_801B2D4C;
extern s32 D_801B2D48;

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void func_8009E008(void)
{
    D_801398C8[0] = rand() * D_80139264 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139264 / 16 >> 15;
    if (D_80139264 < 0x80)
    {
        D_80139264 += 4;
    }
    if (--D_801B2D4C == 0)
    {
        D_801B2D48 += 1;
    }
}
