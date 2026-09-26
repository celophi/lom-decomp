#include "wmap_frame_render.h"
#include "wmap_main.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "common.h"
#include "sdk/libetc.h"
#include "sdk/libgpu.h"
#include "gpu_packet.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"
#include "sdk/libgte.h"
#include "sdk/rand.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_backdrop.h"

/** @brief Map tiles drawn inside the one-tile border of the grid. */
#define WMAP_MAP_VISIBLE_FIRST 1
#define WMAP_MAP_VISIBLE_END (WMAP_MAP_TILES - 1)
/** @brief Projected grid vertices per row and column. */
#define WMAP_MAP_VERTICES (WMAP_MAP_TILES - 1)
/** @brief Ordering-table depth of the map grid and its shadow. */
#define WMAP_MAP_OT_INDEX 175

/** @brief Tile page and palette of the map texture. */
#define WMAP_MAP_TPAGE getTPage(0, 0, 576, 0)
#define WMAP_MAP_CLUT getClut(576, 384)
/** @brief VRAM x of the two map texture pages and y of the lower one. */
#define WMAP_MAP_PAGE_X 576
#define WMAP_MAP_PAGE_X_WRAP 640
#define WMAP_MAP_PAGE_Y_LOW 256
/** @brief Texels per texture page, per map tile, and the page overlap a wrapped tile skips. */
#define WMAP_MAP_PAGE_TEXELS 256
#define WMAP_MAP_TEXEL_TILE 48
#define WMAP_MAP_TEXEL_WRAP 32
/** @brief Last texel column at which a tile still starts on the first page. */
#define WMAP_MAP_TEXEL_EDGE (WMAP_MAP_PAGE_TEXELS - (WMAP_MAP_TEXEL_TILE - WMAP_MAP_TEXEL_WRAP))

/** @brief Subtractive-blend texture page selected for the shadow polygons. */
#define WMAP_SHADOW_TPAGE getTPage(0, 2, 320, 0)
/** @brief Brightest shadow level and the shadow's vertical offset. */
#define WMAP_SHADOW_LEVEL_MAX 64
#define WMAP_SHADOW_OFFSET_Y 10
/** @brief Tiles copied into the shadow: the bottom rows, then the right columns above them. */
#define WMAP_SHADOW_BOTTOM_ROW 22
#define WMAP_SHADOW_RIGHT_COLUMN 23
#define WMAP_SHADOW_TOP_ROW 3

/** @brief POLY_FT4 command bytes for the tile color word. */
#define WMAP_TILE_CODE 0x2C
#define WMAP_TILE_CODE_BLENDED 0x2E

/** @brief World-map sound ids and the pan position of the screen center. */
#define WMAP_SOUND_ZOOM_OUT 1
#define WMAP_SOUND_ZOOM_IN 2
#define WMAP_SOUND_CURSOR 3
#define WMAP_PAN_CENTER 128
#define WMAP_PAN_PER_COLUMN 24

/** @brief Map scroll limit, map scroll speed and the map distance of one cursor step. */
#define WMAP_VIEW_SCROLL_MAX 144
#define WMAP_VIEW_SCROLL_SPEED 4
#define WMAP_VIEW_SCROLL_STEP 48
/** @brief Spirit entries cycled in the spirit view (0 = none). */
#define WMAP_SPIRIT_SELECTIONS 9

/** @brief Projection scale of the map view and of the zoomed-out spirit view. */
#define WMAP_VIEW_SCALE 0x6000
#define WMAP_SPIRIT_VIEW_SCALE 0xC000
/** @brief Frames of the zoom animation; zoom values are 8.8 fixed point. */
#define WMAP_ZOOM_FRAMES 16
#define WMAP_ZOOM_ONE 256

/** @brief Map-to-model scale used to project a map position. */
#define WMAP_MAP_PROJECTION_SCALE 0x14000
/** @brief Map units between two land cells. */
#define WMAP_CELL_SIZE 160

/** @brief Burst particle slots, their first sprite actor, and the burst center. */
#define WMAP_BURST_PARTICLES 110
#define WMAP_BURST_ACTOR_FIRST 6
#define WMAP_BURST_CENTER_X 160
#define WMAP_BURST_CENTER_Y 120
#define WMAP_BURST_ACTOR_SCALE 15
#define WMAP_BURST_ACTOR_SHADE 129
#define WMAP_BURST_SEQUENCE_LARGE 3
#define WMAP_BURST_OT_INDEX 4
/** @brief Spawns per frame and particle slots scanned, per unit of D_801B0FD0. */
#define WMAP_BURST_SPAWNS_PER_UNIT 3
#define WMAP_BURST_SLOTS_PER_UNIT 70
/** @brief Shortest particle lifetime. */
#define WMAP_BURST_LIFETIME_MIN 5
/** @brief Sprite textures of the large and the small burst particles. */
#define WMAP_BURST_TEXTURE_LARGE 6
#define WMAP_BURST_TEXTURE_SMALL 7
/** @brief Sprite shade that draws the texture unmodified. */
#define WMAP_ACTOR_SHADE_NEUTRAL 128

/** @brief Packed color word: three channels and the GPU command byte. */
typedef union
{
    u32 packed;
    struct
    {
        u8 r, g, b, code;
    } channels;
} WmapColor;

