/**
 * @file field_actor_hud.c
 * @brief Field actor HUD renderer: participant panels, HP and status-bar
 *        primitives, digit and line/quad primitive builders, and the image
 *        resource loader used by the field HUD.
 */

#include "common.h"
#include "cdrom.h"
#include "field_effect_render_state.h"
#include "tim.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define FIELD_HUD_HP_MASK 0xFFFFFF
#define FIELD_HUD_DISPLAY_FLAGS_MASK 0xFF000000
#define FIELD_HUD_BLINK_FRAMES 6
#define FIELD_HUD_GLYPH_PERCENT 10
#define FIELD_HUD_GLYPH_ADVANCE 7
#define FIELD_HUD_CLUT getClut(256, 480)
#define FIELD_HUD_BLINK_CLUT getClut(256, 481)

/** @brief Position, state, and presence fields in a 0x54-byte actor record. */
typedef struct
{
    s32 x, y, z;
    u8 pad_0x0c[0x15];
    u8 state;
    u8 pad_0x22[3];
    u8 presence;
    u8 pad_0x26[4];
    s16 action_id;
    u8 tail[0x28];
} FieldPanelActor;
/** @brief Display values, countdown, group, and linked actor in a 0x23C-byte slot. */
typedef struct
{
    u32 maximum_hp;
    u32 current_hp;
    union
    {
        s32 word;
        u8 bytes[4];
    } hp_display;
    u32 flags;
    u32 group;
    u8 pad_0x14[0x38];
    u32 hud_options;
    u8 pad_0x50[0x11D];
    u8 linked_effect_index;
    u8 pad_0x16e[0xA];
    union
    {
        u32 word;
        u8 bytes[4];
    } contact;
    u8 tail[0xC0];
} FieldPanelSlot;
/** @brief Participation flag in a 0x268-byte player record. */
typedef struct
{
    u8 flags;
    u8 tail[0x267];
} FieldPanelPlayer;

/** @brief HP and status gauges in a 0x23C-byte object state. */
typedef struct
{
    u32 maximum_hp;
    u32 current_hp;
    u32 hp_display_flags;
    u8 pad_0c[0x48 - 0x0C];
    u16 status_intensity;
    u16 effect_intensity;
    union
    {
        u32 word;
        struct
        {
            u8 flags;
            u8 panel_type;
            u8 unknown_0x4e;
            u8 unknown_0x4f;
        } bytes;
    } hud;
    u8 pad_50[0x23C - 0x50];
} FieldHudGaugeState;

/** @brief Panel shake and special-gauge values in a 0x268-byte player record. */
typedef struct
{
    u8 pad_0[0x259];
    u8 shake_frame;
    u8 pad_25a[0x25E - 0x25A];
    s16 special_gauge_value;
    s16 special_gauge_maximum;
    u8 pad_262[0x268 - 0x262];
} FieldHudPlayer;

/** @brief Current action in a 0x54-byte actor record. */
typedef struct
{
    u8 pad_0[0x2A];
    s16 action_id;
    u8 pad_2c[0x54 - 0x2C];
} FieldHudActorAction;

/** @brief Portrait palette selection in a 0x54-byte actor record. */
typedef struct
{
    u8 pad_0[0x1C];
    u32 texture_flags;
    u8 pad_0x20[0x54 - 0x20];
} FieldHudActorTexture;

/** @brief Draw context carrying an ordering-table tag at 0xC. */
typedef struct
{
    u8 pad[0xC];
    u32 tag;
} FieldPrimitiveContext;

/** @brief Primitive lists and allocation cursor in the field draw context. */
typedef struct
{
    u8 pad_0[8];
    u32 delta_tag;
    u32 hud_tag;
    u8 pad_10[0x40B8 - 0x10];
    u8 *primitive_cursor;
} FieldHudDrawContext;

/**
 * @brief Draw participant panels and temporary indicators for eligible actors.
 * @param render_context Rendering context forwarded to the panel drawing helper.
 * @note Actor positions use signed fixed-point division, followed by screen clamps.
 * @note Display-value bits 24 through 30 count down after a temporary panel draw.
 */
