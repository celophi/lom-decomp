/**
 * @file field_actor_hud_effects.c
 * @brief Field actor HUD renderer and actor control effects: participant panels,
 *        HP and status-bar primitives, the HUD image loader, control-flag
 *        handlers, horizontal scaling and the ground-shadow renderer.
 *
 * One translation unit: the control-effect handler table D_800EB00C (used by
 * func_80086494) sits inside the HUD's initialized data, between
 * g_field_hud_companion_status_colors and g_field_party_hud_order.
 */

/* ---- Field actor HUD ----------------------------------------------------- */

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
    volatile u32 current_hp;
    volatile u32 hp_display_flags;
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
    u8* primitive_cursor;
} FieldHudDrawContext;

/**
 * @brief Draw participant panels and temporary indicators for eligible actors.
 * @param render_context Rendering context forwarded to the panel drawing helper.
 * @note Actor positions use signed fixed-point division, followed by screen clamps.
 * @note Display-value bits 24 through 30 count down after a temporary panel draw.
 */
void field_draw_actor_hud(u8* render_context)
{
    extern FieldPanelActor g_field_actors[], g_field_effect_records[];
    extern FieldPanelSlot g_field_object_states[];
    extern FieldPanelPlayer g_field_player_records[];
    extern s32 g_field_party_hud_order[];

    extern s32 g_field_active_group, g_field_scene_mode_bit, D_80122B20;
    extern void field_draw_actor_hud_panel(s32, s32, s32, u8*, u32);

    Vec2s position;
    s32 i = 0;
    s32 panel_count = i;
    s32 absent_actor = 0xFF;
    s32 excluded_action;
    FieldPanelActor* single_actor;
    FieldPanelPlayer* single_player;
    FieldPanelActor* paired_actor;
    FieldPanelPlayer* paired_player;
    s16 panel_y;
    s32 boss_drawn;
    FieldPanelActor* anchor_actor;
    s32 group;
    s32 hp_display;
    u32 current_hp;
    u32 displayed_hp;
    s32 world_y;
    s32 projected_y;
    s32 coordinate;
    s32 camera_y;
    s32 hud_ticks;
    u32 updated_hp_display;

    do
    {
        if (g_field_actors[i].presence != absent_actor && (g_field_player_records[i].flags & 1))
        {
            excluded_action = 0x85;
            if (g_field_actors[i].action_id != excluded_action)
            {
                excluded_action = 0x87;
                if (g_field_actors[i].action_id != excluded_action)
                {
                    g_field_object_states[i].hud_options |= 1;
                }
            }
            panel_count++;
        }
        i++;
    } while (i < 3);
    switch (panel_count)
    {
    case 1:
        i = 0;
        do
        {
            single_actor = &g_field_actors[i];
            single_player = &g_field_player_records[i];
            if (single_actor->presence != 0xFF && (single_player->flags & 1))
            {
                field_draw_actor_hud_panel(0x70, 0x10, i, render_context, 0x64);
            }
            i++;
        } while (i < 3);
        break;
    case 2:
        i = 0;
        panel_count = 0;
        do
        {
            paired_actor = &g_field_actors[i];
            paired_player = &g_field_player_records[i];
            if (paired_actor->presence != 0xFF && (paired_player->flags & 1))
            {
                field_draw_actor_hud_panel(0x38 + panel_count * 0x6C, 0x10, i, render_context, 0x64);
                panel_count++;
            }
            i++;
        } while (i < 3);
        break;
    case 3:
        i = 0;
        panel_count = 0;
        do
        {
            if (g_field_actors[g_field_party_hud_order[i]].presence != 0xFF && (g_field_player_records[g_field_party_hud_order[i]].flags & 1))
            {
                panel_y = 0x1C;
                if (i & 1)
                {
                    panel_y = 4;
                }
                field_draw_actor_hud_panel(8 + panel_count * 0x68, panel_y, g_field_party_hud_order[i], render_context, 0x64);
                panel_count++;
            }
            i++;
        } while (i < 3);
        break;
    }
    boss_drawn = 0;
    /* Enemy panels are transient; the boss uses a fixed panel at the bottom. */
    if (g_field_scene_mode_bit != 0)
    {
        i = 3;
        if (D_80122B20 == 0)
        {
            do
            {
                group = g_field_object_states[i].group & 0xF;
                if (group == g_field_active_group && group != 0)
                {
                    hp_display = g_field_object_states[i].hp_display.word;
                    if (hp_display < 0)
                    {
                        if (g_field_actors[i].presence != 0xFF && (hp_display & FIELD_HUD_HP_MASK) && boss_drawn == 0)
                        {
                            field_draw_actor_hud_panel(0x20, 0xC0, i, render_context, 0x190);
                            boss_drawn = 1;
                        }
                    }
                    else if (g_field_actors[i].presence != 0xFF)
                    {
                        current_hp = g_field_object_states[i].current_hp;
                        displayed_hp = hp_display & FIELD_HUD_HP_MASK;
                        if (current_hp < displayed_hp)
                        {
                            g_field_object_states[i].hp_display.word = (hp_display & 0x80FFFFFF) | 0x14000000;
                        }
                        else if (displayed_hp != current_hp)
                        {
                            g_field_object_states[i].hp_display.word = (hp_display & 0x80FFFFFF) | 0x14000000;
                        }
                        if (g_field_object_states[i].hp_display.bytes[3] & 0x7F)
                        {
                            if (!(g_field_object_states[i].flags & 0x100) && (g_field_object_states[i].contact.bytes[0] & 1) &&
                                (g_field_effect_records[g_field_object_states[i].linked_effect_index].state & 0x7F) != 0x2F)
                            {
                                anchor_actor = &g_field_effect_records[g_field_object_states[i].linked_effect_index];
                                {
                                    s32 camera_x = g_field_view_offset_x / 256;
                                    s32 world_x = anchor_actor->x / 256 + 0xA0;
                                    coordinate = camera_x + world_x;
                                    position.x = coordinate;
                                }
                                projected_y = g_field_view_offset_y / 256;
                                world_y = g_field_effect_records[g_field_object_states[i].linked_effect_index].y / 256 + 0x70;
                                projected_y = projected_y + world_y;
                                coordinate =
                                    projected_y - g_field_effect_records[g_field_object_states[i].linked_effect_index].z / 512 - g_field_view_offset_z / 512;
                                position.y = coordinate;

                                coordinate = position.x;
                                if (coordinate + 0x20 >= 0x141)
                                {
                                    position.x = 0x120;
                                }
                                coordinate = position.x;
                                if (coordinate < 0x20)
                                {
                                    position.x = 0x20;
                                }
                                coordinate = position.y;
                                if (coordinate >= 0xD1)
                                {
                                    position.y = 0xD0;
                                }
                                coordinate = position.y;
                                if (coordinate < 0x10)
                                {
                                    position.y = 0x10;
                                }
                            }
                            else
                            {
                                {
                                    s32 camera_x = g_field_view_offset_x / 256;
                                    s32 world_x = g_field_actors[i].x / 256 + 0xA0;
                                    coordinate = camera_x + world_x;
                                    position.x = coordinate;
                                }
                                camera_y = g_field_view_offset_y / 256;
                                {
                                    s32 actor_y = g_field_actors[i].y / 256 + 0x70;
                                    projected_y = camera_y + actor_y;
                                }
                                coordinate = projected_y - g_field_actors[i].z / 512 - g_field_view_offset_z / 512;
                                position.y = coordinate;

                                coordinate = position.x;
                                if (coordinate + 0x20 >= 0x141)
                                {
                                    position.x = 0x120;
                                }
                                coordinate = position.x;
                                if (coordinate < 0x20)
                                {
                                    position.x = 0x20;
                                }
                                coordinate = position.y;
                                if (coordinate >= 0xD1)
                                {
                                    position.y = 0xD0;
                                }
                                coordinate = position.y;
                                if (coordinate < 0x10)
                                {
                                    position.y = 0x10;
                                }
                            }
                            field_draw_actor_hud_panel(position.x - 0x1C, position.y, i, render_context, 0x64);
                            updated_hp_display = g_field_object_states[i].hp_display.word;
                            hud_ticks = ((u32)updated_hp_display >> 24) & 0x7F;
                            if (hud_ticks != 0)
                            {
                                g_field_object_states[i].hp_display.word = (updated_hp_display & 0x80FFFFFF) | (((hud_ticks - 1) & 0x7F) << 24);
                            }
                        }
                    }
                }
                i++;
            } while (i < 13);
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
 */
void field_draw_actor_hud_panel(s32 x, s32 y, s32 slot, u8* render_context, u32 value_per_bar)
{
    u8* field_emit_hud_status_line(u8*, FieldPrimitiveContext*, u32*, s32, s32, s32);
    void* field_emit_hud_percentage(void*, void*, s32, u16*);
    POLY_F4* field_emit_hud_damage_quad(POLY_F4*, s32, u32*);
    POLY_F4* field_emit_hud_healing_quad(POLY_F4*, s32, u32*);
    void* field_emit_actor_portrait(SPRT*, u32*, s32, u32*);
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

    s16* scratch = (s16*)0x1F800000;
    u8* ctx;
    u8* call_ctx;
    u32 special_width;
    s32 scan_slot;
    u32* partial_palette;
    u32* full_palette;
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
    s32* full_color_entry;
    s32* blink_counter;
    s32* blink_counters;
    s32* partial_color_entry;
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
    u32 rising_displayed;
    u32 falling_displayed;
    u32 number_maximum;
    u32 current_value;
    u32 draw_arg;
    u32 mask24;
    u32 fill_mask;
    s32 background_type;
    u8 gauge_type;
    FieldHudPlayer* actor_entry;
    FieldHudPlayer* entry_base;
    FieldHudGaugeState* state;
    u8* label_cursor;
    u8* rising_cursor;
    u8* helper_cursor;
    u8* falling_cursor;
    u8* rising_ot;
    u8* falling_ot;
    u8* sprite_cursor;
    u8* gauge_cursor;

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
    label_cursor = ((FieldHudDrawContext*)ctx)->primitive_cursor;
    if (state->hud.bytes.panel_type < 3U)
    {
        g_field_hud_bar_offset_x = 0x1D;
        g_field_hud_bar_offset_y = 9;
        scratch[0] = x;
        scratch[1] = y;
        label_cursor = field_emit_actor_portrait((SPRT*)label_cursor, &((FieldHudDrawContext*)ctx)->hud_tag, state->hud.bytes.panel_type, (u32*)scratch);
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
        if (g_field_hud_blink_frames[slot] >= FIELD_HUD_BLINK_FRAMES || ++g_field_hud_blink_frames[slot] >= FIELD_HUD_BLINK_FRAMES)
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
        *(u32*)&((SPRT*)sprite_cursor)->r0 = 0x808080;
        setSprt((SPRT*)sprite_cursor);
        ((SPRT*)sprite_cursor)->x0 = (s16)(x + 0x22);
        ((SPRT*)sprite_cursor)->y0 = (s16)(y - 6);
        if (g_field_hud_blink_frames[slot] < 6)
        {
            blink_uv = ((u16)g_field_hud_blink_frames[slot] * 0x10) + 0x2058;
        }
        else
        {
            blink_uv = 0x2058;
        }
        *(u32*)&((SPRT*)sprite_cursor)->w = 0x100010;
        *(s16*)&((SPRT*)sprite_cursor)->u0 = blink_uv;
        ((SPRT*)sprite_cursor)->clut = FIELD_HUD_BLINK_CLUT;
        addPrim(&((FieldHudDrawContext*)ctx)->hud_tag, sprite_cursor);
        sprite_cursor += sizeof(SPRT);
        ((Vec2s*)scratch)->x = x + 0x30;
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
        sprite_cursor = field_emit_hud_percentage(sprite_cursor, ctx, draw_arg, (u16*)scratch);
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
            helper_cursor = field_emit_hud_status_line(helper_cursor, (FieldPrimitiveContext*)call_ctx, (u32*)draw_arg, status_intensity, x, y);
        }
        else
        {
            call_ctx = ctx;
            draw_arg = (u32)g_field_hud_effect_colors;
            status_intensity = state->effect_intensity;
            helper_cursor = field_emit_hud_status_line(helper_cursor, (FieldPrimitiveContext*)call_ctx, (u32*)draw_arg, status_intensity, x, y);
        }
    }
    else if ((gauge_type == 2) && ((u8)D_800FDCEA >= 0x41U))
    {
        draw_arg = (u32)g_field_hud_companion_status_colors;
        helper_cursor = field_emit_hud_status_line(helper_cursor, (FieldPrimitiveContext*)ctx, (u32*)draw_arg, state->status_intensity, x, y);
    }
    /* A special action gauge replaces the HP fill while its maximum is set. */
    gauge_cursor = helper_cursor;
    if (slot < 3 && (entry_base = g_field_player_records, actor_entry = &entry_base[slot], actor_entry->special_gauge_maximum != 0) &&
        g_field_actors[slot].action_id == 0x8E)
    {
        *(u32*)&((POLY_G4*)gauge_cursor)->r0 = 0x202020;
        *(u32*)&((POLY_G4*)gauge_cursor)->r2 = 0x202020;
        setlen((POLY_G4*)gauge_cursor, 8);
        setcode((POLY_G4*)gauge_cursor, 0x38);
        *(u32*)&((POLY_G4*)gauge_cursor)->r1 = 0xFFFFFF;
        *(u32*)&((POLY_G4*)gauge_cursor)->r3 = 0xFFFFFF;
        special_width = (s16)actor_entry->special_gauge_maximum;
        special_width = (s32)(g_field_hud_bar_width * actor_entry->special_gauge_value) / (s32)special_width;
        ((POLY_G4*)gauge_cursor)->x0 = (u16)g_field_hud_bar_offset_x + x;
        ((POLY_G4*)gauge_cursor)->y1 = ((u16)g_field_hud_bar_offset_y + y);
        ((POLY_G4*)gauge_cursor)->x1 = ((u16)g_field_hud_bar_offset_x + x) + special_width;
        ((POLY_G4*)gauge_cursor)->x2 = ((u16)g_field_hud_bar_offset_x + x) - 3;
        ((POLY_G4*)gauge_cursor)->y0 = ((u16)g_field_hud_bar_offset_y + y);
        ((POLY_G4*)gauge_cursor)->y3 = ((POLY_G4*)gauge_cursor)->y1 + (u16)g_field_hud_bar_height;
        ((POLY_G4*)gauge_cursor)->y2 = ((POLY_G4*)gauge_cursor)->y3;
        ((POLY_G4*)gauge_cursor)->x3 = ((u16)g_field_hud_bar_offset_x + x) + special_width - 3;
        special_prim_addr = (s32)gauge_cursor & 0xFFFFFF;
        setaddr(gauge_cursor, getaddr((u8*)&((FieldHudDrawContext*)ctx)->hud_tag));
        gauge_cursor += sizeof(POLY_G4);
        gauge_ot_tag = (((FieldHudDrawContext*)ctx)->hud_tag & 0xFF000000) | special_prim_addr;
        ((FieldHudDrawContext*)ctx)->hud_tag = gauge_ot_tag;
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
                *(u32*)&((POLY_G4*)gauge_cursor)->r0 = (s32)(*partial_color_entry & 0x3F3F3F);
                *(u32*)&((POLY_G4*)gauge_cursor)->r1 = (s32)(*partial_color_entry & 0x7F7F7F);
                partial_color = *partial_color_entry;
                setlen((POLY_G4*)gauge_cursor, 8);
                setcode((POLY_G4*)gauge_cursor, 0x38);
                *(u32*)&((POLY_G4*)gauge_cursor)->r3 = partial_color;
                *(u32*)&((POLY_G4*)gauge_cursor)->r2 = partial_color;
                partial_width = (s32)(g_field_hud_bar_width * (current_value % value_per_bar)) / (s32)value_per_bar;
                ((POLY_G4*)gauge_cursor)->x0 = (u16)g_field_hud_bar_offset_x + x;
                ((POLY_G4*)gauge_cursor)->y1 = ((u16)g_field_hud_bar_offset_y + y);
                ((POLY_G4*)gauge_cursor)->x1 = ((u16)g_field_hud_bar_offset_x + x) + partial_width;
                ((POLY_G4*)gauge_cursor)->x2 = ((u16)g_field_hud_bar_offset_x + x) - 3;
                ((POLY_G4*)gauge_cursor)->y0 = ((u16)g_field_hud_bar_offset_y + y);
                ((POLY_G4*)gauge_cursor)->y3 = ((POLY_G4*)gauge_cursor)->y1 + (u16)g_field_hud_bar_height;
                ((POLY_G4*)gauge_cursor)->y2 = ((POLY_G4*)gauge_cursor)->y3;
                ((POLY_G4*)gauge_cursor)->x3 = ((u16)g_field_hud_bar_offset_x + x) + partial_width - 3;
                *(u32*)&((POLY_G4*)gauge_cursor)->tag =
                    (s32)((*(u32*)&((POLY_G4*)gauge_cursor)->tag & 0xFF000000) | (((FieldHudDrawContext*)ctx)->hud_tag & fill_mask));
                partial_prim_addr = (s32)gauge_cursor & fill_mask;
                gauge_cursor += sizeof(POLY_G4);
                ((FieldHudDrawContext*)ctx)->hud_tag = (s32)((((FieldHudDrawContext*)ctx)->hud_tag & 0xFF000000) | partial_prim_addr);
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
                *(u32*)&((POLY_G4*)gauge_cursor)->r0 = (s32)(*full_color_entry & 0x3F3F3F);
                *(u32*)&((POLY_G4*)gauge_cursor)->r2 = (s32)(*full_color_entry & 0x7F7F7F);
                setlen((POLY_G4*)gauge_cursor, 8);
                setcode((POLY_G4*)gauge_cursor, 0x38);
                full_color = *full_color_entry;
                *(u32*)&((POLY_G4*)gauge_cursor)->r3 = full_color;
                *(u32*)&((POLY_G4*)gauge_cursor)->r1 = full_color;
                full_x = (u16)g_field_hud_bar_offset_x;
                full_right = (u16)g_field_hud_bar_width;
                full_y = (u16)g_field_hud_bar_offset_y;
                full_x += x;
                full_right += full_x;
                ((POLY_G4*)gauge_cursor)->x1 = full_right;
                full_right -= 3;
                full_y += y;
                ((POLY_G4*)gauge_cursor)->x3 = full_right;
                ((POLY_G4*)gauge_cursor)->y1 = full_y;
                ((POLY_G4*)gauge_cursor)->x0 = full_x;
                full_x -= 3;
                ((POLY_G4*)gauge_cursor)->x2 = full_x;
                ((POLY_G4*)gauge_cursor)->y0 = full_y;
                ((POLY_G4*)gauge_cursor)->y3 = ((POLY_G4*)gauge_cursor)->y1 + (u16)g_field_hud_bar_height;
                ((POLY_G4*)gauge_cursor)->y2 = ((POLY_G4*)gauge_cursor)->y3;
                *(u32*)&((POLY_G4*)gauge_cursor)->tag =
                    (s32)((*(u32*)&((POLY_G4*)gauge_cursor)->tag & 0xFF000000) | (((FieldHudDrawContext*)ctx)->hud_tag & 0xFFFFFF));
                full_prim_addr = (s32)gauge_cursor & 0xFFFFFF;
                gauge_cursor += sizeof(POLY_G4);
                gauge_ot_tag = (((FieldHudDrawContext*)ctx)->hud_tag & 0xFF000000) | full_prim_addr;
                ((FieldHudDrawContext*)ctx)->hud_tag = gauge_ot_tag;
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
        state->hp_display_flags = rising_packed_value;
        rising_current = state->current_hp;
        if ((rising_current / value_per_bar) == ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) / (s32)value_per_bar))
        {
            rising_x = (u16)g_field_hud_bar_offset_x;
            rising_x += x;
            ((POLY_F4*)gauge_cursor)->x1 = (s16)(rising_x + ((u32)(g_field_hud_bar_width * (rising_current % value_per_bar)) / value_per_bar));
            rising_ot = (u8*)&((FieldHudDrawContext*)ctx)->delta_tag;
            rising_cursor = gauge_cursor;
            rising_x += ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar);
            ((POLY_F4*)rising_cursor)->x0 = rising_x;
            helper_cursor = (u8*)field_emit_hud_healing_quad((POLY_F4*)rising_cursor, y, (u32*)rising_ot);
        }
        else
        {
            rising_base_x = (u16)g_field_hud_bar_offset_x + x;
            ((POLY_F4*)gauge_cursor)->x1 = rising_base_x;
            ((POLY_F4*)gauge_cursor)->x0 =
                (s16)(rising_base_x +
                      ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar));
            rising_cursor = (u8*)field_emit_hud_healing_quad((POLY_F4*)gauge_cursor, y, (u32*)(u8*)&((FieldHudDrawContext*)ctx)->delta_tag);
            rising_displayed = state->hp_display_flags & FIELD_HUD_HP_MASK;
            rising_wrapped_current = state->current_hp;
            if (rising_displayed - rising_wrapped_current < value_per_bar)
            {
                rising_right = (u16)g_field_hud_bar_offset_x + x + ((u32)(g_field_hud_bar_width * (rising_wrapped_current % value_per_bar)) / value_per_bar);
                ((POLY_F4*)rising_cursor)->x1 = rising_right;
            }
            else
            {
                rising_right = (u16)g_field_hud_bar_offset_x + x;
                ((POLY_F4*)rising_cursor)->x1 = rising_right;
            }
            rising_ot = (u8*)&((FieldHudDrawContext*)ctx)->delta_tag;
            ((POLY_F4*)rising_cursor)->x0 = (s16)(((u16)g_field_hud_bar_offset_x + x) + (u16)g_field_hud_bar_width);
            helper_cursor = (u8*)field_emit_hud_healing_quad((POLY_F4*)rising_cursor, y, (u32*)rising_ot);
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
        state->hp_display_flags = falling_packed_value;
        falling_current = state->current_hp;
        if ((falling_current / value_per_bar) == ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) / (s32)value_per_bar))
        {
            falling_x = (u16)g_field_hud_bar_offset_x;
            falling_x += x;
            ((POLY_F4*)gauge_cursor)->x1 = (s16)(falling_x + ((u32)(g_field_hud_bar_width * (falling_current % value_per_bar)) / value_per_bar));
            falling_ot = (u8*)&((FieldHudDrawContext*)ctx)->delta_tag;
            falling_cursor = gauge_cursor;
            falling_x += ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar);
            ((POLY_F4*)falling_cursor)->x0 = falling_x;
            helper_cursor = (u8*)field_emit_hud_damage_quad((POLY_F4*)falling_cursor, y, (u32*)falling_ot);
        }
        else
        {
            falling_base_x = (u16)g_field_hud_bar_offset_x + x;
            ((POLY_F4*)gauge_cursor)->x1 = falling_base_x;
            ((POLY_F4*)gauge_cursor)->x0 =
                (s16)(falling_base_x +
                      ((s32)(g_field_hud_bar_width * ((s32)(state->hp_display_flags & FIELD_HUD_HP_MASK) % (s32)value_per_bar)) / (s32)value_per_bar));
            falling_cursor = (u8*)field_emit_hud_damage_quad((POLY_F4*)gauge_cursor, y, (u32*)(u8*)&((FieldHudDrawContext*)ctx)->delta_tag);
            falling_displayed = state->hp_display_flags & FIELD_HUD_HP_MASK;
            falling_wrapped_current = state->current_hp;
            if (falling_displayed - falling_wrapped_current < value_per_bar)
            {
                falling_right = (u16)g_field_hud_bar_offset_x + x + ((u32)(g_field_hud_bar_width * (falling_wrapped_current % value_per_bar)) / value_per_bar);
                ((POLY_F4*)falling_cursor)->x1 = falling_right;
            }
            else
            {
                falling_right = (u16)g_field_hud_bar_offset_x + x;
                ((POLY_F4*)falling_cursor)->x1 = falling_right;
            }
            falling_ot = (u8*)&((FieldHudDrawContext*)ctx)->delta_tag;
            ((POLY_F4*)falling_cursor)->x0 = (s16)(((u16)g_field_hud_bar_offset_x + x) + (u16)g_field_hud_bar_width);
            helper_cursor = (u8*)field_emit_hud_damage_quad((POLY_F4*)falling_cursor, y, (u32*)falling_ot);
        }
        gauge_cursor = helper_cursor;
    }
    sprite_cursor = gauge_cursor;
    *(u32*)&((SPRT*)sprite_cursor)->r0 = 0x808080;
    setSprt((SPRT*)sprite_cursor);
    setSemiTrans((SPRT*)sprite_cursor, 1);
    *(u32*)&((SPRT*)sprite_cursor)->x0 = (s32)((y << 0x10) + x);
    background_type = state->hud.bytes.panel_type;
    switch (background_type)
    {
    case 0:
    case 1:
        *(u32*)&((SPRT*)sprite_cursor)->w = 0x180058;
        *(s16*)&((SPRT*)sprite_cursor)->u0 = 0x1000;
        break;
    case 2:
        if ((u8)D_800FDCEA >= 0x41U)
        {
            *(u32*)&((SPRT*)sprite_cursor)->w = 0x180058;
            *(s16*)&((SPRT*)sprite_cursor)->u0 = 0x1000;
        }
        else
        {
            *(u32*)&((SPRT*)sprite_cursor)->w = 0x180058;
            *(s16*)&((SPRT*)sprite_cursor)->u0 = 0x2800;
        }
        break;
    default:
        if (boss_panel != 0)
        {
            *(u32*)&((SPRT*)sprite_cursor)->w = 0x100100;
            *(s16*)&((SPRT*)sprite_cursor)->u0 = 0;
        }
        else
        {
            *(u32*)&((SPRT*)sprite_cursor)->w = 0x100040;
            *(s16*)&((SPRT*)sprite_cursor)->u0 = 0x4000;
        }
        break;
    }
    ((SPRT*)sprite_cursor)->clut = FIELD_HUD_CLUT;
    addPrim(&((FieldHudDrawContext*)ctx)->hud_tag, sprite_cursor);
    sprite_cursor += sizeof(SPRT);
    setDrawTPage((DR_TPAGE*)sprite_cursor, 0, 0, 0x1F);
    addPrim(&((FieldHudDrawContext*)ctx)->hud_tag, sprite_cursor);
    ((FieldHudDrawContext*)render_context)->primitive_cursor = sprite_cursor + sizeof(DR_TPAGE);
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
u8* field_emit_hud_status_line(u8* packet, FieldPrimitiveContext* context, u32* colors, s32 intensity, s32 x, s32 y)
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
    setlen((LINE_G2*)packet, 4);
    *(u32*)&((LINE_G2*)packet)->r0 = first_color;
    setcode((LINE_G2*)packet, 0x50);

    first_component = ((CVECTOR*)colors)[0].r;
    second_component = ((CVECTOR*)colors)[1].r;
    if (first_component == second_component)
    {
        ((LINE_G2*)packet)->r1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2*)packet)->r1 = intensity;
    }
    else
    {
        ((LINE_G2*)packet)->r1 = ~intensity;
    }

    first_component = ((CVECTOR*)colors)[0].g;
    second_component = ((CVECTOR*)colors)[1].g;
    if (first_component == second_component)
    {
        ((LINE_G2*)packet)->g1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2*)packet)->g1 = intensity;
    }
    else
    {
        ((LINE_G2*)packet)->g1 = ~intensity;
    }

    first_component = ((CVECTOR*)colors)[0].b;
    second_component = ((CVECTOR*)colors)[1].b;
    if (first_component == second_component)
    {
        ((LINE_G2*)packet)->b1 = first_component;
    }
    else if (second_component != 0)
    {
        ((LINE_G2*)packet)->b1 = intensity;
    }
    else
    {
        ((LINE_G2*)packet)->b1 = ~intensity;
    }

    line_x = x + 0x18;
    ((LINE_G2*)packet)->x0 = line_x;
    line_y = y + 0x10;
    ((LINE_G2*)packet)->y1 = line_y;
    ((LINE_G2*)packet)->y0 = line_y;
    ((LINE_G2*)packet)->x1 = line_x + ((intensity * 0x23) / 255);

    addPrim(&context->tag, (LINE_G2*)packet);
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
void* field_emit_hud_percentage(void* packet, void* context, s32 value, u16* position)
{
    void* field_emit_hud_glyph(void*, void*, s32, s32*);

    s32 emitted;

    emitted = 0;
    if (value / 100 != 0)
    {
        emitted = 1;
        packet = field_emit_hud_glyph(packet, context, value / 100, (s32*)position);
        value -= 100;
    }
    *position += FIELD_HUD_GLYPH_ADVANCE;
    if (emitted || value / 10 != 0)
    {
        s32 digit;
        digit = value / 10;
        packet = field_emit_hud_glyph(packet, context, digit, (s32*)position);
        value -= digit * 10;
    }
    *position += FIELD_HUD_GLYPH_ADVANCE;
    packet = field_emit_hud_glyph(packet, context, value, (s32*)position);
    *position += FIELD_HUD_GLYPH_ADVANCE;
    return field_emit_hud_glyph(packet, context, FIELD_HUD_GLYPH_PERCENT, (s32*)position);
}