/** @brief Map scroll position (map units) and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

/** @brief World-map sprite actor as read by the animator and the sprite renderer. */
typedef struct
{
    s16 unknown_00;
    s16 unknown_02;
    u8 unknown_04[2];
    u8 scale_index;
    u8 unknown_07[7];
    s16 sequence;
    s16 previous_sequence;
    u8 unknown_12[0x10];
    s16 target_shade;
    s16 shade;
    s16 shade_step;
    u8 unknown_28[4];
} WmapSpriteActor;

/** @brief One particle of the radial burst. */
typedef struct
{
    s16 active;
    s16 angle;
    s32 speed;
    s32 distance;
    s16 lifetime;
    s16 unknown_0e;
    s32 unknown_10;
} WmapBurstParticle;

/** @brief Animation resource slot of a sprite actor. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapAnimationSlot;

/** @brief Screen-space offset of a cursor cell. */
typedef struct
{
    s16 x;
    s16 y;
} WmapPoint;

/** @brief GTE screen coordinate, read as one packed word or as two halves. */
typedef union
{
    s32 packed;
    WmapPoint point;
} WmapScreenPosition;

extern u32 g_wmap_view_sequence_step;
extern s32 g_wmap_view_sequence_timer;
extern void (*g_wmap_view_sequence_steps[])(void);
extern s32 D_80182D88;

extern s32 g_wmap_map_shadow_level;
extern CVECTOR g_wmap_tint_target;
extern WmapColor g_wmap_tint;
extern s32 g_wmap_tint_blending;
extern u8 g_wmap_tint_speed;
extern s32 g_wmap_cursor_limits[];
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_spirit_selection;
extern s32 D_8011CF18;
extern s32 D_8011CF44;
extern s32 g_wmap_view_scroll_enabled;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern WmapView g_wmap_view;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern SVECTOR g_wmap_grid_vertices[];
extern s16 g_wmap_grid_vertex_indices[];
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern WmapView g_wmap_saved_view;
extern s32 D_8011D4FC;
extern WmapView g_wmap_zoom_view;
extern WmapView g_wmap_zoom_step;
extern s32 D_80139288;
extern WmapSpriteActor D_800D9268[];
extern WmapBurstParticle D_801AFBD0[];
extern WmapAnimationSlot D_80139988[];
extern s32 g_wmap_burst_spawning;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern WmapPoint g_wmap_cell_focus_offsets[][3];
extern SVECTOR g_wmap_camera_rotation;
extern VECTOR g_wmap_camera_translation;
extern s32 D_8013B20C;
extern s32 g_wmap_focus_origin_x;
extern s32 g_wmap_focus_origin_y;
extern s32 D_801B1098;
extern s32 D_801B109C;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_8011CF4C;

static s32 wmap_update_map_tint(s32 initialize);
static void wmap_project_map_grid(void);
static void wmap_update_map_shadow(void);
static void wmap_update_map_texcoords(void);
static void wmap_set_map_color(s32 color);

/**
 * @brief Run the current step of the map view sequence.
 * @param initialize Nonzero restarts the sequence before running the step.
 * @return 1 while a step ran, 0 once the sequence has finished.
 */
