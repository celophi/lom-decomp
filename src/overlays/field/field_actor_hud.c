/**
 * @file field_actor_hud.c
 * @brief Field actor HUD renderer: participant panels, per-track gauge/bar
 *        primitives, digit and line/quad primitive builders, and the image
 *        resource loader used by the field HUD.
 *
 * Consolidated translation unit spanning vram 0x80084700 .. 0x80086494.
 * Each function preserves its original declaration environment at block scope
 * so the merged TU compiles byte-for-byte identically to the separate objects.
 */

#include "common.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/* ------------------------------------------------------------------------- */
/* Shared record views (kept distinct per originating function's layout).     */
/* ------------------------------------------------------------------------- */

/** @brief Position, state, and presence fields in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x15];
    u8 state;
    u8 pad22[3];
    u8 presence;
    u8 pad26[4];
    s16 value;
    u8 tail[0x28];
} FieldPanelActor;
/** @brief Display values, countdown, group, and linked actor in a 0x23C-byte slot. */
typedef struct
{
    u32 unknown0;
    u32 current;
    union
    {
        s32 word;
        u8 bytes[4];
    } previous;
    u32 flags;
    u32 group;
    u8 pad14[0x38];
    u32 options;
    u8 pad50[0x11D];
    u8 linked;
    u8 pad16e[0xA];
    u32 state;
    u8 tail[0xC0];
} FieldPanelSlot;
/** @brief Participation flag in a 0x268-byte player record. */
typedef struct
{
    u8 flags;
    u8 tail[0x267];
} FieldPanelPlayer;

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

typedef struct
{
    s16 x;
    s16 y;
} HudPoint;

/** @brief 0x14-byte sprite primitive as written by func_80085FAC. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} Prim;

/** @brief Object holding an ordering-table tag at 0xC. */
typedef struct
{
    u8 pad[0xC];
    s32 unkC;
} OtLike;

typedef struct
{
    u8 pad0[0x1C];
    u32 unk1C;
    u8 pad20[0x54 - 0x20];
} FieldObjRec86184;

/** @brief Draw context carrying an ordering-table tag at 0xC. */
typedef struct
{
    u8 pad[0xC];
    u32 tag;
} FieldPrimitiveContext;

#define B(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define H(p, o) (*(s16 *)((u8 *)(p) + (o)))
#define UH(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define W(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define PTR(p, o) (*(u8 **)((u8 *)(p) + (o)))

/**
 * @brief Draw participant panels and temporary indicators for eligible actors.
 * @param render_context Rendering context forwarded to the panel drawing helper.
 * @note Actor positions use signed fixed-point division, followed by screen clamps.
 * @note Display-value bits 24 through 30 count down after a temporary panel draw.
 */
