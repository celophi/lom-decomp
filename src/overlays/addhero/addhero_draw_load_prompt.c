#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct AddheroElement {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void *draw_handler;
    s32 unkC;
} AddheroElement;

extern u16 D_80146FD4;
extern AddheroElement D_80160940;
extern s32 D_801609A4;
extern u8 *D_80165488;
extern s32 D_801609B0;
extern u8 D_80160588[];
extern s32 D_80160934;
extern u8 D_80160590[];
extern s32 D_80122988;

s32 func_80144018(s32 result, s32 *ot, s32 x, s32 y);
s32 func_80145878(void);
void func_801449F0(void);
s32 func_8014280C(s32 *ot, s32 prim, s32 arg2, s32 arg3);

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

s32 func_80142618(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    AddheroElement *p;

    x = -arg2 + 0x90;
    result = func_80144018(
        func_800A88A0(prim, ot,
                      (u8 *)&D_80146FD4 + D_80146FD4 - 0x30,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80145878() - 1) < 2U)
    {
        D_80160940.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        D_801609A4 = 0xFF;
        func_801449F0();
        D_80165488 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80160940.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            D_80165488 = D_80160588;
        }
        else if (status & 0x220)
        {
            if (D_801609B0 != 0)
            {
                D_80160940.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                D_80165488 = D_80160588;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80160934 = 1;
                D_80165488 = D_80160590;
                p = &D_80160940;
                p->draw_handler = func_8014280C;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x61;
                p->unk4_0 = 1;
                p->y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}
