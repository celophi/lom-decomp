#include "common.h"

extern s32 D_80164FC4;
extern s32 D_80164FC8;
extern s32 D_80164FCC;
extern s32 D_80164FD0;
extern s32 D_80165658;
extern s32 D_8016565C;
extern s32 D_80165660;
extern s32 D_80165664;

extern void func_800158E0(void);
extern void func_800167BC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_80145C0C(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(D_80164FC4);
    func_800167BC(D_80164FC8);
    func_800167BC(D_80164FCC);
    func_800167BC(D_80164FD0);
    func_800167BC(D_80165658);
    func_800167BC(D_8016565C);
    func_800167BC(D_80165660);
    func_800167BC(D_80165664);
    func_800167FC();
}
