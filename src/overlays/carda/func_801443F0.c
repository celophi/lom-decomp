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

extern s32 D_801229B0;
extern u16 D_8014B054;
extern u16 D_8014B090;
extern u16 D_8014B0DA;
extern u8 D_80165B89[];
extern u8 D_80165B91[];
extern u8 D_80165BA8[];
extern s32 D_80165F3C;
extern CardaElement D_80165F80;
extern s32 D_80165FE8;
extern s32 D_80165FEC;
extern s32 D_80165FF4;
extern s32 D_80166074;
extern s32 D_80166078;
extern s32 D_80166118;
extern u8 *D_801663A0;
extern s32 D_80166A80;
extern s32 D_80166ADC;

extern void func_80143DF4(void);
void func_80142E10(s32 *arg);
s32 func_80146694(void);
void func_80149554(void);

/**
 * @brief CARDA card-select redraw dispatch (companion of func_80144050).
 * @note WIP - NOT byte-matching yet (~77%). Structure/tail match; the call
 *       section's saved-register allocation (prim/base/result must share one
 *       saved reg the way the original does, plus a per-branch const-2 reg
 *       split) is a coupled allocation that needs a two-change-at-once fix.
 * @param ot   ordering table pointer.
 * @param prim primitive/packet buffer cursor (also the running accumulator).
 * @param arg2 x base offset.
 * @param arg3 y base offset.
 * @return final func_800A88A0 result.
 */
s32 func_801443F0(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 x;
    s32 code;
    u8 *base;
    CardaElement *p;

    if (D_80166074 >= 0xD)
    {
        if ((u32)(D_80166078 - 2) < 2U)
        {
            x = -arg2 + 0x90;
            base = (u8 *)&D_8014B0DA - 0xA2;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0DA - 0xA2 + D_8014B0DA), 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x7A), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
        }
        else
        {
            x = -arg2 + 0x90;
            base = (u8 *)&D_8014B054 - 0x1C;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B054 - 0x1C + D_8014B054), 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
        }
    }
    else
    {
        x = -arg2 + 0x90;
        base = (u8 *)&D_8014B090 - 0x58;
        prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B090 - 0x58 + D_8014B090), 4, x, -arg3, 2);
        if ((u32)(D_80166078 - 2) < 2U)
        {
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x7A), 4, x, 0xE - arg3, 2);
        }
        else
        {
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
        }
        prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    }

    if (D_80166074 == 0xC)
    {
        func_80149554();
        D_801663A0 = D_80165BA8;
    }
    else if (D_80166074 >= 0xD)
    {
        D_80165FF4 = 0;
        D_80166A80 = 1;
        func_80142E10(&D_80166074);
        D_80166ADC = 0;
        D_80166118 = 1;
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_80165FE8 = 0;
            D_801229B0 = 5;
            D_80165F3C = 1;
            func_80146694();
            D_801663A0 = D_80165B91;
            D_80165FEC = 0xF3;
            D_80165F80.attr.f.state = 0;
        }
        else
        {
            D_801663A0 = D_80165B89;
            p = &D_80165F80;
            p->draw_handler = func_80143DF4;
            p->attr.f.unk0_3 = 1;
            p->attr.f.state = 2;
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x5A;
            p->attr4.word = ((p->attr4.word | 1) & ~0x1FE) | 0x58;
            code = p->attr.word;
            code &= 0x00FFFFFF;
            code |= 0x20000000;
            p->attr.word = code;
        }
    }
    D_80166074 += 1;
    return prim;
}
