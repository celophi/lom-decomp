#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct CardaElement {
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
} CardaElement;

extern u16 D_8014B04E;
extern u16 D_8014B050;
extern CardaElement D_80165F80;
extern s32 D_80165FEC;
extern s32 D_80166078;
extern s32 D_80122988;
extern s32 D_80165FF8;
extern s32 D_80166ADC;
extern s32 D_80166118;
extern u8 *D_801663A0;
extern u8 D_80165B84[];
extern u8 D_80165B88[];
extern u8 D_80165B89[];
extern u8 D_80165B90[];
extern u8 D_80165B91[];

s32 func_80144F28(s32 result, s32 *ot, s32 x, s32 y);
s32 func_80149638(void);
void func_80147C5C(void);
s32 func_80143DF4(s32 *ot, s32 prim, s32 arg2, s32 arg3);

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

s32 func_801439B4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    CardaElement *p;

    x = -arg2 + 0x90;
    result = func_80144F28(
        func_800A88A0(prim, ot,
                      (u8 *)&D_8014B04E + D_8014B04E - 0x16,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        D_80165F80.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_801663A0 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80165F80.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            D_801663A0 = D_80165B84;
        }
        else if (status & 0x220)
        {
            if (D_80165FF8 != 0)
            {
                D_80165F80.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                D_801663A0 = D_80165B84;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80166ADC = 0;
                D_80166118 = 1;
                if ((u32)(D_80166078 - 2) < 2U)
                    D_801663A0 = D_80165B91;
                else
                    D_801663A0 = D_80165B89;

                p = &D_80165F80;
                p->draw_handler = func_80143DF4;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x5A;
                p->unk4_0 = 1;
                p->y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}

s32 func_80143BD4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    CardaElement *p;

    x = -arg2 + 0x90;
    result = func_80144F28(
        func_800A88A0(prim, ot,
                      (u8 *)&D_8014B050 + D_8014B050 - 0x18,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        D_80165F80.attr.f.state = 0;
        func_800AA02C();
        func_800A3938(0x78, 0x80);
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_801663A0 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80165F80.attr.f.state = 0;
            func_800AA02C();
            func_800A3938(0x78, 0x80);
            D_801663A0 = D_80165B84;
        }
        else if (status & 0x220)
        {
            if (D_80165FF8 != 0)
            {
                D_80165F80.attr.f.state = 0;
                func_800AA02C();
                func_800A3938(0x78, 0x80);
                D_801663A0 = D_80165B84;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80166ADC = 0;
                D_80166118 = 1;
                if ((u32)(D_80166078 - 2) < 2U)
                    D_801663A0 = D_80165B90;
                else
                    D_801663A0 = D_80165B88;

                p = &D_80165F80;
                p->draw_handler = func_80143DF4;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x5A;
                p->unk4_0 = 1;
                p->y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}
