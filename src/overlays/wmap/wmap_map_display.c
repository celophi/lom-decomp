/**
 * @file wmap_map_display.c
 * @brief Land sprites, spirit information, and the world-map game.
 */
#include "wmap_map_display.h"
#include "wmap_resource_support.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"
#include "wmap_main.h"
#include "wmap_map_labels.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_sequence_runtime.h"
#include "wmap_land_layout.h"

#define WMAP_GRID_SIZE 6
#define WMAP_CELL_SPACING 48
#define WMAP_CACHE_SLOTS 16
#define WMAP_SPIRIT_COUNT 8
#define WMAP_PACKET_LIMIT 32000
#define WMAP_FADE_STEP 8
#define WMAP_FULL_BRIGHTNESS 128
#define WMAP_BLEND_THRESHOLD 64
#define WMAP_GAME_AREA_SIZE 3
#define WMAP_GAME_RETRY_DELAY 60
#define WMAP_LAND_COUNT 64
#define WMAP_LAND_SCALE_MAX 15
#define WMAP_EMPTY_LAND 0xFF
#define WMAP_DYNAMIC_GLYPH 0xFF
#define WMAP_SPIRIT_CLUT_X 736
#define WMAP_SPIRIT_CLUT_Y 416
#define WMAP_SPIRIT_ACCENT_CLUT_Y 424
#define WMAP_GAME_COUNTDOWN 120
#define WMAP_GAME_ROUND_TIME 900
#define WMAP_INITIAL_SPAWN_DELAY 30

/** @brief Display requested for a land or cell effect. */
typedef enum
{
    WMAP_LAND_HIDDEN = 0,
    WMAP_LAND_MARKER = 1,
    WMAP_LAND_ANIMATED = 2,
    WMAP_LAND_EXPANDED = 5
} WmapLandMode;

/** @brief Progress through a land sprite's scale transition. */
typedef enum
{
    WMAP_TRANSITION_HIDDEN = 0,
    WMAP_TRANSITION_MARKER = 1,
    WMAP_TRANSITION_EXPANDED = 2,
    WMAP_TRANSITION_GROW = 3,
    WMAP_TRANSITION_SHRINK = 4
} WmapLandTransition;

/** @brief Map-game round progression. */
typedef enum
{
    WMAP_GAME_PROMPT = 0,
    WMAP_GAME_COUNTING_DOWN = 1,
    WMAP_GAME_PLAYING = 2
} WmapGamePhase;

/** @brief Map cell's land identifier and effect availability. */
typedef struct
{
    s32 object_id;
    s16 effect_enabled;
    u8 pad06[34];
} WmapDisplayCell;

/** @brief Active drawing environment, depth buckets, and GPU packet cursor. */
typedef struct
{
    u8 pad00[112];
    u_long ordering_table[179];
    void* packet_cursor;
} WmapDisplayContext;

/** @brief Map scroll position and perspective scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapDisplayProjection;

/** @brief VRAM upload area, sprite palettes, and texture-page origin. */
typedef struct
{
    union
    {
        RECT rect;
        struct
        {
            s16 x;
            u8 v_offset;
            u8 y_high;
            s16 width;
            s16 height;
        } uv;
    } upload;
    u16 clut[8];
    u16 tpage_x;
    u16 tpage_y;
} WmapTextureInfo;

/** @brief Per-cell display mode and effect animation counters. */
typedef struct
{
    s16 unk00;
    s16 mode;
    s16 state;
    s16 frame;
    s32 unk08;
    s32 unk0c;
    s16 unk10;
    u8 pad12[2];
    s32 unk14;
    s32 unk18;
} WmapEffectCell;

/** @brief Map marker screen coordinates. */
typedef struct
{
    u16 x;
    u16 y;
} WmapMapPoint;

/** @brief Sprite sequence command and frame duration. */
typedef struct
{
    u8 sequence_id;
    u8 duration;
    u8 _pad02[2];
} WmapAnimationEntry;

/** @brief Packed textured quad within a sprite frame. */
typedef struct
{
    s8 x;
    s8 y;
    u8 u;
    u8 v;
    u8 width;
    u8 height;
    s8 texture_index;
    u8 _pad07[2];
    u8 blend_mode;
    u8 _pad0a[2];
} WmapQuadData;

/** @brief Projected sprite position and depth. */
typedef struct
{
    SVECTOR position;
    s32 blend_mode;
    DVECTOR screen;
    s32 depth;
} WmapSpriteLocals;

/** @brief Unsigned vertex coordinates for a cell effect. */
typedef struct
{
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
} WmapQuadVertex;

/** @brief Four vertices and colors of a cell effect quad. */
typedef struct
{
    WmapQuadVertex vertices[4];
    CVECTOR colors[4];
} WmapQuadTemplate;

/** @brief Texture rectangle and palette for an information glyph. */
typedef struct
{
    u8 u, v, width, height;
    u16 palette;
    u8 pad_06[2];
} WmapGlyph;

/** @brief Screen position and glyph index for an information label. */
typedef struct
{
    u16 x;
    u8 y, glyph;
} WmapGlyphPlacement;

/** @brief Position word and remaining bytes in a spirit sprite packet. */
typedef struct
{
    u32 packed_xy;
    u8 packet_tail[16];
} WmapSpritePosition;

extern WmapSpritePosition D_8004FD9C[8];
extern s32 rand(void);

extern SPRT g_wmap_game_score_label;
extern SPRT g_wmap_game_score_digit;
extern SPRT g_wmap_game_round_label;
extern SPRT g_wmap_game_round_digit;
extern s32 g_wmap_game_displayed_score;
extern s32 g_wmap_game_origin_x;
extern s32 g_wmap_game_origin_y;
extern s32 g_wmap_game_hits;
extern s32 D_800D921C;
extern s32 D_800D9220;
extern s32 D_800DBE78;
extern s32 D_800DCEC0;
extern s32 D_8011CF18;
extern s32 D_8011CF7C;
extern s32 D_8011D4FC;
extern s32 D_8013922C;
extern s32 D_80139230;
extern WmapDisplayCell D_80139290[6][6];
extern s32 D_8013986C;
extern s32 g_wmap_game_phase;
extern s32 D_801398C0;
extern WmapDisplayContext* D_801398EC;
extern WmapDisplayProjection D_80139950;
extern s32 D_8013B258;

extern s32 g_wmap_game_round;
extern s32 g_wmap_game_timer;
extern s32 g_wmap_game_score;

extern SPRT g_wmap_game_continue_prompt;
extern SPRT g_wmap_game_exit_prompt;
extern SPRT g_wmap_game_countdown_sprite;
extern s32 g_wmap_game_spawn_timer;

extern s32 D_800D922C;
extern s32 D_800D9234;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern u8 g_wmap_game_spawn_patterns[][9];
extern s32 D_800500DC[];
extern WmapTextureInfo D_800CBBE8[];
extern s32 D_800DBE74;
extern s32 D_8011CF74;

extern s32 D_8013C628[];
extern s32 D_8019D6D8;
extern s32 D_801ADAF8;
extern WmapMapPoint D_8004FD04[];
extern s16 D_800D036C[];
extern s32 g_wmap_land_scale_steps[];

extern s32 D_800DBE70;
extern SVECTOR D_80139278;
extern SVECTOR D_801398C8;
extern s32 D_80139958;
extern s32 D_80139978;
extern VECTOR D_80182D48;
extern VECTOR D_80182DC0;
extern s32 D_801ADAE0;
extern s32 g_wmap_cell_effect_steps[];
extern WmapQuadTemplate g_wmap_cell_effect_quads[16];
extern WmapEffectCell D_8011D108[6][6];
extern SPRT D_8004FD94[];
extern u8 g_wmap_spirit_sequence_bounds[];
extern u8 g_wmap_spirit_sequences[];
extern u8 g_wmap_spirit_timers[];
extern u8 g_wmap_spirit_frames[];
extern SPRT g_wmap_spirit_level_sprites[16];
extern SPRT g_wmap_spirit_accent_sprites[16];
extern s32 D_80129550;
extern s32 D_8004FC74[];
extern WmapDisplayProjection D_800DCEC8;
extern s32 D_800DCF04;
extern s32 D_80139218;
extern WmapGlyph g_wmap_information_glyphs[];
extern WmapGlyphPlacement g_wmap_information_placements[];
extern s32 g_wmap_information_group_starts[];
extern s32 g_wmap_information_values[];

