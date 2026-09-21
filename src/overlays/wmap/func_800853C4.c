#include "common.h"

extern void func_8006B6EC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);
extern s32 D_801B28A8;
extern s32 D_801B28AC;

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_800853C4(void)
{
    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--D_801B28AC == 0)
    {
        D_801B28A8 += 1;
    }
}
