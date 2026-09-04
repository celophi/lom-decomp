#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern u8 D_80122C02;
extern s32 D_80122C08;

extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);
extern void func_800C8E2C(void);

void func_800C8FA8(void)
{
    s32 i;
    s32 result;
    u8 *base;
    u8 *arg;
    u8 *p;

    result = 0;
    D_80122C02 = 0xFF;
    i = 0;
    p = g_menuLayoutBuffer;
    arg = p + 0x3160;
    base = p;

loop:
    if (base[0x3160] == 0 && *(s32 *)&base[0x3194] != 0) {
        base[0x3160] = base[0x3180];
        func_800B2844(0, arg, 0xFF);
        result = *(s32 *)&base[0x3194];
        *(s32 *)&base[0x3194] = 0;
        D_80122C02 = i;
    } else {
        arg += 0x40;
        i++;
        base += 0x40;
        if (i < 4) {
            goto loop;
        }
    }

    D_80122C08 = result;
    if (result == 0) {
        func_800C8E2C();
    }
}
