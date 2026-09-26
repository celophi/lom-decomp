/**
 * @file field_actor_hud_effects.c
 * @brief Field actor HUD (party, enemy and boss panels with HP and special
 *        attack gauges), the HUD image loader, the object flag handlers, the
 *        fading primitive pool and the ground-shadow renderer.
 *
 * One translation unit: the flag handler table g_field_object_flag_handlers
 * sits inside the HUD's initialized data, between
 * g_field_hud_companion_status_colors and g_field_party_hud_order.
 */

#include "common.h"
#include "cdrom.h"
#include "display.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_effect_render_state.h"
#include "field_runtime.h"
#include "gpu_packet.h"
#include "tim.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/rand.h"

/** @brief Screen center, added to camera-relative positions. */
#define FIELD_SCREEN_CENTER_X (SCREEN_WIDTH / 2)
#define FIELD_SCREEN_CENTER_Y (VRAM_DRAW_HEIGHT / 2)

/* ---- HUD ------------------------------------------------------------------ */

/** @brief FieldObjectState.hud.word bit: show the special attack gauge. */
#define FIELD_HUD_SPECIAL_GAUGE 0x1

/** @brief FieldObjectState.unk8 layout: displayed HP, panel timer and boss flag. */
#define FIELD_HUD_HP_MASK 0xFFFFFF
#define FIELD_HUD_DISPLAY_FLAGS_MASK 0xFF000000
#define FIELD_HUD_TIMER_SHIFT 24
#define FIELD_HUD_TIMER_MASK 0x7F
#define FIELD_HUD_BOSS_SHIFT 31

/** @brief Frames an enemy panel stays up after its HP changes. */
#define FIELD_HUD_PANEL_FRAMES 20

/** @brief HP per gauge bar for party members and enemies, and for the boss. */
#define FIELD_HUD_HP_PER_BAR 100
#define FIELD_HUD_BOSS_HP_PER_BAR 400

/** @brief Panel positions. */
#define FIELD_HUD_SINGLE_X 112
#define FIELD_HUD_PAIR_X 56
#define FIELD_HUD_PAIR_STEP 108
#define FIELD_HUD_TRIO_X 8
#define FIELD_HUD_TRIO_STEP 104
#define FIELD_HUD_TRIO_LOW_Y 28
#define FIELD_HUD_TRIO_HIGH_Y 4
#define FIELD_HUD_PARTY_Y 16
#define FIELD_HUD_BOSS_X 32
#define FIELD_HUD_BOSS_Y 192
#define FIELD_HUD_ENEMY_OFFSET_X 28

/** @brief Screen bounds an enemy panel is clamped to. */
#define FIELD_HUD_MARGIN_X 32
#define FIELD_HUD_MIN_Y 16
#define FIELD_HUD_MAX_Y 208

/** @brief Animation of an anchor effect record that the panel does not follow. */
#define FIELD_HUD_ANCHOR_IGNORED_ANIMATION 0x2F

/** @brief Frames of the panel shake. */
#define FIELD_HUD_SHAKE_FRAMES 6

/** @brief Portrait blink: frames per animation, idle chance mask and the table of blinking objects. */
#define FIELD_HUD_BLINK_FRAMES 6
#define FIELD_HUD_BLINK_CHANCE_MASK 0x3F
#define FIELD_HUD_BLINK_LIST_COUNT 8
#define FIELD_HUD_BLINK_LIST_END 0xFF

/** @brief Glyph indices and spacing of the HP percentage. */
#define FIELD_HUD_GLYPH_PERCENT 10
#define FIELD_HUD_GLYPH_ADVANCE 7

/** @brief Companion kinds (player record 2) at or above this value use the companion panel. */
#define FIELD_HUD_COMPANION_KIND_MIN 0x41

/** @brief Ordering-table entries of the HUD and of the HP change quads. */
#define FIELD_HUD_OT_INDEX 3
#define FIELD_HUD_DELTA_OT_INDEX 2

/** @brief Scratchpad word holding the screen position passed to the portrait and percentage emitters. */
#define FIELD_HUD_SCRATCH_POSITION ((Vec2s*)0x1F800000)

/** @brief Texture page coordinates packed as a u0/v0 halfword. */
#define FIELD_HUD_UV(u, v) (((v) << 8) | (u))

#define FIELD_HUD_CLUT getClut(256, 480)
#define FIELD_HUD_BLINK_CLUT getClut(256, 481)

/** @brief Gauge width in pixels for the portrait, boss and enemy panels. */
#define FIELD_HUD_BAR_WIDTH 54
#define FIELD_HUD_BOSS_BAR_WIDTH 227
#define FIELD_HUD_BAR_HEIGHT 3

/** @brief Horizontal slant of the gauge quads. */
#define FIELD_HUD_BAR_SLANT 3

/** @brief Gauge colors: the dim, mid and full intensity masks of a bar color. */
#define FIELD_HUD_COLOR_DIM 0x3F3F3F
#define FIELD_HUD_COLOR_MID 0x7F7F7F

/** @brief HP bar change steps: the displayed HP moves a third of the way per frame. */
#define FIELD_HUD_RISE_MIN_STEP 3
#define FIELD_HUD_FALL_MIN_STEP 4

/** @brief Length of the special attack gauge line at full intensity. */
#define FIELD_HUD_LINE_LENGTH 35

/** @brief POLY_G4 length and code, written separately from the colors. */
#define FIELD_POLY_G4_LENGTH 8
#define FIELD_POLY_G4_CODE 0x38

/** @brief Gray used for the recovery gauge's dark edge. */
#define FIELD_HUD_RECOVERY_DARK GPU_COLOR_WORD(0x20, 0x20, 0x20)
#define FIELD_HUD_WHITE GPU_COLOR_WORD(0xFF, 0xFF, 0xFF)
#define FIELD_HUD_RED GPU_COLOR_WORD(0xFF, 0, 0)

/**
 * @brief HP and gauge words at the start of FieldObjectState, as the panel reads them.
 * @note The panel re-reads the HP words after every primitive store; without volatile the
 *       compiler keeps them in registers and the code no longer matches.
 */
typedef struct
{
    u32 maximum_hp;
    volatile u32 current_hp;
    /** @brief Displayed HP in the low 24 bits, the panel timer above it and the boss flag in bit 31. */
    volatile u32 displayed_hp;
    u8 unkC[0x48 - 0xC];
    u16 technique_gauge;
    s16 action_charge;
    union
    {
        u32 word;
        struct
        {
            u8 flags;
            u8 object_index;
            u8 unk4E;
            u8 unk4F;
        } bytes;
    } hud;
} FieldHudGauge;

extern u32 g_field_hud_hp_colors[];
extern CVECTOR g_field_hud_status_colors[2];
extern CVECTOR g_field_hud_effect_colors[2];
extern CVECTOR g_field_hud_full_status_colors[2];
extern CVECTOR g_field_hud_companion_status_colors[2];
extern s32 g_field_party_hud_order[FIELD_PARTY_COUNT];
extern s16 g_field_hud_shake_offsets[];
extern s32 g_field_hud_blink_frames[FIELD_PARTY_COUNT];
extern s32 g_field_boss_hud_shake_frame;
extern s32 g_field_hud_bar_height;
extern s32 g_field_hud_bar_width;
extern s32 g_field_hud_bar_offset_x;
extern s32 g_field_hud_bar_offset_y;
extern s32 g_field_active_group;
extern s32 g_field_scene_mode_bit;
extern s32 g_frame_counter;
extern s32 D_80122B20;
extern u8 g_field_pair_indicator_list[];
extern u8 D_800FDCEA;
extern u16 D_800FE01E;
extern u8* g_field_cd_buffer;

/* ---- Object flag handlers and fading primitives ---------------------------- */


/** @brief Number of bits in FieldObjectState.flags that have a handler entry. */
#define FIELD_OBJECT_HANDLER_COUNT 16

/** @brief Handler entry of a flag bit without a handler. */
#define FIELD_OBJECT_HANDLER_NONE 0xFF

/** @brief Handler entries below this value are animation resources, larger ones functions. */
#define FIELD_OBJECT_HANDLER_FUNCTION_MIN 0x100

/** @brief FieldObjectState.flags bits with a handler in g_field_object_flag_handlers; meanings mostly unknown. */
#define FIELD_OBJECT_FLAG_0004 0x0004
#define FIELD_OBJECT_FLAG_0020 0x0020
#define FIELD_OBJECT_FLAG_0040 0x0040
#define FIELD_OBJECT_FLAG_0080 0x0080
#define FIELD_OBJECT_FLAG_0100 0x0100
#define FIELD_OBJECT_FLAG_2000 0x2000
#define FIELD_OBJECT_FLAG_4000 0x4000
#define FIELD_OBJECT_FLAG_8000 0x8000

/** @brief Set for one frame when the object is hit; tints the actor red. */
#define FIELD_OBJECT_FLAG_HIT_FLASH 0x10000000

