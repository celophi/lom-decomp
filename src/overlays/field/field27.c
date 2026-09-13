/**
 * @file field27.c
 * @brief Field HUD gauge/bar renderer, carved from the middle of the unk2_f
 *        fragment (the single-function slot between func_80084700 and
 *        func_80085D30).
 */

#include "common.h"
#include "sdk/libgpu.h"

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u8 pad_0c[0x48 - 0x0C];
    u16 unk48;
    u16 unk4a;
    union
    {
        u32 unk4c;
        struct
        {
            u8 b4c;
            u8 unk4d;
            u8 b4e;
            u8 b4f;
        } bytes;
    } u4c;
    u8 pad_50[0x23C - 0x50];
} State23C;

typedef struct
{
    u8 pad0[0x259];
    u8 unk259;
    u8 pad_25a[0x25E - 0x25A];
    s16 unk25e;
    s16 unk260;
    u8 pad_262[0x268 - 0x262];
} Entry268;

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2a;
    u8 pad_2c[0x54 - 0x2C];
} Rec54;

#define B(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define H(p, o) (*(s16 *)((u8 *)(p) + (o)))
#define UH(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define W(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define PTR(p, o) (*(u8 **)((u8 *)(p) + (o)))

u8 *func_80085D30(u8 *, u8 *, u32 *, u16, s32, s32);
u8 *func_80085E84(u8 *, u8 *, u32, s16 *);
u8 *func_80086030(u8 *, s32, u8 *);
u8 *func_800860CC(u8 *, s32, u8 *);
u8 *func_80086184(u8 *, u8 *, u8, s16 *);
s32 rand(void);

extern u32 D_800EAFD8[];
extern u32 D_800EAFEC[];
extern u32 D_800EAFF4[];
extern u32 D_800EAFFC[];
extern u32 D_800EB004[];
extern s16 D_800EB058[];
extern Entry268 D_800FD818[];
extern u8 D_800FDCEA;
extern Rec54 D_800FDF58[];
extern State23C D_80105AE0[];
extern s32 D_801077F0[];
extern s32 D_8010A000;
extern s32 D_8010A004;
extern s32 D_8010A008;
extern s32 D_8010A00C;
extern s32 D_8010A010;
extern u8 D_80117EC8[];
extern s32 g_frame_counter;

typedef struct
{
    s16 x;
    s16 y;
} HudPoint;

/**
 * @brief Emit the per-track HUD gauge primitives (background bar, fill bar,
 *        delta/tick markers and the closing draw-mode primitive) for one
 *        actor slot, animating the gauge value toward its target.
 * @param x Base screen x for the gauge group.
 * @param y Base screen y (adjusted by the per-slot shake table).
 * @param slot Track/slot index (0..2 use the extended blink/shake path).
 * @param render_context Pointer to the primitive-buffer cursor cell; updated on return.
 * @param value_per_bar Gauge full-scale denominator used for value-to-width scaling.
 * @see decomp.me (100%)
 */