/**
 * @brief Emit a digit or percent glyph from the HUD texture strip.
 * @param packet Primitive-buffer cursor.
 * @param context Draw context containing the HUD ordering-table tag.
 * @param value Glyph index: 0 through 9 for digits, 10 for percent.
 * @param position Packed screen X/Y coordinates.
 * @return Cursor immediately after the sprite.
 */
void* field_emit_hud_glyph(void* packet, void* context, s32 value, s32* position)
{
    SPRT* sprite;
    FieldPrimitiveContext* ot;
    s32 packed_xy;

    sprite = (SPRT*)packet;
    ot = (FieldPrimitiveContext*)context;

    *(u32*)&sprite->r0 = 0x808080;
    setSprt(sprite);
    packed_xy = *position;
    *(s16*)&sprite->u0 = (s16)((value * 8) + 0x1558);
    *(u32*)&sprite->w = 0xB0008;
    sprite->clut = FIELD_HUD_CLUT;
    *(u32*)&sprite->x0 = packed_xy;
    addPrim(&ot->tag, sprite);
    return (u8*)packet + sizeof(SPRT);
}

/**
 * @brief Build a red damage quad from the caller's x0/x1, bordered 3 pixels inward at the bottom, and link it into the OT.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Top edge, added to g_field_hud_bar_offset_y.
 * @param ot Ordering-table tag to link into.
 * @return Pointer just past the quad.
 */
