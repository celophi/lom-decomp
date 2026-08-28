#include "common.h"

extern s32 D_80166B94;
extern s32 D_80166B98;
extern s32 D_80166B9C;
extern s32 D_80166BA0;
extern s32 D_80166108;
extern s32 D_8016610C;
extern s32 D_80166110;
extern s32 D_80166114;

extern s32 func_800167CC(s32);

/** @see decomp.me (100.00%) */
void func_80149FEC(void)
{
    func_800167CC(D_80166B94);
    func_800167CC(D_80166B98);
    func_800167CC(D_80166B9C);
    func_800167CC(D_80166BA0);
}

/** @see decomp.me (100.00%) */
void func_8014A044(void)
{
    func_800167CC(D_80166108);
    func_800167CC(D_8016610C);
    func_800167CC(D_80166110);
    func_800167CC(D_80166114);
}

/** @see decomp.me (100.00%) */
s32 func_8014A09C(void)
{
    if (func_800167CC(D_80166B94) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_80166B98) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80166B9C) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80166BA0) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 func_8014A130(void)
{
    if (func_800167CC(D_80166108) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_8016610C) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80166110) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80166114) == 1)
    {
        return 3;
    }
    return -1;
}
