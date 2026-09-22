#include "common.h"

extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139264;
extern s32 D_801B2D48;
extern s32 D_801B2D4C;

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void func_8009E114(void)
{
    D_801398C8[0] = rand() * D_80139264 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139264 / 16 >> 15;
    if (--D_80139264 < 0)
    {
        D_80182D48[1] = 0;
        D_80182D48[0] = 0;
        D_801398C8[1] = 0;
        D_801398C8[0] = 0;
        D_801B2D48 += 1;
    }
    else if (--D_801B2D4C == 0)
    {
        D_801B2D48 += 1;
    }
}