POLY_F4* field_emit_hud_damage_quad(POLY_F4* prim, s32 y, u32* ot)
{
    extern s32 g_field_hud_bar_height;
    extern s32 g_field_hud_bar_offset_y;

    *((u32*)&prim->r0) = 0xFF;
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
POLY_F4* field_emit_hud_healing_quad(POLY_F4* prim, s32 y, u32* ot)
{
    extern s32 g_field_hud_bar_height;
    extern s32 g_field_hud_bar_offset_y;

    if (prim->x1 == prim->x0)
    {
        prim->x1 = prim->x0 + 1;
    }
    *((u32*)&prim->r0) = 0xFFFFFF;
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
void* field_emit_actor_portrait(SPRT* sprt, u32* ot, s32 index, u32* xy)
{
    extern u8 D_800FDCEA;
    extern u16 D_800FE01E;
    extern FieldHudActorTexture g_field_actors[];

    DR_TPAGE* mode;

    *(u32*)&sprt->r0 = 0x808080;
    setSprt(sprt);
    *(u32*)&sprt->x0 = *xy;
    *(u16*)&sprt->u0 = 0xE800;
    *(u32*)&sprt->w = 0x180018;

    if (index == 2 && D_800FDCEA >= 0x41)
    {
        sprt->clut = (((D_800FE01E & 3) + 0x1EF) << 6) | 0x10;
    }
    else
    {
        sprt->clut = ((index + 0x1F4) << 6) | ((g_field_actors[index].texture_flags >> 19) & 0xF);
    }

    addPrim(ot, sprt);

    mode = (DR_TPAGE*)(sprt + 1);
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
void field_load_vram_resource(s32 id, s16* rect, s32 mode)
{
    extern u8* D_8010D038;
    s32 field_upload_image_resource(RECT * rect, Tim * resource, s32 mode);

    u8* buf = D_8010D038;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    field_upload_image_resource((RECT*)rect, (Tim*)buf, mode);
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
 */
s32 field_upload_image_resource(RECT* rect, Tim* resource, s32 mode)
{
    RECT load_rect;
    s32 offset;
    s32 palette_word;
    TimBlock* image;
    TimDimensions* dimensions;
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
    LoadImage(&load_rect, (u_long*)resource->clut_data);

    image = TIM_PIXEL_BLOCK(resource, offset);
    x = rect->x;
    palette_word = *(s32*)&resource->clut_data[240];
    load_rect.x = x;
    load_rect.y = rect->y;
    dimensions = &image->dimensions;
    load_rect.w = dimensions->width;
    load_rect.h = dimensions->height;
    LoadImage(&load_rect, (u_long*)(image + 1));
    rect->x = dimensions->width;
    rect->y = dimensions->height;
    return palette_word;
}
#include "common.h"
#include "field_effect_render_state.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

/**
 * @brief Former field_actor_control_effects.c: consolidated FIELD actor control, animation-flag, scaling and
 *        ground-shadow/effect-primitive translation unit (vram
 *        0x80086494 .. 0x80087614).
 *
 * Merges the former per-function files func_80086494.c, field337.c,
 * field_set_actor_horizontal_scale.c, field_handle_actor_control_flag_40.c,
 * field_actor_flag_ops.c, func_80086FB8.c and field29.c into a single TU.
 *
 * @note g_field_object_states, g_field_object_parts, D_80107800 and D_801058E0 are each viewed as a
 *       different record type (or element width) by different members, so their
 *       extern declarations are kept at BLOCK scope inside each user with that
 *       user's original type. bcopy has two different prototypes across members
 *       and is likewise declared per-function. Callees left implicitly declared
 *       in their original files (func_80083EEC, field_start_actor_animation,
 *       field_restart_actor_animation, func_80086C00 in field337 / func_80086494) are kept
 *       implicit here to preserve the original codegen.
 */

/* ---- Ground-shadow renderer types --------------------------------------- */

/** @brief Actor position and shadow parameters used by the ground-shadow renderer. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u8 pad_0x0c[0x37 - 0xC];
    u8 shadow_bias;
    u8 pad_0x38[0x3A - 0x38];
    u8 resource_index;
} ShadowActor;

/** @brief First two footprint corners; only their horizontal coordinates are used. */
typedef struct
{
    s16 left_x;
    s16 first_y;
    s16 right_x;
} ShadowFootprint;

/** @brief Projected ground position in the field renderer scratchpad. */
typedef struct
{
    u16 x;
    u16 y;
} ShadowScreenPosition;

/** @brief World position copied to the field renderer scratchpad. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} ShadowWorldPosition;

/** @brief Ground-shadow height in the 0x23C-byte actor slot. */
typedef struct
{
    u8 pad_0x000[0x176];
    s16 shadow_height;
    u8 pad_0x178[0x23C - 0x178];
} ShadowActorSlot;

/** @brief Resource entry view used to select the shadow inset scale. */
typedef struct
{
    u8* start;
    u8* end;
    u8 shadow_scale_mode;
    u8 slot_index;
    u8 pad_0x0a[4];
    s16 unknown_0x0e;
    u32 flags;
} ShadowResourceEntry;

#define SHADOW_SCREEN_POSITION ((ShadowScreenPosition*)0x1F8000C0)
#define SHADOW_WORLD_POSITION ((ShadowWorldPosition*)0x1F8000C4)
#define SHADOW_SCREEN_CENTER_X 160
#define SHADOW_SCREEN_CENTER_Y 112
#define SHADOW_OT_SIZE 4096
#define SHADOW_DEPTH_SHIFT 7

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
} Rec87564;

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} State87564;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0x2A - 0x12];
    s16 unk2A;
} Rec875C4;

