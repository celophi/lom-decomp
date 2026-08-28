#include "common.h"

extern s32 D_80164FC4;
extern s32 D_80164FC8;
extern s32 D_80164FCC;
extern s32 D_80164FD0;
extern s32 D_80165658;
extern s32 D_8016565C;
extern s32 D_80165660;
extern s32 D_80165664;

extern s32 func_800167CC(s32);

/** @see decomp.me (100.00%) */
void func_80146114(void)
{
    func_800167CC(D_80164FC4);
    func_800167CC(D_80164FC8);
    func_800167CC(D_80164FCC);
    func_800167CC(D_80164FD0);
}

/** @see decomp.me (100.00%) */
void func_8014616C(void)
{
    func_800167CC(D_80165658);
    func_800167CC(D_8016565C);
    func_800167CC(D_80165660);
    func_800167CC(D_80165664);
}

/** @see decomp.me (100.00%) */
s32 func_801461C4(void)
{
    if (func_800167CC(D_80164FC4) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_80164FC8) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80164FCC) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80164FD0) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 func_80146258(void)
{
    if (func_800167CC(D_80165658) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_8016565C) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80165660) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80165664) == 1)
    {
        return 3;
    }
    return -1;
}