extern s32 D_801398D0;
extern s32 D_800D7D60;
extern s32 D_800D7D64;
extern s32 D_800D7D68;

void wmap_update_map_game_prompt(void);
void wmap_update_map_game_countdown(void);
void wmap_update_map_game_round(void);
void wmap_spawn_map_game_lands(s32 delay_min, s32 delay_range, s32 timer_min, s32 timer_range);
void wmap_update_map_game_spawns(void);
void wmap_draw_land_marker(s32 map_x, s32 map_y, WmapLandDisplay* marker);
void wmap_draw_land_animation(s32 x, s32 y, WmapLandDisplay* state, s32 resource_index);
void wmap_draw_cell_effect(s32 x, s32 y);
void wmap_draw_spirit_icons(s32 selected_index);
void wmap_draw_spirit_levels(void);
void wmap_draw_spirit_grid(s32 spirit_index);
void wmap_draw_information_labels(void);
void wmap_draw_land(s32 map_x, s32 map_y, s32 resource_index);
void wmap_set_cell_effect_mode(s32 x, s32 y, s32 mode);
s32 wmap_classify_map_point(s32 x, s32 y);

/**
 * @brief Run the map game and draw its nine lands, score, and round counter.
 */
void wmap_update_map_game(void)
{
    s32 timer;
    s32 valid;
    s32 scan_x;
    s32 scan_y;
    s32 render_x;
    s32 render_y;
    SPRT* sprite;

    if (g_wmap_game_start_delay != 0)
    {
        if ((D_801398C0 & PADL1) != 0)
        {
            timer = g_wmap_game_start_delay - 1;
            g_wmap_game_start_delay = timer;
            if (timer == 0)
            {
                valid = 1;
                g_wmap_game_origin_y = D_80139950.y / WMAP_CELL_SPACING;
                g_wmap_game_origin_x = D_80139950.x / WMAP_CELL_SPACING;

                for (scan_y = g_wmap_game_origin_y; scan_y < g_wmap_game_origin_y + WMAP_GAME_AREA_SIZE; scan_y++)
                {
                    for (scan_x = g_wmap_game_origin_x; scan_x < g_wmap_game_origin_x + WMAP_GAME_AREA_SIZE; scan_x++)
                    {
                        if (D_80139290[scan_x][scan_y].object_id == WMAP_EMPTY_LAND)
                        {
                            valid = 0;
                        }
                    }
                }

                if ((D_800DBE78 != 0) || (D_8013986C != 0) || (D_8011CF18 != 0) || (D_80139230 != 0) || (D_8011D4FC != -1))
                {
                    valid = 0;
                }

                if (valid == 0)
                {
                    g_wmap_game_start_delay = WMAP_GAME_RETRY_DELAY;
                    return;
                }

                D_800DCEC0 = 0;
                func_80064F64(0x1154);
                g_wmap_game_score = 0;
                g_wmap_game_displayed_score = 0;
                D_8011CF7C = 0;
                g_wmap_game_phase = WMAP_GAME_PROMPT;
                g_wmap_game_timer = 0x708;
                D_800DBE78 = 1;
                g_wmap_spirit_target_brightness = 0;
                g_wmap_spirit_brightness = 0;
                D_8013B258 = 1;
                g_wmap_game_round = 0;
                g_wmap_game_hits = 15;
                D_800D9220 = 0xF0E0;
            }
        }
        else
        {
            g_wmap_game_start_delay = WMAP_GAME_RETRY_DELAY;
        }
        return;
    }

    switch (g_wmap_game_phase)
    {
    case 0:
        wmap_update_map_game_prompt();
        break;
    case 1:
        wmap_update_map_game_countdown();
        break;
    case 2:
        wmap_update_map_game_round();
        break;
    }

    for (render_y = g_wmap_game_origin_y; render_y < g_wmap_game_origin_y + WMAP_GAME_AREA_SIZE; render_y++)
    {
        for (render_x = g_wmap_game_origin_x; render_x < g_wmap_game_origin_x + WMAP_GAME_AREA_SIZE; render_x++)
        {
            wmap_draw_land(render_x, render_y, D_80139290[render_x][render_y].object_id);
        }
    }

    render_y = g_wmap_game_displayed_score;
    render_x = 0xF0;
    if (g_wmap_game_score == 0)
    {
        sprite = D_801398EC->packet_cursor;
        *sprite = g_wmap_game_score_digit;
        addPrim(&D_801398EC->ordering_table[1], sprite);
        if (D_800D921C < WMAP_PACKET_LIMIT)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
        }
    }
    else if (render_y != 0)
    {
        do
        {
            sprite = D_801398EC->packet_cursor;
            *sprite = g_wmap_game_score_digit;
            sprite->x0 = render_x;
            sprite->u0 = (render_y % 10) * 16;
            render_y /= 10;
            addPrim(&D_801398EC->ordering_table[1], sprite);
            if (D_800D921C < WMAP_PACKET_LIMIT)
            {
                D_800D921C += sizeof(SPRT);
                D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
            }
            render_x -= 16;
        } while (render_y != 0);
    }

    sprite = D_801398EC->packet_cursor;
    *sprite = g_wmap_game_score_label;
    addPrim(&D_801398EC->ordering_table[1], sprite);
    if (D_800D921C < WMAP_PACKET_LIMIT)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
    }

    sprite = D_801398EC->packet_cursor;
    *sprite = g_wmap_game_round_digit;
    sprite->u0 = g_wmap_game_round * 16;
    addPrim(&D_801398EC->ordering_table[1], sprite);
    if (D_800D921C < WMAP_PACKET_LIMIT)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
    }

    sprite = D_801398EC->packet_cursor;
    *sprite = g_wmap_game_round_label;
    addPrim(&D_801398EC->ordering_table[1], sprite);
    if (D_800D921C < WMAP_PACKET_LIMIT)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
    }

    func_8006534C(0x55, 1);

    if (((g_wmap_game_displayed_score - g_wmap_game_score) >= 0) ? ((g_wmap_game_displayed_score - g_wmap_game_score) < 30)
                                                                 : ((g_wmap_game_score - g_wmap_game_displayed_score) < 30))
    {
        if (g_wmap_game_displayed_score < g_wmap_game_score)
        {
            g_wmap_game_displayed_score++;
        }
        if (g_wmap_game_score < g_wmap_game_displayed_score)
        {
            g_wmap_game_displayed_score--;
        }
    }
    else
    {
        if (g_wmap_game_displayed_score < g_wmap_game_score)
        {
            g_wmap_game_displayed_score += 15;
        }
        if (g_wmap_game_score < g_wmap_game_displayed_score)
        {
            g_wmap_game_displayed_score -= 15;
        }
    }

    D_801398C0 = 0;
    D_8013922C = 0;
}

/**
 * @brief Draw the map-game prompt and accept a new round or an exit.
 */
void wmap_update_map_game_prompt(void)
{
    SPRT* sprite;

    sprite = D_801398EC->packet_cursor;

    if (g_wmap_game_round < 8 && g_wmap_game_hits >= 12)
    {
        s32 state_flags;

        state_flags = D_8013922C;
        *sprite = g_wmap_game_continue_prompt;

        if (state_flags & PADRleft)
        {
            g_wmap_game_phase = WMAP_GAME_COUNTING_DOWN;
            g_wmap_game_timer = WMAP_GAME_COUNTDOWN;
            func_800652A8(0x3C, 0x80);
            g_wmap_game_hits = 0;
            g_wmap_game_round++;
        }
        else if (state_flags & PADRright)
        {
            g_wmap_game_start_delay = WMAP_GAME_RETRY_DELAY;
            g_wmap_game_phase = WMAP_GAME_PROMPT;
            D_8011CF7C = 1;
            D_800DCEC0 = 1;
            D_800DBE78 = 2;
            func_80064094();
        }
    }
    else
    {
        s32 state_flags;

        state_flags = D_8013922C;
        *sprite = g_wmap_game_exit_prompt;

        if (state_flags & PADRright)
        {
            g_wmap_game_start_delay = WMAP_GAME_RETRY_DELAY;
            g_wmap_game_phase = WMAP_GAME_PROMPT;
            D_8011CF7C = 1;
            D_800DCEC0 = 1;
            D_800DBE78 = 2;
            func_80064094();
        }
    }

    addPrim(&D_801398EC->ordering_table[1], sprite);

    if (D_800D921C < WMAP_PACKET_LIMIT)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
    }

    func_8006534C(0x55, 1);
}