/* ---- field337.c types ----------------------------------------------------- */

typedef struct
{
    u8 pad0[0x21];
    u8 unk21; /* 0x21 */
    u8 pad22[0x24 - 0x22];
    u8 unk24; /* 0x24 */
    u8 pad25[0x27 - 0x25];
    u8 unk27; /* 0x27 */
    u8 pad28[0x2E - 0x28];
    s16 unk2E; /* 0x2E */
    u8 pad30[0x3A - 0x30];
    u8 unk3A; /* 0x3A */
} Actor;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174; /* 0x174 */
    u8 pad178[0x23C - 0x178];
} ActorRec;

/* ---- field_actor_flag_ops.c types ----------------------------------------- */

/**
 * @brief Field actor state record. Only the fields read here are known; the
 *        object index at 0x3A selects the actor's g_field_object_states slot.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 pad1B[1];
    s32 unk1C;
    u8 pad20[1];
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[1];
    u8 unk27;
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    s16 unk2C;
    u16 unk2E;
    s16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 pad39[1];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[4];
} FieldActorState;

/**
 * @brief Per-actor animation/geometry slot in g_field_object_states; stride 0x23C.
 */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC; /* 0xC flags; bit 0x2000 cleared by the linked-actor helpers */
    u8 pad10[0x170 - 0x10];
    u8 unk170; /* 0x170 index of the linked actor */
    u8 pad171[0x174 - 0x171];
    u32 unk174; /* 0x174 */
    u32 unk178; /* 0x178 bit 1 marks a link to unk170 */
    u8 pad17C[0x23C - 0x17C];
} ActorSlotData;