void func_80084700(u8 *render_context)
{
    extern FieldPanelActor D_800FDF58[], g_field_effect_records[];
    extern FieldPanelSlot D_80105AE0[];
    extern FieldPanelPlayer D_800FD818[];
    extern s32 D_800EB04C[];
    extern s32 D_800F22A0, D_800F22A4, D_800F22A8;
    extern s32 D_800FE754, D_801158A0, D_80122B20;
    extern void func_80084D08(s32, s32, s32, u8 *, u32);

    Vec2s position;
    s32 i = 0;
    s32 count = i;
    s32 j;
    s32 absent = 0xFF;
    FieldPanelSlot *slot = D_80105AE0;
    FieldPanelPlayer *player = D_800FD818;
    FieldPanelActor *actor = D_800FDF58;
    FieldPanelActor *actor_one;
    FieldPanelPlayer *player_one;
    FieldPanelActor *actor_two;
    FieldPanelPlayer *player_two;
    s32 x_two;
    s32 *order;
    s32 x_three;
    s16 y_three;
    s32 boss_drawn;
    FieldPanelActor *enemy;
    FieldPanelSlot *enemy_slot;
    FieldPanelActor *linked;
    s32 group;
    s32 value;
    u32 current;
    u32 previous;
    s32 y;
    s32 z;
    s32 ticks;

    do
    {
        slot = &D_80105AE0[i];
        actor = &D_800FDF58[i];
        if (actor->presence != absent && (player->flags & 1))
        {
            if (actor->value != 0x85 && actor->value != 0x87)
            {
                slot->options |= 1;
            }
            count++;
        }
        player++;
        i++;
    } while (i < 3);
    switch (count)
    {
        case 1:
            i = 0;
            player_one = D_800FD818;
            actor_one = D_800FDF58;
            do
            {
                actor_one = &D_800FDF58[i];
                player_one = &D_800FD818[i];
                if (actor_one->presence != 0xFF && (player_one->flags & 1))
                {
                    func_80084D08(0x70, 0x10, i, render_context, 0x64);
                }
                i++;
            } while (i < 3);
            break;
        case 2:
            i = 0;
            player_two = D_800FD818;
            actor_two = D_800FDF58;
            x_two = 0x38;
            do
            {
                actor_two = &D_800FDF58[i];
                player_two = &D_800FD818[i];
                if (actor_two->presence != 0xFF && (player_two->flags & 1))
                {
                    func_80084D08(x_two, 0x10, i, render_context, 0x64);
                    x_two += 0x6C;
                }
                i++;
            } while (i < 3);
            break;
        case 3:
            i = 0;
            order = D_800EB04C;
            x_three = 8;
            do
            {
                if (D_800FDF58[*order].presence != 0xFF && (D_800FD818[*order].flags & 1))
                {
                    y_three = 0x1C;
                    if (i & 1)
                    {
                        y_three = 4;
                    }
                    func_80084D08(x_three, y_three, *order, render_context, 0x64);
                    x_three += 0x68;
                }
                i++;
                order++;
            } while (i < 3);
            break;
    }
    boss_drawn = 0;
    if (D_801158A0 != 0)
    {
        j = 3;
        if (D_80122B20 == 0)
        {
            enemy_slot = &D_80105AE0[3];
            enemy = &D_800FDF58[3];
            do
            {
                group = enemy_slot->group & 0xF;
                if (group == D_800FE754 && group != 0)
                {
                    value = enemy_slot->previous.word;
                    if (value < 0)
                    {
                        if (enemy->presence != 0xFF && (value & 0xFFFFFF) && boss_drawn == 0)
                        {
                            func_80084D08(0x20, 0xC0, j, render_context, 0x190);
                            boss_drawn = 1;
                        }
                    }
                    else if (enemy->presence != 0xFF)
                    {
                        current = enemy_slot->current;
                        previous = value & 0xFFFFFF;
                        if (current < previous || previous != current)
                        {
                            enemy_slot->previous.word = (value & 0x80FFFFFF) | 0x14000000;
                        }
                        if (enemy_slot->previous.bytes[3] & 0x7F)
                        {
                            if (!(enemy_slot->flags & 0x100) && (*(u8 *)&enemy_slot->state & 1) &&
                                (g_field_effect_records[enemy_slot->linked].state & 0x7F) != 0x2F)
                            {
                                linked = &g_field_effect_records[enemy_slot->linked];
                                position.x = (D_800F22A0 / 256) + (u32)(linked->x / 256 + 0xA0);
                                y = D_800F22A4 / 256 + (g_field_effect_records[enemy_slot->linked].y / 256 + 0x70);
                                z = g_field_effect_records[enemy_slot->linked].z;
                            }
                            else
                            {
                                position.x = (D_800F22A0 / 256) + (u32)(enemy->x / 256 + 0xA0);
                                y = D_800F22A4 / 256 + (enemy->y / 256 + 0x70);
                                z = enemy->z;
                            }
                            position.y = y - z / 512 - D_800F22A8 / 512;
                            if (position.x + 0x20 >= 0x141)
                            {
                                position.x = 0x120;
                            }
                            if (position.x < 0x20)
                            {
                                position.x = 0x20;
                            }
                            if (position.y >= 0xD1)
                            {
                                position.y = 0xD0;
                            }
                            if (position.y < 0x10)
                            {
                                position.y = 0x10;
                            }
                            func_80084D08(position.x - 0x1C, position.y, j, render_context, 0x64);
                            value = enemy_slot->previous.word;
                            ticks = ((u32)value >> 24) & 0x7F;
                            if (ticks != 0)
                            {
                                enemy_slot->previous.word = (value & 0x80FFFFFF) | (((ticks - 1) & 0x7F) << 24);
                            }
                        }
                    }
                }
                enemy_slot++;
                j++;
                enemy++;
            } while (j < 13);
        }
    }
}

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