s32 wmap_run_view_sequence(s32 initialize)
{
    s32 result;

    if (initialize != 0)
    {
        g_wmap_view_sequence_step = 1;
        g_wmap_view_sequence_timer = 1;
    }

    if (g_wmap_view_sequence_step < 2)
    {
        g_wmap_view_sequence_steps[g_wmap_view_sequence_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/** @brief Map view sequence step 0: restart the sequence. */
void wmap_reset_view_sequence(void)
{
    g_wmap_view_sequence_step = 1;
    g_wmap_view_sequence_timer = 1;
}

/** @brief Map view sequence step 1: advance to the next step. */
void wmap_advance_view_sequence(void)
{
    g_wmap_view_sequence_step += 1;
}

/** @brief Clear the world-map state value at D_80182D88. */
void func_800654EC(void)
{
    D_80182D88 = 0;
}

/** @brief Build the map tiles, shadow polygons and texture-page selector of both frames. */
void wmap_init_map_packets(void)
{
    s32 buffer_index;
    s32 row;
    s32 column;
    WmapFrame* buffer;
    POLY_FT4* tile;
    POLY_F4* shadow;

    for (buffer_index = 0; buffer_index < 2; buffer_index++)
    {
        buffer = &g_wmap_frames[buffer_index];
        buffer->tpage_select.tpage = WMAP_SHADOW_TPAGE;
        g_wmap_current_frame = buffer;
        /* Each vertex is cleared as one packed x/y word. */
        *(u32*)&buffer->tpage_select.x2 = 0;
        *(u32*)&buffer->tpage_select.x1 = 0;
        *(u32*)&buffer->tpage_select.x0 = 0;
        SET_BGR0_PACKED(&buffer->tpage_select, 0);
        setPolyFT3(&g_wmap_current_frame->tpage_select);
        for (row = 0; row < WMAP_MAP_TILES; row++)
        {
            for (column = 0; column < WMAP_MAP_TILES; column++)
            {
                tile = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column];
                SET_BGR0_PACKED(tile, 0);
                tile->tpage = WMAP_MAP_TPAGE;
                tile->clut = WMAP_MAP_CLUT;
                setPolyFT4(tile);
            }
        }
        if (g_wmap_map_shadow_level != 0)
        {
            if (g_wmap_map_shadow_level < WMAP_SHADOW_LEVEL_MAX)
            {
                g_wmap_map_shadow_level++;
            }
            for (column = 0; column < WMAP_SHADOW_POLYS; column++)
            {
                shadow = &g_wmap_current_frame->shadow[column];
                setRGB0(shadow, g_wmap_map_shadow_level, g_wmap_map_shadow_level, g_wmap_map_shadow_level);
                setPolyF4(shadow);
                setSemiTrans(shadow, 1);
            }
        }
    }
}

/**
 * @brief Step the map tint toward its target color.
 * @param initialize Sequence callback flag; unused.
 * @return Nonzero while a channel still moved.
 */
static s32 wmap_update_map_tint(s32 initialize)
{
    s32 color;
    s32 changed;

    changed = 0;
    if (g_wmap_tint_target.r > g_wmap_tint.channels.r)
    {
        changed = 1;
        g_wmap_tint.channels.r += g_wmap_tint_speed;
    }
    if (g_wmap_tint_target.r < g_wmap_tint.channels.r)
    {
        changed = 1;
        g_wmap_tint.channels.r -= g_wmap_tint_speed;
    }
    if (g_wmap_tint.channels.g < g_wmap_tint_target.g)
    {
        changed = 1;
        g_wmap_tint.channels.g += g_wmap_tint_speed;
    }
    if (g_wmap_tint_target.g < g_wmap_tint.channels.g)
    {
        changed = 1;
        g_wmap_tint.channels.g -= g_wmap_tint_speed;
    }
    if (g_wmap_tint.channels.b < g_wmap_tint_target.b)
    {
        changed = 1;
        g_wmap_tint.channels.b += g_wmap_tint_speed;
    }
    if (g_wmap_tint_target.b < g_wmap_tint.channels.b)
    {
        changed = 1;
        g_wmap_tint.channels.b -= g_wmap_tint_speed;
    }
    if (g_wmap_tint_blending != 0)
    {
        color = g_wmap_tint.packed & 0xFFFFFF;
        if (color == GPU_TINT_NEUTRAL)
        {
            *(u32*)&g_wmap_tint_target = color;
            g_wmap_tint.packed = color;
            g_wmap_tint.channels.code = WMAP_TILE_CODE;
            g_wmap_tint_blending = 0;
        }
        else
        {
            g_wmap_tint.channels.code = WMAP_TILE_CODE_BLENDED;
        }
    }
    else
    {
        g_wmap_tint.channels.code = WMAP_TILE_CODE;
    }
    wmap_set_map_color(g_wmap_tint.packed);
    return changed;
}

/**
 * @brief Move the map cursor, scroll the map view, and queue the map packets.
 * @param initialize Sequence callback flag; unused.
 * @return Always 1, so the callback stays installed.
 */
s32 wmap_update_map_view(s32 initialize)
{
    s32 left_pan;
    s32 right_pan;
    s32 row;
    s32 selection;
    s32 next_y;
    s32 next_x;
    s32 y_step;
    s32 x_step;
    s32 column;
    s32 row_index;

    if (g_wmap_view_mode == WMAP_VIEW_MODE_HIDDEN)
    {
        func_8006AEE0();
        return 1;
    }
    if ((g_wmap_view_mode == WMAP_VIEW_MODE_MAP) && (g_wmap_view_scroll_mode == 0) && (D_8011CF18 == 0))
    {
        if (g_wmap_buttons_repeat & PADLleft)
        {
            left_pan = ((g_wmap_cursor_column - 2) * WMAP_PAN_PER_COLUMN) + WMAP_PAN_CENTER;
            g_wmap_cursor_column -= 1;
            wmap_play_sound(WMAP_SOUND_CURSOR, left_pan);
            if (g_wmap_cursor_column < 0)
            {
                g_wmap_cursor_column = 0;
                if (g_wmap_view.x > 0)
                {
                    g_wmap_scroll_remaining_x = -WMAP_VIEW_SCROLL_STEP;
                    if (g_wmap_view_scroll_enabled != 0)
                    {
                        g_wmap_view_scroll_mode = 1;
                    }
                }
            }
        }
        if (g_wmap_buttons_repeat & PADLright)
        {
            right_pan = (g_wmap_cursor_column * WMAP_PAN_PER_COLUMN) + WMAP_PAN_CENTER;
            g_wmap_cursor_column += 1;
            wmap_play_sound(WMAP_SOUND_CURSOR, right_pan);
            if (g_wmap_cursor_column > g_wmap_cursor_limits[g_wmap_view_mode])
            {
                g_wmap_cursor_column = g_wmap_cursor_limits[g_wmap_view_mode];
                if (g_wmap_view.x < WMAP_VIEW_SCROLL_MAX)
                {
                    g_wmap_scroll_remaining_x = WMAP_VIEW_SCROLL_STEP;
                    if (g_wmap_view_scroll_enabled != 0)
                    {
                        g_wmap_view_scroll_mode = 1;
                    }
                }
            }
        }
        if (g_wmap_buttons_repeat & PADLup)
        {
            wmap_play_sound(WMAP_SOUND_CURSOR, ((g_wmap_cursor_column - 1) * WMAP_PAN_PER_COLUMN) + WMAP_PAN_CENTER);
            row = g_wmap_cursor_row - 1;
            g_wmap_cursor_row = row;
            if (row < 0)
            {
                g_wmap_cursor_row = 0;
                if (g_wmap_view.y > 0)
                {
                    g_wmap_scroll_remaining_y = -WMAP_VIEW_SCROLL_STEP;
                    if (g_wmap_view_scroll_enabled != 0)
                    {
                        g_wmap_view_scroll_mode = 1;
                    }
                }
            }
        }
        if (g_wmap_buttons_repeat & PADLdown)
        {
            wmap_play_sound(WMAP_SOUND_CURSOR, ((g_wmap_cursor_column - 1) * WMAP_PAN_PER_COLUMN) + WMAP_PAN_CENTER);
            g_wmap_cursor_row += 1;
            if (g_wmap_cursor_row > g_wmap_cursor_limits[g_wmap_view_mode])
            {
                g_wmap_cursor_row = g_wmap_cursor_limits[g_wmap_view_mode];
                if (g_wmap_view.y < WMAP_VIEW_SCROLL_MAX)
                {
                    g_wmap_scroll_remaining_y = WMAP_VIEW_SCROLL_STEP;
                    if (g_wmap_view_scroll_enabled != 0)
                    {
                        g_wmap_view_scroll_mode = 1;
                    }
                }
            }
        }
    }
    if (g_wmap_view_mode == WMAP_VIEW_MODE_SPIRITS)
    {
        if (g_wmap_buttons_repeat & PADLleft)
        {
            selection = g_wmap_spirit_selection - 1;
            g_wmap_spirit_selection = selection;
            if (selection < 0)
            {
                g_wmap_spirit_selection = WMAP_SPIRIT_SELECTIONS - 1;
            }
        }
        if (g_wmap_buttons_repeat & PADLright)
        {
            g_wmap_spirit_selection = (g_wmap_spirit_selection + 1) % WMAP_SPIRIT_SELECTIONS;
        }
    }
    if (g_wmap_view_scroll_mode != 0)
    {
        if (g_wmap_view_mode == WMAP_VIEW_MODE_SPIRITS)
        {
            g_wmap_scroll_remaining_y = 0;
            g_wmap_view_scroll_mode = 0;
            g_wmap_scroll_remaining_x = 0;
        }
        else
        {
            g_wmap_buttons_repeat = 0;
            if ((g_wmap_scroll_remaining_y | g_wmap_scroll_remaining_x) == 0)
            {
                g_wmap_view_scroll_mode = 0;
                if (D_8011CF44 == 0)
                {
                    g_wmap_input_locked = 0;
                }
            }
            if (g_wmap_scroll_remaining_y != 0)
            {
                y_step = -WMAP_VIEW_SCROLL_SPEED;
                if (g_wmap_view_scroll_enabled != 0)
                {
                    g_wmap_input_locked = 1;
                    g_wmap_buttons_held = 0;
                    g_wmap_buttons_repeat = 0;
                    if (g_wmap_scroll_remaining_y > 0)
                    {
                        y_step = WMAP_VIEW_SCROLL_SPEED;
                    }
                    g_wmap_scroll_remaining_y -= y_step;
                    next_y = g_wmap_view.y + y_step;
                    g_wmap_view.y = next_y;
                    if (g_wmap_view_scroll_mode == 1)
                    {
                        if (next_y < 0)
                        {
                            g_wmap_view.y = 0;
                            g_wmap_scroll_remaining_y = 0;
                            g_wmap_buttons_repeat = 0;
                            g_wmap_view_scroll_mode = 0;
                        }
                        if (g_wmap_view.y > WMAP_VIEW_SCROLL_MAX)
                        {
                            g_wmap_view.y = WMAP_VIEW_SCROLL_MAX;
                            g_wmap_scroll_remaining_y = 0;
                            g_wmap_view_scroll_mode = 0;
                            g_wmap_buttons_repeat &= ~(PADLup | PADLdown);
                        }
                    }
                }
            }
            if (g_wmap_scroll_remaining_x != 0)
            {
                x_step = -WMAP_VIEW_SCROLL_SPEED;
                if (g_wmap_view_scroll_enabled != 0)
                {
                    g_wmap_input_locked = 1;
                    g_wmap_buttons_held = 0;
                    g_wmap_buttons_repeat = 0;
                    if (g_wmap_scroll_remaining_x > 0)
                    {
                        x_step = WMAP_VIEW_SCROLL_SPEED;
                    }
                    g_wmap_scroll_remaining_x -= x_step;
                    next_x = g_wmap_view.x + x_step;
                    g_wmap_view.x = next_x;
                    if (g_wmap_view_scroll_mode == 1)
                    {
                        if (next_x < 0)
                        {
                            g_wmap_view.x = 0;
                            g_wmap_scroll_remaining_x = 0;
                            g_wmap_buttons_repeat = 0;
                            g_wmap_view_scroll_mode = 0;
                        }
                        if (g_wmap_view.x > WMAP_VIEW_SCROLL_MAX)
                        {
                            g_wmap_view.x = WMAP_VIEW_SCROLL_MAX;
                            g_wmap_scroll_remaining_x = 0;
                            g_wmap_view_scroll_mode = 0;
                            g_wmap_buttons_repeat &= ~(PADLleft | PADLright);
                        }
                    }
                }
            }
        }
    }
    wmap_update_map_texcoords();
    wmap_project_map_grid();
    for (row_index = WMAP_MAP_VISIBLE_FIRST; row_index < WMAP_MAP_VISIBLE_END; row_index++)
    {
        for (column = WMAP_MAP_VISIBLE_FIRST; column < WMAP_MAP_VISIBLE_END; column++)
        {
            addPrim(&g_wmap_current_frame->ordering_table[WMAP_MAP_OT_INDEX], &g_wmap_current_frame->tiles.flat[row_index * WMAP_MAP_TILES + column]);
        }
    }
    for (column = 0; column < WMAP_SHADOW_POLYS; column++)
    {
        POLY_F4* shadow = &g_wmap_current_frame->shadow[column];

        addPrim(&g_wmap_current_frame->ordering_table[WMAP_MAP_OT_INDEX], shadow);
    }
    addPrim(&g_wmap_current_frame->ordering_table[WMAP_MAP_OT_INDEX], &g_wmap_current_frame->tpage_select);
    return 1;
}

/**
 * @brief Project the map grid vertices into the corners of the four tiles that share each one.
 * @note The next vertex index is computed while the GTE runs the perspective transform.
 */
static void wmap_project_map_grid(void)
{
    SVECTOR position;
    s32 screen_position;
    s32 row;
    s32 column;
    s32 point;
    POLY_FT4* top_left;
    POLY_FT4* top_right;
    POLY_FT4* bottom_left;
    POLY_FT4* bottom_right;

    func_8006AEE0();
    for (row = 0; row < WMAP_MAP_VERTICES; row++)
    {
        top_left = g_wmap_current_frame->tiles.rows[row];
        top_right = top_left + 1;
        bottom_left = top_left + WMAP_MAP_TILES;
        bottom_right = top_left + WMAP_MAP_TILES + 1;
        point = (row + 1) * 4;
        for (column = 0; column < WMAP_MAP_VERTICES;)
        {
            position = g_wmap_grid_vertices[g_wmap_grid_vertex_indices[point]];
            gte_ldv0(&position);
            gte_rtps();
            point = (column + 1) * (WMAP_MAP_TILES * 4) + (row + 1) * 4;
            column++;
            gte_stsxy(&screen_position);
            /* The GTE result is one packed x/y word for each shared corner. */
            *(s32*)&bottom_right->x0 = screen_position;
            *(s32*)&bottom_left->x1 = screen_position;
            *(s32*)&top_right->x2 = screen_position;
            *(s32*)&top_left->x3 = screen_position;
            top_left++;
            top_right++;
            bottom_left++;
            bottom_right++;
        }
    }
    wmap_update_map_shadow();
}

/** @brief Copy the bottom rows and right columns of the map into the shadow polygons, shifted down. */
static void wmap_update_map_shadow(void)
{
    s32 row;
    s32 column;
    POLY_FT4* tile;
    POLY_F4* shadow;

    shadow = g_wmap_current_frame->shadow;
    for (row = WMAP_SHADOW_BOTTOM_ROW; row < WMAP_MAP_VISIBLE_END; row++)
    {
        tile = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + WMAP_MAP_VISIBLE_FIRST];
        for (column = WMAP_MAP_VISIBLE_FIRST; column < WMAP_MAP_VISIBLE_END; column++)
        {
            shadow->x0 = tile->x0;
            shadow->y0 = tile->y0 + WMAP_SHADOW_OFFSET_Y;
            shadow->x1 = tile->x1;
            shadow->y1 = tile->y1 + WMAP_SHADOW_OFFSET_Y;
            shadow->x2 = tile->x2;
            shadow->y2 = tile->y2 + WMAP_SHADOW_OFFSET_Y;
            shadow->x3 = tile->x3;
            shadow->y3 = tile->y3 + WMAP_SHADOW_OFFSET_Y;
            shadow++;
            tile++;
        }
    }
    for (column = WMAP_SHADOW_RIGHT_COLUMN; column < WMAP_MAP_VISIBLE_END; column++)
    {
        tile = &g_wmap_current_frame->tiles.flat[column + WMAP_SHADOW_TOP_ROW * WMAP_MAP_TILES];
        for (row = WMAP_SHADOW_TOP_ROW; row < WMAP_SHADOW_BOTTOM_ROW + 1; row++)
        {
            shadow->x0 = tile->x0;
            shadow->y0 = tile->y0 + WMAP_SHADOW_OFFSET_Y;
            shadow->x1 = tile->x1;
            shadow->y1 = tile->y1 + WMAP_SHADOW_OFFSET_Y;
            shadow->x2 = tile->x2;
            shadow->y2 = tile->y2 + WMAP_SHADOW_OFFSET_Y;
            shadow->x3 = tile->x3;
            shadow->y3 = tile->y3 + WMAP_SHADOW_OFFSET_Y;
            shadow++;
            tile += WMAP_MAP_TILES;
        }
    }
}

/**
 * @brief Recompute the map tile texture coordinates for the current scroll and scale.
 * @note The map texture spans two VRAM pages; each tile keeps the VRAM x and y of its
 *       page in the unused pad2 and pad1 fields until the last pass builds its tpage.
 */
static void wmap_update_map_texcoords(void)
{
    s32 row, column;
    s32 wrapped;
    s32 coordinate, edge;
    POLY_FT4 *tile, *next;

    wrapped = 0;
    for (row = 0; row < WMAP_MAP_VERTICES; row++)
    {
        next = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES];
        next->u0 = next->u2 = g_wmap_view.x + g_wmap_view.projection_scale / ONE;
        for (column = 0; column < WMAP_MAP_VERTICES; column++)
        {
            tile = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column];
            next = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column + 1];
            coordinate = column * g_wmap_view.projection_scale / ONE + g_wmap_view.x;
            edge = coordinate + WMAP_MAP_TEXEL_TILE;
            if (wrapped != 0)
            {
                wrapped = 0;
                tile->u1 = tile->u3 = coordinate + (WMAP_MAP_TEXEL_TILE - WMAP_MAP_TEXEL_WRAP);
                tile->pad2 = WMAP_MAP_PAGE_X_WRAP;
            }
            else
            {
                if (edge < WMAP_MAP_PAGE_TEXELS)
                {
                    tile->pad2 = WMAP_MAP_PAGE_X;
                }
                else
                {
                    tile->pad2 = WMAP_MAP_PAGE_X_WRAP;
                }
                tile->u1 = tile->u3 = edge;
            }
            if (edge < WMAP_MAP_TEXEL_EDGE)
            {
                next->pad2 = WMAP_MAP_PAGE_X;
                next->u0 = next->u2 = edge;
            }
            else
            {
                next->pad2 = WMAP_MAP_PAGE_X_WRAP;
                if (edge >= WMAP_MAP_PAGE_TEXELS)
                {
                    next->u0 = next->u2 = edge;
                }
                else
                {
                    wrapped = 1;
                    next->u0 = next->u2 = edge - WMAP_MAP_TEXEL_WRAP;
                }
            }
        }
        tile = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column];
        coordinate = column * g_wmap_view.projection_scale / ONE + g_wmap_view.x;
        edge = coordinate + WMAP_MAP_TEXEL_TILE;
        if (wrapped != 0)
        {
            wrapped = 0;
            tile->u1 = tile->u3 = coordinate + (WMAP_MAP_TEXEL_TILE - WMAP_MAP_TEXEL_WRAP);
        }
        else
        {
            if (edge < WMAP_MAP_PAGE_TEXELS)
            {
                tile->pad2 = WMAP_MAP_PAGE_X;
            }
            else
            {
                tile->pad2 = WMAP_MAP_PAGE_X_WRAP;
            }
            tile->u1 = tile->u3 = edge;
        }
    }
    wrapped = 0;
    for (column = 0; column < WMAP_MAP_VERTICES; column++)
    {
        next = &g_wmap_current_frame->tiles.flat[column * WMAP_MAP_TILES];
        next->v0 = next->v1 = g_wmap_view.y + g_wmap_view.projection_scale / ONE;
        for (row = 0; row < WMAP_MAP_VERTICES; row++)
        {
            tile = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column];
            next = &g_wmap_current_frame->tiles.flat[(row + 1) * WMAP_MAP_TILES + column];
            coordinate = row * g_wmap_view.projection_scale / ONE + g_wmap_view.y;
            edge = coordinate + WMAP_MAP_TEXEL_TILE;
            if (wrapped != 0)
            {
                wrapped = 0;
                tile->v2 = tile->v3 = coordinate + (WMAP_MAP_TEXEL_TILE - WMAP_MAP_TEXEL_WRAP);
                tile->pad1 = WMAP_MAP_PAGE_Y_LOW;
            }
            else
            {
                if (edge < WMAP_MAP_PAGE_TEXELS)
                {
                    tile->pad1 = 0;
                }
                else
                {
                    tile->pad1 = WMAP_MAP_PAGE_Y_LOW;
                }
                tile->v2 = tile->v3 = edge;
            }
            if (edge < WMAP_MAP_TEXEL_EDGE)
            {
                next->pad1 = WMAP_MAP_PAGE_Y_LOW;
                next->v0 = next->v1 = edge;
            }
            else
            {
                next->pad1 = WMAP_MAP_PAGE_Y_LOW;
                if (edge >= WMAP_MAP_PAGE_TEXELS)
                {
                    next->v0 = next->v1 = edge;
                }
                else
                {
                    wrapped = 1;
                    next->v0 = next->v1 = edge - WMAP_MAP_TEXEL_WRAP;
                }
            }
        }
        tile = &g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column];
        coordinate = row * g_wmap_view.projection_scale / ONE + g_wmap_view.y;
        edge = coordinate + WMAP_MAP_TEXEL_TILE;
        if (wrapped != 0)
        {
            wrapped = 0;
            tile->v2 = tile->v3 = coordinate + (WMAP_MAP_TEXEL_TILE - WMAP_MAP_TEXEL_WRAP);
        }
        else
        {
            if (edge < WMAP_MAP_PAGE_TEXELS)
            {
                tile->pad1 = 0;
            }
            else
            {
                tile->pad1 = WMAP_MAP_PAGE_Y_LOW;
            }
            tile->v2 = tile->v3 = edge;
        }
    }
    for (row = WMAP_MAP_VISIBLE_FIRST; row < WMAP_MAP_VISIBLE_END; row++)
    {
        for (column = WMAP_MAP_VISIBLE_FIRST; column < WMAP_MAP_VISIBLE_END; column++)
        {
            g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column].tpage = getTPage(0, 0, g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column].pad2,
                                                                                         g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column].pad1);
        }
    }
}