/** @brief Entry of g_field_actor_slots; stride 0x244. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24; /* 0x24 */
    u8 pad25[0x23A - 0x25];
    u8 unk23A; /* 0x23A */
    u8 pad23B[0x244 - 0x23B];
} ActorSlot;

/** @brief 0x28-byte record of the D_80107800 table; unk4 is the in-use flag. */
typedef struct
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5[0x23];
} FieldUnkRecord_80086F20;

/* ---- field_handle_actor_control_flag_40.c types --------------------------- */

#define FIELD_ANIMATION_SEQUENCE_MASK 0x7F
#define FIELD_ANIMATION_MIRROR_FLAG 0x80
#define FIELD_ANIMATION_HOLD_LAST_FRAME 0x800

/** @brief Field object state used by the actor-control animation handlers. */
typedef struct
{
    u8 pad00[0x1C];
    s32 state_flags;
    u8 pad20[1];
    u8 animation_sequence;
    u8 pad22[0x24 - 0x22];
    u8 frame_duration_scale;
    u8 pad25[0x27 - 0x25];
    u8 frame_index;
    u8 pad28[0x2E - 0x28];
    u16 animation_repeat_count;
    u8 pad30[0x3A - 0x30];
    u8 object_index;
    u8 pad3B[0x54 - 0x3B];
} FieldObjectRecord;

/** @brief Per-actor runtime slot in g_field_object_states. */
typedef struct
{
    u8 pad000[0x174];
    u32 state;
    u8 pad178[0x23C - 0x178];
} FieldActorSlot;

/* ---- field_set_actor_horizontal_scale.c types ----------------------------- */

typedef struct
{
    u8 pad0[0x3A];
    u8 object_index;
    u8 pad3B[0x54 - 0x3B];
} FieldObjectState;