/** @brief Flags that stop the looping 0x4000/0x8000 effects (also skip knockback and camera tracking). */
#define FIELD_OBJECT_IMMOBILE_FLAGS                                                                                                                            \
    (FIELD_OBJECT_FLAG_0004 | FIELD_OBJECT_FLAG_0020 | FIELD_OBJECT_FLAG_0040 | FIELD_OBJECT_FLAG_0080 | FIELD_OBJECT_FLAG_0100 |                              \
     FIELD_OBJECT_FLAG_KNOCKED_OUT | FIELD_OBJECT_FLAG_2000)

/** @brief FieldObjectState.contact bit: the object is linked to linked_object_index. */
#define FIELD_CONTACT_LINKED_SHIFT 1

/** @brief FieldObjectState.movement bits cleared whenever an actor animation restarts. */
#define FIELD_MOVEMENT_ANIMATION_BITS 0x1800

/** @brief Low seven bits of FieldActor.animation select the animation, bit 7 mirrors it. */
#define FIELD_ANIMATION_SEQUENCE_MASK 0x7F
#define FIELD_ANIMATION_MIRROR_FLAG 0x80

/** @brief FieldActor.control bits set by the flag handlers. */
#define FIELD_ANIMATION_HOLD_LAST_FRAME 0x800
#define FIELD_ACTOR_CONTROL_40000 0x40000

/** @brief Animation held while flag 0x80 is set. */
#define FIELD_ANIMATION_FLAG_0080_POSE 0x14

/** @brief Animation held while flag 0x40 is set. */
#define FIELD_ANIMATION_FLAG_0040_POSE 0x1B

/** @brief Tint component kept by the hit flash (full intensity is 0x80). */
#define FIELD_HIT_FLASH_TINT 0x20

/** @brief FieldObjectPart scale values: 0x40 is full size. */
#define FIELD_PART_SCALE_FULL 0x40
#define FIELD_PART_SCALE_HALF 0x20

/** @brief Number of primitives in the fading primitive pool. */
#define FIELD_FADE_PRIM_COUNT 256

/** @brief Brightness lost by a fading primitive per frame. */
#define FIELD_FADE_PRIM_STEP 0x10

/* ---- Ground shadow ---------------------------------------------------------- */

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

#define SHADOW_SCREEN_POSITION ((ShadowScreenPosition*)0x1F8000C0)
#define SHADOW_WORLD_POSITION ((ShadowWorldPosition*)0x1F8000C4)
#define SHADOW_OT_SIZE 4096
#define SHADOW_DEPTH_SHIFT 7

/** @brief How far @p actor stands above ground height @p height; grows the shadow and pulls its edges in. */
#define SHADOW_ELEVATION(actor, height) (((actor)->y - ((height) << 8)) >> 11)

/** @brief Screen translation applied to the fading primitives. */
typedef struct
{
    u16 x;
    u16 y;
} FieldScreenMotion;

extern u32 g_field_object_flag_handlers[FIELD_OBJECT_HANDLER_COUNT];
extern FieldScreenMotion g_field_screen_scroll;
extern POLY_FT4 g_field_fade_prims[FIELD_FADE_PRIM_COUNT];
extern u16 g_field_fade_prim_depths[FIELD_FADE_PRIM_COUNT];
extern FieldActor* g_field_updating_actor;
extern u32* g_field_scene_record_table;

static void field_draw_actor_hud_panel(s32 x, s32 y, s32 slot, FieldRenderHalf* render_half, u32 hp_per_bar);
static u8* field_emit_hud_status_line(u8* packet, FieldRenderHalf* render_half, CVECTOR* colors, s32 intensity, s32 x, s32 y);
static void* field_emit_hud_percentage(void* packet, FieldRenderHalf* render_half, s32 value, Vec2s* position);
static void* field_emit_hud_glyph(void* packet, FieldRenderHalf* render_half, s32 value, Vec2s* position);
static POLY_F4* field_emit_hud_damage_quad(POLY_F4* prim, s32 y, u_long* ot);
static POLY_F4* field_emit_hud_healing_quad(POLY_F4* prim, s32 y, u_long* ot);
void* field_emit_actor_portrait(SPRT* sprt, u_long* ot, s32 index, Vec2s* position);
static s32 field_upload_image_resource(RECT* rect, Tim* resource, s32 mode);
int abs(int value);
void bcopy(const void* src, void* dst, int size);
void field_restart_actor_animation(FieldActor* actor);
void field_clear_actor_effects(FieldActorSlot* slot);
static inline void field_clear_link_target_flag(s32 object_index);
FieldActor* field_lookup_actor(s32 key);
void field_update_actor_record(s32 actor_id, void* unused);

/**
 * @brief Set the corners of a slanted gauge bar.
 * @param bar Gauge quad.
 * @param x Panel screen x.
 * @param y Panel screen y.
 * @param width Filled width in pixels.
 */
static inline void field_hud_set_bar_geometry(POLY_G4* bar, s32 x, s32 y, s32 width)
{
    bar->x0 = (u16)g_field_hud_bar_offset_x + x;
    bar->y1 = ((u16)g_field_hud_bar_offset_y + y);
    bar->x1 = ((u16)g_field_hud_bar_offset_x + x) + width;
    bar->x2 = ((u16)g_field_hud_bar_offset_x + x) - FIELD_HUD_BAR_SLANT;
    bar->y0 = ((u16)g_field_hud_bar_offset_y + y);
    bar->y3 = bar->y1 + (u16)g_field_hud_bar_height;
    bar->y2 = bar->y3;
    bar->x3 = ((u16)g_field_hud_bar_offset_x + x) + width - FIELD_HUD_BAR_SLANT;
}

/**
 * @brief Set the slanted corners of an HP change quad from its x0/x1 and link it.
 * @param prim Quad whose x0, x1 and color are already set.
 * @param y Panel screen y, added to g_field_hud_bar_offset_y.
 * @param ot Ordering-table entry to link into.
 * @return Pointer just past the quad.
 */
static inline POLY_F4* field_hud_link_change_quad(POLY_F4* prim, s32 y, u_long* ot)
{
    prim->x3 = prim->x1 - FIELD_HUD_BAR_SLANT;
    prim->y1 = ((u16)g_field_hud_bar_offset_y) + y;
    prim->x2 = prim->x0 - FIELD_HUD_BAR_SLANT;
    prim->y0 = ((u16)g_field_hud_bar_offset_y) + y;
    prim->y3 = prim->y1 + ((u16)g_field_hud_bar_height);
    prim->y2 = prim->y3;
    addPrim(ot, prim);
    return prim + 1;
}

/**
 * @brief Draw the party panels and the panels of recently hit enemies.
 * @param render_half Render half that receives the primitives.
 * @note Party members not performing an action get their special attack gauge shown.
 */