/**
 * @brief Count down to the next map-game round and reset its land timers.
 */
void wmap_update_map_game_countdown(void)
{
    SPRT* sprite;
    s32 i;
    s32 j;
    s32 object_id;

    sprite = (SPRT*)D_801398EC->packet_cursor;
    *sprite = g_wmap_game_countdown_sprite;
    addPrim(&D_801398EC->ordering_table[1], sprite);

    if (D_800D921C < WMAP_PACKET_LIMIT)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
    }

    g_wmap_game_timer--;
    if (g_wmap_game_timer == 60)
    {
        for (i = g_wmap_game_origin_y; i < g_wmap_game_origin_y + WMAP_GAME_AREA_SIZE; i++)
        {
            for (j = g_wmap_game_origin_x; j < g_wmap_game_origin_x + WMAP_GAME_AREA_SIZE; j++)
            {
                object_id = D_80139290[j][i].object_id;
                wmap_set_land_display_mode(object_id, 1);
                g_wmap_land_display[object_id].game_timer = 0;
            }
        }
    }

    if (g_wmap_game_timer == 0)
    {
        g_wmap_game_timer = WMAP_GAME_ROUND_TIME;
        g_wmap_game_phase = WMAP_GAME_PLAYING;
        g_wmap_game_spawn_timer = WMAP_INITIAL_SPAWN_DELAY;
    }

    func_8006534C(0x55, 1);
    func_8005FF88(-1);
}

/**
 * @brief Score map-game hits and expire raised lands in the active neighborhood.
 */
void wmap_update_map_game_round(void)
{
    s32 x;
    s32 y;
    s32 object_id;
    s32 step;
    WmapLandDisplay* object;

    g_wmap_game_timer--;
    if (g_wmap_game_timer == 0)
    {
        g_wmap_game_phase = WMAP_GAME_PROMPT;
    }

    D_800D9234 = D_801398C0;
    wmap_update_map_game_spawns();

    y = g_wmap_game_origin_y + D_800DCEF0;
    x = g_wmap_game_origin_x + D_800DCEEC;
    object_id = D_80139290[x][y].object_id;
    object = &g_wmap_land_display[object_id];

    if (D_8013922C & PADRdown)
    {
        if ((u32)(object->transition - 2) < 2)
        {
            step = (g_wmap_game_round + 1) * 5;
            g_wmap_game_score += step;
            g_wmap_game_hits++;
            if (object_id == 0x1F)
            {
                g_wmap_game_score += step;
            }
            D_800D922C = g_wmap_game_score;
            wmap_set_land_display_mode(object_id, 1);
            object->scale_frame = 1;
            func_800652A8(0x3E, 0x80);
        }
        else
        {
            g_wmap_game_score -= (g_wmap_game_round + 1) * 2;
            g_wmap_game_hits--;
            if (g_wmap_game_score < 0)
            {
                g_wmap_game_score = 0;
            }
            func_800652A8(0x3F, 0x80);
        }
    }

    for (y = g_wmap_game_origin_y; y < g_wmap_game_origin_y + WMAP_GAME_AREA_SIZE; y++)
    {
        for (x = g_wmap_game_origin_x; x < g_wmap_game_origin_x + WMAP_GAME_AREA_SIZE; x++)
        {
            object_id = D_80139290[x][y].object_id;
            object = &g_wmap_land_display[object_id];
            if (object->game_timer != 0)
            {
                object->game_timer--;
                if ((object->game_timer == 0) && (object->transition != 1))
                {
                    wmap_set_land_display_mode(D_80139290[x][y].object_id, 1);
                    object->scale_frame = 4;
                    func_800652A8(2, 0x80);
                }
            }
        }
    }
}

/**
 * @brief Schedule randomized timer updates for nearby world-map objects.
 * @param delay_min Base value for the next update delay.
 * @param delay_range Randomized span added to the next update delay.
 * @param timer_min Base value assigned to selected object timers.
 * @param timer_range Randomized span added to selected object timers.
 */
void wmap_spawn_map_game_lands(s32 delay_min, s32 delay_range, s32 timer_min, s32 timer_range)
{
    s32 row;
    s32 column;
    s32 pattern_index;
    s32 object_id;
    WmapLandDisplay* object;

    if (g_wmap_game_spawn_timer-- > 0)
    {
        return;
    }

    g_wmap_game_spawn_timer = ((rand() * delay_range) >> 15) + delay_min;
    column = (rand() % 3) + g_wmap_game_origin_x;
    row = (rand() % 3) + g_wmap_game_origin_y;
    object_id = D_80139290[column][row].object_id;

    if (rand() & 0xFF)
    {
        WmapLandDisplay* object_base;

        object_base = g_wmap_land_display;
        object = &object_base[object_id];
        if (object->transition != 1)
        {
            return;
        }

        object->game_timer = ((rand() * timer_range) >> 15) + timer_min;
        wmap_set_land_display_mode(object_id, 2);
    }
    else
    {
        pattern_index = 0;
        for (row = g_wmap_game_origin_y; row < g_wmap_game_origin_y + WMAP_GAME_AREA_SIZE; row++)
        {
            for (column = g_wmap_game_origin_x; column < g_wmap_game_origin_x + WMAP_GAME_AREA_SIZE; column++)
            {
                object_id = D_80139290[column][row].object_id;
                if (g_wmap_game_spawn_patterns[g_wmap_game_round][pattern_index])
                {
                    WmapLandDisplay* object_base;

                    object_base = g_wmap_land_display;
                    object = &object_base[object_id];
                    if (object->transition == 1)
                    {
                        object->game_timer = ((rand() * timer_range) >> 15) + timer_min;
                        wmap_set_land_display_mode(object_id, 2);
                    }
                }
                pattern_index++;
            }
        }
    }

    func_800652A8(0x3D, 0x80);
}

/**
 * @brief Choose land spawn timing for the current map-game round.
 */
void wmap_update_map_game_spawns(void)
{
    switch (g_wmap_game_round)
    {
    case 1:
        wmap_spawn_map_game_lands(40, 20, 25, 30);
        break;
    case 2:
        wmap_spawn_map_game_lands(30, 20, 20, 25);
        break;
    case 3:
        wmap_spawn_map_game_lands(10, 20, 20, 20);
        break;
    case 4:
        wmap_spawn_map_game_lands(5, 25, 15, 20);
        break;
    default:
        wmap_spawn_map_game_lands(5, 20, 8, 15);
        break;
    }
}

/**
 * @brief Resolve or load the cache slot used by a world-map resource.
 * @param resource Land whose image is needed; resets animation history when no sequence is active.
 * @return Cache slot index, or -1 when the resource cannot be loaded.
 */
