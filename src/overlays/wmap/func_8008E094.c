#include "common.h"

extern void func_8006B6EC(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4);
extern s32 D_801B2A28;
extern s32 D_801B2A2C;

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_8008E094(void)
{
    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6);
    if (--D_801B2A2C == 0)
    {
        D_801B2A28 += 1;
    }
}