/** @brief Run the zoom between the map view and the spirit view (started with triangle). */
void wmap_update_view_zoom(void)
{
    s32 in_y;
    s32 out_y;
    s32 in_scale;
    s32 out_scale;
    s32 mode;
    s32 in_x;
    s32 out_x;

    mode = g_wmap_view_mode;
    switch (mode)
    {
    case WMAP_VIEW_MODE_MAP:
        if ((g_wmap_buttons_repeat & PADRup) && (D_8011CF18 == 0) && (D_8011D4FC == -1))
        {
            g_wmap_view_mode = WMAP_VIEW_MODE_ZOOM_OUT;
            func_8005FF88(-1);
            g_wmap_saved_view = g_wmap_view;
            g_wmap_zoom_view.x = g_wmap_view.x * WMAP_ZOOM_ONE;
            g_wmap_zoom_view.y = g_wmap_view.y * WMAP_ZOOM_ONE;
            g_wmap_zoom_view.projection_scale = g_wmap_view.projection_scale * WMAP_ZOOM_ONE;
            g_wmap_zoom_step.x = -(g_wmap_view.x * (WMAP_ZOOM_ONE / WMAP_ZOOM_FRAMES));
            g_wmap_zoom_step.y = -(g_wmap_view.y * (WMAP_ZOOM_ONE / WMAP_ZOOM_FRAMES));
            g_wmap_zoom_step.projection_scale = WMAP_SPIRIT_VIEW_SCALE * (WMAP_ZOOM_ONE / WMAP_ZOOM_FRAMES) - g_wmap_view.projection_scale * (WMAP_ZOOM_ONE / WMAP_ZOOM_FRAMES);
            wmap_play_sound(WMAP_SOUND_ZOOM_OUT, WMAP_PAN_CENTER);
            g_wmap_map_button_mask = PADLleft | PADLright | PADselect | PADRup | PADRright;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
        }
        return;
    case WMAP_VIEW_MODE_SPIRITS:
        if (g_wmap_buttons_repeat & (PADRup | PADRright))
        {
            g_wmap_view_mode = WMAP_VIEW_MODE_ZOOM_IN;
            D_800D9268[0].target_shade = WMAP_ACTOR_SHADE_NEUTRAL;
            wmap_play_sound(WMAP_SOUND_ZOOM_IN, WMAP_PAN_CENTER);
            g_wmap_map_button_mask = -1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            return;
        }
        break;
    case WMAP_VIEW_MODE_ZOOM_IN:
        g_wmap_buttons_repeat = 0;
        in_x = g_wmap_zoom_view.x - g_wmap_zoom_step.x;
        g_wmap_zoom_view.x = in_x;
        in_y = g_wmap_zoom_view.y - g_wmap_zoom_step.y;
        in_scale = g_wmap_zoom_view.projection_scale - g_wmap_zoom_step.projection_scale;
        g_wmap_zoom_view.y = in_y;
        g_wmap_zoom_view.projection_scale = in_scale;
        g_wmap_view.x = in_x / WMAP_ZOOM_ONE;
        g_wmap_view.y = in_y / WMAP_ZOOM_ONE;
        g_wmap_view.projection_scale = in_scale / WMAP_ZOOM_ONE;
        if (in_scale == WMAP_VIEW_SCALE * WMAP_ZOOM_ONE)
        {
            g_wmap_view = g_wmap_saved_view;
            D_800DBE78 = 0;
            g_wmap_view_mode = WMAP_VIEW_MODE_MAP;
            D_800DBE70 = mode;
            return;
        }
        break;
    case WMAP_VIEW_MODE_ZOOM_OUT:
    {
        s32 target_scale = WMAP_SPIRIT_VIEW_SCALE;

        g_wmap_buttons_repeat = 0;
        out_x = g_wmap_zoom_view.x + g_wmap_zoom_step.x;
        g_wmap_zoom_view.x = out_x;
        out_y = g_wmap_zoom_view.y + g_wmap_zoom_step.y;
        out_scale = g_wmap_zoom_view.projection_scale + g_wmap_zoom_step.projection_scale;
        g_wmap_zoom_view.y = out_y;
        g_wmap_zoom_view.projection_scale = out_scale;
        g_wmap_view.x = out_x / WMAP_ZOOM_ONE;
        g_wmap_view.y = out_y / WMAP_ZOOM_ONE;
        g_wmap_view.projection_scale = out_scale / WMAP_ZOOM_ONE;
        if (out_scale == target_scale * WMAP_ZOOM_ONE)
        {
            g_wmap_view.x = 0;
            g_wmap_view.projection_scale = target_scale;
            D_800DBE78 = 0;
            g_wmap_view_mode = WMAP_VIEW_MODE_SPIRITS;
            g_wmap_view.y = 0;
            D_800DBE70 = 1;
        }
        break;
    }
    }
}