s32 wmap_resolve_land_image(WmapLandDisplay* resource)
{
    s32 i;
    s32 slot;
    s32 oldest_slot;
    u32 oldest_frame;
    WmapCacheEntry* cache;
    WmapCacheEntry* search_cache;
    static void* const keep[] __attribute__((section(".discard"))) = {&&count_fail};

    {
        s32 resource_id;

        resource_id = resource->resource_id;
        if (resource_id == 0x1F)
        {
            return 0x10;
        }
        if (D_800500DC[resource_id] == -1)
        {
            return -1;
        }

        i = 0;
        search_cache = g_wmap_land_image_cache;
        while (1)
        {
            slot = i;
            if (search_cache->resource_id == resource_id)
            {
                break;
            }
            i++;
            if (i >= WMAP_CACHE_SLOTS)
            {
                slot = -1;
                break;
            }
            search_cache++;
        }
    }

    if (slot < 0)
    {
        D_801ADAF8 = 1;
        if (D_8019D6D8 != 0)
        {
            goto fail;
        }

        i = 0;
        if (D_800DBE74 >= 10)
        {
        count_fail:
            return -1;
        }

        {
            s32 empty_id;

            empty_id = -1;
            search_cache = g_wmap_land_image_cache;
            while (1)
            {
                slot = i;
                if (search_cache->resource_id == empty_id)
                {
                    break;
                }
                i++;
                if (i >= WMAP_CACHE_SLOTS)
                {
                    slot = -1;
                    break;
                }
                search_cache++;
            }
        }

        if (slot < 0)
        {
            goto replace_oldest;
        }

        {
            WmapCacheEntry* free_entry;
            WmapCacheEntry* cache_base;
            u8* image;
            s16 resource_id;

            cache_base = g_wmap_land_image_cache;
            free_entry = &cache_base[slot];
            resource_id = resource->resource_id;
            free_entry->resource_id = resource_id;
            image = (u8*)D_8013C628 + D_8013C628[resource_id];
            LoadImage(&D_800CBBE8[slot].upload.rect, (u_long*)image);
            free_entry->animation_data = image + 0x2004;
            free_entry->loaded_frame = D_8011CF74;
        }
    }

    if (resource->animation_cursor == 0)
    {
        resource->previous_animation_index = -1;
    }
    return slot;

replace_oldest:
    oldest_slot = -1;
    oldest_frame = 0x7FFFFFFF;
    slot = 0;
    do
    {
        cache = &g_wmap_land_image_cache[slot];
        if (g_wmap_land_display[cache->resource_id].transition == 0)
        {
            if (cache->loaded_frame < oldest_frame)
            {
                oldest_frame = cache->loaded_frame;
                oldest_slot = slot;
            }
        }
        slot++;
    } while (slot < WMAP_CACHE_SLOTS);

    {
        WmapCacheEntry* oldest_entry;
        WmapCacheEntry* cache_base;
        u8* image;
        s16 resource_id;

        cache_base = g_wmap_land_image_cache;
        /* The cache must contain an inactive land before replacement is requested. */
        oldest_entry = &cache_base[oldest_slot];
        if (oldest_entry->busy != 0)
        {
            return -1;
        }

        i = -1;
        oldest_entry->resource_id = i;
        resource_id = resource->resource_id;
        oldest_entry->resource_id = resource_id;
        image = (u8*)D_8013C628 + D_8013C628[resource_id];
        LoadImage(&D_800CBBE8[oldest_slot].upload.rect, (u_long*)image);
        image += 0x2004;
        oldest_entry->animation_data = image;
        oldest_entry->loaded_frame = D_8011CF74;
        resource->animation_cursor = 0;
        resource->previous_animation_index = i;
        return oldest_slot;
    }

fail:
    return -1;
}

/**
 * @brief Build and enqueue a world-map marker sprite.
 * @param map_x Map-grid X coordinate.
 * @param map_y Map-grid Y coordinate.
 * @param marker Marker record containing the texture selection.
 */
void wmap_draw_land_marker(s32 map_x, s32 map_y, WmapLandDisplay* marker)
{
    SPRT* sprite;
    s32 depth;
    s32 type;
    SVECTOR position;
    u32 screen;
    u16* screen_ptr;
    s32 projected_z;

    sprite = D_801398EC->packet_cursor;

    switch (D_8013986C)
    {
    case 0:
    {
        s32 scale;
        s32 z_bucket;
        s32 sprite_y;

        scale = D_80139950.scale;
        position.vx = ((((map_x - 1) * 160) - ((D_80139950.x * 0x14000) / scale)) * 0x6000) / scale;
        position.vy = ((((map_y - 1) * 160) - ((D_80139950.y * 0x14000) / scale)) * 0x6000) / scale;
        position.vz = 0;

        gte_ldv0(&position);
        gte_rtps();
        gte_stsxy(&screen);
        gte_stszotz(&projected_z);
        screen_ptr = (u16*)&screen;

        if ((s32)(screen_ptr[1] << 16) < 0)
        {
            return;
        }

        z_bucket = (0x1B91 - projected_z) / 4;
        depth = z_bucket + 0x2F;
        if (depth < 0x2E || depth > 0xAE)
        {
            depth = 0x2E;
        }

        sprite->x0 = screen_ptr[0] - 8;
        sprite_y = screen_ptr[1] - 22;
        sprite->y0 = sprite_y;
        break;
    }
    case 1:
    {
        s32 sprite_x;
        s32 sprite_y;

        sprite_x = D_8004FD04[map_x + map_y * 6].x;
        sprite_y = D_8004FD04[map_x + map_y * 6].y;
        depth = 0xAE - map_y;
        sprite->x0 = sprite_x;
        sprite->y0 = sprite_y;
        break;
    }
    default:
        return;
    }

    *(u32*)&sprite->r0 = 0x808080;

    {
        s32 texture;
        s32 u_index;
        s32 v_index;

        texture = D_800D036C[marker->resource_id];
        u_index = texture & 7;
        sprite->u0 = u_index << 5;
        v_index = texture / 8;
        sprite->v0 = v_index << 5;

        switch (v_index)
        {
        case 0:
            sprite->clut = (u_index << 6) | 0x582E;
            type = 12;
            break;
        case 1:
            sprite->clut = (u_index << 6) | 0x5A2E;
            type = 12;
            break;
        case 2:
            sprite->clut = (u_index << 6) | 0x5C2E;
            type = 12;
            break;
        case 3:
            sprite->clut = (u_index << 6) | 0x5E2E;
            type = 12;
            break;
        }
    }

    *(u32*)&sprite->w = PACK_U16_PAIR(32, 32);
    setSprt(sprite);
    setSemiTrans(sprite, 1);

    addPrim(&D_801398EC->ordering_table[depth], sprite);

    if (D_800D921C < WMAP_PACKET_LIMIT)
    {
        D_800D921C += sizeof(SPRT);
        D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
    }

    func_8006534C(type, depth);
}

/**
 * @brief Update and render an animated world-map sprite.
 * @param x World-map X cell coordinate.
 * @param y World-map Y cell coordinate.
 * @param state Animation and rendering state for the sprite.
 * @param resource_index Resident image-cache slot and texture set.
 */
