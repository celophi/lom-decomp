#include "common.h"

extern s32 D_8013B298;

void func_80055830(s32 delay_min, s32 delay_range, s32 duration_min, s32 duration_range);

/**
 * @brief Update state-dependent world-map effect timing.
 */
void func_80055AF4(void)
{
    switch (D_8013B298)
    {
    case 1:
        func_80055830(40, 20, 25, 30);
        break;
    case 2:
        func_80055830(30, 20, 20, 25);
        break;
    case 3:
        func_80055830(10, 20, 20, 20);
        break;
    case 4:
        func_80055830(5, 25, 15, 20);
        break;
    default:
        func_80055830(5, 20, 8, 15);
        break;
    }
}
