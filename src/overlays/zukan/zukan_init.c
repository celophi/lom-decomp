#include "common.h"

extern s32 D_80157D38;
extern s32 D_80157D58;
extern s32 D_80157D60;
extern s32 D_80157D64;
extern s32 D_80157D68;
extern s32 D_80157D6C;
extern s32 D_80157D70;
extern s32 D_80157D74;

s32 func_80141094(s32 arg0, s32 arg1)
{
    s32 ret;
    volatile s32 pad[2];

    D_80157D38 = arg1;
    D_80157D6C = (arg0 + 3) & ~3;
    func_80141144((ret = arg0 + 0x8000, arg0));
    func_800AA02C();
    func_801424D0(0x100, 0x100, 0x100, 6);
    D_80157D64 = 0;
    D_80157D58 = 1;
    D_80157D60 = 0;
    D_80157D74 = 0;
    D_80157D70 = 0;
    D_80157D68 = 0;
    func_801427E0(arg1);
    do {
        do {
            return ret;
        } while (0);
    } while (0);
}
