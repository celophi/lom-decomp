#include "common.h"

extern s32 D_801609A8;
extern void *D_80165488;
extern u8 D_8016057C[];

extern s32 func_8001724C(s32);
extern s32 func_8001729C(s32);
extern void func_80145FC0(void);
extern s32 func_80146070(void);

/** @see decomp.me (100.00%) */
void func_80145824(void)
{
    func_8001729C(D_801609A8);
    func_80145FC0();
    func_8001724C(D_801609A8 * 0x10);
    D_80165488 = D_8016057C;
}

/** @see decomp.me (100.00%) */
s32 func_80145878(void)
{
    s32 busy_slot;

    busy_slot = func_80146070();
    if (busy_slot != -1)
    {
        func_8001729C(D_801609A8);
        func_8001724C(D_801609A8 * 0x10);
    }
    return busy_slot;
}