typedef struct
{
    u8 pad0[0x2E];
    u8 scale_z;
    u8 pad2F[0x33 - 0x2F];
    u8 scale_x;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

/* ---- func_80086494.c types ------------------------------------------------ */

/** @brief Color and animation selector bytes in a 0x54-byte object record. */
typedef struct
{
    u8 pad0[0x18];
    u8 unk18, unk19, unk1a;
    u8 pad1b[10];
    u8 unk25;
    u8 pad26[0x14];
    u8 unk3a;
    u8 tail[0x19];
} FieldControlRecord;
/** @brief Current and previous control flags in a 0x23C-byte runtime state. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkc;
    u8 pad10[0x168];
    union
    {
        s32 word;
        struct
        {
            u8 low[2];
            u8 owner;
            u8 high;
        } bytes;
    } status;
    s32 unk17c;
    u8 tail[0xBC];
} FieldControlState;
/** @brief Animation activity and kind in a 0x244-byte animation actor. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x201];
    u16 unk226;
    u8 pad228[0x12];
    u8 unk23a;
    u8 tail[9];
} FieldControlActor;
/** @brief Color bytes in a 0x48-byte visual record. */
typedef struct
{
    u8 pad0[0xE];
    u8 unke, unkf, unk10;
    u8 tail[0x37];
} FieldControlVisual;

/* ---- func_80086FB8.c types ------------------------------------------------ */

/** @brief Screen translation applied to active effect primitives. */
typedef struct
{
    u16 x;
    u16 y;
} FieldEffectMotion;

/* ---- non-conflicting file-scope extern data ------------------------------- */

extern u32 D_800EB00C[];
extern FieldControlActor D_800FB3C8[];
extern FieldControlRecord g_field_actors[];
extern ActorSlot g_field_actor_slots[];
extern FieldEffectMotion D_801077FC;

extern ShadowResourceEntry g_field_resource_entries[];
extern Rec87564* D_8010A01C;
extern s32 g_field_scene_record_table;

/**
 * @brief Dispatch one changed actor control flag and refresh its displayed colors.
 * @param index Object, runtime-state and animation-actor slot index.
 * @note Action values below 0x100 name animations; larger values are callbacks.
 */
void func_80086494(s32 index)
{
    extern FieldControlState g_field_object_states[];
    extern FieldControlVisual g_field_object_parts[];
    s32 flags_before;
    s32 changed_or_current;
    s32 previous_flags;
    s32 record_offset;
    s32 animation_slot;
    u32 changed_flags;
    s32 flags_current;
    s32 old_flags;
    s32 scan_flags;
    s32 bit_index;
    s32 clear_mask;
    s32 animation_bit;
    s32 highest_bit;
    s32 bit_mask;
    u16 animation_kind;
    u32* action;
    u32* action_base;
    u32 action_value;
    FieldControlActor* actor;
    FieldControlRecord* record;
    FieldControlState* runtime;

    record_offset = index * 0x54;
    record = (FieldControlRecord*)(record_offset + (u8*)g_field_actors);
    runtime = &g_field_object_states[index];
    flags_before = runtime->unkc;
    actor = &D_800FB3C8[index];
    if (flags_before & 0x23E4)
    {
        runtime->unkc &= ~0x4000;
        runtime->unkc &= 0xFFFF7FFF;
    }
    flags_current = runtime->unkc;
    previous_flags = runtime->unk17c;
    changed_or_current = (flags_current ^ previous_flags) | flags_current;
    if (changed_or_current != 0)
    {
        if (actor->unk24 == 0)
        {
            do /* a loop form lets loop.c hoist the callback argument; the original searches with a label */
            {
                bit_mask = 0x8000;
                bit_index = 0xF;
                animation_slot = index + 0x40;
                action_base = D_800EB00C;
                action = action_base + bit_index;
            find_action:
                if ((changed_or_current & bit_mask) && (action_value = *action, (action_value != 0xFF)))
                {
                    if (action_value < 0x100U)
                    {
                        if (runtime->unkc & bit_mask)
                        {
                            func_80083EEC(index, animation_slot, action_value);
                            field_start_actor_animation(animation_slot, 0, 0);
                        }
                    }
                    else
                    {
                        ((void (*)(FieldControlRecord*, s32))action_value)((FieldControlRecord*)(record_offset + (u8*)g_field_actors),
                                                                           runtime->unkc & bit_mask);
                    }
                    clear_mask = ~bit_mask;
                    runtime->unk17c = (s32)((runtime->unk17c & clear_mask) | (runtime->unkc & bit_mask));
                }
                else
                {
                    action--;
                    bit_index--;
                    bit_mask >>= 1;
                    if (bit_index >= 0)
                    {
                        goto find_action;
                    }
                }
            } while (0);
        }
        else
        {
            if (!(flags_current & 0x8000) && (previous_flags & 0x8000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                field_clear_actor_effects(actor);
                runtime->unk17c &= 0xFFFF7FFF;
            }
            else if (!(runtime->unkc & 0x4000) && (runtime->unk17c & 0x4000))
            {
                actor->unk24 = 0U;
                actor->unk23a = 0;
                field_clear_actor_effects(actor);
                runtime->unk17c &= -0x4001;
            }
            else
            {
                changed_flags = runtime->unkc;
                old_flags = runtime->unk17c;
                changed_flags = (changed_flags ^ old_flags) & old_flags;
                if (changed_flags != 0)
                {
                    scan_flags = (s32)changed_flags;
                    highest_bit = 0xF;
                    animation_bit = 1;
                find_highest:
                    if ((scan_flags & (animation_bit << highest_bit)) == 0)
                    {
                        highest_bit -= 1;
                        goto find_highest;
                    }
                    animation_kind = actor->unk226;
                    animation_bit = 0x20;
                    switch (animation_kind)
                    {
                    case 5:
                        animation_bit = 0;
                        break;
                    case 6:
                        animation_bit = 1;
                        break;
                    case 7:
                        animation_bit = 2;
                        break;
                    case 8:
                        animation_bit = 3;
                        break;
                    case 11:
                        animation_bit = 4;
                        break;
                    case 10:
                        animation_bit = 5;
                        break;
                    case 9:
                        animation_bit = 6;
                        break;
                    case 12:
                        animation_bit = 7;
                        break;
                    }
                    if (highest_bit == animation_bit)
                    {
                        actor->unk24 = 0U;
                        actor->unk23a = 0;
                        field_clear_actor_effects(actor);
                        if ((runtime->status.word & 1) && (runtime->status.bytes.owner == (record->unk3a + 0x40)))
                        {
                            record->unk25 = 0;
                            runtime->status.word = (s32)(runtime->status.word & ~1);
                        }
                    }
                }
            }
        }
    }
    if (runtime->unkc & 0x10000000)
    {
        g_field_actors[index].unk1a = 0x20;
        g_field_actors[index].unk19 = 0x20;
        runtime->unkc = (s32)(runtime->unkc & 0xEFFFFFFF);
    }
    else
    {
        g_field_actors[index].unk18 = g_field_object_parts[index].unke;
        g_field_actors[index].unk19 = g_field_object_parts[index].unkf;
        g_field_actors[index].unk1a = g_field_object_parts[index].unk10;
    }
}

/**
 * @brief Enter an actor control state and play animation 7 on its +0x40 object.
 * @param arg0 Actor object record.
 * @param arg1 Nonzero to run the transition; zero does nothing.
 */
void func_80086850(Actor* arg0, s32 arg1)
{
    extern ActorRec g_field_object_states[];

    if (arg1 != 0)
    {
        arg0->unk2E = 1;
        arg0->unk27 = 0;
        arg0->unk21 &= 0x80;
        arg0->unk24 = 1;
        g_field_object_states[arg0->unk3A].unk174 &= ~0x1800;
        field_restart_actor_animation(arg0);
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x7);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        func_80086C00(arg0->unk3A);
    }
}

/**
 * @brief Enter an actor control state and play animation 0xA on its +0x40 object.
 * @param arg0 Actor object record.
 * @param arg1 Nonzero to run the transition; zero does nothing.
 */
void func_800868FC(Actor* arg0, s32 arg1)
{
    extern ActorRec g_field_object_states[];

    if (arg1 != 0)
    {
        arg0->unk2E = 1;
        arg0->unk27 = 0;
        arg0->unk21 &= 0x80;
        arg0->unk24 = 1;
        g_field_object_states[arg0->unk3A].unk174 &= ~0x1800;
        field_restart_actor_animation(arg0);
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xA);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
}

/**
 * @brief Set a field actor's horizontal model scale to half-size or full-size.
 * @param object Field object whose actor-part definition is updated.
 * @param half_scale Non-zero for half-size X/Z scale, zero for full-size scale.
 */
void field_set_actor_horizontal_scale(FieldObjectState* object, s32 half_scale)
{
    extern FieldActorPartDef g_field_object_parts[];

    if (half_scale != 0)
    {
        g_field_object_parts[object->object_index].scale_z = 0x20;
        g_field_object_parts[object->object_index].scale_x = 0x20;
    }
    else
    {
        g_field_object_parts[object->object_index].scale_z = 0x40;
        g_field_object_parts[object->object_index].scale_x = 0x40;
    }
}

/**
 * @brief Handle actor control flag 0x40 becoming active for a field object.
 * @param object Field object whose actor slot and animation state are updated.
 * @param is_set Non-zero when actor control flag 0x40 is currently set.
 */
void func_800869FC(FieldObjectRecord* object, s32 is_set)
{
    s32 func_80083EEC(u8 object_index, s32 actor_index, s32 animation_id);
    void field_start_actor_animation(s32 actor_index, s32 arg1, s32 arg2);
    void field_restart_actor_animation(FieldObjectRecord * object);
    void func_80086C00(u8 object_index);
    extern FieldActorSlot g_field_object_states[];
    u8 animation_sequence;

    if (is_set != 0)
    {
        func_80083EEC(object->object_index, object->object_index + 0x40, 9);
        field_start_actor_animation(object->object_index + 0x40, 0, 0);
        animation_sequence = object->animation_sequence;
        if ((animation_sequence & FIELD_ANIMATION_SEQUENCE_MASK) != 0x1B)
        {
            object->animation_sequence = (animation_sequence & FIELD_ANIMATION_MIRROR_FLAG) + 0x1B;
            object->animation_repeat_count = 1;
            object->frame_index = 0;
            object->frame_duration_scale = 1;
            g_field_object_states[object->object_index].state &= ~0x1800;
            field_restart_actor_animation(object);
            object->state_flags |= FIELD_ANIMATION_HOLD_LAST_FRAME;
        }
        func_80086C00(object->object_index);
    }
}

/**
 * @brief Enter or leave the actor's 0x14 control state.
 *
 * With @p flag set, puts the actor into control state 0x14, keeps its slot
 * flags 0x1800 clear, sets bits 0x40800 of unk1C, and runs func_80086C00 on
 * it. With @p flag clear, resets the control state, plays animation 0x91 on
 * the actor's +0x40 object, and clears bit 0x40000 of unk1C.
 *
 * @param rec Actor state record.
 * @param flag Nonzero to enter the state, zero to leave it.
 */
void func_80086ACC(FieldActorState* rec, s32 flag)
{
    void field_restart_actor_animation(FieldActorState*);
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);
    void func_80086C00(s32 idx);
    extern ActorSlotData g_field_object_states[];

    if (flag != 0)
    {
        rec->unk2E = 1;
        rec->unk24 = 1;
        rec->unk27 = 0;
        rec->unk21 = (rec->unk21 & 0x80) + 0x14;
        g_field_object_states[rec->unk3A].unk174 &= ~0x1800;
        field_restart_actor_animation(rec);
        rec->unk1C |= 0x40800;
        func_80086C00(rec->unk3A);
    }
    else
    {
        rec->unk27 = 0;
        rec->unk2E = 1;
        rec->unk24 = 1;
        rec->unk21 &= 0x80;
        g_field_object_states[rec->unk3A].unk174 &= ~0x1800;
        field_restart_actor_animation(rec);
        func_80083EEC(rec->unk3A, rec->unk3A + 0x40, 0x91);
        field_start_actor_animation(rec->unk3A + 0x40, 0, 0);
        rec->unk1C &= 0xFFFBFFFF;
    }
}

