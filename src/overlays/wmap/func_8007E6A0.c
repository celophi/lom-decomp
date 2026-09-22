#include "wmap_sequence_runtime.h"
#include "common.h"

extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DEC;
extern s32 D_801B2760;
extern s32 D_801B2764;

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007E6A0(void)
{
    func_8006D014(D_800D94D0, D_801399F8, 4, 1, D_80182DEC, 8, 2);
    if (--D_801B2764 == 0)
    {
        D_801B2760 += 1;
    }
}