void wmap_draw_land_animation(s32 x, s32 y, WmapLandDisplay* state, s32 resource_index)
{
    WmapCacheEntry* resource;
    WmapQuadScale* scale;
    WmapTextureInfo* texture;
    WmapTextureInfo* texture_base;
    WmapAnimationEntry* animation;
    WmapQuadData* quad;
    POLY_FT4* packet;
    WmapSpriteLocals locals;
    DVECTOR* screen_ptr;
    s32 depth_index;
    s32 ot_depth;
    s32 count;
    s32 sequence_id;
    s32 sequence_end;
    s32 phase_delta;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
    u8* animation_data;
    u32 address_mask;
    u32 tag_mask;

    phase_delta = g_wmap_land_scale_steps[state->transition];
    state->scale_frame += phase_delta;
    resource = &g_wmap_land_image_cache[resource_index];

    if ((s8)state->scale_frame >= 15)
    {
        state->scale_frame = 15;
        state->transition = 2;
    }

    if ((s8)state->scale_frame <= 0)
    {
        state->transition = 1;
        return;
    }

    projection_scale = D_80139950.scale;
    locals.position.vx = (((x - 1) * 0xA0 - (D_80139950.x * 0x14000) / projection_scale) * 0x6000) / projection_scale;
    locals.position.vy = (((y - 1) * 0xA0 - (D_80139950.y * 0x14000) / projection_scale) * 0x6000) / projection_scale;
    locals.position.vz = 0;

    gte_ldv0(&locals.position);
    gte_rtps();
    {
        DVECTOR* projected_screen;

        projected_screen = &locals.screen;
        gte_stsxy(projected_screen);
        gte_stszotz(&locals.depth);

        if (((u16)projected_screen->vy << 16) < 0)
        {
            return;
        }
    }

    /* The image tail holds relative sequence and quad offsets. */
    animation_data = resource->animation_data;
    if (state->previous_animation_index != state->animation_index)
    {
        state->previous_animation_index = state->animation_index;
        animation_offset = *(s16*)(animation_data + state->animation_index * 2);
        state->frame_timer = 1;
        state->animation_start = animation_data + animation_offset;
        state->animation_cursor = state->animation_start;
    }

    sequence_end = 0xFF;
    if (state->frame_timer != sequence_end)
    {
        state->frame_timer--;
    }

    if (state->frame_timer == 0)
    {
        animation = (WmapAnimationEntry*)state->animation_cursor;
        sequence_id = animation->sequence_id;
        state->frame_timer = animation->duration;
        if (sequence_id == sequence_end)
        {
            animation = (WmapAnimationEntry*)state->animation_start;
            state->animation_cursor = (u8*)animation;
            sequence_id = animation->sequence_id;
            state->frame_timer = animation->duration;
        }
        state->animation_cursor += sizeof(WmapAnimationEntry);
        state->quad_data = (s8*)(animation_data + *(s16*)(animation_data + 0x40 + sequence_id * 2));
    }

    /* Each frame begins with a byte count followed by packed quad records. */
    quad = (WmapQuadData*)state->quad_data;
    count = *(s8*)quad;
    quad = (WmapQuadData*)((s8*)quad + 1);
    if ((u32)(count - 1) >= 32)
    {
        func_80064F14(animation_data, sequence_end, projection_scale, resource);
        return;
    }

    depth_index = (0x1B91 - locals.depth) / 4;
    ot_depth = depth_index + 0x2E;
    if ((u32)depth_index >= 0x81)
    {
        ot_depth = 0x2E;
    }

    do
    {
        packet = (POLY_FT4*)D_801398EC->packet_cursor;
        screen_ptr = &locals.screen;
        texture_base = D_800CBBE8;
        address_mask = 0x00FFFFFF;
        tag_mask = 0xFF000000;
        scale = &g_wmap_land_quad_scales[(s8)state->scale_frame];

        packet->x0 = locals.screen.vx + ((quad->x * scale->x0) >> 8);
        packet->x1 = locals.screen.vx + (((quad->x + (s8)quad->width) * scale->x1) >> 8);
        packet->x2 = locals.screen.vx + ((quad->x * scale->x2) >> 8);
        packet->x3 = locals.screen.vx + (((quad->x + (s8)quad->width) * scale->x3) >> 8);
        packet->y0 = screen_ptr->vy + ((quad->y * scale->y0) >> 8);
        packet->y1 = screen_ptr->vy + ((quad->y * scale->y1) >> 8);
        packet->y2 = screen_ptr->vy + (((quad->y + (s8)quad->height) * scale->y2) >> 8);
        packet->y3 = screen_ptr->vy + (((quad->y + (s8)quad->height) * scale->y3) >> 8);

        texture_offset = resource_index * sizeof(WmapTextureInfo);
        texture = &texture_base[resource_index];
        packet->u0 = quad->u;
        packet->u1 = quad->u + quad->width;
        packet->u2 = quad->u;
        packet->u3 = quad->u + quad->width;
        packet->v0 = quad->v + texture->upload.uv.v_offset;
        packet->v1 = quad->v + texture->upload.uv.v_offset;
        packet->v2 = quad->v + quad->height + texture->upload.uv.v_offset;
        packet->v3 = quad->v + quad->height + texture->upload.uv.v_offset;
        *(u32*)&packet->r0 = 0x80808080;
        locals.blend_mode = (s8)quad->blend_mode;
        packet->tpage = getTPage(0, quad->blend_mode & 3, texture->tpage_x, texture->tpage_y);
        packet->clut = *(u16*)((u8*)texture_base->clut + (quad->texture_index * 2 + texture_offset));
        setPolyFT4(packet);
        setSemiTrans(packet, 1);
        packet->tag = (packet->tag & tag_mask) | (ot_depth[D_801398EC->ordering_table] & address_mask);
        ot_depth[D_801398EC->ordering_table] = (ot_depth[D_801398EC->ordering_table] & tag_mask) | ((u32)packet & address_mask);

        if (D_800D921C < WMAP_PACKET_LIMIT)
        {
            D_800D921C += sizeof(POLY_FT4);
            D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(POLY_FT4);
        }

        quad++;
        count--;
    } while (count != 0);
}

/**
 * @brief Update world-map object display states for the visible map grid.
 */
void wmap_draw_lands(void)
{
    MATRIX matrix;
    VECTOR translation;
    SVECTOR rotation;
    VECTOR world_position;
    s32 x;
    s32 y;
    s32 object_id;
    s32 state;
    s32 part_index;
    s32 distance_state;

    rotation.vx = D_80139278.vx + D_801398C8.vx;
    rotation.vy = D_80139278.vy + D_801398C8.vy;
    rotation.vz = D_80139278.vz + D_801398C8.vz;

    world_position.vx = D_80182DC0.vx + D_80182D48.vx;
    world_position.vy = D_80182DC0.vy + D_80182D48.vy;
    world_position.vz = D_80182DC0.vz + D_80182D48.vz;

    translation = world_position;
    translation.vz = (translation.vz * D_80139958) / 0x6000;

    RotMatrix(&rotation, &matrix);
    TransMatrix(&matrix, &translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);

    for (y = 0; y < WMAP_GRID_SIZE; y++)
    {
        for (x = 0; x < WMAP_GRID_SIZE; x++)
        {
            object_id = D_80139290[x][y].object_id;

            if (D_8013986C == 1)
            {
                state = 1;
            }
            else if (object_id == D_80139978)
            {
                state = 2;
            }
            else
            {
                s32 base_state;

                distance_state = wmap_classify_map_point(x * WMAP_CELL_SPACING, y * WMAP_CELL_SPACING);
                base_state = D_800DBE70;
                if (base_state != 2)
                {
                    state = base_state;
                }
                if ((base_state == 2) || (distance_state < state))
                {
                    state = distance_state;
                }
            }

            if (object_id >= 0x100)
            {
                state = 5;
                object_id &= 0xFF;
            }

            if (object_id != WMAP_EMPTY_LAND)
            {
                WmapLandDisplay* current_object;

                current_object = &g_wmap_land_display[object_id];
                if ((s8)current_object->scale_frame >= 0x10)
                {
                    current_object->scale_frame = 0xF;
                }
                else if ((s8)current_object->scale_frame < 0)
                {
                    current_object->scale_frame = 0;
                }

                if (state == 5)
                {
                    current_object->display_mode = 2;
                    current_object->transition = 2;
                    current_object->scale_frame = 0xF;
                }
                else if (state != current_object->display_mode)
                {
                    switch (current_object->display_mode)
                    {
                    case 0:
                        current_object->display_mode = state;
                        if (state == 1)
                        {
                            current_object->transition = state;
                        }
                        else
                        {
                            current_object->transition = 3;
                        }
                        break;
                    case 1:
                        current_object->display_mode = state;
                        if (state != 0)
                        {
                            current_object->transition = 3;
                        }
                        else
                        {
                            current_object->transition = 0;
                            current_object->scale_frame = 0;
                        }
                        break;
                    case 2:
                        current_object->display_mode = state;
                        if (state != 0)
                        {
                            current_object->transition = 4;
                        }
                        else
                        {
                            current_object->transition = 0;
                            current_object->scale_frame = 0;
                        }
                        break;
                    }
                }

                if (state != 0)
                {
                    WmapLandDisplay* active_object;

                    active_object = &g_wmap_land_display[object_id];
                    if (active_object->transition != 1)
                    {
                        part_index = wmap_resolve_land_image(active_object);
                        if ((part_index != -1) && (g_wmap_land_image_cache[part_index].busy == 0))
                        {
                            wmap_draw_land_animation(x, y, active_object, part_index);
                        }
                        else
                        {
                            wmap_draw_land_marker(x, y, active_object);
                        }
                    }
                    else
                    {
                        wmap_draw_land_marker(x, y, active_object);
                    }
                }
            }

            if ((D_8011D4FC == -1) || (D_801ADAE0 != 0))
            {
                state = 0;
            }
            wmap_set_cell_effect_mode(x, y, state);

            if ((D_80139290[x][y].effect_enabled != 0) && (D_8013986C == 0))
            {
                wmap_draw_cell_effect(x, y);
            }
        }
    }
}