/**
 * @brief Builds and links a shaded two-point line primitive.
 * @param packet Primitive-buffer cursor where the line is written.
 * @param context Draw context containing the ordering-table tag.
 * @param colors Two packed endpoint colors used to derive the line shading.
 * @param intensity Fallback endpoint intensity and horizontal length scale.
 * @param x Base horizontal position.
 * @param y Base vertical position.
 * @return Pointer immediately after the emitted primitive, or @p packet when intensity is zero.
 */
u8 *func_80085D30(u8 *packet, FieldPrimitiveContext *context, u32 *colors, s32 intensity, s32 x, s32 y)
{
    u8 first_component;
    u8 second_component;
    s16 line_x;
    s16 line_y;
    u32 first_color;

    if (intensity == 0)
    {
        return packet;
    }

    first_color = colors[0];
    setlen((LINE_G2 *)packet, 4);
    *(u32 *)&((LINE_G2 *)packet)->r0 = first_color;
    setcode((LINE_G2 *)packet, 0x50);

    first_component = ((u8 *)colors)[0];
    second_component = ((u8 *)colors)[4];
    if (first_component == second_component)
    {
        ((LINE_G2 *)packet)->r1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2 *)packet)->r1 = intensity;
    }
    else
    {
        ((LINE_G2 *)packet)->r1 = ~intensity;
    }

    first_component = ((u8 *)colors)[1];
    second_component = ((u8 *)colors)[5];
    if (first_component == second_component)
    {
        ((LINE_G2 *)packet)->g1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2 *)packet)->g1 = intensity;
    }
    else
    {
        ((LINE_G2 *)packet)->g1 = ~intensity;
    }

    first_component = ((u8 *)colors)[2];
    second_component = ((u8 *)colors)[6];
    if (first_component == second_component)
    {
        ((LINE_G2 *)packet)->b1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2 *)packet)->b1 = intensity;
    }
    else
    {
        ((LINE_G2 *)packet)->b1 = ~intensity;
    }

    line_x = x + 0x18;
    ((LINE_G2 *)packet)->x0 = line_x;
    line_y = y + 0x10;
    ((LINE_G2 *)packet)->y1 = line_y;
    ((LINE_G2 *)packet)->y0 = line_y;
    ((LINE_G2 *)packet)->x1 = line_x + ((intensity * 0x23) / 255);

    addPrim(&context->tag, (LINE_G2 *)packet);
    return packet + sizeof(LINE_G2);
}

/**
 * @brief Emit a right-aligned 3-digit decimal number followed by a trailing
 *        glyph, advancing the horizontal cursor 7 units per column.
 * @param arg0 Primitive-buffer cursor passed through the digit emitter.
 * @param arg1 Opaque draw context forwarded to func_80085FAC.
 * @param arg2 Value to render (hundreds/tens/units).
 * @param arg3 Pointer to the current X cursor, bumped by 7 after each column.
 * @note Leading zeros in the hundreds/tens columns are suppressed until the
 *       first non-zero digit is emitted.
 */
void func_80085E84(void *arg0, void *arg1, s32 arg2, u16 *arg3)
{
    void *func_80085FAC(void *, void *, s32, s32 *);

    s32 emitted;

    emitted = 0;
    if (arg2 / 100 != 0)
    {
        emitted = 1;
        arg0 = func_80085FAC(arg0, arg1, arg2 / 100, (s32 *)arg3);
        arg2 -= 100;
    }
    *arg3 += 7;
    if (emitted || arg2 / 10 != 0)
    {
        s32 digit;
        digit = arg2 / 10;
        arg0 = func_80085FAC(arg0, arg1, digit, (s32 *)arg3);
        arg2 -= digit * 10;
    }
    *arg3 += 7;
    arg0 = func_80085FAC(arg0, arg1, arg2, (s32 *)arg3);
    *arg3 += 7;
    func_80085FAC(arg0, arg1, 10, (s32 *)arg3);
}

