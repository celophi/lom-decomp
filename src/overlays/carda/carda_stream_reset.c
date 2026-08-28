#include "common.h"

extern s32 D_801660A0;
extern void *D_801663A0;
extern u8 D_80165B78[];

extern s32 func_8001724C(s32);
extern s32 func_8001729C(s32);
extern void func_80149FEC(void);
extern s32 func_8014A09C(void);

/** @see decomp.me (100.00%) */
void func_801495E4(void)
{
    func_8001729C(D_801660A0);
    func_80149FEC();
    func_8001724C(D_801660A0 * 0x10);
    D_801663A0 = D_80165B78;
}

/** @see decomp.me (100.00%) */
s32 func_80149638(void)
{
    s32 busy_slot;

    busy_slot = func_8014A09C();
    if (busy_slot != -1)
    {
        func_8001729C(D_801660A0);
        func_8001724C(D_801660A0 * 0x10);
    }
    return busy_slot;
}
