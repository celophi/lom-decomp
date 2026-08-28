#include "common.h"

typedef struct
{
    s32 word;
} AddheroAttrWord;

typedef struct
{
    AddheroAttrWord attr;
    s32 unk4;
    s32 unk8;
} AddheroPacket;

extern s32 D_80160920;
extern AddheroPacket D_8016094C;
extern s32 D_80122988;
extern s32 D_801609BC;
extern s32 D_80160928;
extern s32 D_80160938;

extern void func_80140D60(void);
extern void func_80140790(void);
extern void func_801408B0(void);

void func_801406A8(void)
{
    s32 delta;

    func_80140D60();
    D_80160920 += 2;
    if ((D_8016094C.attr.word & 0x7F) == 2)
    {
        func_80140790();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_801408B0();
    if (D_801609BC != 0)
    {
        s32 base = D_80160928;
        delta = (D_80160938 - D_80160928) / D_801609BC;
        D_801609BC -= 1;
        D_80160928 += delta;
    }
    else
    {
        D_80160928 = D_80160938;
    }
}