/**
 * @brief Set the color word (color and command byte) of every visible map tile.
 * @param color Packed color word, see WmapColor.
 */
static void wmap_set_map_color(s32 color)
{
    s32 row;
    s32 column;

    for (row = WMAP_MAP_VISIBLE_FIRST; row < WMAP_MAP_VISIBLE_END; row++)
    {
        for (column = WMAP_MAP_VISIBLE_FIRST; column < WMAP_MAP_VISIBLE_END; column++)
        {
            SET_BGR0_PACKED(&g_wmap_current_frame->tiles.flat[row * WMAP_MAP_TILES + column], color);
        }
    }
}

/**
 * @brief Start tinting the map toward a color.
 * @param color Target color, packed as in WmapColor; the command byte is ignored.
 * @return Always 1.
 */
s32 wmap_start_map_tint(s32 color)
{
    /* The packed word is reinterpreted as the color bytes. */
    g_wmap_tint_target = *(CVECTOR*)&color;
    D_80139288 = 1;
    wmap_install_callback(wmap_update_map_tint);
    return 1;
}

/**
 * @brief Spawn radial burst particles, move and draw them, and count the live ones.
 * @return Number of live particles.
 */
s32 wmap_update_burst_particles(void)
{
    s32 i;
    s32 active;
    s32 remaining;
    s32 random_value;
    WmapScreenPosition screen;
    WmapSpriteActor* actor;
    WmapBurstParticle* particle;
    s32 actor_index;

    active = 0;
    remaining = D_801B0FD0 * WMAP_BURST_SPAWNS_PER_UNIT;
    for (i = 0; i < D_801B0FD0 * WMAP_BURST_SLOTS_PER_UNIT; i++)
    {
        actor_index = i + WMAP_BURST_ACTOR_FIRST;
        particle = &D_801AFBD0[i];
        if (particle->active == 0)
        {
            actor = &D_800D9268[actor_index];
            if (g_wmap_burst_spawning != 0)
            {
                actor->unknown_02 = 0;
                actor->scale_index = WMAP_BURST_ACTOR_SCALE;
                actor->sequence = ((rand() * D_801B0FD0) >> 15) + 1;
                actor->previous_sequence = -1;
                actor->target_shade = WMAP_BURST_ACTOR_SHADE;
                actor->shade = WMAP_BURST_ACTOR_SHADE;
                particle->active = 1;
                particle->angle = (u32)rand() >> 3;
                particle->distance = 0;
                particle->speed = rand() * D_801B0FD0;
                random_value = rand();
                particle->lifetime = ((random_value * ((D_801B0FD0 * 5) << 2)) >> 15) + WMAP_BURST_LIFETIME_MIN;
                if (--remaining == 0)
                {
                    break;
                }
            }
        }
    }
    for (i = 0; i < WMAP_BURST_PARTICLES; i++)
    {
        particle = &D_801AFBD0[i];
        actor = &D_800D9268[WMAP_BURST_ACTOR_FIRST + i];
        if (particle->active != 0)
        {
            particle->distance += particle->speed;
            screen.point.x = ((particle->distance * ccos(particle->angle)) >> 24) + WMAP_BURST_CENTER_X;
            screen.point.y = ((particle->distance * csin(particle->angle)) >> 24) + WMAP_BURST_CENTER_Y;
            wmap_step_actor_animation(actor, &D_80139988[WMAP_BURST_ACTOR_FIRST + i]);
            if (D_800D9268[i + WMAP_BURST_ACTOR_FIRST].sequence == WMAP_BURST_SEQUENCE_LARGE)
            {
                wmap_draw_actor_sprite(actor, screen.packed, WMAP_BURST_TEXTURE_LARGE, WMAP_BURST_OT_INDEX, 0);
            }
            else
            {
                wmap_draw_actor_sprite(actor, screen.packed, WMAP_BURST_TEXTURE_SMALL, WMAP_BURST_OT_INDEX, 0);
            }
            particle->lifetime--;
            if (particle->lifetime == 0)
            {
                particle->active = 0;
            }
            active++;
        }
    }
    return active;
}

