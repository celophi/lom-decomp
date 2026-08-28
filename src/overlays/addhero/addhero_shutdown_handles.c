#include "common.h"

extern s32 D_80164B0C;
extern s32 D_80164B10;
extern s32 D_80164B14;
extern s32 D_80164B18;
extern s32 D_801651A0;
extern s32 D_801651A4;
extern s32 D_801651A8;
extern s32 D_801651AC;

extern void func_800158E0(void);
extern void func_800167BC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_80145A9C(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(D_80164B0C);
    func_800167BC(D_80164B10);
    func_800167BC(D_80164B14);
    func_800167BC(D_80164B18);
    func_800167BC(D_801651A0);
    func_800167BC(D_801651A4);
    func_800167BC(D_801651A8);
    func_800167BC(D_801651AC);
    func_800167FC();
}
