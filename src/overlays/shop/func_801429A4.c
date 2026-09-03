#include "common.h"

typedef struct { u16 id; u16 count; s32 value; } ShopEntry;
typedef struct { s16 x; s16 y; s16 w; s16 h; } ShopRect;

extern s32 D_801451D0;
extern s32 D_801451D4;
extern s32 D_801451D8;
extern s32 D_80122988;
extern s32 D_8012271C;
extern s32 D_80145240;
extern s32 D_80145244;
extern s32 D_80145250;
extern s32 D_80145CDC;
extern u8 D_800EC3F8[];

extern void func_800A3938();
extern s32 func_800A88A0(s32, s32 *, void *, s32, s32, s32, s32);
extern void func_800AA02C(void);

/**
 * @see (100%)
 */
s32 func_801429A4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    ShopRect pos;
    u16 *entry;
    u16 *entry2;
    u8 *item;
    u8 *base;
    u8 *glyph;
    u8 *p1;
    u8 *p2;
    u8 *p3;
    s32 value;
    s32 color;
    s32 status;
    s32 y;
    s32 y2;
    s32 state;

    state = D_801451D8;
    y = arg3;
    if ((state & 7) == 2)
    {
        status = D_80122988;
        if (status & 0xF000)
        {
            func_800A3938(0x7D, 0x80);
            D_801451D0 ^= 1;
            goto render;
        }

        if ((status & 0x220) && D_801451D0 == 0)
        {
            entry = (u16 *)((D_80145CDC * 8) + D_80145250);
            if (*entry & 0x8000)
            {
                *(u8 *)((((*entry & 0x7FFF) << 6) + D_80145244)) = 0;
                *(u16 *)((D_80145CDC * 8) + D_80145250) = 0xFFFFU;
            }
            else
            {
                item = (u8 *)D_8012271C;
                item += *entry;
                item[0x25E0] -= *(u8 *)&D_80145240;
                entry2 = (u16 *)((D_80145CDC * 8) + D_80145250);
                {
                    u16 count2;
                    s32 data_base;
                    count2 = entry2[1];
                    data_base = D_8012271C;
                    entry2[1] = count2 - *(u16 *)&D_80145240;
                    if (*(u8 *)(data_base + entry2[0] + 0x25E0) == 0)
                        entry2[0] = 0xFFFFU;
                }
            }
            func_800A3938(0xB4, 0x80);
            value = *(u32 *)(D_8012271C + 0x2C) +
                *(s32 *)((D_80145CDC * 8) + D_80145250 + 4) * D_80145240;
            *(s32 *)(D_8012271C + 0x2C) = value;
            if ((u32)value > 0x989680U)
                *(u32 *)(D_8012271C + 0x2C) = 0x989680U;
            D_80145240 = 1;
            D_801451D4 = 0;
            D_801451D8 &= ~7;
            goto input_done;
        }

        status = D_80122988;
        if ((status & 0x40) || ((status & 0x220) && D_801451D0))
        {
            D_801451D8 &= ~7;
            func_800A3938(0x7F, 0x80);
            D_801451D4 = 0;
input_done:
            func_800AA02C();
        }
    }

render:
    glyph = D_800EC3F8;
    { s32 hi; s32 lo; hi = glyph[1] << 8; base = glyph - 0x34; lo = D_800EC3F8[0]; p1 = (u8 *)(lo + (hi + (s32)base)); }
    prim = func_800A88A0(prim, ot, p1, 4, 0x60 - arg2, -y, 2);

    color = 4;
    { s32 hi; s32 lo; hi = base[0x37] << 8; lo = base[0x36]; p2 = (u8 *)(lo + (hi + (s32)base)); }
    if (D_801451D0 != 0) color = 5;
    prim = func_800A88A0(prim, ot, p2, color, 0x58 - arg2, (y2 = 0x10 - y), 1);

    color = 5;
    { s32 hi; s32 lo; hi = base[0x39] << 8; lo = base[0x38]; p3 = (u8 *)(lo + (hi + (s32)base)); }
    if (D_801451D0 != 0) color = 4;
    prim = func_800A88A0(prim, ot, p3, color, 0x68 - arg2, y2, 0);
    return prim;
}
