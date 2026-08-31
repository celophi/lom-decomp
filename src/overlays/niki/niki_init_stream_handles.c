#include "common.h"

extern s32 D_80164F08;
extern s32 D_80164F18;
extern s32 D_80164FC4;
extern s32 D_80164FC8;
extern s32 D_80164FCC;
extern s32 D_80164FD0;
extern s32 D_80165658;
extern s32 D_8016565C;
extern s32 D_80165660;
extern s32 D_80165664;

extern void func_800158E0(void);
extern s32 func_800167AC(s32, s32, s32, s32);
extern void func_800167DC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_80145A40(void)
{
    func_800158E0();
    func_800167EC();
    D_80164FC4 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    D_80164FC8 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    D_80164FCC = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    D_80164FD0 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    D_80165658 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    D_8016565C = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    D_80165660 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    D_80165664 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(D_80164FC4);
    func_800167DC(D_80164FC8);
    func_800167DC(D_80164FCC);
    func_800167DC(D_80164FD0);
    func_800167DC(D_80165658);
    func_800167DC(D_8016565C);
    func_800167DC(D_80165660);
    func_800167DC(D_80165664);
    func_800167FC();
    D_80164F08 = 0;
    D_80164F18 = 0;
}
