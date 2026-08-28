#include "common.h"

extern s32 D_80164B70;
extern void *D_80164E18;
extern u8 D_801606D0[];

extern s32 func_8001724C(s32);
extern s32 func_8001729C(s32);
extern void func_80146114(void);
extern s32 func_801461C4(void);

/** @see decomp.me (100.00%) */
void func_80145994(void)
{
    func_8001729C(D_80164B70);
    func_80146114();
    func_8001724C(D_80164B70 * 0x10);
    D_80164E18 = D_801606D0;
}

/** @see decomp.me (100.00%) */
s32 func_801459E8(void)
{
    s32 busy_slot;

    busy_slot = func_801461C4();
    if (busy_slot != -1)
    {
        func_8001729C(D_80164B70);
        func_8001724C(D_80164B70 * 0x10);
    }
    return busy_slot;
}
