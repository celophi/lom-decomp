#include "wmap_effect_primitives.h"
#include "common.h"

extern void func_800A66C0(void);
extern void func_800A6800(void);
extern s32 D_801B2E40;
extern s32 D_801B2E44;

/** @brief Run three drawing updates and advance when the countdown expires. */
void func_800A79E4(void)
{
    s32 remaining_ticks;

    func_8006AEE0();
    func_800A66C0();
    func_800A6800();
    remaining_ticks = D_801B2E44 - 1;
    D_801B2E44 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2E40 += 1;
    }
}