/**
 * @brief Clears a linked actor's 0x2000 flag when this actor's bit 1 is set.
 *
 * For actor @p idx, if bit 1 of its 0x178 word is set, clears bit 13 (0x2000)
 * of the 0xC flags word belonging to the actor referenced by its 0x170 byte.
 *
 * @param idx Actor slot index.
 */
void func_80086C00(s32 idx)
{
    extern ActorSlotData g_field_object_states[];
    ActorSlotData* base = g_field_object_states;
    ActorSlotData* e = &base[idx];

    if ((e->unk178 >> 1) & 1)
    {
        ActorSlotData* e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}

/**
 * @brief Play animation 0xC or 0xD on the actor's +0x40 object and set unk25.
 *
 * With @p arg1 set, plays 0xC, marks unk25 = 0xFE, and clears the linked
 * actor's 0x2000 flag as func_80086C00 does. With @p arg1 clear, plays 0xD
 * and zeroes unk25.
 *
 * @param arg0 Actor state record.
 * @param arg1 Selects the 0xC (nonzero) or 0xD (zero) path.
 */
void func_80086C70(FieldActorState* arg0, s32 arg1)
{
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);
    extern ActorSlotData g_field_object_states[];

    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xC);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0xFE;
        {
            ActorSlotData* base = g_field_object_states;
            ActorSlotData* e = &base[arg0->unk3A];
            if ((e->unk178 >> 1) & 1)
            {
                ActorSlotData* e2 = &base[e->unk170];
                e2->unkC &= ~0x2000;
            }
        }
    }
    else
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xD);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0;
    }
}

/**
 * @brief Clears a linked actor's 0x2000 flag from a record's actor index.
 *
 * Uses the 0x3A index byte of @p p to select an actor; if bit 1 of its 0x178
 * word is set, clears bit 13 (0x2000) of the 0xC flags word belonging to the
 * actor referenced by its 0x170 byte.
 *
 * @param p Actor state record viewed as bytes.
 */
void func_80086D5C(u8* p)
{
    extern ActorSlotData g_field_object_states[];
    ActorSlotData* base = g_field_object_states;
    ActorSlotData* e = &base[p[0x3A]];

    if ((e->unk178 >> 1) & 1)
    {
        ActorSlotData* e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}

/**
 * @brief Play animation 0xE on the actor's +0x40 object, or clear its slot bytes 0x24 and 0x23A.
 * @param arg0 Actor state record.
 * @param arg1 Nonzero plays the animation, zero clears the slot bytes.
 */
void func_80086DD0(FieldActorState* arg0, s32 arg1)
{
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);

    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xE);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @brief Play animation 0x19 on the actor's +0x40 object, or clear its slot bytes 0x24 and 0x23A.
 * @param arg0 Actor state record.
 * @param arg1 Nonzero plays the animation, zero clears the slot bytes.
 */
void func_80086E78(FieldActorState* arg0, s32 arg1)
{
    s32 func_80083EEC(u8, s32, s32);
    void field_start_actor_animation(s32, s32, s32);

    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x19);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @brief Clear the unk4 flag byte across all 256 records of the D_80107800
 *        table.
 */
void func_80086F20(void)
{
    extern FieldUnkRecord_80086F20 D_80107800[];
    s32 i;

    for (i = 0xFF; i >= 0; i--)
    {
        D_80107800[i].unk4 = 0;
    }
}

/**
 * @brief Stores a record into the first free slot of the D_80107800 table.
 *
 * Scans up to 256 records for one whose unk4 flag byte is clear, copies 0x28
 * bytes from @p src into it, and writes @p value to the parallel D_801058E0
 * half-word slot.
 *
 * @param src Source record, 0x28 bytes.
 * @param value Halfword stored in the parallel D_801058E0 slot.
 */
void func_80086F48(const void* src, s16 value)
{
    void* bcopy(const void*, void*, int);
    extern FieldUnkRecord_80086F20 D_80107800[];
    extern s16 D_801058E0[];
    s32 i = 0;
    s16* slot = D_801058E0;
    FieldUnkRecord_80086F20* entry = D_80107800;

    for (; i < 0x100; i++)
    {
        if (entry->unk4 == 0)
        {
            bcopy(src, entry, 0x28);
            *slot = value;
            return;
        }
        slot++;
        entry++;
    }
}

/**
 * @brief Fade, translate, and enqueue active primitives from the 256-entry effect pool.
 * @param buffer Ordering table and packet-buffer state, with the write cursor at offset 0x40B8.
 */
void func_80086FB8(u8* buffer)
{
    extern POLY_FT4 D_80107800[];
    extern u16 D_801058E0[];
    extern void bcopy(void*, void*, s32);
    POLY_FT4* source;
    POLY_FT4* output;
    u32* ordering;
    s32 i;
    u8 color;

    output = *(POLY_FT4**)(buffer + 0x40B8);
    ordering = (u32*)buffer;
    source = D_80107800;

    for (i = 0; i < 256;)
    {
        color = source->r0;
        if (color != 0)
        {
            if (color < 0x10)
            {
                color = 0;
            }
            else
            {
                color -= 0x10;
            }

            source->b0 = color;
            source->g0 = color;
            source->r0 = color;
            setSemiTrans(source, 1);
            source->x0 += D_801077FC.x;
            source->x1 += D_801077FC.x;
            source->x2 += D_801077FC.x;
            source->x3 += D_801077FC.x;
            source->y0 += D_801077FC.y;
            source->y1 += D_801077FC.y;
            source->y2 += D_801077FC.y;
            source->y3 += D_801077FC.y;

            bcopy(source, output, sizeof(POLY_FT4));
            addPrim(&ordering[D_801058E0[i]], output);
            output++;
        }

        i++;
        source++;
    }

    *(POLY_FT4**)(buffer + 0x40B8) = output;
}

/**
 * @brief Project an actor's ground shadow and append its textured quad.
 * @param actor World position, shadow bias and resource slot.
 * @param primitives Next free POLY_FT4 in the primitive buffer.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param footprint First two footprint corners supplying the horizontal bounds.
 * @return Next free primitive, unchanged if the shadow has collapsed.
 */
