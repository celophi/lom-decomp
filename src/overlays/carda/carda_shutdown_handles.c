#include "common.h"

extern s32 D_80166108;
extern s32 D_8016610C;
extern s32 D_80166110;
extern s32 D_80166114;
extern s32 D_80166B94;
extern s32 D_80166B98;
extern s32 D_80166B9C;
extern s32 D_80166BA0;

extern void func_800158E0(void);
extern void func_800167BC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);

/** @see decomp.me (100.00%) */
void func_8014986C(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(D_80166B94);
    func_800167BC(D_80166B98);
    func_800167BC(D_80166B9C);
    func_800167BC(D_80166BA0);
    func_800167BC(D_80166108);
    func_800167BC(D_8016610C);
    func_800167BC(D_80166110);
    func_800167BC(D_80166114);
    func_800167FC();
}
