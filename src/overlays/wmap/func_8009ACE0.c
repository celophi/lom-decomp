#include "common.h"

extern s16 D_801AFBD2;
extern s16 D_800D926A;
extern s32 D_801B2C54;
extern s32 D_801B2C58;
extern void func_80099754(s32 arg);

/** @brief Advance a world-map sub-state, clearing a flag when a marker hits its cap. */
void func_8009ACE0(void)
{
    func_80099754(1);
    if (D_801AFBD2 == 0x400)
    {
        D_800D926A = 0;
    }
    if (--D_801B2C58 == 0)
    {
        D_801B2C54 += 1;
    }
}
