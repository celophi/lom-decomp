#include "common.h"

extern void func_80099754(s32);
extern s32 D_800D9164;
extern s16 D_801AFBD2;

/**
 * @brief Update the world map and clear its flag when the tracked value reaches 0x3E0.
 * @return Zero at the target value, otherwise one.
 */
s32 func_8009A3E0(void)
{
    func_80099754(1);
    if (D_801AFBD2 == 0x3E0)
    {
        D_800D9164 = 0;
        return 0;
    }
    return 1;
}