void field_draw_actor_hud(u8 *render_context)
{
    extern FieldPanelActor g_field_actors[], g_field_effect_records[];
    extern FieldPanelSlot g_field_object_states[];
    extern FieldPanelPlayer g_field_player_records[];
    extern s32 g_field_party_hud_order[];

    extern s32 g_field_active_group, g_field_scene_mode_bit, D_80122B20;
    extern void field_draw_actor_hud_panel(s32, s32, s32, u8 *, u32);

    Vec2s position;
    s32 i = 0;
    s32 count = i;
    s32 j;
    s32 absent = 0xFF;
    FieldPanelSlot *slot = g_field_object_states;
    FieldPanelPlayer *player = g_field_player_records;
    FieldPanelActor *actor = g_field_actors;
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
        slot = &g_field_object_states[i];
        actor = &g_field_actors[i];
        if (actor->presence != absent && (player->flags & 1))
        {
            if (actor->action_id != 0x85 && actor->action_id != 0x87)
            {
                slot->hud_options |= 1;
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
            player_one = g_field_player_records;
            actor_one = g_field_actors;
            do
            {
                actor_one = &g_field_actors[i];
                player_one = &g_field_player_records[i];
                if (actor_one->presence != 0xFF && (player_one->flags & 1))
                {
                    field_draw_actor_hud_panel(0x70, 0x10, i, render_context, 0x64);
                }
                i++;
            } while (i < 3);
            break;
        case 2:
            i = 0;
            player_two = g_field_player_records;
            actor_two = g_field_actors;
            x_two = 0x38;
            do
            {
                actor_two = &g_field_actors[i];
                player_two = &g_field_player_records[i];
                if (actor_two->presence != 0xFF && (player_two->flags & 1))
                {
                    field_draw_actor_hud_panel(x_two, 0x10, i, render_context, 0x64);
                    x_two += 0x6C;
                }
                i++;
            } while (i < 3);
            break;
        case 3:
            i = 0;
            order = g_field_party_hud_order;
            x_three = 8;
            do
            {
                if (g_field_actors[*order].presence != 0xFF && (g_field_player_records[*order].flags & 1))
                {
                    y_three = 0x1C;
                    if (i & 1)
                    {
                        y_three = 4;
                    }
                    field_draw_actor_hud_panel(x_three, y_three, *order, render_context, 0x64);
                    x_three += 0x68;
                }
                i++;
                order++;
            } while (i < 3);
            break;
    }
    boss_drawn = 0;
    /* Enemy panels are transient; the boss uses a fixed panel at the bottom. */
    if (g_field_scene_mode_bit != 0)
    {
        j = 3;
        if (D_80122B20 == 0)
        {
            enemy_slot = &g_field_object_states[3];
            enemy = &g_field_actors[3];
            do
            {
                group = enemy_slot->group & 0xF;
                if (group == g_field_active_group && group != 0)
                {
                    value = enemy_slot->hp_display.word;
                    if (value < 0)
                    {
                        if (enemy->presence != 0xFF && (value & FIELD_HUD_HP_MASK) && boss_drawn == 0)
                        {
                            field_draw_actor_hud_panel(0x20, 0xC0, j, render_context, 0x190);
                            boss_drawn = 1;
                        }
                    }
                    else if (enemy->presence != 0xFF)
                    {
                        current = enemy_slot->current_hp;
                        previous = value & FIELD_HUD_HP_MASK;
                        if (current < previous || previous != current)
                        {
                            enemy_slot->hp_display.word = (value & 0x80FFFFFF) | 0x14000000;
                        }
                        if (enemy_slot->hp_display.bytes[3] & 0x7F)
                        {
                            if (!(enemy_slot->flags & 0x100) && (enemy_slot->contact.bytes[0] & 1) &&
                                (g_field_effect_records[enemy_slot->linked_effect_index].state & 0x7F) != 0x2F)
                            {
                                linked = &g_field_effect_records[enemy_slot->linked_effect_index];
                                position.x = (g_field_view_offset_x / 256) + (u32)(linked->x / 256 + 0xA0);
                                y = g_field_view_offset_y / 256 + (g_field_effect_records[enemy_slot->linked_effect_index].y / 256 + 0x70);
                                z = g_field_effect_records[enemy_slot->linked_effect_index].z;
                            }
                            else
                            {
                                position.x = (g_field_view_offset_x / 256) + (u32)(enemy->x / 256 + 0xA0);
                                y = g_field_view_offset_y / 256 + (enemy->y / 256 + 0x70);
                                z = enemy->z;
                            }
                            position.y = y - z / 512 - g_field_view_offset_z / 512;
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
                            field_draw_actor_hud_panel(position.x - 0x1C, position.y, j, render_context, 0x64);
                            value = enemy_slot->hp_display.word;
                            ticks = ((u32)value >> 24) & 0x7F;
                            if (ticks != 0)
                            {
                                enemy_slot->hp_display.word = (value & 0x80FFFFFF) | (((ticks - 1) & 0x7F) << 24);
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
 * @brief Draw an actor panel and animate its displayed HP toward the current HP.
 * @param x Base screen x for the gauge group.
 * @param y Base screen y (adjusted by the per-slot shake table).
 * @param slot Actor index (0..2 use the extended blink/shake path).
 * @param render_context Draw context whose primitive cursor is advanced on return.
 * @param value_per_bar Gauge full-scale denominator used for value-to-width scaling.
 * @see decomp.me (100%)
 */
void field_draw_actor_hud_panel(s32 x, s32 y, s32 slot, u8 *render_context, u32 value_per_bar)
{
    u8 *field_emit_hud_status_line(u8 *, FieldPrimitiveContext *, u32 *, s32, s32, s32);
    void *field_emit_hud_percentage(void *, void *, s32, u16 *);
    POLY_F4 *field_emit_hud_damage_quad(POLY_F4 *, s32, u32 *);
    POLY_F4 *field_emit_hud_healing_quad(POLY_F4 *, s32, u32 *);
    void *field_emit_actor_portrait(SPRT *, u32 *, s32, u32 *);
    s32 rand(void);
    extern u32 g_field_hud_hp_colors[];
    extern u32 g_field_hud_status_colors[];
    extern u32 g_field_hud_effect_colors[];
    extern u32 g_field_hud_full_status_colors[];
    extern u32 g_field_hud_companion_status_colors[];
    extern s16 g_field_hud_shake_offsets[];
    extern FieldHudPlayer g_field_player_records[];
    extern u8 D_800FDCEA;
    extern FieldHudActorAction g_field_actors[];
    extern FieldHudGaugeState g_field_object_states[];
    extern s32 g_field_hud_blink_frames[];
    extern s32 g_field_boss_hud_shake_frame;
    extern s32 g_field_hud_bar_height;
    extern s32 g_field_hud_bar_width;
    extern s32 g_field_hud_bar_offset_x;
    extern s32 g_field_hud_bar_offset_y;
    extern u8 D_80117EC8[];
    extern s32 g_frame_counter;

    s16 *scratch = (s16 *)0x1F800000;
    u8 *ctx;
    u8 *call_ctx;
    u32 special_width;
    s32 scan_slot;
    u32 *partial_palette;
    u32 *full_palette;
    u32 boss_panel;
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
    u32 boss_flag;
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
    FieldHudPlayer *actor_entry;
    FieldHudPlayer *entry_base;
    FieldHudGaugeState *state;
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
        if (g_field_player_records[slot].shake_frame < 6U)
        {
            y += g_field_hud_shake_offsets[g_field_player_records[slot].shake_frame];
            if (g_field_player_records[slot].shake_frame != 0)
            {
                g_field_player_records[slot].shake_frame--;
            }
            else
            {
                g_field_player_records[slot].shake_frame = 0xFF;
            }
        }
    }
    state = &g_field_object_states[slot];
    boss_flag = (u32)state->hp_display_flags >> 0x1F;
    boss_panel = boss_flag;
    if ((boss_flag != 0) && (g_field_boss_hud_shake_frame < 6))
    {
        y += g_field_hud_shake_offsets[g_field_boss_hud_shake_frame];
        if (g_field_boss_hud_shake_frame != 0)
        {
            g_field_boss_hud_shake_frame--;
        }
        else
        {
            g_field_boss_hud_shake_frame = 0xFF;
        }
    }
    ctx = render_context;
    label_cursor = ((FieldHudDrawContext *)ctx)->primitive_cursor;
    if (state->hud.bytes.panel_type < 3U)
    {
        g_field_hud_bar_offset_x = 0x1D;
        g_field_hud_bar_offset_y = 9;
        scratch[0] = x;
        scratch[1] = y;
        label_cursor = field_emit_actor_portrait((SPRT *)label_cursor, &((FieldHudDrawContext *)ctx)->hud_tag,
                                                state->hud.bytes.panel_type, (u32 *)scratch);
        g_field_hud_bar_width = 0x36;
        g_field_hud_bar_height = 3;
    }
    else if (boss_panel != 0)
    {
        g_field_hud_bar_offset_x = 0xF;
        g_field_hud_bar_offset_y = 6;
        g_field_hud_bar_width = 0xE3;
        g_field_hud_bar_height = 3;
    }
    else
    {
        g_field_hud_bar_offset_x = 4;
        g_field_hud_bar_offset_y = 6;
        g_field_hud_bar_width = 0x36;
        g_field_hud_bar_height = 3;
    }
    sprite_cursor = label_cursor;
    if (slot < 3)
    {
        if (g_field_hud_blink_frames[slot] >= FIELD_HUD_BLINK_FRAMES ||
            ++g_field_hud_blink_frames[slot] >= FIELD_HUD_BLINK_FRAMES)
        {
            for (scan_slot = 0; scan_slot < 8; scan_slot++)
            {
                if (D_80117EC8[scan_slot] == 0xFF)
                {
                    break;
                }
                if (D_80117EC8[scan_slot] == slot)
                {
                    g_field_hud_blink_frames[slot] = 0;
                    break;
                }
            }
            blink_counters = g_field_hud_blink_frames;
            blink_counter = blink_counters + slot;
            if ((*blink_counter != 0) && !(rand() & 0x3F))
            {
                *blink_counter = 0;
            }
        }
        *(u32 *)&((SPRT *)sprite_cursor)->r0 = 0x808080;
        setSprt((SPRT *)sprite_cursor);
        ((SPRT *)sprite_cursor)->x0 = (s16)(x + 0x22);
        ((SPRT *)sprite_cursor)->y0 = (s16)(y - 6);
        if (g_field_hud_blink_frames[slot] < 6)
        {
            blink_uv = ((u16)g_field_hud_blink_frames[slot] * 0x10) + 0x2058;
        }
        else
        {
            blink_uv = 0x2058;
        }
        *(u32 *)&((SPRT *)sprite_cursor)->w = 0x100010;
        *(s16 *)&((SPRT *)sprite_cursor)->u0 = blink_uv;
        ((SPRT *)sprite_cursor)->clut = FIELD_HUD_BLINK_CLUT;
        addPrim(&((FieldHudDrawContext *)ctx)->hud_tag, sprite_cursor);
        sprite_cursor += sizeof(SPRT);
        ((Vec2s *)scratch)->x = x + 0x30;
        scratch[1] = y;
        number_current = state->current_hp;
        number_maximum = state->maximum_hp;
        draw_arg = number_current * 0x64;
        if (number_maximum != 0)
        {
            draw_arg = draw_arg / number_maximum;
        }
        if ((draw_arg == 0) && (number_current != 0))
        {
            draw_arg = 1;
        }
        sprite_cursor = field_emit_hud_percentage(sprite_cursor, ctx, draw_arg, (u16 *)scratch);
    }
    gauge_type = state->hud.bytes.panel_type;
    helper_cursor = sprite_cursor;
    if (gauge_type < 2U)
    {
        if (state->hud.word & 1)
        {
            if (state->status_intensity == 0xFF)
            {
                if (g_frame_counter & 8)
                {
                    draw_arg = (u32)g_field_hud_full_status_colors;
                }
                else
                {
                    draw_arg = (u32)g_field_hud_status_colors;
                }
            }
            else
            {
                draw_arg = (u32)g_field_hud_status_colors;
            }
            status_intensity = state->status_intensity;
            call_ctx = ctx;
            helper_cursor = field_emit_hud_status_line(helper_cursor, (FieldPrimitiveContext *)call_ctx,
                                                     (u32 *)draw_arg, status_intensity, x, y);
        }
        else
        {
            call_ctx = ctx;
            draw_arg = (u32)g_field_hud_effect_colors;
            status_intensity = state->effect_intensity;
            helper_cursor = field_emit_hud_status_line(helper_cursor, (FieldPrimitiveContext *)call_ctx,
                                                     (u32 *)draw_arg, status_intensity, x, y);
        }
    }
    else if ((gauge_type == 2) && ((u8)D_800FDCEA >= 0x41U))
    {
        draw_arg = (u32)g_field_hud_companion_status_colors;
        helper_cursor = field_emit_hud_status_line(helper_cursor, (FieldPrimitiveContext *)ctx,
                                                 (u32 *)draw_arg, state->status_intensity, x, y);
    }
    /* A special action gauge replaces the HP fill while its maximum is set. */
    gauge_cursor = helper_cursor;
    if (slot < 3 &&
        (entry_base = g_field_player_records, actor_entry = &entry_base[slot],
         actor_entry->special_gauge_maximum != 0) &&
        g_field_actors[slot].action_id == 0x8E)
    {
        *(u32 *)&((POLY_G4 *)gauge_cursor)->r0 = 0x202020;
        *(u32 *)&((POLY_G4 *)gauge_cursor)->r2 = 0x202020;
        setlen((POLY_G4 *)gauge_cursor, 8);
        setcode((POLY_G4 *)gauge_cursor, 0x38);
        *(u32 *)&((POLY_G4 *)gauge_cursor)->r1 = 0xFFFFFF;
        *(u32 *)&((POLY_G4 *)gauge_cursor)->r3 = 0xFFFFFF;
        special_width = (s16)actor_entry->special_gauge_maximum;
        special_width = (s32)(g_field_hud_bar_width * actor_entry->special_gauge_value) / (s32)special_width;
        ((POLY_G4 *)gauge_cursor)->x0 = (u16)g_field_hud_bar_offset_x + x;
        ((POLY_G4 *)gauge_cursor)->y1 = ((u16)g_field_hud_bar_offset_y + y);
        ((POLY_G4 *)gauge_cursor)->x1 = ((u16)g_field_hud_bar_offset_x + x) + special_width;
        ((POLY_G4 *)gauge_cursor)->x2 = ((u16)g_field_hud_bar_offset_x + x) - 3;
        ((POLY_G4 *)gauge_cursor)->y0 = ((u16)g_field_hud_bar_offset_y + y);
        ((POLY_G4 *)gauge_cursor)->y3 = ((POLY_G4 *)gauge_cursor)->y1 + (u16)g_field_hud_bar_height;
        ((POLY_G4 *)gauge_cursor)->y2 = ((POLY_G4 *)gauge_cursor)->y3;
        ((POLY_G4 *)gauge_cursor)->x3 = ((u16)g_field_hud_bar_offset_x + x) + special_width - 3;
        special_prim_addr = (s32)gauge_cursor & 0xFFFFFF;
        setaddr(gauge_cursor, getaddr((u8 *)&((FieldHudDrawContext *)ctx)->hud_tag));
        gauge_cursor += sizeof(POLY_G4);
        gauge_ot_tag = (((FieldHudDrawContext *)ctx)->hud_tag & 0xFF000000) | special_prim_addr;
        ((FieldHudDrawContext *)ctx)->hud_tag = gauge_ot_tag;
    }
    else
    {
        current_value = state->current_hp;
        if (current_value != 0)
        {
            partial_bar_count = current_value / value_per_bar;
            palette_index = partial_bar_count & 3;
            if (partial_bar_count >= 3U)
            {
                palette_index |= 2;
            }
            fill_mask = FIELD_HUD_HP_MASK;
            if ((s32)((state->hp_display_flags & fill_mask) - current_value) < (s32)value_per_bar)
            {
                partial_palette = g_field_hud_hp_colors;
                partial_color_entry = &partial_palette[palette_index];
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r0 = (s32)(*partial_color_entry & 0x3F3F3F);
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r1 = (s32)(*partial_color_entry & 0x7F7F7F);
                partial_color = *partial_color_entry;
                setlen((POLY_G4 *)gauge_cursor, 8);
                setcode((POLY_G4 *)gauge_cursor, 0x38);
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r3 = partial_color;
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r2 = partial_color;
                partial_width = (s32)(g_field_hud_bar_width * (current_value % value_per_bar)) / (s32)value_per_bar;
                ((POLY_G4 *)gauge_cursor)->x0 = (u16)g_field_hud_bar_offset_x + x;
                ((POLY_G4 *)gauge_cursor)->y1 = ((u16)g_field_hud_bar_offset_y + y);
                ((POLY_G4 *)gauge_cursor)->x1 = ((u16)g_field_hud_bar_offset_x + x) + partial_width;
                ((POLY_G4 *)gauge_cursor)->x2 = ((u16)g_field_hud_bar_offset_x + x) - 3;
                ((POLY_G4 *)gauge_cursor)->y0 = ((u16)g_field_hud_bar_offset_y + y);
                ((POLY_G4 *)gauge_cursor)->y3 = ((POLY_G4 *)gauge_cursor)->y1 + (u16)g_field_hud_bar_height;
                ((POLY_G4 *)gauge_cursor)->y2 = ((POLY_G4 *)gauge_cursor)->y3;
                ((POLY_G4 *)gauge_cursor)->x3 = ((u16)g_field_hud_bar_offset_x + x) + partial_width - 3;
                *(u32 *)&((POLY_G4 *)gauge_cursor)->tag = (s32)((*(u32 *)&((POLY_G4 *)gauge_cursor)->tag & 0xFF000000) | (((FieldHudDrawContext *)ctx)->hud_tag & fill_mask));
                partial_prim_addr = (s32)gauge_cursor & fill_mask;
                gauge_cursor += sizeof(POLY_G4);
                ((FieldHudDrawContext *)ctx)->hud_tag = (s32)((((FieldHudDrawContext *)ctx)->hud_tag & 0xFF000000) | partial_prim_addr);
            }
            value_or_bar_count = (u32)state->current_hp / value_per_bar;
            full_palette_level = value_or_bar_count - 1;
            if (value_or_bar_count != 0)
            {
                palette_index = full_palette_level & 3;
                if (full_palette_level >= 3U)
                {
                    palette_index |= 2;
                }
                full_palette = g_field_hud_hp_colors;
                full_color_entry = &full_palette[palette_index];
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r0 = (s32)(*full_color_entry & 0x3F3F3F);
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r2 = (s32)(*full_color_entry & 0x7F7F7F);
                setlen((POLY_G4 *)gauge_cursor, 8);
                setcode((POLY_G4 *)gauge_cursor, 0x38);
                full_color = *full_color_entry;
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r3 = full_color;
                *(u32 *)&((POLY_G4 *)gauge_cursor)->r1 = full_color;
                full_x = (u16)g_field_hud_bar_offset_x;
                full_right = (u16)g_field_hud_bar_width;
                full_y = (u16)g_field_hud_bar_offset_y;
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
                ((POLY_G4 *)gauge_cursor)->y3 = ((POLY_G4 *)gauge_cursor)->y1 + (u16)g_field_hud_bar_height;
                ((POLY_G4 *)gauge_cursor)->y2 = ((POLY_G4 *)gauge_cursor)->y3;
                *(u32 *)&((POLY_G4 *)gauge_cursor)->tag = (s32)((*(u32 *)&((POLY_G4 *)gauge_cursor)->tag & 0xFF000000) | (((FieldHudDrawContext *)ctx)->hud_tag & 0xFFFFFF));
                full_prim_addr = (s32)gauge_cursor & 0xFFFFFF;
                gauge_cursor += sizeof(POLY_G4);
                gauge_ot_tag = (((FieldHudDrawContext *)ctx)->hud_tag & 0xFF000000) | full_prim_addr;
                ((FieldHudDrawContext *)ctx)->hud_tag = gauge_ot_tag;
            }
        }
    }
    /* Animate the low 24 bits; retain the boss flag and temporary-panel timer. */
    mask24 = FIELD_HUD_HP_MASK;
    packed_display_value = state->hp_display_flags;
    value_or_bar_count = state->current_hp;
    displayed_value = packed_display_value & mask24;
    if (displayed_value < value_or_bar_count)
    {
        if ((u32)(value_or_bar_count - displayed_value) >= 3U)
        {
            rising_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            rising_packed_value &= packed_display_value;
            rising_value = displayed_value + ((value_or_bar_count - displayed_value) / 3);
        }
        else
        {
            rising_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            rising_packed_value &= packed_display_value;
            rising_value = displayed_value + 1;
        }
        rising_packed_value |= rising_value & mask24;
        *(volatile u32 *)&state->hp_display_flags = rising_packed_value;
        rising_current = *(volatile u32 *)&state->current_hp;
        if ((rising_current / value_per_bar) == ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) / (s32)value_per_bar))
        {
            rising_x = (u16)g_field_hud_bar_offset_x;
            rising_x += x;
            ((POLY_F4 *)gauge_cursor)->x1 =
                (s16)(rising_x + ((u32)(g_field_hud_bar_width * (rising_current % value_per_bar)) / value_per_bar));
            rising_ot = (u8 *)&((FieldHudDrawContext *)ctx)->delta_tag;
            rising_cursor = gauge_cursor;
            rising_x += ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar);
            ((POLY_F4 *)rising_cursor)->x0 = rising_x;
            helper_cursor = (u8 *)field_emit_hud_healing_quad((POLY_F4 *)rising_cursor, y, (u32 *)rising_ot);
        }
        else
        {
            rising_base_x = (u16)g_field_hud_bar_offset_x + x;
            ((POLY_F4 *)gauge_cursor)->x1 = rising_base_x;
            ((POLY_F4 *)gauge_cursor)->x0 =
                (s16)(rising_base_x +
                      ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar));
            rising_cursor = (u8 *)field_emit_hud_healing_quad((POLY_F4 *)gauge_cursor, y, (u32 *)(u8 *)&((FieldHudDrawContext *)ctx)->delta_tag);
            rising_wrapped_current = state->current_hp;
            if ((u32)((state->hp_display_flags & FIELD_HUD_HP_MASK) - rising_wrapped_current) < value_per_bar)
            {
                rising_right = (u16)g_field_hud_bar_offset_x + x +
                               ((u32)(g_field_hud_bar_width * (rising_wrapped_current % value_per_bar)) / value_per_bar);
                ((POLY_F4 *)rising_cursor)->x1 = rising_right;
            }
            else
            {
                rising_right = (u16)g_field_hud_bar_offset_x + x;
                ((POLY_F4 *)rising_cursor)->x1 = rising_right;
            }
            rising_ot = (u8 *)&((FieldHudDrawContext *)ctx)->delta_tag;
            ((POLY_F4 *)rising_cursor)->x0 = (s16)(((u16)g_field_hud_bar_offset_x + x) + (u16)g_field_hud_bar_width);
            helper_cursor = (u8 *)field_emit_hud_healing_quad((POLY_F4 *)rising_cursor, y, (u32 *)rising_ot);
        }
        gauge_cursor = helper_cursor;
    }
    else if (value_or_bar_count < displayed_value)
    {
        if ((u32)(displayed_value - value_or_bar_count) >= 4U)
        {
            falling_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            falling_packed_value &= packed_display_value;
            falling_value = displayed_value - ((displayed_value - value_or_bar_count) / 3);
        }
        else
        {
            falling_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            falling_packed_value &= packed_display_value;
            falling_value = displayed_value - 1;
        }
        falling_packed_value |= falling_value & mask24;
        *(volatile u32 *)&state->hp_display_flags = falling_packed_value;
        falling_current = *(volatile u32 *)&state->current_hp;
        if ((falling_current / value_per_bar) == ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) / (s32)value_per_bar))
        {
            falling_x = (u16)g_field_hud_bar_offset_x;
            falling_x += x;
            ((POLY_F4 *)gauge_cursor)->x1 =
                (s16)(falling_x + ((u32)(g_field_hud_bar_width * (falling_current % value_per_bar)) / value_per_bar));
            falling_ot = (u8 *)&((FieldHudDrawContext *)ctx)->delta_tag;
            falling_cursor = gauge_cursor;
            falling_x +=
                ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar);
            ((POLY_F4 *)falling_cursor)->x0 = falling_x;
            helper_cursor = (u8 *)field_emit_hud_damage_quad((POLY_F4 *)falling_cursor, y, (u32 *)falling_ot);
        }
        else
        {
            falling_base_x = (u16)g_field_hud_bar_offset_x + x;
            ((POLY_F4 *)gauge_cursor)->x1 = falling_base_x;
            ((POLY_F4 *)gauge_cursor)->x0 =
                (s16)(falling_base_x +
                      ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar));
            falling_cursor = (u8 *)field_emit_hud_damage_quad((POLY_F4 *)gauge_cursor, y, (u32 *)(u8 *)&((FieldHudDrawContext *)ctx)->delta_tag);
            falling_wrapped_current = state->current_hp;
            if ((u32)((state->hp_display_flags & FIELD_HUD_HP_MASK) - falling_wrapped_current) < value_per_bar)
            {
                falling_right = (u16)g_field_hud_bar_offset_x + x +
                                ((u32)(g_field_hud_bar_width * (falling_wrapped_current % value_per_bar)) / value_per_bar);
                ((POLY_F4 *)falling_cursor)->x1 = falling_right;
            }
            else
            {
                falling_right = (u16)g_field_hud_bar_offset_x + x;
                ((POLY_F4 *)falling_cursor)->x1 = falling_right;
            }
            falling_ot = (u8 *)&((FieldHudDrawContext *)ctx)->delta_tag;
            ((POLY_F4 *)falling_cursor)->x0 = (s16)(((u16)g_field_hud_bar_offset_x + x) + (u16)g_field_hud_bar_width);
            helper_cursor = (u8 *)field_emit_hud_damage_quad((POLY_F4 *)falling_cursor, y, (u32 *)falling_ot);
        }
        gauge_cursor = helper_cursor;
    }
    sprite_cursor = gauge_cursor;
    *(u32 *)&((SPRT *)sprite_cursor)->r0 = 0x808080;
    setSprt((SPRT *)sprite_cursor);
    setSemiTrans((SPRT *)sprite_cursor, 1);
    *(u32 *)&((SPRT *)sprite_cursor)->x0 = (s32)((y << 0x10) + x);
    background_type = state->hud.bytes.panel_type;
    switch (background_type)
    {
        case 0:
        case 1:
            *(u32 *)&((SPRT *)sprite_cursor)->w = 0x180058;
            *(s16 *)&((SPRT *)sprite_cursor)->u0 = 0x1000;
            break;
        case 2:
            if ((u8)D_800FDCEA >= 0x41U)
            {
                *(u32 *)&((SPRT *)sprite_cursor)->w = 0x180058;
                *(s16 *)&((SPRT *)sprite_cursor)->u0 = 0x1000;
            }
            else
            {
                *(u32 *)&((SPRT *)sprite_cursor)->w = 0x180058;
                *(s16 *)&((SPRT *)sprite_cursor)->u0 = 0x2800;
            }
            break;
        default:
            if (boss_panel != 0)
            {
                *(u32 *)&((SPRT *)sprite_cursor)->w = 0x100100;
                *(s16 *)&((SPRT *)sprite_cursor)->u0 = 0;
            }
            else
            {
                *(u32 *)&((SPRT *)sprite_cursor)->w = 0x100040;
                *(s16 *)&((SPRT *)sprite_cursor)->u0 = 0x4000;
            }
            break;
    }
    ((SPRT *)sprite_cursor)->clut = FIELD_HUD_CLUT;
    addPrim(&((FieldHudDrawContext *)ctx)->hud_tag, sprite_cursor);
    sprite_cursor += sizeof(SPRT);
    setDrawTPage((DR_TPAGE *)sprite_cursor, 0, 0, 0x1F);
    addPrim(&((FieldHudDrawContext *)ctx)->hud_tag, sprite_cursor);
    ((FieldHudDrawContext *)render_context)->primitive_cursor = sprite_cursor + sizeof(DR_TPAGE);
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
u8 *field_emit_hud_status_line(u8 *packet, FieldPrimitiveContext *context, u32 *colors, s32 intensity, s32 x, s32 y)
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

    first_component = ((CVECTOR *)colors)[0].r;
    second_component = ((CVECTOR *)colors)[1].r;
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

    first_component = ((CVECTOR *)colors)[0].g;
    second_component = ((CVECTOR *)colors)[1].g;
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

    first_component = ((CVECTOR *)colors)[0].b;
    second_component = ((CVECTOR *)colors)[1].b;
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
 * @brief Emit a right-aligned HP percentage followed by the percent
 *        glyph, advancing the horizontal cursor 7 units per column.
 * @param packet Primitive-buffer cursor passed through the digit emitter.
 * @param context Opaque draw context forwarded to field_emit_hud_glyph.
 * @param value HP percentage from 0 through 100.
 * @param position Pointer to the current X cursor, bumped by 7 after each column.
 * @return Cursor immediately after the percent glyph.
 * @note Leading zeros in the hundreds/tens columns are suppressed until the
 *       first non-zero digit is emitted.
 */
void *field_emit_hud_percentage(void *packet, void *context, s32 value, u16 *position)
{
    void *field_emit_hud_glyph(void *, void *, s32, s32 *);

    s32 emitted;

    emitted = 0;
    if (value / 100 != 0)
    {
        emitted = 1;
        packet = field_emit_hud_glyph(packet, context, value / 100, (s32 *)position);
        value -= 100;
    }
    *position += FIELD_HUD_GLYPH_ADVANCE;
    if (emitted || value / 10 != 0)
    {
        s32 digit;
        digit = value / 10;
        packet = field_emit_hud_glyph(packet, context, digit, (s32 *)position);
        value -= digit * 10;
    }
    *position += FIELD_HUD_GLYPH_ADVANCE;
    packet = field_emit_hud_glyph(packet, context, value, (s32 *)position);
    *position += FIELD_HUD_GLYPH_ADVANCE;
    return field_emit_hud_glyph(packet, context, FIELD_HUD_GLYPH_PERCENT, (s32 *)position);
}

/**
 * @brief Emit a digit or percent glyph from the HUD texture strip.
 * @param packet Primitive-buffer cursor.
 * @param context Draw context containing the HUD ordering-table tag.
 * @param value Glyph index: 0 through 9 for digits, 10 for percent.
 * @param position Packed screen X/Y coordinates.
 * @return Cursor immediately after the sprite.
 * @see decomp.me (100%) TODO
 */
void *field_emit_hud_glyph(void *packet, void *context, s32 value, s32 *position)
{
    SPRT *sprite;
    FieldPrimitiveContext *ot;
    s32 packed_xy;

    sprite = (SPRT *)packet;
    ot = (FieldPrimitiveContext *)context;

    *(u32 *)&sprite->r0 = 0x808080;
    setSprt(sprite);
    packed_xy = *position;
    *(s16 *)&sprite->u0 = (s16)((value * 8) + 0x1558);
    *(u32 *)&sprite->w = 0xB0008;
    sprite->clut = FIELD_HUD_CLUT;
    *(u32 *)&sprite->x0 = packed_xy;
    addPrim(&ot->tag, sprite);
    return (u8 *)packet + sizeof(SPRT);
}

/**
 * @brief Build a red damage quad from the caller's x0/x1, bordered 3 pixels inward at the bottom, and link it into the OT.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to g_field_hud_bar_offset_y.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4 *field_emit_hud_damage_quad(POLY_F4 *prim, s32 y, u32 *ot)
{
    extern s32 g_field_hud_bar_height;
    extern s32 g_field_hud_bar_offset_y;

    *((u32 *)&prim->r0) = 0xFF;
    setPolyF4(prim);
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)g_field_hud_bar_offset_y) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)g_field_hud_bar_offset_y) + y;
    prim->y3 = prim->y1 + ((u16)g_field_hud_bar_height);
    prim->y2 = prim->y3;
    addPrim(ot, prim);
    return prim + 1;
}

/**
 * @brief Build a white healing quad like field_emit_hud_damage_quad, widening a zero-width x0/x1 pair to one pixel first.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to g_field_hud_bar_offset_y.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4 *field_emit_hud_healing_quad(POLY_F4 *prim, s32 y, u32 *ot)
{
    extern s32 g_field_hud_bar_height;
    extern s32 g_field_hud_bar_offset_y;

    if (prim->x1 == prim->x0)
    {
        prim->x1 = prim->x0 + 1;
    }
    *((u32 *)&prim->r0) = 0xFFFFFF;
    setPolyF4(prim);
    prim->x3 = prim->x1 - 3;
    prim->y1 = ((u16)g_field_hud_bar_offset_y) + y;
    prim->x2 = prim->x0 - 3;
    prim->y0 = ((u16)g_field_hud_bar_offset_y) + y;
    prim->y3 = prim->y1 + ((u16)g_field_hud_bar_height);
    prim->y2 = prim->y3;
    addPrim(ot, prim);
    return prim + 1;
}

/**
 * @brief Emit an actor portrait and its texture-page selection.
 * @param sprt Sprite primitive to populate.
 * @param ot Ordering table the primitive (and its tpage) are linked into.
 * @param index Party slot; slot 2 selects the companion palette when applicable.
 * @param xy Packed screen position copied into the sprite's x0/y0.
 * @return Pointer just past the appended DR_TPAGE primitive.
 */
void *field_emit_actor_portrait(SPRT *sprt, u32 *ot, s32 index, u32 *xy)
{
    extern u8 D_800FDCEA;
    extern u16 D_800FE01E;
    extern FieldHudActorTexture g_field_actors[];

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
        sprt->clut = ((index + 0x1F4) << 6) | ((g_field_actors[index].texture_flags >> 19) & 0xF);
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
 * @brief Loads a VRAM resource from disc and uploads it via field_upload_image_resource.
 *
 * Queues a CD read of resource @p id (masked to 16 bits) into the shared field
 * CD buffer @c D_8010D038, waits for the queue to drain, then hands the loaded
 * blob to field_upload_image_resource to upload into VRAM using @p rect and @p mode.
 *
 * @param id Resource index; masked to 16 bits for the CD queue.
 * @param rect Destination rectangle forwarded to field_upload_image_resource.
 * @param mode Upload mode forwarded to field_upload_image_resource.
 */
void field_load_vram_resource(s32 id, s16 *rect, s32 mode)
{
    extern u8 *D_8010D038;
    s32 field_upload_image_resource(RECT *rect, Tim *resource, s32 mode);

    u8 *buf = D_8010D038;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    field_upload_image_resource((RECT *)rect, (Tim *)buf, mode);
}

/**
 * @brief Upload a two-part image resource (palette/CLUT block followed by the
 *        pixel block) into VRAM and report the word at byte offset 0x1F4.
 * @param rect Destination framebuffer rectangle; supplies the upload x/y
 *             (from @c rect->x / @c rect->y) and the CLUT destination x/y (from
 *             @c rect->w / @c rect->h), and returns the pixel width/height in x/y.
 * @param resource TIM file containing a palette block followed by a pixel block.
 * @param mode When non-zero, upload the CLUT as a single 1-tall run of
 *             width*height entries; when zero, upload it with its natural
 *             width and height.
 * @return Packed palette entries 240 and 241 (the word at byte offset 0x1F4).
 * @see decomp.me (100.00%)
 */
s32 field_upload_image_resource(RECT *rect, Tim *resource, s32 mode)
{
    RECT load_rect;
    s32 offset;
    s32 palette_word;
    TimBlock *image;
    TimDimensions *dimensions;
    u16 x;

    /* The caller supplies separate pixel and palette destinations. */
    dimensions = &resource->clut_block.dimensions;
    offset = resource->clut_block.bnum;
    if (mode != 0)
    {
        setRECT(&load_rect, rect->w, rect->h, dimensions->width * dimensions->height, 1);
    }
    else
    {
        load_rect.x = rect->w;
        load_rect.y = rect->h;
        load_rect.w = dimensions->width;
        load_rect.h = dimensions->height;
    }
    LoadImage(&load_rect, (u_long *)resource->clut_data);

    image = TIM_PIXEL_BLOCK(resource, offset);
    x = rect->x;
    palette_word = *(s32 *)&resource->clut_data[240];
    load_rect.x = x;
    load_rect.y = rect->y;
    dimensions = &image->dimensions;
    load_rect.w = dimensions->width;
    load_rect.h = dimensions->height;
    LoadImage(&load_rect, (u_long *)(image + 1));
    rect->x = dimensions->width;
    rect->y = dimensions->height;
    return palette_word;
}