/** @brief Clear every burst particle, give its actor the default animation, and enable spawning. */
void wmap_init_burst_particles(void)
{
    s32 i;

    for (i = 0; i < WMAP_BURST_PARTICLES; i++)
    {
        D_801AFBD0[i].active = 0;
        D_80139988[WMAP_BURST_ACTOR_FIRST + i].data = D_8011D538;
    }
    D_801B0FD0 = 1;
    g_wmap_burst_spawning = 1;
}

/** @brief Land sequence step: scroll to the cursor cell and record the projected view origin. */
void wmap_begin_cell_focus(void)
{
    s16 screen_y;
    SVECTOR position;
    MATRIX matrix;
    WmapScreenPosition screen;

    D_8013B20C = 1;
    func_8006D8F0(1);
    func_8006D870(1);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_cell_focus_offsets[g_wmap_cursor_row][g_wmap_cursor_column].x;
    g_wmap_scroll_remaining_y = g_wmap_cell_focus_offsets[g_wmap_cursor_row][g_wmap_cursor_column].y;
    RotMatrix(&g_wmap_camera_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_camera_translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    position.vz = 0;
    position.vx = ((g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
    position.vy = ((g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&screen);
    screen_y = screen.point.y;
    D_800DBE70 = 1;
    D_800D9268[0].target_shade = 0;
    D_801B109C = 4;
    g_wmap_focus_origin_x = screen.point.x;
    g_wmap_focus_origin_y = screen_y;
    D_801B1098++;
}

/** @brief Land sequence step: project the focused land cell relative to the view and advance. */
void wmap_project_focus_position(void)
{
    MATRIX matrix;
    SVECTOR position;

    RotMatrix(&g_wmap_camera_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_camera_translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * WMAP_CELL_SIZE - g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
    position.vy = (((D_8011D530 - 1) * WMAP_CELL_SIZE - g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) / g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&D_8011CF4C);
    D_8013B20C = 0;
    D_801B1098++;
}