/**
 * @brief Render the animated world-map quad set for one map cell.
 * @param x World-map grid X coordinate.
 * @param y World-map grid Y coordinate.
 */
void wmap_draw_cell_effect(s32 x, s32 y)
{
    WmapEffectCell* cell;
    WmapQuadVertex base;
    WmapQuadVertex transformed[4];
    s32 sxy0;
    s32 sxy1;
    s32 sxy2;
    s32 sxy3;
    s32 projected_x;
    s32 projected_y;
    s32 i;
    s32 color_frame;
    s32 frame_delta;

    cell = &D_8011D108[x][y];
    frame_delta = g_wmap_cell_effect_steps[cell->state];
    cell->frame += frame_delta;
    if (cell->frame >= 0x11)
    {
        cell->frame = 0x10;
    }
    else if (cell->frame < 0)
    {
        cell->frame = 0;
    }

    if (cell->frame >= 0xF)
    {
        cell->state = 2;
    }

    if (cell->frame <= 0)
    {
        cell->state = 1;
        return;
    }

    projected_x = (D_80139950.x * 0x14000) / D_80139950.scale;
    projected_y = (D_80139950.y * 0x14000) / D_80139950.scale;
    base.x = ((((x - 1) * 0xA0) - projected_x) * 0x6000) / D_80139950.scale + 0xA;
    base.y = ((((y - 1) * 0xA0) - projected_y) * 0x6000) / D_80139950.scale + 0xC;
    base.z = 0;

    color_frame = cell->frame * 7;

    for (i = 0; i < 16; i++)
    {
        WmapQuadTemplate* source;
        POLY_G4* poly;

        source = &g_wmap_cell_effect_quads[i];
        poly = D_801398EC->packet_cursor;

        transformed[0].x = base.x + source->vertices[0].x;
        transformed[0].y = base.y + source->vertices[0].y;
        transformed[0].z = base.z + source->vertices[0].z;
        transformed[1].x = base.x + source->vertices[1].x;
        transformed[1].y = base.y + source->vertices[1].y;
        transformed[1].z = base.z + source->vertices[1].z;
        transformed[2].x = base.x + source->vertices[2].x;
        transformed[2].y = base.y + source->vertices[2].y;
        transformed[2].z = base.z + source->vertices[2].z;
        transformed[3].x = base.x + source->vertices[3].x;
        transformed[3].y = base.y + source->vertices[3].y;
        transformed[3].z = base.z + source->vertices[3].z;

        gte_ldv3(&transformed[0], &transformed[1], &transformed[2]);
        gte_rtpt();

        *(u32*)&poly->r0 = func_8006CF40(source->colors[0], color_frame);
        *(u32*)&poly->r1 = func_8006CF40(source->colors[1], color_frame);
        *(u32*)&poly->r2 = func_8006CF40(source->colors[2], color_frame);
        *(u32*)&poly->r3 = func_8006CF40(source->colors[3], color_frame);

        if ((D_8011D4FC == 3) || (D_8011D4FC == 7) || (D_8011D4FC == 0x1B))
        {
            poly->r0 = poly->r1 = poly->r2 = poly->r3 = 0;
        }

        gte_stsxy3(&sxy0, &sxy1, &sxy2);
        gte_ldv0(&transformed[3]);
        gte_rtps();

        *(u32*)&poly->x0 = sxy0;
        *(u32*)&poly->x1 = sxy1;
        *(u32*)&poly->x2 = sxy2;
        gte_stsxy(&sxy3);
        *(u32*)&poly->x3 = sxy3;

        setlen(poly, 8);
        setcode(poly, 0x3A);
        addPrim(&D_801398EC->ordering_table[174], poly);

        if (D_800D921C < WMAP_PACKET_LIMIT)
        {
            D_800D921C += sizeof(POLY_G4);
            D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(POLY_G4);
        }
    }

    func_8006534C(0xAE, 0xAE);
}

/**
 * @brief Reset world-map objects and effect cells to their initial state.
 */
void wmap_init_land_display(void)
{
    s32 i;
    s32 x;
    s32 y;

    for (i = 0; i < WMAP_LAND_COUNT; i++)
    {
        g_wmap_land_display[i].object_id = i;
        g_wmap_land_display[i].resource_id = i;
        g_wmap_land_display[i].previous_animation_index = 0;
        g_wmap_land_display[i].animation_index = 0;
        g_wmap_land_display[i].scale_frame = 0;
        g_wmap_land_display[i].transition = 0;
        g_wmap_land_display[i].display_mode = 0;
        g_wmap_land_display[i].frame_timer = 0;
    }

    for (y = 0; y < WMAP_GRID_SIZE; y++)
    {
        for (x = 0; x < WMAP_GRID_SIZE; x++)
        {
            WmapEffectCell cell;

            D_8011D108[x][y].unk00 = i;
            D_8011D108[x][y].frame = 0;
            D_8011D108[x][y].state = 0;
            D_8011D108[x][y].mode = 0;
            D_8011D108[x][y].unk10 = 0;
            cell = D_8011D108[x][y];
        }
    }
}

/**
 * @brief Advance selected spirit animations and draw their eight icons.
 * @param selected_index Animation index to advance, or -1 to advance every animation.
 */
void wmap_draw_spirit_icons(s32 selected_index)
{
    s32 i;

    for (i = 0; i < WMAP_SPIRIT_COUNT; i++)
    {
        SPRT* sprite;
        u8 frame;

        if (selected_index == -1 || selected_index == i)
        {
            g_wmap_spirit_timers[i]--;
        }

        if ((s8)g_wmap_spirit_timers[i] <= 0)
        {
            frame = g_wmap_spirit_frames[i] + 2;
            g_wmap_spirit_frames[i] = frame;
            if ((s8)frame >= g_wmap_spirit_sequence_bounds[i + 1])
            {
                g_wmap_spirit_frames[i] = g_wmap_spirit_sequence_bounds[i];
            }
            g_wmap_spirit_timers[i] = g_wmap_spirit_sequences[(s8)g_wmap_spirit_frames[i] + 1];
        }

        sprite = D_801398EC->packet_cursor;
        *sprite = D_8004FD94[i];
        sprite->r0 = sprite->g0 = sprite->b0 = g_wmap_spirit_brightness;
        sprite->u0 = (g_wmap_spirit_sequences[(s8)g_wmap_spirit_frames[i]] % 5) * 24;
        sprite->v0 = (g_wmap_spirit_sequences[(s8)g_wmap_spirit_frames[i]] / 5) * 24;

        if (g_wmap_spirit_brightness < WMAP_BLEND_THRESHOLD)
        {
            setSemiTrans(sprite, 1);
        }

        addPrim(&D_801398EC->ordering_table[4], sprite);

        if (D_800D921C < WMAP_PACKET_LIMIT)
        {
            D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
            D_800D921C += sizeof(SPRT);
        }
    }

    func_8006534C(0x3D, 4);
}

/**
 * @brief Draw spirit strength and accent sprites for the selected cell.
 */
