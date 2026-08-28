#include "common.h"

extern s32 D_80164B0C;
extern s32 D_80164B10;
extern s32 D_80164B14;
extern s32 D_80164B18;
extern s32 D_801651A0;
extern s32 D_801651A4;
extern s32 D_801651A8;
extern s32 D_801651AC;

extern s32 func_800167CC(s32);

/** @see decomp.me (100.00%) */
void func_80145FC0(void)
{
    func_800167CC(D_80164B0C);
    func_800167CC(D_80164B10);
    func_800167CC(D_80164B14);
    func_800167CC(D_80164B18);
}

/** @see decomp.me (100.00%) */
void func_80146018(void)
{
    func_800167CC(D_801651A0);
    func_800167CC(D_801651A4);
    func_800167CC(D_801651A8);
    func_800167CC(D_801651AC);
}

/** @see decomp.me (100.00%) */
s32 func_80146070(void)
{
    if (func_800167CC(D_80164B0C) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_80164B10) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80164B14) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80164B18) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 func_80146104(void)
{
    if (func_800167CC(D_801651A0) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_801651A4) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_801651A8) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_801651AC) == 1)
    {
        return 3;
    }
    return -1;
}
