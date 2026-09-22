#include "wmap_effect_primitives.h"
#include "common.h"

extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A66C0(void);

/** @brief World-map step: run the two sub-steps, then advance after the timer. */
void func_800A7AA0(void)
{
    func_8006AEE0();
    func_800A66C0();
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}
