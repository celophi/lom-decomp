#include "common.h"

extern s32 D_80166ADC;
extern s32 D_80166AE0;
extern s32 D_80166B8C;
extern s32 D_80166B94;
extern s32 D_80166B98;
extern s32 D_80166B9C;
extern s32 D_80166BA0;
extern s32 D_80166108;
extern s32 D_8016610C;
extern s32 D_80166110;
extern s32 D_80166114;

extern void func_800158E0(void);
extern s32 func_800167AC(s32, s32, s32, s32);
extern void func_800167DC(s32);
extern void func_800167EC(void);
extern void func_800167FC(void);
extern s32 func_8002054C(s32);

/** @see decomp.me (100.00%) */
void func_80149690(void)
{
    func_800158E0();
    func_800167EC();
    D_80166B94 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    D_80166B98 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    D_80166B9C = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    D_80166BA0 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    D_80166108 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    D_8016610C = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    D_80166110 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    D_80166114 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(D_80166B94);
    func_800167DC(D_80166B98);
    func_800167DC(D_80166B9C);
    func_800167DC(D_80166BA0);
    func_800167DC(D_80166108);
    func_800167DC(D_8016610C);
    func_800167DC(D_80166110);
    func_800167DC(D_80166114);
    func_800167FC();
    D_80166B8C = func_8002054C(-1);
    D_80166ADC = 0;
    D_80166AE0 = 0;
}