void func_80084D08(s32 x, s32 y, s32 slot, u8 *render_context, u32 value_per_bar)
{
    s16 *scratch = (s16 *)0x1F800000;
    u8 *ctx;
    u8 *call_ctx;
    u32 special_width;
    s32 scan_slot;
    u32 *partial_palette;
    u32 *full_palette;
    u32 alternate_layout;
    s16 full_x;
    s16 full_y;
    s16 full_right;
    s32 partial_width;
    s16 rising_base_x;
    s16 falling_base_x;
    s16 rising_right;
    s16 falling_right;
    s16 blink_uv;
    s32 *full_color_entry;
    s32 *blink_counter;
    s32 *blink_counters;
    s32 *partial_color_entry;
    s32 special_prim_addr;
    s32 rising_x;
    s32 falling_x;
    s32 full_color;
    s32 partial_prim_addr;
    s32 full_prim_addr;
    s32 partial_color;
    s32 palette_index;
    s32 gauge_ot_tag;
    s32 rising_value;
    s32 falling_value;
    s32 rising_packed_value;
    s32 falling_packed_value;
    u16 status_intensity;
    u32 number_current;
    u32 displayed_value;
    u32 packed_display_value;
    u32 partial_bar_count;
    u32 shake_flag;
    u32 full_palette_level;
    u32 value_or_bar_count;
    u32 rising_current;
    u32 rising_wrapped_current;
    u32 falling_current;
    u32 falling_wrapped_current;
    u32 number_maximum;
    u32 current_value;
    u32 draw_arg;
    u32 mask24;
    u32 fill_mask;
    s32 background_type;
    u8 gauge_type;
    Entry268 *actor_entry;
    Entry268 *entry_base;
    State23C *state;
    u8 *label_cursor;
    u8 *rising_cursor;
    u8 *helper_cursor;
    u8 *falling_cursor;
    u8 *rising_ot;
    u8 *falling_ot;
    u8 *sprite_cursor;
    u8 *gauge_cursor;

    if (slot < 3)
    {
        if (D_800FD818[slot].unk259 < 6U)
        {
            y += D_800EB058[D_800FD818[slot].unk259];
            if (D_800FD818[slot].unk259 != 0)
            {
                D_800FD818[slot].unk259--;
            }
            else
            {
                D_800FD818[slot].unk259 = 0xFF;
            }
        }
    }
    state = &D_80105AE0[slot];
    shake_flag = (u32)state->unk8 >> 0x1F;
    alternate_layout = shake_flag;
    if ((shake_flag != 0) && (D_8010A000 < 6))
    {
        y += D_800EB058[D_8010A000];
        if (D_8010A000 != 0)
        {
            D_8010A000--;
        }
        else
        {
            D_8010A000 = 0xFF;
        }
    }
    ctx = render_context;
    label_cursor = PTR(ctx, 0x40B8);
    if (state->u4c.bytes.unk4d < 3U)
    {
        D_8010A00C = 0x1D;
        D_8010A010 = 9;
        scratch[0] = x;
        scratch[1] = y;
        label_cursor = func_80086184(label_cursor, ctx + 0xC, state->u4c.bytes.unk4d, scratch);
        D_8010A008 = 0x36;
        D_8010A004 = 3;
    }
    else if (alternate_layout != 0)
    {
        D_8010A00C = 0xF;
        D_8010A010 = 6;
        D_8010A008 = 0xE3;
        D_8010A004 = 3;
    }
    else
    {
        D_8010A00C = 4;
        D_8010A010 = 6;
        D_8010A008 = 0x36;
        D_8010A004 = 3;
    }
    sprite_cursor = label_cursor;
    if (slot < 3)
    {
        if (D_801077F0[slot] < 6)
        {
            D_801077F0[slot]++;
            if (D_801077F0[slot] < 6)
            {
                goto build_first_prim;
            }
        }
        for (scan_slot = 0; scan_slot < 8; scan_slot++)
        {
            if (D_80117EC8[scan_slot] == 0xFF)
            {
                break;
            }
            if (D_80117EC8[scan_slot] == slot)
            {
                D_801077F0[slot] = 0;
                break;
            }
        }
        blink_counters = D_801077F0;
        blink_counter = blink_counters + slot;
        if ((*blink_counter != 0) && !(rand() & 0x3F))
        {
            *blink_counter = 0;
        }
    build_first_prim:
        W(sprite_cursor, 0x4) = 0x808080;
        B(sprite_cursor, 0x3) = 4;
        B(sprite_cursor, 0x7) = 0x64;
        H(sprite_cursor, 0x8) = (s16)(x + 0x22);
        H(sprite_cursor, 0xA) = (s16)(y - 6);
        if (D_801077F0[slot] < 6)
        {
            blink_uv = ((u16)D_801077F0[slot] * 0x10) + 0x2058;
        }
        else
        {
            blink_uv = 0x2058;
        }
        W(sprite_cursor, 0x10) = 0x100010;
        H(sprite_cursor, 0xC) = blink_uv;
        H(sprite_cursor, 0xE) = 0x7850;
        W(sprite_cursor, 0x0) = (s32)((W(sprite_cursor, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
        W(ctx, 0xC) = (s32)((W(ctx, 0xC) & 0xFF000000) | ((s32)sprite_cursor & 0xFFFFFF));
        sprite_cursor += 0x14;
        ((HudPoint *)scratch)->x = x + 0x30;
        scratch[1] = y;
        number_current = state->unk4;
        number_maximum = state->unk0;
        draw_arg = number_current * 0x64;
        if (number_maximum != 0)
        {
            draw_arg = draw_arg / number_maximum;
        }
        if ((draw_arg == 0) && (number_current != 0))
        {
            draw_arg = 1;
        }
        sprite_cursor = func_80085E84(sprite_cursor, ctx, draw_arg, scratch);
    }
    gauge_type = state->u4c.bytes.unk4d;
    helper_cursor = sprite_cursor;
    if (gauge_type < 2U)
    {
        if (state->u4c.unk4c & 1)
        {
            if (state->unk48 == 0xFF)
            {
                if (g_frame_counter & 8)
                {
                    draw_arg = (u32)D_800EAFFC;
                }
                else
                {
                    draw_arg = (u32)D_800EAFEC;
                }
            }
            else
            {
                draw_arg = (u32)D_800EAFEC;
            }
            status_intensity = state->unk48;
            call_ctx = ctx;
            helper_cursor = func_80085D30(helper_cursor, call_ctx, (u32 *)draw_arg, status_intensity, (s32)x, (s32)y);
        }
        else
        {
            call_ctx = ctx;
            draw_arg = (u32)D_800EAFF4;
            status_intensity = state->unk4a;
            helper_cursor = func_80085D30(helper_cursor, call_ctx, (u32 *)draw_arg, status_intensity, (s32)x, (s32)y);
        }
    }
    else if ((gauge_type == 2) && ((u8)D_800FDCEA >= 0x41U))
    {
        draw_arg = (u32)D_800EB004;
        helper_cursor = func_80085D30(helper_cursor, ctx, (u32 *)draw_arg, state->unk48, (s32)x, (s32)y);
    }
    gauge_cursor = helper_cursor;
    if (slot < 3)
    {
        entry_base = D_800FD818;
        actor_entry = &entry_base[slot];
        if ((actor_entry->unk260 != 0) && (D_800FDF58[slot].unk2a == 0x8E))
        {
            W(gauge_cursor, 0x4) = 0x202020;
            W(gauge_cursor, 0x14) = 0x202020;
            B(gauge_cursor, 0x3) = 8;
            B(gauge_cursor, 0x7) = 0x38;
            W(gauge_cursor, 0xC) = 0xFFFFFF;
            W(gauge_cursor, 0x1C) = 0xFFFFFF;
            special_width = (s16)actor_entry->unk260;
            special_width = (s32)(D_8010A008 * actor_entry->unk25e) / (s32)special_width;
            ((POLY_G4 *)gauge_cursor)->x0 = (u16)D_8010A00C + x;
            ((POLY_G4 *)gauge_cursor)->y1 = ((u16)D_8010A010 + y);
            ((POLY_G4 *)gauge_cursor)->x1 = ((u16)D_8010A00C + x) + special_width;
            ((POLY_G4 *)gauge_cursor)->x2 = ((u16)D_8010A00C + x) - 3;
            ((POLY_G4 *)gauge_cursor)->y0 = ((u16)D_8010A010 + y);
            ((POLY_G4 *)gauge_cursor)->y3 = ((POLY_G4 *)gauge_cursor)->y1 + (u16)D_8010A004;
            ((POLY_G4 *)gauge_cursor)->y2 = ((POLY_G4 *)gauge_cursor)->y3;
            ((POLY_G4 *)gauge_cursor)->x3 = ((u16)D_8010A00C + x) + special_width - 3;
            special_prim_addr = (s32)gauge_cursor & 0xFFFFFF;
            ((P_TAG *)gauge_cursor)->addr = ((P_TAG *)(ctx + 0xC))->addr;
            gauge_cursor += 0x24;
            gauge_ot_tag = (W(ctx, 0xC) & 0xFF000000) | special_prim_addr;
            goto link_gauge;
        }
    }
    current_value = state->unk4;
    if (current_value != 0)
    {
        partial_bar_count = current_value / value_per_bar;
        palette_index = partial_bar_count & 3;
        if (partial_bar_count >= 3U)
        {
            palette_index |= 2;
        }
        fill_mask = 0xFFFFFF;
        if ((s32)((state->unk8 & fill_mask) - current_value) < (s32)value_per_bar)
        {
            partial_palette = D_800EAFD8;
            partial_color_entry = &partial_palette[palette_index];
            W(gauge_cursor, 0x4) = (s32)(*partial_color_entry & 0x3F3F3F);
            W(gauge_cursor, 0xC) = (s32)(*partial_color_entry & 0x7F7F7F);
            partial_color = *partial_color_entry;
            B(gauge_cursor, 0x3) = 8;
            B(gauge_cursor, 0x7) = 0x38;
            W(gauge_cursor, 0x1C) = partial_color;
            W(gauge_cursor, 0x14) = partial_color;
            partial_width = (s32)(D_8010A008 * (current_value % value_per_bar)) / (s32)value_per_bar;
            ((POLY_G4 *)gauge_cursor)->x0 = (u16)D_8010A00C + x;
            ((POLY_G4 *)gauge_cursor)->y1 = ((u16)D_8010A010 + y);
            ((POLY_G4 *)gauge_cursor)->x1 = ((u16)D_8010A00C + x) + partial_width;
            ((POLY_G4 *)gauge_cursor)->x2 = ((u16)D_8010A00C + x) - 3;
            ((POLY_G4 *)gauge_cursor)->y0 = ((u16)D_8010A010 + y);
            ((POLY_G4 *)gauge_cursor)->y3 = ((POLY_G4 *)gauge_cursor)->y1 + (u16)D_8010A004;
            ((POLY_G4 *)gauge_cursor)->y2 = ((POLY_G4 *)gauge_cursor)->y3;
            ((POLY_G4 *)gauge_cursor)->x3 = ((u16)D_8010A00C + x) + partial_width - 3;
            W(gauge_cursor, 0x0) = (s32)((W(gauge_cursor, 0x0) & 0xFF000000) | (W(ctx, 0xC) & fill_mask));
            partial_prim_addr = (s32)gauge_cursor & fill_mask;
            gauge_cursor += 0x24;
            W(ctx, 0xC) = (s32)((W(ctx, 0xC) & 0xFF000000) | partial_prim_addr);
        }
        value_or_bar_count = (u32)state->unk4 / value_per_bar;
        full_palette_level = value_or_bar_count - 1;
        if (value_or_bar_count != 0)
        {
            palette_index = full_palette_level & 3;
            if (full_palette_level >= 3U)
            {
                palette_index |= 2;
            }
            full_palette = D_800EAFD8;
            full_color_entry = &full_palette[palette_index];
            W(gauge_cursor, 0x4) = (s32)(*full_color_entry & 0x3F3F3F);
            W(gauge_cursor, 0x14) = (s32)(*full_color_entry & 0x7F7F7F);
            B(gauge_cursor, 0x3) = 8;
            B(gauge_cursor, 0x7) = 0x38;
            full_color = *full_color_entry;
            W(gauge_cursor, 0x1C) = full_color;
            W(gauge_cursor, 0xC) = full_color;
            full_x = (u16)D_8010A00C;
            full_right = (u16)D_8010A008;
            full_y = (u16)D_8010A010;
            full_x += x;
            full_right += full_x;
            ((POLY_G4 *)gauge_cursor)->x1 = full_right;
            full_right -= 3;
            full_y += y;
            ((POLY_G4 *)gauge_cursor)->x3 = full_right;
            ((POLY_G4 *)gauge_cursor)->y1 = full_y;
            ((POLY_G4 *)gauge_cursor)->x0 = full_x;
            full_x -= 3;
            ((POLY_G4 *)gauge_cursor)->x2 = full_x;
            ((POLY_G4 *)gauge_cursor)->y0 = full_y;
            ((POLY_G4 *)gauge_cursor)->y3 = ((POLY_G4 *)gauge_cursor)->y1 + (u16)D_8010A004;
            ((POLY_G4 *)gauge_cursor)->y2 = ((POLY_G4 *)gauge_cursor)->y3;
            W(gauge_cursor, 0x0) = (s32)((W(gauge_cursor, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
            full_prim_addr = (s32)gauge_cursor & 0xFFFFFF;
            gauge_cursor += 0x24;
            gauge_ot_tag = (W(ctx, 0xC) & 0xFF000000) | full_prim_addr;
        link_gauge:
            W(ctx, 0xC) = gauge_ot_tag;
        }
    }
    mask24 = 0xFFFFFF;
    packed_display_value = state->unk8;
    value_or_bar_count = state->unk4;
    displayed_value = packed_display_value & mask24;
    if (displayed_value < value_or_bar_count)
    {
        if ((u32)(value_or_bar_count - displayed_value) >= 3U)
        {
            rising_packed_value = 0xFF000000;
            rising_packed_value &= packed_display_value;
            rising_value = displayed_value + ((value_or_bar_count - displayed_value) / 3);
        }
        else
        {
            rising_packed_value = 0xFF000000;
            rising_packed_value &= packed_display_value;
            rising_value = displayed_value + 1;
        }
        rising_packed_value |= rising_value & mask24;
        *(volatile u32 *)&state->unk8 = rising_packed_value;
        rising_current = *(volatile u32 *)&state->unk4;
        if ((rising_current / value_per_bar) == ((s32)(state->unk8 & 0xFFFFFF) / (s32)value_per_bar))
        {
            rising_x = (u16)D_8010A00C;
            rising_x += x;
            H(gauge_cursor, 0xC) =
                (s16)(rising_x + ((u32)(D_8010A008 * (rising_current % value_per_bar)) / value_per_bar));
            rising_ot = ctx + 8;
            rising_cursor = gauge_cursor;
            rising_x += ((s32)(D_8010A008 * ((s32)(state->unk8 & 0xFFFFFF) % (s32)value_per_bar)) / (s32)value_per_bar);
            H(rising_cursor, 0x8) = rising_x;
            helper_cursor = func_800860CC(rising_cursor, y, rising_ot);
        }
        else
        {
            rising_base_x = (u16)D_8010A00C + x;
            H(gauge_cursor, 0xC) = rising_base_x;
            H(gauge_cursor, 0x8) =
                (s16)(rising_base_x +
                      ((s32)(D_8010A008 * ((s32)(state->unk8 & 0xFFFFFF) % (s32)value_per_bar)) / (s32)value_per_bar));
            rising_cursor = func_800860CC(gauge_cursor, y, ctx + 8);
            rising_wrapped_current = state->unk4;
            if ((u32)((state->unk8 & 0xFFFFFF) - rising_wrapped_current) < value_per_bar)
            {
                rising_right = (u16)D_8010A00C + x +
                               ((u32)(D_8010A008 * (rising_wrapped_current % value_per_bar)) / value_per_bar);
                H(rising_cursor, 0xC) = rising_right;
            }
            else
            {
                rising_right = (u16)D_8010A00C + x;
                H(rising_cursor, 0xC) = rising_right;
            }
            rising_ot = ctx + 8;
            H(rising_cursor, 0x8) = (s16)(((u16)D_8010A00C + x) + (u16)D_8010A008);
            helper_cursor = func_800860CC(rising_cursor, y, rising_ot);
        }
        gauge_cursor = helper_cursor;
        goto animate_done;
    }
    if (value_or_bar_count < displayed_value)
    {
        if ((u32)(displayed_value - value_or_bar_count) >= 4U)
        {
            falling_packed_value = 0xFF000000;
            falling_packed_value &= packed_display_value;
            falling_value = displayed_value - ((displayed_value - value_or_bar_count) / 3);
        }
        else
        {
            falling_packed_value = 0xFF000000;
            falling_packed_value &= packed_display_value;
            falling_value = displayed_value - 1;
        }
        falling_packed_value |= falling_value & mask24;
        *(volatile u32 *)&state->unk8 = falling_packed_value;
        falling_current = *(volatile u32 *)&state->unk4;
        if ((falling_current / value_per_bar) == ((s32)(state->unk8 & 0xFFFFFF) / (s32)value_per_bar))
        {
            falling_x = (u16)D_8010A00C;
            falling_x += x;
            H(gauge_cursor, 0xC) =
                (s16)(falling_x + ((u32)(D_8010A008 * (falling_current % value_per_bar)) / value_per_bar));
            falling_ot = ctx + 8;
            falling_cursor = gauge_cursor;
            falling_x +=
                ((s32)(D_8010A008 * ((s32)(state->unk8 & 0xFFFFFF) % (s32)value_per_bar)) / (s32)value_per_bar);
            H(falling_cursor, 0x8) = falling_x;
            helper_cursor = func_80086030(falling_cursor, y, falling_ot);
        }
        else
        {
            falling_base_x = (u16)D_8010A00C + x;
            H(gauge_cursor, 0xC) = falling_base_x;
            H(gauge_cursor, 0x8) =
                (s16)(falling_base_x +
                      ((s32)(D_8010A008 * ((s32)(state->unk8 & 0xFFFFFF) % (s32)value_per_bar)) / (s32)value_per_bar));
            falling_cursor = func_80086030(gauge_cursor, y, ctx + 8);
            falling_wrapped_current = state->unk4;
            if ((u32)((state->unk8 & 0xFFFFFF) - falling_wrapped_current) < value_per_bar)
            {
                falling_right = (u16)D_8010A00C + x +
                                ((u32)(D_8010A008 * (falling_wrapped_current % value_per_bar)) / value_per_bar);
                H(falling_cursor, 0xC) = falling_right;
            }
            else
            {
                falling_right = (u16)D_8010A00C + x;
                H(falling_cursor, 0xC) = falling_right;
            }
            falling_ot = ctx + 8;
            H(falling_cursor, 0x8) = (s16)(((u16)D_8010A00C + x) + (u16)D_8010A008);
            helper_cursor = func_80086030(falling_cursor, y, falling_ot);
        }
        gauge_cursor = helper_cursor;
    }
animate_done:
    sprite_cursor = gauge_cursor;
    W(sprite_cursor, 0x4) = 0x808080;
    B(sprite_cursor, 0x3) = 4;
    B(sprite_cursor, 0x7) = 0x66;
    W(sprite_cursor, 0x8) = (s32)((y << 0x10) + x);
    background_type = state->u4c.bytes.unk4d;
    switch (background_type)
    {
        case 0:
        case 1:
            W(sprite_cursor, 0x10) = 0x180058;
            H(sprite_cursor, 0xC) = 0x1000;
            break;
        case 2:
            if ((u8)D_800FDCEA >= 0x41U)
            {
                W(sprite_cursor, 0x10) = 0x180058;
                H(sprite_cursor, 0xC) = 0x1000;
            }
            else
            {
                W(sprite_cursor, 0x10) = 0x180058;
                H(sprite_cursor, 0xC) = 0x2800;
            }
            break;
        default:
            if (alternate_layout != 0)
            {
                W(sprite_cursor, 0x10) = 0x100100;
                H(sprite_cursor, 0xC) = 0;
            }
            else
            {
                W(sprite_cursor, 0x10) = 0x100040;
                H(sprite_cursor, 0xC) = 0x4000;
            }
            break;
    }
    H(sprite_cursor, 0xE) = 0x7810;
    W(sprite_cursor, 0x0) = (s32)((W(sprite_cursor, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
    W(ctx, 0xC) = (s32)((W(ctx, 0xC) & 0xFF000000) | ((s32)sprite_cursor & 0xFFFFFF));
    sprite_cursor += 0x14;
    B(sprite_cursor, 0x3) = 1;
    W(sprite_cursor, 0x4) = 0xE100001F;
    W(sprite_cursor, 0x0) = (s32)((W(sprite_cursor, 0x0) & 0xFF000000) | (W(ctx, 0xC) & 0xFFFFFF));
    W(ctx, 0xC) = (s32)((W(ctx, 0xC) & 0xFF000000) | ((s32)sprite_cursor & 0xFFFFFF));
    PTR(render_context, 0x40B8) = sprite_cursor + 8;
}