void field_draw_actor_hud(FieldRenderHalf* render_half)
{
    /* Effect records share the actor record layout (position and animation). */
    extern FieldActor g_field_effect_records[];

    Vec2s position;
    s32 i = 0;
    s32 panel_count = i;
    s32 unused_presence = FIELD_ACTOR_UNUSED;
    s32 command;
    FieldActor* single_actor;
    FieldPlayerRecord* single_player;
    FieldActor* paired_actor;
    FieldPlayerRecord* paired_player;
    s16 panel_y;
    s32 boss_drawn;
    FieldActor* anchor;
    s32 group;
    s32 hp_display;
    u32 current_hp;
    u32 displayed_hp;
    s32 world_y;
    s32 projected_y;
    s32 coordinate;
    s32 camera_y;
    s32 timer;
    u32 updated_hp_display;

    do
    {
        if (g_field_actors[i].presence != unused_presence && (g_field_player_records[i].head.bytes.flags & FIELD_PLAYER_ACTIVE))
        {
            /* each command goes through the local; comparing the constants directly changes the code */
            command = FIELD_ACTOR_COMMAND_ACTION;
            if (g_field_actors[i].command != command)
            {
                command = FIELD_ACTOR_COMMAND_INSTRUMENT;
                if (g_field_actors[i].command != command)
                {
                    g_field_object_states[i].hud.word |= FIELD_HUD_SPECIAL_GAUGE;
                }
            }
            panel_count++;
        }
        i++;
    } while (i < FIELD_PARTY_COUNT);
    switch (panel_count)
    {
    case 1:
        for (i = 0; i < FIELD_PARTY_COUNT; i++)
        {
            single_actor = &g_field_actors[i];
            single_player = &g_field_player_records[i];
            if (single_actor->presence != FIELD_ACTOR_UNUSED && (single_player->head.bytes.flags & FIELD_PLAYER_ACTIVE))
            {
                field_draw_actor_hud_panel(FIELD_HUD_SINGLE_X, FIELD_HUD_PARTY_Y, i, render_half, FIELD_HUD_HP_PER_BAR);
            }
        }
        break;
    case 2:
        panel_count = 0;
        for (i = 0; i < FIELD_PARTY_COUNT; i++)
        {
            paired_actor = &g_field_actors[i];
            paired_player = &g_field_player_records[i];
            if (paired_actor->presence != FIELD_ACTOR_UNUSED && (paired_player->head.bytes.flags & FIELD_PLAYER_ACTIVE))
            {
                field_draw_actor_hud_panel(FIELD_HUD_PAIR_X + panel_count * FIELD_HUD_PAIR_STEP, FIELD_HUD_PARTY_Y, i, render_half, FIELD_HUD_HP_PER_BAR);
                panel_count++;
            }
        }
        break;
    case 3:
        panel_count = 0;
        for (i = 0; i < FIELD_PARTY_COUNT; i++)
        {
            if (g_field_actors[g_field_party_hud_order[i]].presence != FIELD_ACTOR_UNUSED &&
                (g_field_player_records[g_field_party_hud_order[i]].head.bytes.flags & FIELD_PLAYER_ACTIVE))
            {
                panel_y = FIELD_HUD_TRIO_LOW_Y;
                if (i & 1)
                {
                    panel_y = FIELD_HUD_TRIO_HIGH_Y;
                }
                field_draw_actor_hud_panel(FIELD_HUD_TRIO_X + panel_count * FIELD_HUD_TRIO_STEP, panel_y, g_field_party_hud_order[i], render_half,
                                           FIELD_HUD_HP_PER_BAR);
                panel_count++;
            }
        }
        break;
    }
    boss_drawn = 0;
    /* Enemy panels are transient; the boss uses a fixed panel at the bottom. */
    if (g_field_scene_mode_bit != 0)
    {
        if (D_80122B20 == 0)
        {
            for (i = FIELD_PARTY_COUNT; i < FIELD_ACTOR_COUNT; i++)
            {
                group = g_field_object_states[i].group_flags & 0xF;
                if (group == g_field_active_group && group != 0)
                {
                    hp_display = g_field_object_states[i].unk8.word;
                    if (hp_display < 0)
                    {
                        if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED && (hp_display & FIELD_HUD_HP_MASK) && boss_drawn == 0)
                        {
                            field_draw_actor_hud_panel(FIELD_HUD_BOSS_X, FIELD_HUD_BOSS_Y, i, render_half, FIELD_HUD_BOSS_HP_PER_BAR);
                            boss_drawn = 1;
                        }
                    }
                    else if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED)
                    {
                        current_hp = g_field_object_states[i].unk4.word;
                        displayed_hp = hp_display & FIELD_HUD_HP_MASK;
                        if (current_hp < displayed_hp)
                        {
                            g_field_object_states[i].unk8.word =
                                (hp_display & ~(FIELD_HUD_TIMER_MASK << FIELD_HUD_TIMER_SHIFT)) | (FIELD_HUD_PANEL_FRAMES << FIELD_HUD_TIMER_SHIFT);
                        }
                        else if (displayed_hp != current_hp)
                        {
                            g_field_object_states[i].unk8.word =
                                (hp_display & ~(FIELD_HUD_TIMER_MASK << FIELD_HUD_TIMER_SHIFT)) | (FIELD_HUD_PANEL_FRAMES << FIELD_HUD_TIMER_SHIFT);
                        }
                        if (g_field_object_states[i].unk8.bytes[3] & FIELD_HUD_TIMER_MASK)
                        {
                            if (!(g_field_object_states[i].flags & FIELD_OBJECT_FLAG_0100) && (g_field_object_states[i].contact.bytes.flags & 1) &&
                                (g_field_effect_records[g_field_object_states[i].linked_effect_index].animation & 0x7F) != FIELD_HUD_ANCHOR_IGNORED_ANIMATION)
                            {
                                anchor = &g_field_effect_records[g_field_object_states[i].linked_effect_index];
                                {
                                    s32 camera_x = g_field_view_offset_x / 256;
                                    s32 world_x = anchor->x / 256 + FIELD_SCREEN_CENTER_X;
                                    coordinate = camera_x + world_x;
                                    position.x = coordinate;
                                }
                                projected_y = g_field_view_offset_y / 256;
                                world_y = g_field_effect_records[g_field_object_states[i].linked_effect_index].y / 256 + FIELD_SCREEN_CENTER_Y;
                                projected_y = projected_y + world_y;
                                coordinate =
                                    projected_y - g_field_effect_records[g_field_object_states[i].linked_effect_index].z / 512 - g_field_view_offset_z / 512;
                                position.y = coordinate;

                                coordinate = position.x;
                                if (coordinate + FIELD_HUD_MARGIN_X > SCREEN_WIDTH)
                                {
                                    position.x = SCREEN_WIDTH - FIELD_HUD_MARGIN_X;
                                }
                                coordinate = position.x;
                                if (coordinate < FIELD_HUD_MARGIN_X)
                                {
                                    position.x = FIELD_HUD_MARGIN_X;
                                }
                                coordinate = position.y;
                                if (coordinate > FIELD_HUD_MAX_Y)
                                {
                                    position.y = FIELD_HUD_MAX_Y;
                                }
                                coordinate = position.y;
                                if (coordinate < FIELD_HUD_MIN_Y)
                                {
                                    position.y = FIELD_HUD_MIN_Y;
                                }
                            }
                            else
                            {
                                {
                                    s32 camera_x = g_field_view_offset_x / 256;
                                    s32 world_x = g_field_actors[i].x / 256 + FIELD_SCREEN_CENTER_X;
                                    coordinate = camera_x + world_x;
                                    position.x = coordinate;
                                }
                                camera_y = g_field_view_offset_y / 256;
                                {
                                    s32 actor_y = g_field_actors[i].y / 256 + FIELD_SCREEN_CENTER_Y;
                                    projected_y = camera_y + actor_y;
                                }
                                coordinate = projected_y - g_field_actors[i].z / 512 - g_field_view_offset_z / 512;
                                position.y = coordinate;

                                coordinate = position.x;
                                if (coordinate + FIELD_HUD_MARGIN_X > SCREEN_WIDTH)
                                {
                                    position.x = SCREEN_WIDTH - FIELD_HUD_MARGIN_X;
                                }
                                coordinate = position.x;
                                if (coordinate < FIELD_HUD_MARGIN_X)
                                {
                                    position.x = FIELD_HUD_MARGIN_X;
                                }
                                coordinate = position.y;
                                if (coordinate > FIELD_HUD_MAX_Y)
                                {
                                    position.y = FIELD_HUD_MAX_Y;
                                }
                                coordinate = position.y;
                                if (coordinate < FIELD_HUD_MIN_Y)
                                {
                                    position.y = FIELD_HUD_MIN_Y;
                                }
                            }
                            field_draw_actor_hud_panel(position.x - FIELD_HUD_ENEMY_OFFSET_X, position.y, i, render_half, FIELD_HUD_HP_PER_BAR);
                            updated_hp_display = g_field_object_states[i].unk8.word;
                            timer = ((u32)updated_hp_display >> FIELD_HUD_TIMER_SHIFT) & FIELD_HUD_TIMER_MASK;
                            if (timer != 0)
                            {
                                g_field_object_states[i].unk8.word = (updated_hp_display & ~(FIELD_HUD_TIMER_MASK << FIELD_HUD_TIMER_SHIFT)) |
                                                                     (((timer - 1) & FIELD_HUD_TIMER_MASK) << FIELD_HUD_TIMER_SHIFT);
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Draw one HUD panel and move its displayed HP toward the current HP.
 * @param x Screen x of the panel.
 * @param y Screen y of the panel, before the shake offset.
 * @param slot Object index; party members (0 to 2) also get the blinking portrait and the HP percentage.
 * @param render_half Render half whose primitive cursor is advanced.
 * @param hp_per_bar HP of one gauge bar; each full bar is drawn in the next color.
 */
static void field_draw_actor_hud_panel(s32 x, s32 y, s32 slot, FieldRenderHalf* render_half, u32 hp_per_bar)
{
    Vec2s* scratch = FIELD_HUD_SCRATCH_POSITION;
    FieldRenderHalf* ctx;
    FieldRenderHalf* call_ctx;
    u32 recovery_width;
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
    s32 rising_x;
    s32 falling_x;
    s32 full_color;
    s32 partial_color;
    s32 palette_index;
    s32 rising_value;
    s32 falling_value;
    s32 rising_packed_value;
    s32 falling_packed_value;
    u16 intensity;
    u32 current_hp;
    u32 displayed_value;
    u32 packed_display_value;
    u32 partial_bar_count;
    u32 boss_flag;
    u32 full_palette_level;
    u32 hp_or_bar_count; /* full bar count, later the current HP; two locals change the register allocation */
    u32 rising_current;
    u32 rising_wrapped_current;
    u32 falling_current;
    u32 falling_wrapped_current;
    u32 rising_displayed;
    u32 falling_displayed;
    u32 maximum_hp;
    u32 current_value;
    u32 hp_percent;
    CVECTOR* colors;
    u32 mask24;
    u32 fill_mask;
    s32 background_type;
    u8 gauge_type;
    FieldPlayerRecord* player;
    FieldPlayerRecord* players;
    FieldHudGauge* state;
    u8* label_cursor;
    POLY_F4* rising_cursor;
    u8* helper_cursor;
    POLY_F4* falling_cursor;
    u_long* rising_ot;
    u_long* falling_ot;
    u8* sprite_cursor;
    u8* gauge_cursor;
    POLY_G4* bar;

    if (slot < FIELD_PARTY_COUNT)
    {
        if (g_field_player_records[slot].hit_state < FIELD_HUD_SHAKE_FRAMES)
        {
            y += g_field_hud_shake_offsets[g_field_player_records[slot].hit_state];
            if (g_field_player_records[slot].hit_state != 0)
            {
                g_field_player_records[slot].hit_state--;
            }
            else
            {
                g_field_player_records[slot].hit_state = FIELD_HUD_SHAKE_IDLE;
            }
        }
    }
    state = (FieldHudGauge*)&g_field_object_states[slot];
    boss_flag = (u32)state->displayed_hp >> FIELD_HUD_BOSS_SHIFT;
    boss_panel = boss_flag;
    if ((boss_flag != 0) && (g_field_boss_hud_shake_frame < FIELD_HUD_SHAKE_FRAMES))
    {
        y += g_field_hud_shake_offsets[g_field_boss_hud_shake_frame];
        if (g_field_boss_hud_shake_frame != 0)
        {
            g_field_boss_hud_shake_frame--;
        }
        else
        {
            g_field_boss_hud_shake_frame = FIELD_HUD_SHAKE_IDLE;
        }
    }
    ctx = render_half;
    label_cursor = ctx->primitive_cursor;
    if (state->hud.bytes.object_index < FIELD_PARTY_COUNT)
    {
        g_field_hud_bar_offset_x = 29;
        g_field_hud_bar_offset_y = 9;
        scratch->x = x;
        scratch->y = y;
        label_cursor = field_emit_actor_portrait((SPRT*)label_cursor, &ctx->ordering_table[FIELD_HUD_OT_INDEX], state->hud.bytes.object_index, scratch);
        g_field_hud_bar_width = FIELD_HUD_BAR_WIDTH;
        g_field_hud_bar_height = FIELD_HUD_BAR_HEIGHT;
    }
    else if (boss_panel != 0)
    {
        g_field_hud_bar_offset_x = 15;
        g_field_hud_bar_offset_y = 6;
        g_field_hud_bar_width = FIELD_HUD_BOSS_BAR_WIDTH;
        g_field_hud_bar_height = FIELD_HUD_BAR_HEIGHT;
    }
    else
    {
        g_field_hud_bar_offset_x = 4;
        g_field_hud_bar_offset_y = 6;
        g_field_hud_bar_width = FIELD_HUD_BAR_WIDTH;
        g_field_hud_bar_height = FIELD_HUD_BAR_HEIGHT;
    }
    sprite_cursor = label_cursor;
    if (slot < FIELD_PARTY_COUNT)
    {
        if (g_field_hud_blink_frames[slot] >= FIELD_HUD_BLINK_FRAMES || ++g_field_hud_blink_frames[slot] >= FIELD_HUD_BLINK_FRAMES)
        {
            for (scan_slot = 0; scan_slot < FIELD_HUD_BLINK_LIST_COUNT; scan_slot++)
            {
                if (g_field_pair_indicator_list[scan_slot] == FIELD_HUD_BLINK_LIST_END)
                {
                    break;
                }
                if (g_field_pair_indicator_list[scan_slot] == slot)
                {
                    g_field_hud_blink_frames[slot] = 0;
                    break;
                }
            }
            blink_counters = g_field_hud_blink_frames;
            blink_counter = blink_counters + slot;
            if ((*blink_counter != 0) && !(rand() & FIELD_HUD_BLINK_CHANCE_MASK))
            {
                *blink_counter = 0;
            }
        }
        SET_BGR0_PACKED(sprite_cursor, GPU_TINT_NEUTRAL);
        setSprt((SPRT*)sprite_cursor);
        ((SPRT*)sprite_cursor)->x0 = (s16)(x + 34);
        ((SPRT*)sprite_cursor)->y0 = (s16)(y - 6);
        if (g_field_hud_blink_frames[slot] < FIELD_HUD_BLINK_FRAMES)
        {
            blink_uv = ((u16)g_field_hud_blink_frames[slot] * 16) + FIELD_HUD_UV(0x58, 0x20);
        }
        else
        {
            blink_uv = FIELD_HUD_UV(0x58, 0x20);
        }
        SET_SPRT_WH_PACKED(sprite_cursor, 16, 16);
        SET_SPRT_UV0_PACKED(sprite_cursor, blink_uv);
        ((SPRT*)sprite_cursor)->clut = FIELD_HUD_BLINK_CLUT;
        addPrim(&ctx->ordering_table[FIELD_HUD_OT_INDEX], sprite_cursor);
        sprite_cursor += sizeof(SPRT);
        scratch->x = x + 48;
        scratch->y = y;
        current_hp = state->current_hp;
        maximum_hp = state->maximum_hp;
        hp_percent = current_hp * 100;
        if (maximum_hp != 0)
        {
            hp_percent = hp_percent / maximum_hp;
        }
        if ((hp_percent == 0) && (current_hp != 0))
        {
            hp_percent = 1;
        }
        sprite_cursor = field_emit_hud_percentage(sprite_cursor, ctx, hp_percent, scratch);
    }
    gauge_type = state->hud.bytes.object_index;
    helper_cursor = sprite_cursor;
    if (gauge_type < 2)
    {
        if (state->hud.word & FIELD_HUD_SPECIAL_GAUGE)
        {
            if (state->technique_gauge == 0xFF)
            {
                if (g_frame_counter & 8)
                {
                    colors = g_field_hud_full_status_colors;
                }
                else
                {
                    colors = g_field_hud_status_colors;
                }
            }
            else
            {
                colors = g_field_hud_status_colors;
            }
            intensity = state->technique_gauge;
            call_ctx = ctx;
            helper_cursor = field_emit_hud_status_line(helper_cursor, call_ctx, colors, intensity, x, y);
        }
        else
        {
            call_ctx = ctx;
            colors = g_field_hud_effect_colors;
            intensity = state->action_charge;
            helper_cursor = field_emit_hud_status_line(helper_cursor, call_ctx, colors, intensity, x, y);
        }
    }
    else if ((gauge_type == 2) && (D_800FDCEA >= FIELD_HUD_COMPANION_KIND_MIN))
    {
        colors = g_field_hud_companion_status_colors;
        helper_cursor = field_emit_hud_status_line(helper_cursor, ctx, colors, state->technique_gauge, x, y);
    }
    /* A knocked-down member shows the recovery gauge instead of the HP bars. */
    gauge_cursor = helper_cursor;
    if (slot < FIELD_PARTY_COUNT && (players = g_field_player_records, player = &players[slot], player->revive_delay != 0) &&
        g_field_actors[slot].command == FIELD_ACTOR_COMMAND_KNOCKED_DOWN)
    {
        bar = (POLY_G4*)gauge_cursor;
        SET_BGR0_PACKED(bar, FIELD_HUD_RECOVERY_DARK);
        SET_POLY_G4_BGR2_PACKED(bar, FIELD_HUD_RECOVERY_DARK);
        setlen(bar, FIELD_POLY_G4_LENGTH);
        setcode(bar, FIELD_POLY_G4_CODE);
        SET_POLY_G4_BGR1_PACKED(bar, FIELD_HUD_WHITE);
        SET_POLY_G4_BGR3_PACKED(bar, FIELD_HUD_WHITE);
        recovery_width = (s16)player->revive_delay;
        recovery_width = (s32)(g_field_hud_bar_width * player->revive_time) / (s32)recovery_width;
        field_hud_set_bar_geometry(bar, x, y, recovery_width);
        addPrim(&ctx->ordering_table[FIELD_HUD_OT_INDEX], bar);
        gauge_cursor += sizeof(POLY_G4);
    }
    else
    {
        current_value = state->current_hp;
        if (current_value != 0)
        {
            partial_bar_count = current_value / hp_per_bar;
            palette_index = partial_bar_count & 3;
            if (partial_bar_count >= 3)
            {
                palette_index |= 2;
            }
            fill_mask = FIELD_HUD_HP_MASK;
            if ((s32)((state->displayed_hp & fill_mask) - current_value) < (s32)hp_per_bar)
            {
                partial_palette = g_field_hud_hp_colors;
                partial_color_entry = &partial_palette[palette_index];
                bar = (POLY_G4*)gauge_cursor;
                SET_BGR0_PACKED(bar, (s32)(*partial_color_entry & FIELD_HUD_COLOR_DIM));
                SET_POLY_G4_BGR1_PACKED(bar, (s32)(*partial_color_entry & FIELD_HUD_COLOR_MID));
                partial_color = *partial_color_entry;
                setlen(bar, FIELD_POLY_G4_LENGTH);
                setcode(bar, FIELD_POLY_G4_CODE);
                SET_POLY_G4_BGR3_PACKED(bar, partial_color);
                SET_POLY_G4_BGR2_PACKED(bar, partial_color);
                partial_width = (s32)(g_field_hud_bar_width * (current_value % hp_per_bar)) / (s32)hp_per_bar;
                field_hud_set_bar_geometry(bar, x, y, partial_width);
                addPrim(&ctx->ordering_table[FIELD_HUD_OT_INDEX], bar);
                gauge_cursor += sizeof(POLY_G4);
            }
            hp_or_bar_count = (u32)state->current_hp / hp_per_bar;
            full_palette_level = hp_or_bar_count - 1;
            if (hp_or_bar_count != 0)
            {
                palette_index = full_palette_level & 3;
                if (full_palette_level >= 3)
                {
                    palette_index |= 2;
                }
                full_palette = g_field_hud_hp_colors;
                full_color_entry = &full_palette[palette_index];
                bar = (POLY_G4*)gauge_cursor;
                SET_BGR0_PACKED(bar, (s32)(*full_color_entry & FIELD_HUD_COLOR_DIM));
                SET_POLY_G4_BGR2_PACKED(bar, (s32)(*full_color_entry & FIELD_HUD_COLOR_MID));
                setlen(bar, FIELD_POLY_G4_LENGTH);
                setcode(bar, FIELD_POLY_G4_CODE);
                full_color = *full_color_entry;
                SET_POLY_G4_BGR3_PACKED(bar, full_color);
                SET_POLY_G4_BGR1_PACKED(bar, full_color);
                full_x = (u16)g_field_hud_bar_offset_x;
                full_right = (u16)g_field_hud_bar_width;
                full_y = (u16)g_field_hud_bar_offset_y;
                full_x += x;
                full_right += full_x;
                bar->x1 = full_right;
                full_right -= FIELD_HUD_BAR_SLANT;
                full_y += y;
                bar->x3 = full_right;
                bar->y1 = full_y;
                bar->x0 = full_x;
                full_x -= FIELD_HUD_BAR_SLANT;
                bar->x2 = full_x;
                bar->y0 = full_y;
                bar->y3 = bar->y1 + (u16)g_field_hud_bar_height;
                bar->y2 = bar->y3;
                addPrim(&ctx->ordering_table[FIELD_HUD_OT_INDEX], bar);
                gauge_cursor += sizeof(POLY_G4);
            }
        }
    }
    /* Animate the displayed HP (low 24 bits); keep the boss flag and the panel timer. */
    mask24 = FIELD_HUD_HP_MASK;
    packed_display_value = state->displayed_hp;
    hp_or_bar_count = state->current_hp;
    displayed_value = packed_display_value & mask24;
    if (displayed_value < hp_or_bar_count)
    {
        if (hp_or_bar_count - displayed_value >= FIELD_HUD_RISE_MIN_STEP)
        {
            rising_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            rising_packed_value &= packed_display_value;
            rising_value = displayed_value + ((hp_or_bar_count - displayed_value) / 3);
        }
        else
        {
            rising_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            rising_packed_value &= packed_display_value;
            rising_value = displayed_value + 1;
        }
        rising_packed_value |= rising_value & mask24;
        state->displayed_hp = rising_packed_value;
        rising_current = state->current_hp;
        if ((rising_current / hp_per_bar) == ((s32)(state->displayed_hp & FIELD_HUD_HP_MASK) / (s32)hp_per_bar))
        {
            rising_x = (u16)g_field_hud_bar_offset_x;
            rising_x += x;
            ((POLY_F4*)gauge_cursor)->x1 = (s16)(rising_x + ((u32)(g_field_hud_bar_width * (rising_current % hp_per_bar)) / hp_per_bar));
            rising_ot = &ctx->ordering_table[FIELD_HUD_DELTA_OT_INDEX];
            rising_cursor = (POLY_F4*)gauge_cursor;
            rising_x += ((s32)(g_field_hud_bar_width * ((s32)(state->displayed_hp & FIELD_HUD_HP_MASK) % (s32)hp_per_bar)) / (s32)hp_per_bar);
            rising_cursor->x0 = rising_x;
            helper_cursor = (u8*)field_emit_hud_healing_quad(rising_cursor, y, rising_ot);
        }
        else
        {
            rising_base_x = (u16)g_field_hud_bar_offset_x + x;
            ((POLY_F4*)gauge_cursor)->x1 = rising_base_x;
            ((POLY_F4*)gauge_cursor)->x0 =
                (s16)(rising_base_x + ((s32)(g_field_hud_bar_width * ((s32)(state->displayed_hp & FIELD_HUD_HP_MASK) % (s32)hp_per_bar)) / (s32)hp_per_bar));
            rising_cursor = field_emit_hud_healing_quad((POLY_F4*)gauge_cursor, y, &ctx->ordering_table[FIELD_HUD_DELTA_OT_INDEX]);
            rising_displayed = state->displayed_hp & FIELD_HUD_HP_MASK;
            rising_wrapped_current = state->current_hp;
            if (rising_displayed - rising_wrapped_current < hp_per_bar)
            {
                rising_right = (u16)g_field_hud_bar_offset_x + x + ((u32)(g_field_hud_bar_width * (rising_wrapped_current % hp_per_bar)) / hp_per_bar);
                rising_cursor->x1 = rising_right;
            }
            else
            {
                rising_right = (u16)g_field_hud_bar_offset_x + x;
                rising_cursor->x1 = rising_right;
            }
            rising_ot = &ctx->ordering_table[FIELD_HUD_DELTA_OT_INDEX];
            rising_cursor->x0 = (s16)(((u16)g_field_hud_bar_offset_x + x) + (u16)g_field_hud_bar_width);
            helper_cursor = (u8*)field_emit_hud_healing_quad(rising_cursor, y, rising_ot);
        }
        gauge_cursor = helper_cursor;
    }
    else if (hp_or_bar_count < displayed_value)
    {
        if (displayed_value - hp_or_bar_count >= FIELD_HUD_FALL_MIN_STEP)
        {
            falling_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            falling_packed_value &= packed_display_value;
            falling_value = displayed_value - ((displayed_value - hp_or_bar_count) / 3);
        }
        else
        {
            falling_packed_value = FIELD_HUD_DISPLAY_FLAGS_MASK;
            falling_packed_value &= packed_display_value;
            falling_value = displayed_value - 1;
        }
        falling_packed_value |= falling_value & mask24;
        state->displayed_hp = falling_packed_value;
        falling_current = state->current_hp;
        if ((falling_current / hp_per_bar) == ((s32)(state->displayed_hp & FIELD_HUD_HP_MASK) / (s32)hp_per_bar))
        {
            falling_x = (u16)g_field_hud_bar_offset_x;
            falling_x += x;
            ((POLY_F4*)gauge_cursor)->x1 = (s16)(falling_x + ((u32)(g_field_hud_bar_width * (falling_current % hp_per_bar)) / hp_per_bar));
            falling_ot = &ctx->ordering_table[FIELD_HUD_DELTA_OT_INDEX];
            falling_cursor = (POLY_F4*)gauge_cursor;
            falling_x += ((s32)(g_field_hud_bar_width * ((s32)(state->displayed_hp & FIELD_HUD_HP_MASK) % (s32)hp_per_bar)) / (s32)hp_per_bar);
            falling_cursor->x0 = falling_x;
            helper_cursor = (u8*)field_emit_hud_damage_quad(falling_cursor, y, falling_ot);
        }
        else
        {
            falling_base_x = (u16)g_field_hud_bar_offset_x + x;
            ((POLY_F4*)gauge_cursor)->x1 = falling_base_x;
            ((POLY_F4*)gauge_cursor)->x0 =
                (s16)(falling_base_x + ((s32)(g_field_hud_bar_width * ((s32)(state->displayed_hp & FIELD_HUD_HP_MASK) % (s32)hp_per_bar)) / (s32)hp_per_bar));
            falling_cursor = field_emit_hud_damage_quad((POLY_F4*)gauge_cursor, y, &ctx->ordering_table[FIELD_HUD_DELTA_OT_INDEX]);
            falling_displayed = state->displayed_hp & FIELD_HUD_HP_MASK;
            falling_wrapped_current = state->current_hp;
            if (falling_displayed - falling_wrapped_current < hp_per_bar)
            {
                falling_right = (u16)g_field_hud_bar_offset_x + x + ((u32)(g_field_hud_bar_width * (falling_wrapped_current % hp_per_bar)) / hp_per_bar);
                falling_cursor->x1 = falling_right;
            }
            else
            {
                falling_right = (u16)g_field_hud_bar_offset_x + x;
                falling_cursor->x1 = falling_right;
            }
            falling_ot = &ctx->ordering_table[FIELD_HUD_DELTA_OT_INDEX];
            falling_cursor->x0 = (s16)(((u16)g_field_hud_bar_offset_x + x) + (u16)g_field_hud_bar_width);
            helper_cursor = (u8*)field_emit_hud_damage_quad(falling_cursor, y, falling_ot);
        }
        gauge_cursor = helper_cursor;
    }
    sprite_cursor = gauge_cursor;
    SET_BGR0_PACKED(sprite_cursor, GPU_TINT_NEUTRAL);
    setSprt((SPRT*)sprite_cursor);
    setSemiTrans((SPRT*)sprite_cursor, 1);
    SET_SPRT_XY0_WORD(sprite_cursor, (s32)((y << 16) + x));
    background_type = state->hud.bytes.object_index;
    switch (background_type)
    {
    case 0:
    case 1:
        SET_SPRT_WH_PACKED(sprite_cursor, 88, 24);
        SET_SPRT_UV0_PACKED(sprite_cursor, FIELD_HUD_UV(0, 0x10));
        break;
    case 2:
        if (D_800FDCEA >= FIELD_HUD_COMPANION_KIND_MIN)
        {
            SET_SPRT_WH_PACKED(sprite_cursor, 88, 24);
            SET_SPRT_UV0_PACKED(sprite_cursor, FIELD_HUD_UV(0, 0x10));
        }
        else
        {
            SET_SPRT_WH_PACKED(sprite_cursor, 88, 24);
            SET_SPRT_UV0_PACKED(sprite_cursor, FIELD_HUD_UV(0, 0x28));
        }
        break;
    default:
        if (boss_panel != 0)
        {
            SET_SPRT_WH_PACKED(sprite_cursor, 256, 16);
            SET_SPRT_UV0_PACKED(sprite_cursor, FIELD_HUD_UV(0, 0));
        }
        else
        {
            SET_SPRT_WH_PACKED(sprite_cursor, 64, 16);
            SET_SPRT_UV0_PACKED(sprite_cursor, FIELD_HUD_UV(0, 0x40));
        }
        break;
    }
    ((SPRT*)sprite_cursor)->clut = FIELD_HUD_CLUT;
    addPrim(&ctx->ordering_table[FIELD_HUD_OT_INDEX], sprite_cursor);
    sprite_cursor += sizeof(SPRT);
    setDrawTPage((DR_TPAGE*)sprite_cursor, 0, 0, getTPage(0, 0, 960, 256));
    addPrim(&ctx->ordering_table[FIELD_HUD_OT_INDEX], sprite_cursor);
    render_half->primitive_cursor = sprite_cursor + sizeof(DR_TPAGE);
}

/**
 * @brief Emit the shaded line of a special attack or companion gauge.
 * @param packet Primitive-buffer cursor where the line is written.
 * @param render_half Render half whose HUD ordering-table entry receives the line.
 * @param colors Empty and full end colors; channels that differ fade with @p intensity.
 * @param intensity Gauge value, 0 to 255; sets the line length.
 * @param x Panel screen x.
 * @param y Panel screen y.
 * @return Pointer just past the line, or @p packet when @p intensity is zero.
 */
static u8* field_emit_hud_status_line(u8* packet, FieldRenderHalf* render_half, CVECTOR* colors, s32 intensity, s32 x, s32 y)
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

    first_color = *(u32*)&colors[0];
    setlen((LINE_G2*)packet, 4);
    SET_BGR0_PACKED(packet, first_color);
    setcode((LINE_G2*)packet, 0x50);

    first_component = colors[0].r;
    second_component = colors[1].r;
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

    first_component = colors[0].g;
    second_component = colors[1].g;
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

    first_component = colors[0].b;
    second_component = colors[1].b;
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

    line_x = x + 24;
    ((LINE_G2*)packet)->x0 = line_x;
    line_y = y + 16;
    ((LINE_G2*)packet)->y1 = line_y;
    ((LINE_G2*)packet)->y0 = line_y;
    ((LINE_G2*)packet)->x1 = line_x + ((intensity * FIELD_HUD_LINE_LENGTH) / 255);

    addPrim(&render_half->ordering_table[FIELD_HUD_OT_INDEX], (LINE_G2*)packet);
    return packet + sizeof(LINE_G2);
}

/**
 * @brief Emit a right-aligned HP percentage followed by the percent glyph.
 * @param packet Primitive-buffer cursor.
 * @param render_half Render half forwarded to field_emit_hud_glyph.
 * @param value HP percentage, 0 to 100.
 * @param position Screen position; x advances 7 pixels per column.
 * @return Pointer just past the percent glyph.
 * @note Leading zeros in the hundreds and tens columns are left blank.
 */
static void* field_emit_hud_percentage(void* packet, FieldRenderHalf* render_half, s32 value, Vec2s* position)
{
    s32 emitted;

    emitted = 0;
    if (value / 100 != 0)
    {
        emitted = 1;
        packet = field_emit_hud_glyph(packet, render_half, value / 100, position);
        value -= 100;
    }
    position->x += FIELD_HUD_GLYPH_ADVANCE;
    if (emitted || value / 10 != 0)
    {
        s32 digit;
        digit = value / 10;
        packet = field_emit_hud_glyph(packet, render_half, digit, position);
        value -= digit * 10;
    }
    position->x += FIELD_HUD_GLYPH_ADVANCE;
    packet = field_emit_hud_glyph(packet, render_half, value, position);
    position->x += FIELD_HUD_GLYPH_ADVANCE;
    return field_emit_hud_glyph(packet, render_half, FIELD_HUD_GLYPH_PERCENT, position);
}

/**
 * @brief Emit a digit or percent glyph from the HUD texture strip.
 * @param packet Primitive-buffer cursor.
 * @param render_half Render half whose HUD ordering-table entry receives the sprite.
 * @param value Glyph index: 0 to 9 for digits, 10 for percent.
 * @param position Packed screen x/y.
 * @return Pointer just past the sprite.
 */
static void* field_emit_hud_glyph(void* packet, FieldRenderHalf* render_half, s32 value, Vec2s* position)
{
    SPRT* sprite;
    s32 packed_xy;

    sprite = (SPRT*)packet;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    packed_xy = *(s32*)position;
    SET_SPRT_UV0_PACKED(sprite, (s16)((value * 8) + FIELD_HUD_UV(0x58, 0x15)));
    SET_SPRT_WH_PACKED(sprite, 8, 11);
    sprite->clut = FIELD_HUD_CLUT;
    SET_SPRT_XY0_WORD(sprite, packed_xy);
    addPrim(&render_half->ordering_table[FIELD_HUD_OT_INDEX], sprite);
    return sprite + 1;
}

/**
 * @brief Finish and link a red damage quad whose x0 and x1 are already set.
 * @param prim Quad; the lower edge is slanted 3 pixels to the left.
 * @param y Panel screen y, added to g_field_hud_bar_offset_y.
 * @param ot Ordering-table entry to link into.
 * @return Pointer just past the quad.
 */
static POLY_F4* field_emit_hud_damage_quad(POLY_F4* prim, s32 y, u_long* ot)
{
    SET_BGR0_PACKED(prim, FIELD_HUD_RED);
    setPolyF4(prim);
    return field_hud_link_change_quad(prim, y, ot);
}

/**
 * @brief Finish and link a white healing quad, widening a zero-width quad to one pixel.
 * @param prim Quad whose x0 and x1 are already set.
 * @param y Panel screen y, added to g_field_hud_bar_offset_y.
 * @param ot Ordering-table entry to link into.
 * @return Pointer just past the quad.
 */
static POLY_F4* field_emit_hud_healing_quad(POLY_F4* prim, s32 y, u_long* ot)
{
    if (prim->x1 == prim->x0)
    {
        prim->x1 = prim->x0 + 1;
    }
    SET_BGR0_PACKED(prim, FIELD_HUD_WHITE);
    setPolyF4(prim);
    return field_hud_link_change_quad(prim, y, ot);
}

/**
 * @brief Emit a party member's portrait and its texture page.
 * @param sprt Sprite primitive to fill.
 * @param ot Ordering-table entry the sprite and its DR_TPAGE are linked into.
 * @param index Party slot; slot 2 uses the companion palette for companion kinds 0x41 and up.
 * @param position Screen position of the portrait.
 * @return Pointer just past the DR_TPAGE.
 */
void* field_emit_actor_portrait(SPRT* sprt, u_long* ot, s32 index, Vec2s* position)
{
    DR_TPAGE* mode;

    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    SET_SPRT_XY0_WORD(sprt, *(u32*)position);
    SET_SPRT_UV0_PACKED(sprt, FIELD_HUD_UV(0, 0xE8));
    SET_SPRT_WH_PACKED(sprt, 24, 24);

    if (index == 2 && D_800FDCEA >= FIELD_HUD_COMPANION_KIND_MIN)
    {
        sprt->clut = (((D_800FE01E & 3) + 0x1EF) << 6) | 0x10;
    }
    else
    {
        sprt->clut = ((index + 0x1F4) << 6) | ((g_field_actors[index].control.word >> 19) & 0xF);
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
 * @brief Read an image resource from disc into the field CD buffer and upload it to VRAM.
 * @param id Resource index; only the low 16 bits are used.
 * @param rect Pixel and CLUT destinations, see field_upload_image_resource.
 * @param mode CLUT upload mode, see field_upload_image_resource.
 * @return Result of field_upload_image_resource.
 */
s32 field_load_vram_resource(s32 id, RECT* rect, s32 mode)
{
    u8* buf = g_field_cd_buffer;

    cdrom_queue_read(id & 0xFFFF, buf);
    cdrom_wait_queue_empty();
    return field_upload_image_resource(rect, (Tim*)buf, mode);
}

/**
 * @brief Upload a TIM's CLUT block and pixel block into VRAM.
 * @param rect x/y: pixel destination, w/h: CLUT destination; returns the pixel width and height in x/y.
 * @param resource TIM file with a CLUT block followed by a pixel block.
 * @param mode Nonzero uploads the CLUT as one row of width * height entries; zero keeps its shape.
 * @return CLUT entries 240 and 241 as one packed word.
 */
static s32 field_upload_image_resource(RECT* rect, Tim* resource, s32 mode)
{
    RECT load_rect;
    s32 offset;
    s32 palette_word;
    TimBlock* image;
    TimDimensions* dimensions;
    u16 x;

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

/**
 * @brief Run the handlers of an object's changed flags and refresh its actor tint.
 * @param index Object index.
 * @note A handler entry below 0x100 is an animation resource played on the object's effect slot.
 */
void field_update_object_effects(s32 index)
{
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
    u32* handler;
    u32* handlers;
    u32 handler_value;
    FieldActorSlot* slot;
    FieldActor* actor;
    FieldObjectState* state;

    /* offset first: &g_field_actors[index] adds the base first and loses the shared offset */
    record_offset = index * sizeof(FieldActor);
    actor = (FieldActor*)(record_offset + (u8*)g_field_actors);
    state = &g_field_object_states[index];
    flags_before = state->flags;
    slot = &g_field_actor_slots[FIELD_OBJECT_EFFECT_SLOT_BASE + index];
    if (flags_before & FIELD_OBJECT_IMMOBILE_FLAGS)
    {
        state->flags &= ~FIELD_OBJECT_FLAG_4000;
        state->flags &= 0xFFFF7FFF;
    }
    flags_current = state->flags;
    previous_flags = state->previous_flags;
    changed_or_current = (flags_current ^ previous_flags) | flags_current;
    if (changed_or_current != 0)
    {
        if (slot->active == 0)
        {
            do /* a loop statement here lets loop.c hoist the callback argument out of the search */
            {
                bit_mask = 0x8000;
                bit_index = FIELD_OBJECT_HANDLER_COUNT - 1;
                animation_slot = index + FIELD_OBJECT_EFFECT_SLOT_BASE;
                handlers = g_field_object_flag_handlers;
                handler = handlers + bit_index;
            find_handler:
                if ((changed_or_current & bit_mask) && (handler_value = *handler, (handler_value != FIELD_OBJECT_HANDLER_NONE)))
                {
                    if (handler_value < FIELD_OBJECT_HANDLER_FUNCTION_MIN)
                    {
                        if (state->flags & bit_mask)
                        {
                            field_start_builtin_animation(index, animation_slot, handler_value);
                            field_start_actor_animation(animation_slot, 0, 0);
                        }
                    }
                    else
                    {
                        ((void (*)(FieldActor*, s32))handler_value)((FieldActor*)(record_offset + (u8*)g_field_actors), state->flags & bit_mask);
                    }
                    clear_mask = ~bit_mask;
                    state->previous_flags = (s32)((state->previous_flags & clear_mask) | (state->flags & bit_mask));
                }
                else
                {
                    handler--;
                    bit_index--;
                    bit_mask >>= 1;
                    if (bit_index >= 0)
                    {
                        goto find_handler;
                    }
                }
            } while (0);
        }
        else
        {
            if (!(flags_current & FIELD_OBJECT_FLAG_8000) && (previous_flags & FIELD_OBJECT_FLAG_8000))
            {
                slot->active = 0U;
                slot->track_mask = 0;
                field_clear_actor_effects(slot);
                state->previous_flags &= 0xFFFF7FFF;
            }
            else if (!(state->flags & FIELD_OBJECT_FLAG_4000) && (state->previous_flags & FIELD_OBJECT_FLAG_4000))
            {
                slot->active = 0U;
                slot->track_mask = 0;
                field_clear_actor_effects(slot);
                state->previous_flags &= ~FIELD_OBJECT_FLAG_4000;
            }
            else
            {
                changed_flags = state->flags;
                old_flags = state->previous_flags;
                changed_flags = (changed_flags ^ old_flags) & old_flags;
                if (changed_flags != 0)
                {
                    scan_flags = (s32)changed_flags;
                    highest_bit = FIELD_OBJECT_HANDLER_COUNT - 1;
                    animation_bit = 1;
                    while (1)
                    {
                        if (scan_flags & (animation_bit << highest_bit))
                        {
                            break;
                        }
                        highest_bit--;
                    }
                    /* Flag bit whose handler plays the slot's current animation resource. */
                    animation_kind = slot->status.parts.animation_id;
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
                        slot->active = 0U;
                        slot->track_mask = 0;
                        field_clear_actor_effects(slot);
                        if ((state->contact.word & 1) && (state->contact.bytes.controller_index == (actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE)))
                        {
                            actor->presence = 0;
                            state->contact.word = (s32)(state->contact.word & ~1);
                        }
                    }
                }
            }
        }
    }
    if (state->flags & FIELD_OBJECT_FLAG_HIT_FLASH)
    {
        g_field_actors[index].tint_blue = FIELD_HIT_FLASH_TINT;
        g_field_actors[index].tint_green = FIELD_HIT_FLASH_TINT;
        state->flags = (s32)(state->flags & ~FIELD_OBJECT_FLAG_HIT_FLASH);
    }
    else
    {
        g_field_actors[index].tint_red = g_field_object_parts[index].tint_red;
        g_field_actors[index].tint_green = g_field_object_parts[index].tint_green;
        g_field_actors[index].tint_blue = g_field_object_parts[index].tint_blue;
    }
}

/**
 * @brief Flag 0x0004 handler: restart the actor's animation and play effect resource 7.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set; zero does nothing.
 */
void field_handle_object_flag_0004(FieldActor* actor, s32 is_set)
{
    if (is_set != 0)
    {
        actor->animation_state = 1;
        actor->animation_frame = 0;
        actor->animation &= FIELD_ANIMATION_MIRROR_FLAG;
        actor->animation_active = 1;
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_ANIMATION_BITS;
        field_restart_actor_animation(actor);
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 7);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
        field_clear_link_target_flag(actor->object_index);
    }
}

/**
 * @brief Flag 0x0020 handler: restart the actor's animation and play effect resource 10.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set; zero does nothing.
 */
void field_handle_object_flag_0020(FieldActor* actor, s32 is_set)
{
    if (is_set != 0)
    {
        actor->animation_state = 1;
        actor->animation_frame = 0;
        actor->animation &= FIELD_ANIMATION_MIRROR_FLAG;
        actor->animation_active = 1;
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_ANIMATION_BITS;
        field_restart_actor_animation(actor);
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 10);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
    }
}

/**
 * @brief Flag 0x0400 handler: set the actor's horizontal model scale to half or full size.
 * @param actor Actor whose object part is updated.
 * @param half_scale Nonzero for half-size X/Z scale, zero for full size.
 */
void field_set_actor_horizontal_scale(FieldActor* actor, s32 half_scale)
{
    if (half_scale != 0)
    {
        g_field_object_parts[actor->object_index].scale_z = FIELD_PART_SCALE_HALF;
        g_field_object_parts[actor->object_index].scale_x = FIELD_PART_SCALE_HALF;
    }
    else
    {
        g_field_object_parts[actor->object_index].scale_z = FIELD_PART_SCALE_FULL;
        g_field_object_parts[actor->object_index].scale_x = FIELD_PART_SCALE_FULL;
    }
}

/**
 * @brief Flag 0x0040 handler: play effect resource 9 and hold the actor in animation 0x1B.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set; zero does nothing.
 */
void field_handle_object_flag_0040(FieldActor* actor, s32 is_set)
{
    u8 animation;

    if (is_set != 0)
    {
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 9);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
        animation = actor->animation;
        if ((animation & FIELD_ANIMATION_SEQUENCE_MASK) != FIELD_ANIMATION_FLAG_0040_POSE)
        {
            actor->animation = (animation & FIELD_ANIMATION_MIRROR_FLAG) + FIELD_ANIMATION_FLAG_0040_POSE;
            actor->animation_state = 1;
            actor->animation_frame = 0;
            actor->animation_active = 1;
            g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_ANIMATION_BITS;
            field_restart_actor_animation(actor);
            actor->control.word |= FIELD_ANIMATION_HOLD_LAST_FRAME;
        }
        field_clear_link_target_flag(actor->object_index);
    }
}

/**
 * @brief Flag 0x0080 handler: hold the actor in animation 0x14, or release it and play effect resource 0x91.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set, zero when it was cleared.
 */
void field_handle_object_flag_0080(FieldActor* actor, s32 is_set)
{
    if (is_set != 0)
    {
        actor->animation_state = 1;
        actor->animation_active = 1;
        actor->animation_frame = 0;
        actor->animation = (actor->animation & FIELD_ANIMATION_MIRROR_FLAG) + FIELD_ANIMATION_FLAG_0080_POSE;
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_ANIMATION_BITS;
        field_restart_actor_animation(actor);
        actor->control.word |= FIELD_ACTOR_CONTROL_40000 | FIELD_ANIMATION_HOLD_LAST_FRAME;
        field_clear_link_target_flag(actor->object_index);
    }
    else
    {
        actor->animation_frame = 0;
        actor->animation_state = 1;
        actor->animation_active = 1;
        actor->animation &= FIELD_ANIMATION_MIRROR_FLAG;
        g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_ANIMATION_BITS;
        field_restart_actor_animation(actor);
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0x91);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
        actor->control.word &= ~FIELD_ACTOR_CONTROL_40000;
    }
}

/**
 * @brief Clear flag 0x2000 of the object that @p object_index is linked to, if it is linked.
 * @param object_index Object index.
 */
static inline void field_clear_link_target_flag(s32 object_index)
{
    FieldObjectState* states = g_field_object_states;
    FieldObjectState* state = &states[object_index];

    if ((state->contact.word >> FIELD_CONTACT_LINKED_SHIFT) & 1)
    {
        FieldObjectState* target = &states[state->linked_object_index];
        target->flags &= ~FIELD_OBJECT_FLAG_2000;
    }
}

/**
 * @brief Flag 0x0100 handler: play effect resource 12 and hide the actor, or play 13 and show it.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set, zero when it was cleared.
 */
void field_handle_object_flag_0100(FieldActor* actor, s32 is_set)
{
    if (is_set != 0)
    {
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 12);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
        actor->presence = FIELD_ACTOR_HIDDEN;
        field_clear_link_target_flag(actor->object_index);
    }
    else
    {
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 13);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
        actor->presence = 0;
    }
}

/**
 * @brief Knocked-out flag handler: clear flag 0x2000 of the object the actor is linked to.
 * @param actor Actor of the object.
 */
void field_handle_object_knocked_out(FieldActor* actor)
{
    field_clear_link_target_flag(actor->object_index);
}

/**
 * @brief Flag 0x8000 handler: play effect resource 14, or stop the object's effect slot.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set, zero when it was cleared.
 */
void field_handle_object_flag_8000(FieldActor* actor, s32 is_set)
{
    if (is_set != 0)
    {
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 14);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
    }
    else
    {
        g_field_actor_slots[actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE].active = 0;
        g_field_actor_slots[actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE].track_mask = 0;
    }
}