void wmap_draw_spirit_levels(void)
{
    s32 sprite_indices[WMAP_SPIRIT_COUNT];
    s32 x;
    s32 y;
    s32 i;
    s32 color;

    if (g_wmap_spirit_brightness == 0)
    {
        return;
    }

    x = D_80139950.x / WMAP_CELL_SPACING + D_800DCEEC;
    y = D_80139950.y / WMAP_CELL_SPACING + D_800DCEF0;

    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    if (x >= 6)
    {
        x = 5;
    }
    if (y >= 6)
    {
        y = 5;
    }

    color = g_wmap_spirit_brightness + (D_8011CF74 & 8);
    if (color < 0)
    {
        color = 0;
    }

    if (D_8011CF18 == 2)
    {
        func_8005D7A0(func_8005D8FC(), sprite_indices);
    }
    else if (D_80129550 == 1)
    {
        func_8005C404(x, y, D_8011D4FC, x, y, sprite_indices);
    }
    else
    {
        func_8005D6B8(x, y, sprite_indices);
    }

    i = 0;
    do
    {
        SPRT* primary_sprite;
        SPRT* secondary_sprite;

        primary_sprite = D_801398EC->packet_cursor;
        *primary_sprite = g_wmap_spirit_accent_sprites[sprite_indices[i]];
        primary_sprite->r0 = primary_sprite->g0 = primary_sprite->b0 = color;
        *(u32*)&primary_sprite->x0 = D_8004FD9C[i].packed_xy;
        primary_sprite->clut = getClut(WMAP_SPIRIT_CLUT_X, i + WMAP_SPIRIT_ACCENT_CLUT_Y);
        setSemiTrans(primary_sprite, 1);
        addPrim(&D_801398EC->ordering_table[1], primary_sprite);
        if (D_800D921C < WMAP_PACKET_LIMIT)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
        }

        secondary_sprite = D_801398EC->packet_cursor;
        *secondary_sprite = g_wmap_spirit_level_sprites[sprite_indices[i]];
        secondary_sprite->r0 = secondary_sprite->g0 = secondary_sprite->b0 = g_wmap_spirit_brightness;
        *(u32*)&secondary_sprite->x0 = D_8004FD9C[i].packed_xy;
        secondary_sprite->clut = getClut(WMAP_SPIRIT_CLUT_X, i + WMAP_SPIRIT_CLUT_Y);
        secondary_sprite->x0 += 4;
        secondary_sprite->y0 += 0x14;
        if (g_wmap_spirit_brightness < WMAP_BLEND_THRESHOLD)
        {
            setSemiTrans(secondary_sprite, 1);
        }
        addPrim(&D_801398EC->ordering_table[1], secondary_sprite);
        if (D_800D921C < WMAP_PACKET_LIMIT)
        {
            D_800D921C += sizeof(SPRT);
            D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
        }

        i++;
    } while (i < WMAP_SPIRIT_COUNT);

    func_8006534C(0x3D, 1);
}

/**
 * @brief Draw one spirit's strength across the map, blinking the selected cell.
 * @param spirit_index Spirit palette and strength index, from zero through seven.
 */
void wmap_draw_spirit_grid(s32 spirit_index)
{
    /* Partial WMAP decompilation: 86.301650% (gcc280_g0). */

    s32 sprite_indices[WMAP_SPIRIT_COUNT];
    s32 target_x;
    s32 target_y;
    s32 cur_x;
    s32 cur_y;
    s32 base_index;
    SPRT* sprite;

    if (g_wmap_spirit_brightness == 0)
    {
        return;
    }

    target_x = D_800DCEC8.x / WMAP_CELL_SPACING + D_800DCEEC;
    target_y = D_800DCEC8.y / WMAP_CELL_SPACING + D_800DCEF0;

    cur_y = 0;
    base_index = 0;
    do
    {
        cur_x = 0;
        do
        {
            if (D_80129550 == 1 && D_80139290[target_x][target_y].effect_enabled != 0)
            {
                func_8005C404(cur_x, cur_y, D_8011D4FC, target_x, target_y, sprite_indices);
            }
            else
            {
                func_8005D6B8(cur_x, cur_y, sprite_indices);
            }

            if (cur_x == target_x && cur_y == target_y && (D_8011CF74 & 4))
            {
                sprite = D_801398EC->packet_cursor;
                *sprite = g_wmap_spirit_level_sprites[sprite_indices[spirit_index]];
                *(u32*)&sprite->x0 = D_8004FC74[base_index + cur_x];
                sprite->u0 = 0xD0;
                sprite->v0 = 0;
                sprite->r0 = sprite->g0 = sprite->b0 = (u8)g_wmap_spirit_brightness;
                sprite->clut = 0x6AAE;
                if (g_wmap_spirit_brightness < WMAP_BLEND_THRESHOLD)
                {
                    setSemiTrans(sprite, 1);
                }
                addPrim(&D_801398EC->ordering_table[1], sprite);
                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
                }
            }

            sprite = D_801398EC->packet_cursor;
            *sprite = g_wmap_spirit_level_sprites[sprite_indices[spirit_index]];
            *(u32*)&sprite->x0 = D_8004FC74[base_index + cur_x];
            sprite->r0 = sprite->g0 = sprite->b0 = (u8)g_wmap_spirit_brightness;
            sprite->clut = getClut(WMAP_SPIRIT_CLUT_X, spirit_index + WMAP_SPIRIT_CLUT_Y);
            if (g_wmap_spirit_brightness < WMAP_BLEND_THRESHOLD)
            {
                setSemiTrans(sprite, 1);
            }
            addPrim(&D_801398EC->ordering_table[1], sprite);
            if (D_800D921C < WMAP_PACKET_LIMIT)
            {
                D_800D921C += sizeof(SPRT);
                D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
            }

            cur_x++;
        } while (cur_x < WMAP_GRID_SIZE);

        cur_y++;
        base_index += WMAP_GRID_SIZE;
    } while (cur_y < WMAP_GRID_SIZE);

    func_8006534C(0x3D, 1);
}

/** @brief Update map rendering and approach the requested fade intensity. */
void wmap_update_map_display(void)
{
    if (g_wmap_spirit_brightness != 0)
    {
        switch (D_8013986C)
        {
        case 0:
            wmap_draw_spirit_icons(-1);
            wmap_draw_spirit_levels();
            wmap_draw_information_labels();
            break;
        case 1:
            if (D_800DCF04 != 0)
            {
                wmap_draw_spirit_icons(D_800DCF04 - 1);
                wmap_draw_spirit_grid(D_800DCF04 - 1);
            }
            break;
        }
    }
    if (D_8011CF74 & 1)
    {
        D_80139218 = (D_80139218 + 1) & 7;
    }
    /* Fade requests use multiples of eight; the two tests also catch overshoot. */
    if (g_wmap_spirit_brightness > g_wmap_spirit_target_brightness)
    {
        g_wmap_spirit_brightness -= WMAP_FADE_STEP;
    }
    if (g_wmap_spirit_brightness < g_wmap_spirit_target_brightness)
    {
        g_wmap_spirit_brightness += WMAP_FADE_STEP;
    }
    if (g_wmap_spirit_brightness >= 0x81)
    {
        g_wmap_spirit_target_brightness = WMAP_FULL_BRIGHTNESS;
        g_wmap_spirit_brightness = WMAP_FULL_BRIGHTNESS;
    }
}

/** @brief Draw the enabled map information labels and substitute dynamic glyphs. */
void wmap_draw_information_labels(void)
{
    /* Partial WMAP decompilation: 98.924730% (gcc280_g0). */

    s32 dynamic_index;
    s32 group;
    s32 i;
    s32 end;
    s32 next_offset;

    s32 glyph;
    u16 palette;
    WmapGlyphPlacement* placement;
    WmapGlyph* image;
    SPRT* packet;

    if ((D_8013922C & (PADLup | PADLright | PADLdown | PADLleft)) || D_801398D0 != 0)
    {
        g_wmap_information_groups = 0;
    }
    if (!(D_801398C0 & PADRup) || D_8011D4FC == -1)
    {
        g_wmap_information_groups = 0;
        return;
    }
    if (g_wmap_information_groups == 0)
    {
        func_8005D018(D_800DCEEC + D_800DCEF0 * 3, D_80139950.x / WMAP_CELL_SPACING + D_800DCEEC, D_80139950.y / WMAP_CELL_SPACING + D_800DCEF0,
                      &g_wmap_information_groups, g_wmap_information_values, D_8011D4FC);
    }
    dynamic_index = 0;
    for (group = 0, next_offset = 4; group < 10; next_offset += 4, group++)
    {
        if ((g_wmap_information_groups >> group) & 1)
        {
            i = g_wmap_information_group_starts[group];
            end = *(s32*)((u8*)g_wmap_information_group_starts + next_offset);
            for (; i < end; i++)
            {
                placement = &g_wmap_information_placements[i];
                glyph = placement->glyph;
                if (glyph == WMAP_DYNAMIC_GLYPH)
                {
                    glyph = g_wmap_information_values[dynamic_index++];
                }
                packet = (SPRT*)D_801398EC->packet_cursor;
                packet->x0 = placement->x;
                image = &g_wmap_information_glyphs[glyph];
                packet->y0 = placement->y;
                packet->u0 = image->u + 192;
                packet->v0 = image->v + 96;
                packet->w = image->width;
                packet->h = image->height;
                palette = image->palette;
                setlen(packet, 4);
                packet->r0 = 128;
                packet->g0 = 128;
                packet->b0 = 128;
                packet->code = 100;
                packet->clut = ((palette + 432) << 6) | 46;
                addPrim(&D_801398EC->ordering_table[2], packet);
                if (D_800D921C < WMAP_PACKET_LIMIT)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->packet_cursor = (u8*)D_801398EC->packet_cursor + sizeof(SPRT);
                }
            }
        }
    }
    func_8006534C(59, 2);
}

