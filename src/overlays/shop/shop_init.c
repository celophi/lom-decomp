#include "common.h"

extern s32 D_801451B8;
extern s32 D_801451BC;
extern s32 D_801451C0;
extern s32 D_801451C4;
extern s32 D_801451C8;
extern s32 D_801451CC;

extern void func_80067F8C(void);
extern void func_800AA02C(void);
extern void field_text_reset_scratch(void);
extern void field_text_reset_windows(void);
extern void func_80063194(void);
extern s32 func_80140164(s32);
extern s32 func_801404A8(s32, s32, s32, s32);
extern void func_80140798();

void func_80140004(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    D_801451CC = arg0;
    arg0 += 0x8008;
    func_80067F8C();
    D_801451BC = 0;
    D_801451C8 = 0;
    D_801451C4 = arg1;
    D_801451B8 = arg5;
    func_800AA02C();
    if (D_801451C4 != 0)
    {
        arg0 = func_801404A8(arg0, arg2, arg3, arg4);
    }
    else
    {
        arg0 = func_80140164(arg0);
    }
    D_801451C0 = arg0;
}

s32 func_801400D4(s32 arg0)
{
    u8 *p;

    p = (u8 *)D_801451CC + D_801451BC * 0x4004;
    *(void **)(p + 0x4000) = p;
    field_text_reset_scratch();
    func_80140798(arg0, p);
    func_80063194();
    D_801451BC ^= 1;
    if (D_801451C8 != 0)
    {
        field_text_reset_windows();
    }
    return D_801451C8;
}