/**
 * @brief Flag 0x4000 handler: play effect resource 25, or stop the object's effect slot.
 * @param actor Actor of the object.
 * @param is_set Nonzero when the flag became set, zero when it was cleared.
 */
void field_handle_object_flag_4000(FieldActor* actor, s32 is_set)
{
    if (is_set != 0)
    {
        field_start_builtin_animation(actor->object_index, actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 25);
        field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
    }
    else
    {
        g_field_actor_slots[actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE].active = 0;
        g_field_actor_slots[actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE].track_mask = 0;
    }
}

/**
 * @brief Free every primitive of the fading primitive pool.
 */
void field_clear_fade_prims(void)
{
    s32 i;

    for (i = FIELD_FADE_PRIM_COUNT - 1; i >= 0; i--)
    {
        g_field_fade_prims[i].r0 = 0;
    }
}

/**
 * @brief Copy a primitive into the first free entry of the fading primitive pool.
 * @param prim POLY_FT4-sized primitive to copy; a zero red component marks a free entry.
 * @param depth Ordering-table index the primitive is drawn at.
 */
void field_add_fade_prim(const void* prim, s16 depth)
{
    s32 i;

    for (i = 0; i < FIELD_FADE_PRIM_COUNT; i++)
    {
        if (g_field_fade_prims[i].r0 == 0)
        {
            bcopy(prim, &g_field_fade_prims[i], sizeof(POLY_FT4));
            g_field_fade_prim_depths[i] = depth;
            return;
        }
    }
}