POLY_FT4* field_render_actor_ground_shadow(ShadowActor* actor, POLY_FT4* primitives, s32* ordering_table, ShadowFootprint* footprint)
{
    extern ShadowActorSlot g_field_object_states[];
    extern ShadowResourceEntry g_field_resource_entries[];
    s16 edge_y;
    s32 shadow_height;
    s16 right_x;
    s32 ground_y;
    s32 ground_height_fixed;
    s32 world_z;
    s32 bias_bits;
    s32 world_x;
    s32 depth;
    s32 projection_value;
    s32 camera_offset;
    s32 horizontal_offset;
    s32 left_inset;
    s32 right_inset;
    s32 diameter;
    s32 camera_z;
    s32 screen_x;
    u8 resource_index;
    s32 camera_x;
    s32 actor_screen_x;
    s32 camera_y;
    s32 actor_depth_y;
    s32 camera_screen_y;
    s32 edge_work;
    s32 scale_work;
    s32 shadow_bias;
    ShadowScreenPosition* screen;
    ShadowWorldPosition* scratch;

    /* Project the ground point; negative fixed-point values round toward zero. */
    screen = SHADOW_SCREEN_POSITION;
    scratch = SHADOW_WORLD_POSITION;
    camera_offset = g_field_view_offset_x;
    world_x = actor->x;
    scratch->y = 0;
    scratch->x = world_x;
    world_z = actor->z;
    ground_height_fixed = world_z;
    scratch->z = ground_height_fixed;
    if (camera_offset < 0)
    {
        camera_offset += 0xFF;
    }
    projection_value = world_x;
    camera_x = camera_offset >> 8;
    if (projection_value < 0)
    {
        projection_value += 0xFF;
    }
    camera_offset = g_field_view_offset_y;
    actor_screen_x = (projection_value >> 8) + SHADOW_SCREEN_CENTER_X;
    screen_x = camera_x + actor_screen_x;
    screen->x = screen_x;
    if (camera_offset < 0)
    {
        camera_offset += 0xFF;
    }
    projection_value = world_z;
    camera_y = camera_offset >> 8;
    camera_screen_y = camera_y + SHADOW_SCREEN_CENTER_Y;
    if (projection_value < 0)
    {
        projection_value += 0x1FF;
    }
    camera_z = g_field_view_offset_z;
    actor_depth_y = projection_value >> 9;
    ground_y = camera_screen_y - actor_depth_y;
    if (camera_z < 0)
    {
        camera_z += 0x1FF;
    }
    screen->y = (u16)(ground_y - (camera_z >> 9));

    /* The resource profile widens the height-dependent inset by 5/4. */
    resource_index = actor->resource_index;
    bias_bits = actor->shadow_bias << 24;
    shadow_height = g_field_object_states[resource_index].shadow_height;
    shadow_bias = bias_bits >> 24;
    if (g_field_resource_entries[resource_index].shadow_scale_mode != 0)
    {
        ground_height_fixed = shadow_height << 8;
        horizontal_offset = bias_bits >> 26;
        left_inset = actor->y;
        scale_work = (u16)footprint->left_x;
        left_inset = (left_inset - ground_height_fixed) >> 11;
        left_inset += horizontal_offset;
        edge_work = left_inset << 2;
        left_inset = edge_work - -left_inset;
        edge_work = screen_x + scale_work;
        if (left_inset < 0)
        {
            left_inset += 3;
        }
        left_inset >>= 2;
        left_inset = edge_work - left_inset;
        primitives->x0 = left_inset;
        primitives->x2 = left_inset;

        right_inset = actor->y;
        edge_work = (u16)footprint->right_x;
        right_inset = (right_inset - ground_height_fixed) >> 11;
        right_inset -= horizontal_offset;
        scale_work = right_inset << 2;
        horizontal_offset = screen->x;
        right_inset = scale_work - -right_inset;
        horizontal_offset += edge_work;
        if (right_inset < 0)
        {
            right_inset += 3;
        }
        right_inset >>= 2;
        right_x = horizontal_offset + right_inset;
    }
    else
    {
        s32 height_delta;
        s32 ground_height_fixed_local;
        s32 horizontal_offset_local;
        s32 left_inset_local;
        s32 right_inset_local;
        s32 scale_work_local;

        ground_height_fixed_local = shadow_height << 8;
        horizontal_offset_local = bias_bits >> 26;
        edge_work = (u16)footprint->left_x;
        left_inset_local = actor->y;
        edge_work = screen_x - -edge_work;
        left_inset_local = (left_inset_local - ground_height_fixed_local) >> 11;
        edge_work -= left_inset_local;
        edge_work += horizontal_offset_local;
        primitives->x0 = edge_work;
        primitives->x2 = edge_work;

        do /* scope blocks reproduce the original scheduling of this branch */
        {
            right_inset_local = screen->x;
        } while (0);
        scale_work_local = (u16)footprint->right_x;
        do
        {
            height_delta = actor->y;
            right_inset_local += scale_work_local;
        } while (0);
        height_delta -= ground_height_fixed_local;
        height_delta >>= 11;
        right_inset_local += height_delta;
        right_x = right_inset_local - horizontal_offset_local;
    }
    do
    {
        primitives->x1 = right_x;
        primitives->x3 = right_x;
    } while (0);

    /* Reject inverted horizontal bounds or a vertical diameter below two pixels. */
    projection_value = shadow_height << 8;
    diameter = (s16)footprint->left_x;
    left_inset = (s16)footprint->right_x;
    camera_offset = primitives->x0;
    diameter -= left_inset;
    diameter >>= 1;
    left_inset = actor->y;
    diameter = abs(diameter);
    left_inset -= projection_value;
    left_inset >>= 11;
    diameter += left_inset;
    projection_value = shadow_bias >> 2;
    diameter -= projection_value;
    if (primitives->x1 >= camera_offset && diameter >= 2)
    {
        if (shadow_height != 0)
        {
            diameter >>= 1;
            edge_y = screen->y - diameter + shadow_height;
            primitives->y1 = edge_y;
            primitives->y0 = edge_y;
            edge_y = screen->y + diameter + shadow_height;
            primitives->y3 = edge_y;
            primitives->y2 = edge_y;
        }
        else
        {
            diameter >>= 1;
            edge_y = screen->y - diameter;
            primitives->y1 = edge_y;
            primitives->y0 = edge_y;
            edge_y = screen->y + diameter;
            primitives->y3 = edge_y;
            primitives->y2 = edge_y;
        }

        /* Neutral modulation with subtractive blending for the shadow texture. */
        SET_BGR0_PACKED(primitives, GPU_TINT_NEUTRAL);
        setPolyFT4(primitives);
        setSemiTrans(primitives, 1);
        primitives->u3 = 0x40;
        primitives->u1 = 0x40;
        primitives->v1 = 0x50;
        primitives->v0 = 0x50;
        primitives->v3 = 0x70;
        primitives->v2 = 0x70;
        setTPage(primitives, 0, 2, 960, 256);
        primitives->u2 = 0;
        primitives->u0 = 0;
        setClut(primitives, 256, 481);

        depth = actor->z >> SHADOW_DEPTH_SHIFT;
        if (depth < 0)
        {
            addPrim(&ordering_table[0], primitives);
            primitives++;
        }
        else if (depth >= SHADOW_OT_SIZE)
        {
            addPrim(&ordering_table[SHADOW_OT_SIZE - 1], primitives);
            primitives++;
        }
        else
        {
            addPrim(&ordering_table[actor->z >> SHADOW_DEPTH_SHIFT], primitives);
            primitives++;
        }
    }
    return primitives;
}

/**
 * @brief Hand the actor's animation record to func_800B2198 and cache the record.
 * @param arg0 Actor record whose 0x3A index selects the g_field_object_states slot.
 */
void func_80087564(Rec87564* arg0)
{
    extern State87564 g_field_object_states[];

    D_8010A01C = arg0;
    func_800B2198(g_field_object_states[arg0->unk3A].unk14, g_field_object_states);
}

/**
 * @brief Return the current scene record table.
 * @return Value of g_field_scene_record_table.
 */
s32 func_800875B4(void)
{
    return g_field_scene_record_table;
}

/**
 * @brief Report whether the current record's 0x10 and 0x2A halfwords are both zero.
 * @return 1 when both are zero, 0 when only 0x2A is nonzero, -1 when the record
 *         lookup fails.
 */
s32 func_800875C4(void)
{
    Rec875C4* rec = func_80087C9C();
    s32 result;

    if (rec == (Rec875C4*)-1)
    {
        return -1;
    }

    result = 0;
    if (rec->unk10 == 0)
    {
        result = rec->unk2A == 0;
    }

    return result;
}
