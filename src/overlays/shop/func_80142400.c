#include "common.h"

typedef struct { u16 id; u16 count; s32 value; } ShopEntry;
typedef struct { s16 x; s16 y; s16 w; s16 h; } ShopRect;

extern s32 D_801451D0;
extern s32 D_801451D4;
extern s32 D_801451D8;
extern s32 D_80145238;
extern s32 D_8014523C;
extern s32 D_80122988;
extern s32 D_8012271C;
extern s32 D_80145240;
extern s32 D_80145244;
extern s32 D_80145250;
extern s32 D_80145CDC;
extern u8 D_800EC3F8[];
extern void func_80142284();

extern s32 field_find_free_inventory_record(s32);
extern void field_copy_inventory_record(s32, s32);
extern void func_800A3938();
extern s32 func_800A88A0(s32 prim, s32 *ot, void *text, s32 color, s32 x, s32 y, s32 mode);
extern void field_reset_input_repeat();

/**
 * @brief Apply a pending shop quantity confirmation and draw the amount prompt.
 *
 * When the shop is in the quantity-select sub-state ((D_801451D8 & 7) == 2) and
 * a confirm/cancel button is pending in @ref D_80122988, this commits the
 * purchase or sale: for item-slot entries it clamps the quantity, runs the
 * per-unit slot loop (@ref field_copy_inventory_record), and for both entry kinds deducts the
 * money and decrements the remaining count, retiring the entry when it reaches
 * zero. A successful change (var_s7) rearms the confirmation widget and refreshes
 * it via @ref field_reset_input_repeat. Always draws the three quantity-prompt glyphs.
 *
 * @param ot   Ordering-table pointer used for emitted primitives.
 * @param prim Primitive-buffer write cursor.
 * @param arg2 Horizontal layout offset (subtracted from every x).
 * @param arg3 Vertical layout offset (subtracted from every row y).
 * @return Advanced primitive-buffer write cursor.
 *
 * @note WIP: 90.26% match. Structure, control flow, field reads and reloads are
 *       correct; the residual is a coupled register-coloring permutation (prim
 *       lands in $s7 rather than $s2, etc.) plus constant hold-vs-rematerialize
 *       differences in the rearm block, still to be closed before it matches.
 * @see decomp.me (90.26%) TODO: no scratch link yet
 */
s32 func_80142400(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    ShopRect pos;
    u16 *entry;
    s32 var_s1;
    s32 var_s7;
    s32 var_s0;
    ShopEntry *var_a0;
    s16 var_v0;
    u8 *glyph;
    u8 *base;
    u8 *p1;
    u8 *p2;
    u8 *p3;
    s32 color;
    s32 y;
    s32 y2;
    s32 status;

    var_s7 = 0;
    if ((D_801451D8 & 7) == 2)
    {
        status = D_80122988;
        if (status & 0xF000)
        {
            func_800A3938(0x7D, 0x80);
            D_801451D0 ^= 1;
            goto render;
        }
        if ((status & 0x220) && (D_801451D0 == 0))
        {
            var_s1 = field_find_free_inventory_record(0x7D);
            entry = (u16 *)((D_80145CDC * 8) + D_80145250);
            if (*entry & 0x8000)
            {
                s32 temp_v0 = var_s1 - 0x25E0;
                s32 temp_v1 = (D_8012271C - temp_v0) >> 6;
                if (temp_v1 < D_80145240)
                {
                    D_80145240 = temp_v1;
                    var_s7 = 1;
                }
                var_s0 = 0;
                if (D_80145240 > 0)
                {
                    do
                    {
                        var_s0++;
                        field_copy_inventory_record(var_s1, D_80145244 + ((*(u16 *)((D_80145CDC * 8) + D_80145250) & 0x7FFF) << 6));
                        var_s1 += 0x40;
                    } while (var_s0 < D_80145240);
                }
                func_800A3938(0xB4, 0x80);
                var_a0 = (ShopEntry *)((D_80145CDC * 8) + D_80145250);
                *(s32 *)(D_8012271C + 0x2C) = *(s32 *)(D_8012271C + 0x2C) - var_a0->value * D_80145240;
                if (var_a0->count != 0)
                {
                    var_v0 = var_a0->count - *(u16 *)&D_80145240;
                    goto set_count;
                }
            }
            else
            {
                s32 temp_a0 = 0x63 - *(u8 *)(D_8012271C + *entry + 0x25E0);
                if (temp_a0 < D_80145240)
                {
                    D_80145240 = temp_a0;
                    var_s7 = 1;
                }
                {
                    u8 q = *(u8 *)&D_80145240;
                    *(u8 *)(D_8012271C + *entry + 0x25E0) += q;
                    func_800A3938(0xB4, 0x80, q);
                }
                var_a0 = (ShopEntry *)((D_80145CDC * 8) + D_80145250);
                *(s32 *)(D_8012271C + 0x2C) = *(s32 *)(D_8012271C + 0x2C) - var_a0->value * D_80145240;
                if (var_a0->count != 0)
                {
                    var_v0 = var_a0->count - *(u16 *)&D_80145240;
set_count:
                    var_a0->count = var_v0;
                    if ((var_v0 & 0xFFFF) == 0)
                    {
                        var_a0->id = 0xFFFF;
                    }
                }
            }
            D_80145240 = 1;
            D_801451D4 = 0;
            D_801451D8 &= ~7;
            if (var_s7 != 0)
            {
                D_801451D8 = (((((D_801451D8 & ~7) | 1) & ~0x78) | 8) & 0xFFFF007F) | 0x1000;
                D_8014523C = 1;
                *(u8 *)((u8 *)&D_801451D8 + 2) = 0x70;
                *(s32 *)((u8 *)&D_801451D8 + 8) = (s32)&func_80142284;
                D_80145238 = 1;
                *(s32 *)((u8 *)&D_801451D8 + 4) = ((*(s32 *)((u8 *)&D_801451D8 + 4) | 1) & ~0x1FE) | 0x20;
                D_801451D8 &= 0xFFFFFF;
                field_reset_input_repeat(1, 0xFFFFFF);
            }
            goto call_finish;
        }
        status = D_80122988;
        if ((status & 0x40) || ((status & 0x220) && (D_801451D0 != 0)))
        {
            D_801451D8 &= ~7;
            func_800A3938(0x7F, 0x80);
            D_801451D4 = 0;
call_finish:
            field_reset_input_repeat();
        }
    }

render:
    y = arg3;
    glyph = D_800EC3F8;
    {
        s32 hi = glyph[1] << 8;
        s32 lo;
        base = glyph - 0x34;
        lo = D_800EC3F8[0];
        p1 = (u8 *)(lo + (hi + (s32)base));
    }
    prim = func_800A88A0(prim, ot, p1, 4, 0x60 - arg2, -y, 2);

    color = 4;
    {
        s32 hi = base[0x37] << 8;
        s32 lo = base[0x36];
        p2 = (u8 *)(lo + (hi + (s32)base));
    }
    if (D_801451D0 != 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, p2, color, 0x48 - arg2, (y2 = 0x10 - y), 1);

    color = 5;
    {
        s32 hi = base[0x39] << 8;
        s32 lo = base[0x38];
        p3 = (u8 *)(lo + (hi + (s32)base));
    }
    if (D_801451D0 != 0)
    {
        color = 4;
    }
    return func_800A88A0(prim, ot, p3, color, 0x68 - arg2, y2, 0);
}
