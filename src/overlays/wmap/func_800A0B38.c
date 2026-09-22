#include "common.h"

extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139268;
extern s32 D_801B2D8C;
extern s32 D_801B2D88;

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void func_800A0B38(void)
{
    D_801398C8[0] = rand() * D_80139268 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139268 / 16 >> 15;
    if (D_80139268 < 0x80)
    {
        D_80139268 += 4;
    }
    if (--D_801B2D8C == 0)
    {
        D_801B2D88 += 1;
    }
}
