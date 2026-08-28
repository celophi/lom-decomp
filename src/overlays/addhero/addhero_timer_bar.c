#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s32 unk14;
    s16 unk18;
    s16 unk1A;
    s32 unk1C;
    s16 unk20;
    s16 unk22;
} AddheroPolyG4Words;

extern s32 D_80164A4C;
extern s32 D_80164A54;

s32 func_80142A0C(s32 arg0, s32 *arg1)
{
    AddheroPolyG4Words *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (AddheroPolyG4Words *)arg0;
    if (D_80164A4C != 0)
    {
        elapsed = func_8002054C(-1) - D_80164A54;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        g->unk4 = 0xFF;
        g->unkC = 0xFFFF;
        g->unk1C = 0xFF0000;
        ((u8 *)g)[3] = 8;
        g->unk14 = color;
        ((u8 *)g)[7] = 0x38;
        g->unk18 = 0;
        g->unk8 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        g->unk20 = extent >> 8;
        g->unk10 = extent >> 8;
        g->unk12 = 0;
        g->unkA = 0;
        g->unk22 = 0x2C;
        g->unk1A = 0x2C;
        g->unk0 = (g->unk0 & 0xFF000000) | (*arg1 & 0xFFFFFF);
        *arg1 = (*arg1 & 0xFF000000) | (arg0 & 0xFFFFFF);
        arg0 += 0x24;
    }
    return arg0;
}