/**
 * @brief Build a textured sprite primitive and link it into an ordering table.
 *
 * Fills the primitive at @p arg0 with fixed colour, code, texture, clut, and
 * size fields (the UV taken from @p arg2), copies the packed xy from @p arg3,
 * then splices the primitive ahead of the tag stored at @c arg1->unkC.
 *
 * @param arg0 Primitive packet to populate.
 * @param arg1 Object holding the ordering-table tag at @c unkC.
 * @param arg2 UV selector; scaled by 8 and biased by 0x1558.
 * @param arg3 Source of the packed xy word written to the primitive.
 * @return Pointer just past the emitted primitive (@p arg0 + 0x14).
 * @see decomp.me (100%) TODO
 */
void *func_80085FAC(void *arg0, void *arg1, s32 arg2, s32 *arg3)
{
    Prim *p;
    OtLike *ot;
    s32 temp;

    p = (Prim *)arg0;
    ot = (OtLike *)arg1;

    p->unk4 = 0x808080;
    ((u8 *)p)[3] = 4;
    ((u8 *)p)[7] = 0x64;
    temp = *arg3;
    p->unkC = (s16)((arg2 * 8) + 0x1558);
    p->unk10 = 0xB0008;
    p->unkE = 0x7810;
    p->unk8 = temp;
    p->unk0 = (p->unk0 & 0xFF000000) | (ot->unkC & 0xFFFFFF);
    ot->unkC = (ot->unkC & 0xFF000000) | ((s32)p & 0xFFFFFF);
    return (u8 *)arg0 + 0x14;
}

/**
 * @brief Build a black flat quad from the caller's x0/x1, bordered 3 pixels inward at the bottom, and link it into the OT.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to D_8010A010.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4 *func_80086030(POLY_F4 *prim, s32 y, u32 *ot)
{
    extern s32 D_8010A004;
    extern s32 D_8010A010;

    *((u32 *)&prim->r0) = 0xFF;
    ((P_TAG *)prim)->len = 5, ((P_TAG *)prim)->code = 0x28;
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)D_8010A010) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)D_8010A010) + y;
    prim->y3 = prim->y1 + ((u16)D_8010A004);
    prim->y2 = prim->y3;
    ((P_TAG *)prim)->addr = (u32)((P_TAG *)ot)->addr,
        ((P_TAG *)ot)->addr = (u32)prim;
    return prim + 1;
}

/**
 * @brief Build a white flat quad like func_80086030, widening a zero-width x0/x1 pair to one pixel first.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to D_8010A010.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4 *func_800860CC(POLY_F4 *prim, s32 y, u32 *ot)
{
    extern s32 D_8010A004;
    extern s32 D_8010A010;

    if (prim->x1 == prim->x0) {
        prim->x1 = prim->x0 + 1;
    }
    *((u32 *)&prim->r0) = 0xFFFFFF;
    ((P_TAG *)prim)->len = 5, ((P_TAG *)prim)->code = 0x28;
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)D_8010A010) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)D_8010A010) + y;
    prim->y3 = prim->y1 + ((u16)D_8010A004);
    prim->y2 = prim->y3;
    ((P_TAG *)prim)->addr = (u32)((P_TAG *)ot)->addr,
        ((P_TAG *)ot)->addr = (u32)prim;
    return prim + 1;
}

/**
 * @brief Build the textured sprite primitive for a field object icon.
 * @param sprt Sprite primitive to populate.
 * @param ot Ordering table the primitive (and its tpage) are linked into.
 * @param index Object slot index; selects clut/tpage and a special case at 2.
 * @param xy Packed screen position copied into the sprite's x0/y0.
 * @return Pointer just past the appended DR_TPAGE primitive.
 */
