#include "common.h"

extern s32 D_800F22AC;
extern s32 D_80157D68;
extern s32 D_80157D70;
extern s32 D_80157D74;

/**
 * @brief Build the current frame and advance the encyclopedia transition state.
 * @see decomp.me (100%)
 */
void func_8014128C(s32 arg0)
{
    func_801418A4(arg0);
    func_80141C08(arg0);
    func_80141DF4(arg0);
    D_800F22AC += 1;
    func_80141354();

    if (D_80157D68 != 0)
    {
        D_80157D70 += (D_80157D74 - D_80157D70) / D_80157D68;
        D_80157D68 -= 1;
        return;
    }
    D_80157D70 = D_80157D74;
}
