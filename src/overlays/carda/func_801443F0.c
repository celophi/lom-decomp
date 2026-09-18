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
void func_80142E10(void);
s32 func_80146694(void);
void func_80149554(void);

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

/**
 * @brief Draw the active CARDA glyphs and advance the associated state.
 * @param ot Ordering table used for the glyph primitives.
 * @param prim Current primitive packet cursor.
 * @param arg2 Horizontal placement offset.
 * @param arg3 Vertical placement offset.
 * @return Updated primitive packet cursor.
 */
s32 func_801443F0(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    if (D_80166074 >= 0xD)
    {
        if ((u32)(D_80166078 - 2) < 2U)
        {
            s32 x;
            u8 *base;

            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0DA - 0xA2 + D_8014B0DA), 4, x, -arg3, 2);
            base = (u8 *)&D_8014B0DA - 0xA2;
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x7A), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
        }
        else
        {
            s32 x;
            u8 *base;

            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B054 - 0x1C + D_8014B054), 4, x, -arg3, 2);
            base = (u8 *)&D_8014B054 - 0x1C;
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
        }
    }
    else
    {
        s32 x;
        s32 mode = 2;
        u8 *base;
        u16 *anchor;

        x = -arg2 + 0x90;
        anchor = &D_8014B090;
        base = (u8 *)anchor - 0x58;
        prim = func_800A88A0(prim, ot, base + *anchor, 4, x, -arg3, mode);
        if ((u32)(D_80166078 - 2) < 2U)
        {
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x7A), 4, x, 0xE - arg3, mode);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, mode);
        }
        else
        {
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, mode);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, mode);
        }
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
        func_80142E10();
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
            CardaElement *p;

            D_801663A0 = D_80165B89;
            p = &D_80165F80;
            p->draw_handler = func_80143DF4;
            p->attr.f.unk0_3 = 1;
            p->attr.f.state = 2;
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x5A;
            p->unk4_0 = 1;
            p->y = 0x2C;
            SET_ELEM_CODE(p, 0x20);
        }
    }
    D_80166074 += 1;
    return prim;
}