/**
 * @brief Fade, scroll and draw the active primitives of the fading primitive pool.
 * @param render_half Render half whose ordering table and primitive cursor receive the copies.
 */
void field_draw_fade_prims(FieldRenderHalf* render_half)
{
    POLY_FT4* source;
    POLY_FT4* output;
    u_long* ordering_table;
    s32 i;
    u8 color;

    output = (POLY_FT4*)render_half->primitive_cursor;
    ordering_table = render_half->ordering_table;
    source = g_field_fade_prims;

    for (i = 0; i < FIELD_FADE_PRIM_COUNT;)
    {
        color = source->r0;
        if (color != 0)
        {
            if (color < FIELD_FADE_PRIM_STEP)
            {
                color = 0;
            }
            else
            {
                color -= FIELD_FADE_PRIM_STEP;
            }

            source->b0 = color;
            source->g0 = color;
            source->r0 = color;
            setSemiTrans(source, 1);
            source->x0 += g_field_screen_scroll.x;
            source->x1 += g_field_screen_scroll.x;
            source->x2 += g_field_screen_scroll.x;
            source->x3 += g_field_screen_scroll.x;
            source->y0 += g_field_screen_scroll.y;
            source->y1 += g_field_screen_scroll.y;
            source->y2 += g_field_screen_scroll.y;
            source->y3 += g_field_screen_scroll.y;

            bcopy(source, output, sizeof(POLY_FT4));
            addPrim(&ordering_table[g_field_fade_prim_depths[i]], output);
            output++;
        }

        i++;
        source++;
    }

    render_half->primitive_cursor = (u8*)output;
}