void *func_80086184(SPRT *sprt, u32 *ot, s32 index, u32 *xy)
{
    extern u8 D_800FDCEA;
    extern u16 D_800FE01E;
    extern FieldObjRec86184 D_800FDF58[];

    DR_TPAGE *mode;

    *(u32 *)&sprt->r0 = 0x808080;
    setSprt(sprt);
    *(u32 *)&sprt->x0 = *xy;
    *(u16 *)&sprt->u0 = 0xE800;
    *(u32 *)&sprt->w = 0x180018;

    if (index == 2 && D_800FDCEA >= 0x41)
    {
        sprt->clut = (((D_800FE01E & 3) + 0x1EF) << 6) | 0x10;
    }
    else
    {
        sprt->clut = ((index + 0x1F4) << 6) | ((D_800FDF58[index].unk1C >> 19) & 0xF);
    }

    addPrim(ot, sprt);

    mode = (DR_TPAGE *)(sprt + 1);
    if (index >= 2)
    {
        setDrawTPage(mode, 0, 0, getTPage(0, 1, 0x340 - (index << 6), 0));
    }
    else
    {
        setDrawTPage(mode, 0, 0, getTPage(0, 1, 0x380 - (index << 7), 0));
    }
    addPrim(ot, mode);
    return mode + 1;
}

/**
 * @brief Loads a VRAM resource from disc and uploads it via func_80086374.
 *
 * Queues a CD read of resource @p id (masked to 16 bits) into the shared field
 * CD buffer @c D_8010D038, waits for the queue to drain, then hands the loaded
 * blob to func_80086374 to upload into VRAM using @p rect and @p arg2.
 *
 * @param id Resource index; masked to 16 bits for the CD queue.
 * @param rect Destination rectangle forwarded to func_80086374.
 * @param arg2 Upload mode forwarded to func_80086374.
 */
void field_load_vram_resource(s32 id, s16 *rect, s32 arg2)
{
    extern u8 *D_8010D038;
    void cdrom_queue_read(s32 resource_index, void *dst_buffer);
    void cdrom_wait_queue_empty(void);
    s32 func_80086374(RECT *rect, u8 *data, s32 mode);

    u8 *buf = D_8010D038;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    func_80086374((RECT *)rect, buf, arg2);
}

/**
 * @brief Upload a two-part image resource (palette/CLUT block followed by the
 *        pixel block) into VRAM and report the payload's trailing status word.
 * @param rect Destination framebuffer rectangle; supplies the upload x/y
 *             (from @c rect->x / @c rect->y) and the CLUT width/height (from
 *             @c rect->w / @c rect->h), and is updated on return to the pixel
 *             block's dimensions.
 * @param data Resource blob. Offset 0x8 holds the pixel-block byte offset,
 *             0x10 the CLUT dimensions, 0x14 the CLUT pixels, and 0x1F4 the
 *             status word returned to the caller.
 * @param mode When non-zero, upload the CLUT as a single 1-tall run of
 *             width*height entries; when zero, upload it with its natural
 *             width and height.
 * @return The status word stored at @c data+0x1F4.
 * @note The nested @c do{}while(0) wrappers reproduce the original codegen and
 *       are required to match; do not remove them.
 * @see decomp.me (100.00%)
 */
s32 func_80086374(RECT *rect, u8 *data, s32 mode)
{
    RECT load_rect;
    s32 offset;
    s32 ret;
    u8 *image;
    u8 *dims;
    u16 x;

    dims = data + 0x10;
    do { do { do { do { offset = *(s32 *)(data + 8); } while (0); } while (0); } while (0); } while (0);
    if (mode != 0)
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = *(u16 *)dims * *(u16 *)(dims + 2);
        load_rect.h = 1;
    }
    else
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = *(u16 *)dims;
        do { do { load_rect.h = *(u16 *)(dims + 2); } while (0); } while (0);
    }
    LoadImage(&load_rect, (u_long *)(data + 0x14));

    offset += 8;
    image = data + offset;
    x = rect->x;
    ret = *(s32 *)(data + 0x1F4);
    load_rect.x = x;
    load_rect.y = rect->y;
    dims = image + 8;
    load_rect.w = *(u16 *)dims;
    load_rect.h = *(u16 *)(dims + 2);
    LoadImage(&load_rect, (u_long *)(image + 0xC));
    rect->x = *(u16 *)dims;
    rect->y = *(u16 *)(dims + 2);
    return ret;
}