/**
 * @brief Clamp the land scale frame and select its display transition.
 * @param index Land display index.
 * @param mode Hidden (zero), marker (one), animated (two), or fully expanded (five).
 */
void wmap_set_land_display_mode(s32 index, s32 mode)
{
    s8 scale_frame;
    u8 old_mode;
    WmapLandDisplay* land;

    land = &g_wmap_land_display[index];
    scale_frame = land->scale_frame;
    if (scale_frame > WMAP_LAND_SCALE_MAX)
    {
        land->scale_frame = WMAP_LAND_SCALE_MAX;
    }
    else if (scale_frame < 0)
    {
        land->scale_frame = 0;
    }
    if (mode == WMAP_LAND_EXPANDED)
    {
        land->display_mode = WMAP_LAND_ANIMATED;
        land->transition = WMAP_TRANSITION_EXPANDED;
        land->scale_frame = WMAP_LAND_SCALE_MAX;
        return;
    }
    old_mode = land->display_mode;
    if (mode != old_mode)
    {
        switch (old_mode)
        {
        case WMAP_LAND_HIDDEN:
            land->display_mode = mode;
            if (mode == WMAP_LAND_MARKER)
            {
                land->transition = mode;
                return;
            }
            land->transition = WMAP_TRANSITION_GROW;
            return;
        case WMAP_LAND_MARKER:
            land->display_mode = mode;
            if (mode != WMAP_LAND_HIDDEN)
            {
                land->transition = WMAP_TRANSITION_GROW;
                return;
            }
            land->transition = WMAP_TRANSITION_HIDDEN;
            land->scale_frame = 0;
            return;
        case WMAP_LAND_ANIMATED:
            land->display_mode = mode;
            if (mode != WMAP_LAND_HIDDEN)
            {
                land->transition = WMAP_TRANSITION_SHRINK;
                return;
            }
            land->transition = WMAP_TRANSITION_HIDDEN;

            land->scale_frame = 0;
            return;
        }
    }
}

/**
 * @brief Upload a land image and retain its animation data and upload frame.
 * @param resource Resource selecting the packed image data.
 * @param slot Destination cache slot.
 */
void wmap_upload_land_image(WmapLandDisplay* resource, s32 slot)
{
    WmapCacheEntry* cache;
    WmapCacheEntry* base;
    s16 resource_id;
    u8* image;

    base = g_wmap_land_image_cache;
    cache = &base[slot];
    resource_id = resource->resource_id;
    cache->resource_id = resource_id;
    image = (u8*)D_8013C628 + D_8013C628[resource_id];
    LoadImage(&D_800CBBE8[slot].upload.rect, (u_long*)image);
    cache->animation_data = image + 0x2004;
    cache->loaded_frame = D_8011CF74;
}

/**
 * @brief Draw a cached map resource, or its marker while the image is unavailable.
 * @param map_x Map-grid X coordinate.
 * @param map_y Map-grid Y coordinate.
 * @param resource_index Index of the world-map resource to draw.
 */
void wmap_draw_land(s32 map_x, s32 map_y, s32 resource_index)
{
    WmapLandDisplay* resource;
    s32 slot;

    resource = &g_wmap_land_display[resource_index];
    if (resource->transition == 1)
    {
        wmap_draw_land_marker(map_x, map_y, resource);
    }
    else
    {
        slot = wmap_resolve_land_image(resource);
        if (slot != -1 && g_wmap_land_image_cache[slot].busy == 0)
        {
            wmap_draw_land_animation(map_x, map_y, resource, slot);
        }
        else
        {
            wmap_draw_land_marker(map_x, map_y, resource);
        }
    }
}

/** @brief Initialize the 16 reusable slots while preserving their animation pointers. */
void wmap_init_land_image_cache(void)
{
    u32 slot_index;
    for (slot_index = 0; slot_index < (u32)WMAP_CACHE_SLOTS; slot_index++)
    {
        g_wmap_land_image_cache[slot_index].slot_index = slot_index;
        g_wmap_land_image_cache[slot_index].resource_id = -1;
        g_wmap_land_image_cache[slot_index].loaded_frame = 0;
        g_wmap_land_image_cache[slot_index].busy = 0;
    }
}

/**
 * @brief Reserved world-map entry point; performs no work.
 */
void func_80058298(void)
{
}

/**
 * @brief Classify a point and apply the current land-display limit.
 * @param x Map-space X coordinate.
 * @param y Map-space Y coordinate.
 * @param scale Unused projection scale supplied by the travel renderer.
 * @return Hidden (zero), marker (one), or animated land (two).
 */
s32 wmap_get_point_display_mode(s32 x, s32 y, s32 scale)
{
    s32 current = wmap_classify_map_point(x, y);
    if (D_800DBE70 != 2)
    {
        s32 limit = D_800DBE70;
        if (current >= limit)
        {
            return limit;
        }
    }
    return current;
}

/** @brief Enable eight slots and copy their initial table values. */
void wmap_init_spirit_animation(void)
{
    s32 index;
    for (index = 0; index < WMAP_SPIRIT_COUNT; index++)
    {
        g_wmap_spirit_timers[index] = 1;
        g_wmap_spirit_frames[index] = g_wmap_spirit_sequence_bounds[index + 1];
    }
}

/**
 * @brief Select a cell effect transition when its requested mode changes.
 * @param x Map-grid X coordinate.
 * @param y Map-grid Y coordinate.
 * @param mode Requested mode: zero, one, or two.
 */
void wmap_set_cell_effect_mode(s32 x, s32 y, s32 mode)
{
    WmapEffectCell* cell;

    cell = &D_8011D108[x][y];
    if (mode != cell->mode)
    {
        switch (cell->mode)
        {
        case 0:
            cell->mode = mode;
            if (mode == 1)
            {
                cell->state = mode;
                return;
            }
            cell->state = 3;
            return;
        case 1:
            cell->mode = mode;
            if (mode != 0)
            {
                cell->state = 3;
                return;
            }
            cell->state = 0;
            cell->frame = 0;
            return;
        case 2:
            cell->mode = mode;
            if (mode != 0)
            {
                cell->state = 4;
                return;
            }
            cell->state = 0;
            cell->frame = 0;
            return;
        }
    }
}

/**
 * @brief Classify a point against two world-map coordinate regions.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @return Region code: zero, one, or two.
 */
s32 wmap_classify_map_point(s32 x, s32 y)
{
    s32 dy;
    u32 dx;

    dx = x - D_80139950.x;
    dy = y - D_80139950.y;
    if ((dx < 0x61U) && (dy >= 0) && (dy < 0x61))
    {
        if ((D_8013986C == 3) || (D_8013986C == 1))
        {
            return 0;
        }
        return 2;
    }
    if ((u32)(dx - 16) < 97 && dy >= 16 && dy < 113)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Return one; the purpose of this entry point is unknown.
 * @return Always one.
 */
s32 func_80058488(void)
{
    return 1;
}

/**
 * @brief Reserved world-map entry point; performs no work.
 */
void func_80058490(void)
{
}

/**
 * @brief Clear three world-map state values.
 */
void func_80058498(void)
{
    D_800D7D60 = 0;
    D_800D7D64 = 0;
    D_800D7D68 = 0;
}
