#include "common.h"

typedef struct CardaElementHead {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
} CardaElementHead;

extern s32 D_80166068;
extern CardaElementHead D_80165F8C;
extern s32 D_80122988;
extern s32 D_80165FFC;
extern s32 D_80166104;
extern s32 D_80165F38;

void func_80140830(void)
{
    s32 delta;

    func_80141230();
    D_80166068 += 2;
    if ((D_80165F8C.attr.word & 0x7F) == 2)
    {
        func_80140918();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_80140BAC();
    if (D_80165FFC != 0)
    {
        s32 base = D_80166104;
        delta = (D_80165F38 - D_80166104) / D_80165FFC;
        D_80165FFC -= 1;
        D_80166104 += delta;
    }
    else
    {
        D_80166104 = D_80165F38;
    }
}
