#include "common.h"

extern s32 D_80164A4C;
extern s32 D_80164A60;
extern s32 D_80164B0C;
extern s32 D_80164B10;
extern s32 D_80164B14;
extern s32 D_80164B18;
extern s32 D_801651A0;
extern s32 D_801651A4;
extern s32 D_801651A8;
extern s32 D_801651AC;

extern void func_800158E0(void);
extern s32 func_800167AC(s32, s32, s32, s32);
extern void func_800167DC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_801458D0(void)
{
    func_800158E0();
    func_800167EC();
    D_80164B0C = func_800167AC(0xF4000001, 4, 0x2000, 0);
    D_80164B10 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    D_80164B14 = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    D_80164B18 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    D_801651A0 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    D_801651A4 = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    D_801651A8 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    D_801651AC = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(D_80164B0C);
    func_800167DC(D_80164B10);
    func_800167DC(D_80164B14);
    func_800167DC(D_80164B18);
    func_800167DC(D_801651A0);
    func_800167DC(D_801651A4);
    func_800167DC(D_801651A8);
    func_800167DC(D_801651AC);
    func_800167FC();
    D_80164A4C = 0;
    D_80164A60 = 0;
}