/**
 * @brief Project an actor's ground shadow and append its textured quad.
 * @param actor Actor casting the shadow.
 * @param prim Next free POLY_FT4 in the primitive buffer.
 * @param ordering_table Depth ordering table with 4096 entries.
 * @param footprint First two footprint corners supplying the horizontal bounds.
 * @return Next free primitive, unchanged if the shadow has collapsed.
 */
POLY_FT4* field_render_actor_ground_shadow(FieldActor* actor, POLY_FT4* prim, s32* ordering_table, ShadowFootprint* footprint)
{
    ShadowScreenPosition* screen;
    ShadowWorldPosition* world;
    s32 size;
    s32 height;
    s32 diameter;
    s32 depth;

    screen = SHADOW_SCREEN_POSITION;
    world = SHADOW_WORLD_POSITION;
    world->x = actor->x;
    world->y = 0;
    world->z = actor->z;
    screen->x = g_field_view_offset_x / 256 + (world->x / 256 + FIELD_SCREEN_CENTER_X);
    screen->y = g_field_view_offset_y / 256 + (world->y / 256 + FIELD_SCREEN_CENTER_Y) - world->z / 512 - g_field_view_offset_z / 512;

    size = actor->height;
    height = g_field_object_states[actor->object_index].movement.half.hi;
    if (g_field_resource_entries[actor->object_index].unk8 != 0)
    {
        /* The resource profile widens the height-dependent inset by 5/4. */
        prim->x2 = prim->x0 = screen->x + footprint->left_x - (SHADOW_ELEVATION(actor, height) + (size >> 2)) * 5 / 4;
        prim->x3 = prim->x1 = screen->x + footprint->right_x + (SHADOW_ELEVATION(actor, height) - (size >> 2)) * 5 / 4;
    }
    else
    {
        prim->x2 = prim->x0 = screen->x + footprint->left_x - SHADOW_ELEVATION(actor, height) + (size >> 2);
        prim->x3 = prim->x1 = screen->x + footprint->right_x + SHADOW_ELEVATION(actor, height) - (size >> 2);
    }

    /* Reject inverted horizontal bounds or a vertical diameter below two pixels. */
    diameter = abs((footprint->left_x - footprint->right_x) >> 1) + SHADOW_ELEVATION(actor, height) - (size >> 2);
    if (prim->x0 <= prim->x1 && diameter >= 2)
    {
        if (height != 0)
        {
            prim->y0 = prim->y1 = screen->y - (diameter >> 1) + height;
            prim->y2 = prim->y3 = screen->y + (diameter >> 1) + height;
        }
        else
        {
            prim->y0 = prim->y1 = screen->y - (diameter >> 1);
            prim->y2 = prim->y3 = screen->y + (diameter >> 1);
        }

        /* Neutral modulation with subtractive blending for the shadow texture. */
        SET_BGR0_PACKED(prim, GPU_TINT_NEUTRAL);
        setPolyFT4(prim);
        setSemiTrans(prim, 1);
        prim->u3 = 0x40;
        prim->u1 = 0x40;
        prim->v1 = 0x50;
        prim->v0 = 0x50;
        prim->v3 = 0x70;
        prim->v2 = 0x70;
        setTPage(prim, 0, 2, 960, 256);
        prim->u2 = 0;
        prim->u0 = 0;
        setClut(prim, 256, 481);

        depth = actor->z >> SHADOW_DEPTH_SHIFT;
        if (depth < 0)
        {
            addPrim(&ordering_table[0], prim);
            prim++;
        }
        else if (depth >= SHADOW_OT_SIZE)
        {
            addPrim(&ordering_table[SHADOW_OT_SIZE - 1], prim);
            prim++;
        }
        else
        {
            addPrim(&ordering_table[actor->z >> SHADOW_DEPTH_SHIFT], prim);
            prim++;
        }
    }
    return prim;
}

/**
 * @brief Remember the updating actor and run the update of its spawned actor.
 * @param actor Actor being updated.
 */
void field_update_spawned_actor(FieldActor* actor)
{
    g_field_updating_actor = actor;
    field_update_actor_record(g_field_object_states[actor->object_index].key, g_field_object_states);
}

/**
 * @brief Return the scene record table.
 * @return Base of the scene record table.
 */
u32* field_get_scene_record_table(void)
{
    return g_field_scene_record_table;
}

/**
 * @brief Report whether the actor with @p key is idle (no script wait and no command).
 * @param key Object key to look up.
 * @return 1 when idle, 0 when busy, -1 when no actor has @p key.
 */
s32 field_is_actor_idle(s32 key)
{
    FieldActor* actor = field_lookup_actor(key);
    s32 result;

    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }

    result = 0;
    if (actor->unk10 == 0)
    {
        result = actor->command == 0;
    }

    return result;
}
