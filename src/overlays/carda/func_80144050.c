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
    union {
        u32 word;
        struct {
            u32 unk4_0 : 1;
            u32 y : 8;
            u32 unk4_9 : 23;
        } f;
    } attr4;
    void *draw_handler;
    s32 unkC;
} CardaElement;

extern s32 D_80122988;
extern u16 D_8014B05E;
extern u16 D_8014B0B4;
extern u16 D_8014B0EC;
extern CardaElement D_80165F80;
extern s32 D_80165FEC;
extern s32 D_80165FF8;
extern s32 D_80166074;
extern s32 D_80166078;
extern s32 D_801660F8;
extern u8 *D_801663A0;
extern u8 D_80165B84[];

extern void func_801443F0(void);

s32 func_80144F28(s32 result, s32 *ot, s32 x, s32 y);
s32 func_80149638(void);
void func_80147C5C(void);

s32 func_80144050(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 x;
    s32 status;
    s32 code;
    u8 *base;
    CardaElement *p;
    s32 aa3;

    aa3 = arg3;
    if ((u32)(D_80166078 - 2) < 2U)
    {
        prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0B4 - 0x7C + D_8014B0B4), 4, -arg2 + 0x90, -aa3, 2);
        base = (u8 *)&D_8014B0B4 - 0x7C;
        prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x24), 4, -arg2 + 0x90, 0xE - aa3, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0EC - 0xB4 + D_8014B0EC), 4, -arg2 + 0x90, -aa3, 2);
    }
    x = -arg2 + 0x90;
    prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B05E - 0x26 + D_8014B05E), 4, x, 0x1C - aa3, 2);
    prim = func_80144F28(prim, ot, x, 0x2A - aa3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        func_800A3938(0x7D, 0x80);
        D_80165F80.attr.f.state = 0;
        func_800AA02C();
        D_80165FEC = 0xFF;
        func_80147C5C();
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_801663A0 = D_80165B84;
        }
        else
        {
            D_801663A0 = 0;
        }
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            goto block_common;
        }
        if (!(status & 0x220))
        {
            goto done;
        }
        if (D_80165FF8 == 0)
        {
            goto draw;
        }
    block_common:
        D_801660F8 = 1;
        func_800A3938(0x7D, 0x80);
        D_80165F80.attr.f.state = 0;
        func_800AA02C();
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_80165FEC = 0xF9;
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_801663A0 = 0;
        }
        else
        {
            D_801663A0 = D_80165B84;
        }
        goto done;
    draw:
        func_800A3938(0x7E, 0x80);
        D_80165F80.attr.f.state = 0;
        func_80147C5C();
        D_80166074 = 0;
        p = &D_80165F80;
        p->attr.f.unk0_3 = 1;
        p->attr.f.state = 1;
        if ((u32)(D_80166078 - 2) < 2U)
        {
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x4C;
            code = p->attr.word;
            code &= 0x00FFFFFF;
            code |= 0x20000000;
            p->attr.word = code;
            p->attr4.word = ((p->attr4.word | 1) & ~0x1FE) | 0x90;
        }
        else
        {
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x5A;
            code = p->attr.word;
            code &= 0x00FFFFFF;
            code |= 0x20000000;
            p->attr.word = code;
            p->attr4.word = ((p->attr4.word | 1) & ~0x1FE) | 0x58;
        }
        p->draw_handler = func_801443F0;
    }
done:
    return prim;
}
