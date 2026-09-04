#include "common.h"
extern s32 D_80122988;
extern s32 D_8012298C;
extern unsigned short D_8014B4D4[];
extern u8 D_80165F48[];
extern s32 D_80165F38;
extern s32 D_80165F80[];
extern s32 D_80165FE8;
extern s32 D_80165FFC;
extern s32 D_80166104;
s32 func_800A88A0();
void func_800A3938();
void func_80067F5C();

s32 func_80146EDC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 *p;
    s32 delta;
    s32 i;
    s32 result;
    s32 j;
    struct { short x, y, w, h; } pos;

    result = arg1;
    i = 0;
    if (D_80165FE8 > 0) {
        do {
            delta = i * 0xE - D_80166104;
            if ((u32)(delta + 0xD) < 0x9D) {
                result = func_800A88A0(result, arg0,
                    (void *)((u8 *)D_8014B4D4 + D_8014B4D4[D_80165F48[i]]),
                    4, -arg2 + 0x80, delta - arg3, 2);
            }
            i++;
        } while (i < D_80165FE8);
    }

    if (D_80165FFC == 0) {
        if (D_80122988 & 0x1000) {
            if (D_80166104 != 0) {
                func_800A3938(0x7D, 0x80);
                D_80165FFC = 4;
                D_80165F38 -= 0xE;
            }
        } else if (D_80122988 & 0x4000) {
            if ((D_80165FE8 * 0xE - D_80166104) >= 0x8D) {
                func_800A3938(0x7D, 0x80);
                D_80165FFC = 4;
                D_80165F38 += 0xE;
            }
        } else if (D_80122988 & 0x220) {
            func_800A3938(0x7E, 0x80);
            D_8012298C = 0x20;
            p = D_80165F80;
            j = 0;
            do {
                *p &= ~7;
                j++;
                p += 3;
            } while (j < 8);
            func_80067F5C(8);
        }
    }
    return result;
}
