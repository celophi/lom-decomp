#include "overlays/wsel/wsel.h"
#include "main/audio/akao.h"
#include "main/audio/akao_cmd.h"
#include "main/audio/game_audio.h"
#include "main/cd_resources.h"
#include "main/cdrom.h"
#include "main/controller.h"
#include "main/controller_internal.h"
#include "common/saved_game.h"
#include "main/display.h"
#include "common/gpu_packet.h"
#include "common/overlay_audio.h"
#include "common/pad.h"
#include "common/pad_repeat.h"
#include "common/screen_fade_words.h"
#include "main/scene_state.h"
#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>
#include <memory.h>
#include "common/tim.h"
#include "common/vector.h"

#define WSEL_HERO_ANCHOR_X 32
#define WSEL_HERO_ANCHOR_Y 40
#define WSEL_HERO_SHADOW_Y 32
#define WSEL_HERO_SHADOW_U 184
#define WSEL_HERO_SHADOW_V 6
#define WSEL_HERO_SHADOW_WIDTH 32
#define WSEL_HERO_SHADOW_HEIGHT 10
#define WSEL_MASK_SIZE 96
#define WSEL_MASK_MARGIN 11
#define WSEL_MASK_END (WSEL_MASK_MARGIN + WSEL_MASK_SIZE)
#define WSEL_MASK_MAX_SHADE 64
#define WSEL_MASK_FADE_STEP 4
#define WSEL_SPRITE_COUNT 8
/** Poses in each hero pose table: a standing pose, then one per weapon; only pose 0 is drawn. */
#define WSEL_HERO_POSE_COUNT 12
#define WSEL_SPRITE_STRIP_WIDTH 128
#define WSEL_SPRITE_BAND_HEIGHT 256
/** Distance of the cursor layer's black drop shadow below and right of the layer itself. */
#define WSEL_SHADOW_OFFSET 2
/** Pixel size of the unit that hero pose cells and offsets are measured in. */
#define WSEL_HERO_UNIT 8

#define WSEL_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3)
#define WSEL_FINAL_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3 | PAD_BTN_START)
#define WSEL_CANCEL_BUTTONS PAD_BTN_CIRCLE

/* Sprite layers in g_wsel_sprites; wsel_load_resources uploads TIM n to layer n. */
#define WSEL_SPRITE_LAND_MAP 0
#define WSEL_SPRITE_WORLD_MAP 1
#define WSEL_SPRITE_CURSOR 2
#define WSEL_SPRITE_WORLD_OVERLAY 3
#define WSEL_SPRITE_HERO_DEFAULT 4
#define WSEL_SPRITE_HERO_ALTERNATE 5
#define WSEL_SPRITE_HERO_SHADOW 6
#define WSEL_SPRITE_PROMPT 7

/* Land map grid: 19x19 cells of 16 pixels, one cell of border before the first. */
#define WSEL_MAP_CELLS 19
#define WSEL_CELL_SIZE 16
#define WSEL_SUBCELLS 6
#define WSEL_CELL_INDEX(x, y) ((x) + (y) * WSEL_MAP_CELLS)
#define WSEL_SUBCELL(row, col) ((row) * WSEL_SUBCELLS + (col))
#define WSEL_CLIP_INSET 12
#define WSEL_ZOOM_U0 96
#define WSEL_ZOOM_U1 224
#define WSEL_ZOOM_V0 48
#define WSEL_ZOOM_V1 168
#define WSEL_SHADE_ADDITIVE 256
#define WSEL_SHADE_DARKEN 160
#define WSEL_SHADE_LIGHTEN 304 /* additive; the tile colour keeps only the low byte */
#define WSEL_CURSOR_MOVE_FRAMES 4
#define WSEL_CURSOR_MIN 16
#define WSEL_CURSOR_MAX_X 208
#define WSEL_CURSOR_MAX_Y 112
#define WSEL_CURSOR_START_X 112
#define WSEL_CURSOR_START_Y 64
#define WSEL_MAP_SCROLL_MIN_X -96
#define WSEL_MAP_SCROLL_MIN_Y -192
#define WSEL_MAP_SCROLL_START_X -64
#define WSEL_MAP_SCROLL_START_Y -112

/* Zoom between the world-map region under the land map and the full screen. */
#define WSEL_RECT_CORNERS 2
#define WSEL_ZOOM_FRAMES 18
#define WSEL_ZOOM_SMALL_LEFT 96
#define WSEL_ZOOM_SMALL_TOP 48
#define WSEL_ZOOM_SMALL_RIGHT 224
#define WSEL_ZOOM_SMALL_BOTTOM 168
#define WSEL_ZOOM_LARGE_LEFT -46
#define WSEL_ZOOM_LARGE_TOP -90
#define WSEL_ZOOM_LARGE_RIGHT 338
#define WSEL_ZOOM_LARGE_BOTTOM 286
#define WSEL_LAND_FADE_FRAMES 16
#define WSEL_LAND_FADE_STEP (GPU_TINT_NEUTRAL_LEVEL / WSEL_LAND_FADE_FRAMES)

/** @brief Build an axis-aligned quad from the two corners of a rectangle. */
#define WSEL_QUAD_FROM_RECT(quad, rect)                                                                                                                        \
    ((quad).x0 = (quad).x2 = (rect).corners[0].x, (quad).x1 = (quad).x3 = (rect).corners[1].x, (quad).y0 = (quad).y1 = (rect).corners[0].y,                    \
     (quad).y2 = (quad).y3 = (rect).corners[1].y)

/**
 * @brief Column and row of the land grid cell under the cursor.
 */
#define WSEL_CURSOR_CELL_COLUMN() ((-g_wsel_map_scroll.x + g_wsel_cursor.x - WSEL_CELL_SIZE) / WSEL_CELL_SIZE)
#define WSEL_CURSOR_CELL_ROW() ((-g_wsel_map_scroll.y + g_wsel_cursor.y - WSEL_CELL_SIZE) / WSEL_CELL_SIZE)

/** @brief Top-level WSEL screen state held in g_wsel_state. */
typedef enum
{
    WSEL_STATE_WORLD_MAP = 0,   /**< World overview; confirm zooms in, cancel leaves. */
    WSEL_STATE_SELECT_CELL = 1, /**< Land map shown; the cursor picks a grid cell. */
    WSEL_STATE_ZOOM_IN = 2,     /**< Zooming the world map up to full screen. */
    WSEL_STATE_CONFIRM = 3,     /**< Selected cell masked; waiting for the final confirm. */
    WSEL_STATE_ZOOM_OUT = 4,    /**< Zooming back down to the world overview. */
    WSEL_STATE_ZOOMED = 5       /**< Zoom finished; overlay shown until confirm is held. */
} WselState;

/** @brief Value returned by wsel_main in g_wsel_exit_state; WSEL_EXIT_NONE keeps running. */
typedef enum
{
    WSEL_EXIT_NONE = 0,
    WSEL_EXIT_CELL_CHOSEN = 1,
    WSEL_EXIT_CANCELLED = 3
} WselExit;

/** @brief Rectangle given by its top-left and bottom-right corners. */
typedef struct
{
    Vec2s corners[2];
} WselRect;

/** @brief Four corners of a screen-space quadrilateral. */
typedef struct
{
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} WselQuadCoords;

/** @brief One sprite layer: texture page, CLUT, source rectangle, and screen position. */
typedef struct
{
    u8 tpage_mode;
    u8 blend_mode;
    u8 semi_trans;
    u8 brightness;
    u16 tpage_x;
    u16 tpage_y;
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
    u16 width;
    u16 height;
    u16 x;
    u16 y;
} WselSprite;

/** @brief Source cell and screen offset of one hero pose, all in WSEL_HERO_UNIT units. */
typedef struct
{
    u8 u;
    u8 v;
    u8 width;
    u8 height;
    u8 x_offset;
    u8 y_offset;
} WselHeroPose;

extern s32 g_playtime_vsync_origin;
extern u8 g_wsel_land_map_tim[];
extern u8 g_wsel_world_map_tim[];
extern u8 g_wsel_cursor_tim[];
extern u8 g_wsel_world_overlay_tim[];
extern u8 g_wsel_hero_tim_default[];
extern u8 g_wsel_hero_tim_alternate[];
extern u8 g_wsel_hero_shadow_tim[];
extern u8 g_wsel_prompt_tim[];
extern u8 g_wsel_cell_occupied[];
extern u8 g_wsel_cell_edges[WSEL_MAP_CELLS * WSEL_MAP_CELLS][WSEL_SUBCELLS * WSEL_SUBCELLS];
extern WselSprite g_wsel_sprites[WSEL_SPRITE_COUNT];
extern WselHeroPose g_wsel_hero_poses_default[WSEL_HERO_POSE_COUNT];
extern WselHeroPose g_wsel_hero_poses_alternate[WSEL_HERO_POSE_COUNT];
extern WselRenderBuffer* g_wsel_render_context;
extern s32 g_wsel_buffer_index;
extern s32 g_wsel_exit_state;
extern WselRect g_wsel_zoom_rect;
extern s32 g_wsel_buttons_held;
extern s32 g_wsel_transition_timer;
extern Vec2s g_wsel_map_scroll;
extern Vec2s g_wsel_cursor;
extern WselRect g_wsel_zoom_target;
extern Vec2s g_wsel_map_scroll_target;
extern Vec2s g_wsel_cursor_target;
extern s32 g_wsel_mask_shade;
extern s32 g_wsel_state;
extern s32 g_wsel_map_scroll_frames;
extern s32 g_wsel_cursor_frames;

static void wsel_run_loop(WselRenderBuffer* buffers);
static void wsel_init();
static void wsel_draw_frame(WselRenderBuffer* buffer);
static void wsel_update_scroll(void);
static void* wsel_draw_selection_mask(TILE* tile, u_long* ot);
static void* wsel_draw_zoom_quad(POLY_FT4* poly, u_long* ot, WselQuadCoords* coords, s32 semi, s32 color);
static void* wsel_draw_cell_shading(void* prim, u_long* ot);
static void* wsel_draw_shade_tile(TILE* tile, u_long* ot, s32 x, s32 y, s32 intensity);
static void* wsel_draw_sprite(SPRT* prim, u_long* ot, s32 index);
static void wsel_update_input(void);
static POLY_FT4* wsel_draw_hero(POLY_FT4* poly, u_long* ot, s32 alternate);
static void wsel_load_resources(void);
static void wsel_reset_scroll(void);
static void wsel_upload_tim(u8* tim_data, s32 index);
static void wsel_update_pad_repeat(void);

/**
 * @brief Initialize and run the WSEL overlay until an exit state is selected.
 * @param buffers The two display buffers, back to back.
 * @return Selected WSEL exit state.
 */
s32 wsel_main(WselRenderBuffer* buffers)
{
    SceneState* scene_state = SCENE_STATE;
    WselRenderBuffer* context;

    g_wsel_render_context = context = buffers;
    g_wsel_buffer_index = 0;
    wsel_init();

    scene_state->map_id = 0;
    scene_state->object_index = 0;
    scene_state->camera_x = 0;
    scene_state->camera_y = 0;
    scene_state->camera_z = 0;

    do
    {
        wsel_run_loop(context);
    } while (g_wsel_exit_state == WSEL_EXIT_NONE);

    g_playtime_vsync_origin = VSync(-1);
    return g_wsel_exit_state;
}

/**
 * @brief Draw and present frames, alternating display buffers, until an exit state is set.
 * @param buffers The two display buffers, back to back.
 */
static void wsel_run_loop(WselRenderBuffer* buffers)
{
    WselRenderBuffer* buffer;
    u_long* ot;
    s32 unused[2];

    buffer = buffers;
    ClearOTagR(buffers[0].screen.ot, FRAME_ORDERING_TABLE_LENGTH);
    ClearOTagR(buffers[1].screen.ot, FRAME_ORDERING_TABLE_LENGTH);
    VSync(0);
    PutDispEnv(&buffers[0].screen.disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        ot = buffer->screen.ot;
        ClearOTagR(ot, FRAME_ORDERING_TABLE_LENGTH);
        buffer->screen.prim_cursor = buffer->screen.packets;
        VSync(1);
        update_and_draw_fade(&buffer->screen);
        wsel_draw_frame(buffer);
        wsel_update_input();
        DrawSync(0);
        set_controller_vsync_interval(FRAME_VSYNC_INTERVAL);
        VSync(FRAME_VSYNC_INTERVAL);
        ClearImage(&buffer->screen.clear_rect, 0, 0, 0);
        if (buffer == buffers)
        {
            buffer = &buffers[1];
            g_wsel_buffer_index = 1;
        }
        else
        {
            buffer = buffers;
            g_wsel_buffer_index = 0;
        }
        PutDispEnv(&buffer->screen.disp_env);
        PutDrawEnv(&buffer->screen.draw_env);
        DrawOTag(&ot[FRAME_ORDERING_TABLE_LENGTH - 1]);
        update_controllers();
        cdrom_process_state();
    } while (g_wsel_exit_state == WSEL_EXIT_NONE);
    reset_controller_vsync_state();
    VSync(0);
}

/**
 * @brief Set up both display buffers, clear VRAM, start the fade-in, and load the resources.
 * @param buffers The two display buffers, back to back.
 */
static void wsel_init(WselRenderBuffer* buffers)
{
    RECT vram_rect;

    SetGeomScreen(SCREEN_PROJECTION_DISTANCE);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    buffers[0].screen.clear_rect.x = 0;
    buffers[0].screen.clear_rect.y = 0;
    buffers[0].screen.clear_rect.w = SCREEN_WIDTH;
    buffers[0].screen.clear_rect.h = SCREEN_HEIGHT;
    buffers[1].screen.clear_rect.y = VRAM_BACK_DISP_Y;
    buffers[1].screen.clear_rect.x = 0;
    buffers[1].screen.clear_rect.w = SCREEN_WIDTH;
    buffers[1].screen.clear_rect.h = SCREEN_HEIGHT;

    setRECT(&vram_rect, 0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    ClearImage(&vram_rect, 0, 0, 0);

    reset_fade_state();
    set_fade_target(FADE_NEUTRAL, FADE_NEUTRAL, FADE_NEUTRAL, SCREEN_FADE_FRAMES);
    SetDefDispEnv(&buffers[0].screen.disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&buffers[1].screen.disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&buffers[0].screen.draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&buffers[1].screen.draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    buffers[1].screen.draw_env.dtd = 0;
    buffers[0].screen.draw_env.dtd = 0;
    wsel_load_resources();
    g_wsel_exit_state = WSEL_EXIT_NONE;
}

#include "../../common/overlay_audio/load_song_from_disc.inc.c"

#include "../../common/overlay_audio/stop_song.inc.c"

#include "../../common/overlay_audio/play_loaded_song.inc.c"

#include "../../common/overlay_audio/play_sfx.inc.c"

#include "../../common/screen_fade_words/reset_fade_state.inc.c"

#include "../../common/screen_fade_words/update_and_draw_fade.inc.c"

#include "../../common/screen_fade_words/set_fade_target.inc.c"

/**
 * @brief Draw the current WSEL screen state and advance its zoom and fade transitions.
 * @param buffer Display buffer whose ordering table and packet area receive the frame.
 */
static void wsel_draw_frame(WselRenderBuffer* buffer)
{
    WselQuadCoords quad;
    s32 timer;
    s32 i;
    u8* prim;
    u_long* ot;

    prim = (u8*)buffer->screen.prim_cursor;
    ot = buffer->screen.ot;

    switch (g_wsel_state)
    {
    case WSEL_STATE_SELECT_CELL:
        /* Fade the land map and cursor in while the zoomed world map fades out. */
        if (g_wsel_transition_timer != 0)
        {
            timer = --g_wsel_transition_timer;
            g_wsel_sprites[WSEL_SPRITE_CURSOR].brightness = g_wsel_sprites[WSEL_SPRITE_LAND_MAP].brightness =
                (WSEL_LAND_FADE_FRAMES - timer) * WSEL_LAND_FADE_STEP;
            if (timer == 0)
            {
                g_wsel_sprites[WSEL_SPRITE_LAND_MAP].semi_trans = 0;
                g_wsel_sprites[WSEL_SPRITE_CURSOR].semi_trans = 1;
            }
            else
            {
                g_wsel_sprites[WSEL_SPRITE_LAND_MAP].semi_trans = 1;
                g_wsel_sprites[WSEL_SPRITE_LAND_MAP].blend_mode = GPU_BLEND_ADD;
                g_wsel_sprites[WSEL_SPRITE_CURSOR].semi_trans = 1;
                g_wsel_sprites[WSEL_SPRITE_CURSOR].blend_mode = GPU_BLEND_ADD;
            }
        }
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_CURSOR);
        prim = wsel_draw_cell_shading(prim, ot);
        if (g_wsel_transition_timer != 0)
        {
            WSEL_QUAD_FROM_RECT(quad, g_wsel_zoom_rect);
            prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 1, g_wsel_transition_timer * WSEL_LAND_FADE_STEP);
        }
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_LAND_MAP);
        wsel_update_scroll();
        break;

    case WSEL_STATE_ZOOM_IN:
        for (i = 0; i < WSEL_RECT_CORNERS; i++)
        {
            g_wsel_zoom_rect.corners[i].x += (g_wsel_zoom_target.corners[i].x - g_wsel_zoom_rect.corners[i].x) / g_wsel_transition_timer;
            g_wsel_zoom_rect.corners[i].y += (g_wsel_zoom_target.corners[i].y - g_wsel_zoom_rect.corners[i].y) / g_wsel_transition_timer;
        }
        WSEL_QUAD_FROM_RECT(quad, g_wsel_zoom_rect);
        prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 0, GPU_TINT_NEUTRAL_LEVEL);
        if (--g_wsel_transition_timer == 0)
        {
            g_wsel_state = WSEL_STATE_ZOOMED;
            g_wsel_transition_timer = WSEL_LAND_FADE_FRAMES;
        }
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_OVERLAY);
        prim = (u8*)wsel_draw_hero((POLY_FT4*)prim, ot, g_saved_game.layout.characters[FIELD_PARTY_HERO].info.bits.type);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_MAP);
        break;

    case WSEL_STATE_CONFIRM:
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_CURSOR);
        prim = wsel_draw_cell_shading(prim, ot);
        prim = wsel_draw_selection_mask((TILE*)prim, ot);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_LAND_MAP);
        wsel_update_scroll();
        break;

    case WSEL_STATE_ZOOM_OUT:
        for (i = 0; i < WSEL_RECT_CORNERS; i++)
        {
            g_wsel_zoom_rect.corners[i].x += (g_wsel_zoom_target.corners[i].x - g_wsel_zoom_rect.corners[i].x) / g_wsel_transition_timer;
            g_wsel_zoom_rect.corners[i].y += (g_wsel_zoom_target.corners[i].y - g_wsel_zoom_rect.corners[i].y) / g_wsel_transition_timer;
        }
        WSEL_QUAD_FROM_RECT(quad, g_wsel_zoom_rect);
        prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 0, GPU_TINT_NEUTRAL_LEVEL);
        if (--g_wsel_transition_timer == 0)
        {
            g_wsel_state = WSEL_STATE_WORLD_MAP;
            g_wsel_transition_timer = 0;
        }
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_OVERLAY);
        prim = (u8*)wsel_draw_hero((POLY_FT4*)prim, ot, g_saved_game.layout.characters[FIELD_PARTY_HERO].info.bits.type);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_MAP);
        break;

    case WSEL_STATE_ZOOMED:
        for (i = 0; i < WSEL_RECT_CORNERS; i++)
        {
            g_wsel_zoom_rect.corners[i].x = g_wsel_zoom_target.corners[i].x;
            g_wsel_zoom_rect.corners[i].y = g_wsel_zoom_target.corners[i].y;
        }
        WSEL_QUAD_FROM_RECT(quad, g_wsel_zoom_rect);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_PROMPT);
        prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 0, GPU_TINT_NEUTRAL_LEVEL);
        if (g_wsel_buttons_held & WSEL_CONFIRM_BUTTONS)
        {
            play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);
            g_wsel_state = WSEL_STATE_SELECT_CELL;
            g_wsel_transition_timer = WSEL_LAND_FADE_FRAMES;
        }
        /* fall through */
    case WSEL_STATE_WORLD_MAP:
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_OVERLAY);
        prim = (u8*)wsel_draw_hero((POLY_FT4*)prim, ot, g_saved_game.layout.characters[FIELD_PARTY_HERO].info.bits.type);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_MAP);
        break;
    }
    buffer->screen.prim_cursor = (u_long*)prim;
}

/**
 * @brief Step the map scroll and cursor toward their targets and position their sprites.
 * @note Each point covers the remaining distance evenly over its remaining frame count.
 */
static void wsel_update_scroll(void)
{
    if (g_wsel_map_scroll_frames != 0)
    {
        g_wsel_map_scroll.x += (g_wsel_map_scroll_target.x - g_wsel_map_scroll.x) / g_wsel_map_scroll_frames;
        g_wsel_map_scroll.y += (g_wsel_map_scroll_target.y - g_wsel_map_scroll.y) / g_wsel_map_scroll_frames;
        g_wsel_map_scroll_frames--;
    }
    else
    {
        g_wsel_map_scroll.x = g_wsel_map_scroll_target.x;
        g_wsel_map_scroll.y = g_wsel_map_scroll_target.y;
    }

    if (g_wsel_cursor_frames != 0)
    {
        g_wsel_cursor.x += (g_wsel_cursor_target.x - g_wsel_cursor.x) / g_wsel_cursor_frames;
        g_wsel_cursor.y += (g_wsel_cursor_target.y - g_wsel_cursor.y) / g_wsel_cursor_frames;
        g_wsel_cursor_frames--;
    }
    else
    {
        g_wsel_cursor.x = g_wsel_cursor_target.x;
        g_wsel_cursor.y = g_wsel_cursor_target.y;
    }

    g_wsel_sprites[WSEL_SPRITE_CURSOR].x = g_wsel_cursor.x - WSEL_MASK_MARGIN;
    g_wsel_sprites[WSEL_SPRITE_CURSOR].y = g_wsel_cursor.y - WSEL_MASK_MARGIN;
    g_wsel_sprites[WSEL_SPRITE_LAND_MAP].x = g_wsel_map_scroll.x;
    g_wsel_sprites[WSEL_SPRITE_LAND_MAP].y = g_wsel_map_scroll.y;
}

/**
 * @brief Fade a subtractive mask around the selected 96-pixel square.
 * @param tile Storage for four tiles and a draw-mode packet.
 * @param ot Ordering-table entry receiving the mask primitives.
 * @return Packet cursor immediately after the draw-mode packet.
 * @see decomp.me (100%) https://decomp.me/scratch/KjkRk
 */
static void* wsel_draw_selection_mask(TILE* tile, u_long* ot)
{
    DR_TPAGE* draw_mode;

    if (g_wsel_mask_shade < WSEL_MASK_MAX_SHADE)
    {
        g_wsel_mask_shade += WSEL_MASK_FADE_STEP;
    }

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, 0, 0);
    setWH(tile, SCREEN_WIDTH, g_wsel_sprites[WSEL_SPRITE_CURSOR].y + WSEL_MASK_MARGIN);
    addPrim(ot, tile);
    tile++;

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, 0, g_wsel_sprites[WSEL_SPRITE_CURSOR].y + WSEL_MASK_END);
    setWH(tile, SCREEN_WIDTH, VRAM_DRAW_HEIGHT - tile->y0);
    addPrim(ot, tile);
    tile++;

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, 0, g_wsel_sprites[WSEL_SPRITE_CURSOR].y + WSEL_MASK_MARGIN);
    setWH(tile, g_wsel_sprites[WSEL_SPRITE_CURSOR].x + WSEL_MASK_MARGIN, WSEL_MASK_SIZE);
    addPrim(ot, tile);
    tile++;

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, g_wsel_sprites[WSEL_SPRITE_CURSOR].x + WSEL_MASK_END, g_wsel_sprites[WSEL_SPRITE_CURSOR].y + WSEL_MASK_MARGIN);
    setWH(tile, SCREEN_WIDTH - tile->x0, WSEL_MASK_SIZE);
    addPrim(ot, tile);
    tile++;

    draw_mode = (DR_TPAGE*)tile;
    setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_SUBTRACT, 0, 0));
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Draw the zoomed map-preview quad textured from sprite layer 1.
 * @param poly Storage for the textured quad.
 * @param ot Ordering-table entry receiving the quad.
 * @param coords Screen-space corners of the quad.
 * @param semi Nonzero to draw the quad semi-transparent.
 * @param color Grey level applied to all three colour channels.
 * @return Packet cursor immediately after the quad.
 */
static void* wsel_draw_zoom_quad(POLY_FT4* poly, u_long* ot, WselQuadCoords* coords, s32 semi, s32 color)
{
    setPolyFT4(poly);
    SET_BGR0(poly, color, color, color);
    setSemiTrans(poly, semi);

    poly->x0 = coords->x0;
    poly->x1 = coords->x1;
    poly->x2 = coords->x2;
    poly->x3 = coords->x3;
    poly->y0 = coords->y0;
    poly->y1 = coords->y1;
    poly->y2 = coords->y2;
    poly->y3 = coords->y3;

    poly->u0 = poly->u2 = WSEL_ZOOM_U0;
    poly->u1 = poly->u3 = WSEL_ZOOM_U1;
    poly->v0 = poly->v1 = WSEL_ZOOM_V0;
    poly->v2 = poly->v3 = WSEL_ZOOM_V1;
    setClut(poly, g_wsel_sprites[WSEL_SPRITE_WORLD_MAP].clut_x, g_wsel_sprites[WSEL_SPRITE_WORLD_MAP].clut_y);
    setTPage(poly, 1, 1, g_wsel_sprites[WSEL_SPRITE_WORLD_MAP].tpage_x, g_wsel_sprites[WSEL_SPRITE_WORLD_MAP].tpage_y);
    addPrim(ot, poly);
    return poly + 1;
}

/**
 * @brief Shade the 6x6 sub-cells of the map cell under the selection cursor.
 * @param prim Next free primitive-packet address.
 * @param ot Ordering-table entry receiving the emitted primitives.
 * @return Next free packet address.
 * @note An occupied cell is darkened as a whole. Otherwise the shading is clipped to
 *       the selection square by a pair of draw-environment packets, and with button
 *       Square held the sub-cells without an edge (plus the neighbouring cells'
 *       shared border sub-cells) are brightened.
 */
static void* wsel_draw_cell_shading(void* prim, u_long* ot)
{
    DRAWENV draw_env;
    s32 cell_x;
    s32 cell_y;
    s32 scroll_x;
    s32 scroll_y;
    s32 draw_x;
    s32 draw_y;
    s32 row;
    s32 col;
    u8* p = prim;

    cell_x = WSEL_CURSOR_CELL_COLUMN();
    cell_y = WSEL_CURSOR_CELL_ROW();

    if (g_wsel_cell_occupied[WSEL_CELL_INDEX(cell_x, cell_y)] != 0)
    {
        for (row = 0; row < WSEL_SUBCELLS; row++)
        {
            for (col = 0; col < WSEL_SUBCELLS; col++)
            {
                p = wsel_draw_shade_tile((TILE*)p, ot, g_wsel_sprites[WSEL_SPRITE_CURSOR].x + col * WSEL_CELL_SIZE + WSEL_MASK_MARGIN,
                                         g_wsel_sprites[WSEL_SPRITE_CURSOR].y + row * WSEL_CELL_SIZE + WSEL_MASK_MARGIN, WSEL_SHADE_DARKEN);
            }
        }
    }
    else
    {
        SetDrawEnv((DR_ENV*)p, &g_wsel_render_context[g_wsel_buffer_index ^ 1].screen.draw_env);
        addPrim(ot, p);
        p += sizeof(DR_ENV);

        scroll_x = (-g_wsel_map_scroll.x + g_wsel_cursor.x - WSEL_CELL_SIZE) % WSEL_CELL_SIZE;
        scroll_y = (-g_wsel_map_scroll.y + g_wsel_cursor.y - WSEL_CELL_SIZE) % WSEL_CELL_SIZE;

        if (g_wsel_buttons_held & PAD_BTN_SQUARE)
        {
            for (row = 0; row < WSEL_SUBCELLS; row++)
            {
                for (col = 0; col < WSEL_SUBCELLS; col++)
                {
                    if (g_wsel_cell_edges[WSEL_CELL_INDEX(cell_x, cell_y)][WSEL_SUBCELL(row, col)] == 0)
                    {
                        p = wsel_draw_shade_tile((TILE*)p, ot, col * WSEL_CELL_SIZE - scroll_x, row * WSEL_CELL_SIZE - scroll_y, WSEL_SHADE_LIGHTEN);
                    }
                }
            }

            for (col = 0; col < WSEL_SUBCELLS; col++)
            {
                if (g_wsel_cell_edges[WSEL_CELL_INDEX(cell_x, cell_y + 1)][WSEL_SUBCELL(WSEL_SUBCELLS - 1, col)] == 0)
                {
                    p = wsel_draw_shade_tile((TILE*)p, ot, col * WSEL_CELL_SIZE - scroll_x, WSEL_MASK_SIZE - scroll_y, WSEL_SHADE_LIGHTEN);
                }
            }

            for (row = 0; row < WSEL_SUBCELLS; row++)
            {
                if (g_wsel_cell_edges[WSEL_CELL_INDEX(cell_x + 1, cell_y)][WSEL_SUBCELL(row, WSEL_SUBCELLS - 1)] == 0)
                {
                    p = wsel_draw_shade_tile((TILE*)p, ot, WSEL_MASK_SIZE - scroll_x, row * WSEL_CELL_SIZE - scroll_y, WSEL_SHADE_LIGHTEN);
                }
            }

            if (g_wsel_cell_edges[WSEL_CELL_INDEX(cell_x + 1, cell_y + 1)][WSEL_SUBCELL(WSEL_SUBCELLS - 1, WSEL_SUBCELLS - 1)] == 0)
            {
                p = wsel_draw_shade_tile((TILE*)p, ot, WSEL_MASK_SIZE - scroll_x, WSEL_MASK_SIZE - scroll_y, WSEL_SHADE_LIGHTEN);
            }
        }

        draw_x = g_wsel_sprites[WSEL_SPRITE_CURSOR].x + WSEL_CLIP_INSET;
        draw_y = g_wsel_sprites[WSEL_SPRITE_CURSOR].y + WSEL_CLIP_INSET + VRAM_BACK_DRAW_Y;
        if (g_wsel_buffer_index != 0)
        {
            draw_y = g_wsel_sprites[WSEL_SPRITE_CURSOR].y + WSEL_CLIP_INSET + SCREEN_HEIGHT;
        }
        SetDefDrawEnv(&draw_env, draw_x, draw_y, WSEL_MASK_SIZE, WSEL_MASK_SIZE);
        SetDrawEnv((DR_ENV*)p, &draw_env);
        addPrim(ot, p);
        p += sizeof(DR_ENV);
    }
    return p;
}

/**
 * @brief Draw one 16x16 shading tile and the draw-mode packet that blends it.
 * @param tile Storage for the tile followed by its draw-mode packet.
 * @param ot Ordering-table entry receiving both primitives.
 * @param x Screen x of the tile.
 * @param y Screen y of the tile.
 * @param intensity Below WSEL_SHADE_ADDITIVE, darken by this amount; otherwise brighten by its low byte.
 * @return Packet cursor immediately after the draw-mode packet.
 */
static void* wsel_draw_shade_tile(TILE* tile, u_long* ot, s32 x, s32 y, s32 intensity)
{
    DR_TPAGE* draw_mode;

    setTile(tile);
    if (intensity < WSEL_SHADE_ADDITIVE)
    {
        SET_BGR0(tile, -intensity, -intensity, -intensity);
    }
    else
    {
        SET_BGR0(tile, intensity, intensity, intensity);
    }
    setXY0(tile, x, y);
    setWH(tile, WSEL_CELL_SIZE, WSEL_CELL_SIZE);
    setSemiTrans(tile, 1);
    addPrim(ot, tile);

    draw_mode = (DR_TPAGE*)(tile + 1);
    if (intensity < WSEL_SHADE_ADDITIVE)
    {
        setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_SUBTRACT, 0, 0));
    }
    else
    {
        setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_ADD, 0, 0));
    }
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Draw one sprite layer as 128x256 SPRT tiles, each preceded by its texture page.
 * @param prim Next free primitive in the packet buffer.
 * @param ot Ordering table entry the primitives are linked into.
 * @param index Sprite layer in g_wsel_sprites; layer 2 also gets a black drop shadow.
 * @return Next free primitive after the ones written.
 */
static void* wsel_draw_sprite(SPRT* prim, u_long* ot, s32 index)
{
    WselSprite* sprite;
    DR_TPAGE* draw_mode;
    s32 passes;
    s32 x;
    s32 y;
    u8 brightness;
    s32 strip_x;
    s32 band_y;
    s32 width_left;
    s32 height_left;
    s32 strip_width;
    s32 band_height;
    s32 tpage_x;
    s32 tpage_y;
    s32 u;
    s32 v;

    sprite = &g_wsel_sprites[index];
    if (index == WSEL_SPRITE_CURSOR)
    {
        passes = 2;
        x = sprite->x - 1;
        y = sprite->y - 1;
    }
    else
    {
        passes = 1;
        x = sprite->x;
        y = sprite->y;
    }
    brightness = sprite->brightness;

    for (; passes != 0; passes--)
    {
        band_y = y;
        band_height = WSEL_SPRITE_BAND_HEIGHT;
        height_left = sprite->height;
        tpage_y = sprite->tpage_y;
        v = sprite->v;
        if (height_left <= WSEL_SPRITE_BAND_HEIGHT)
        {
            band_height = height_left;
        }
        do
        {
            strip_x = x;
            width_left = sprite->width;
            tpage_x = sprite->tpage_x;
            u = sprite->u;
            strip_width = WSEL_SPRITE_STRIP_WIDTH;
            if (width_left <= WSEL_SPRITE_STRIP_WIDTH)
            {
                strip_width = width_left;
            }
            while (1)
            {
                setSprt(prim);
                SET_BGR0(prim, brightness, brightness, brightness);
                setSemiTrans(prim, sprite->semi_trans);
                setXY0(prim, strip_x, band_y);
                setUV0(prim, u, v);
                setWH(prim, strip_width, band_height);
                prim->clut = getClut(sprite->clut_x, sprite->clut_y);
                addPrim(ot, prim);
                prim++;

                draw_mode = (DR_TPAGE*)prim;
                /* The shadow (second) pass of the shadowed layer is not blended. */
                setDrawTPage(draw_mode, 0, 0,
                             getTPage(sprite->tpage_mode, (index == WSEL_SPRITE_CURSOR && passes == 1) ? 0 : sprite->blend_mode, tpage_x, tpage_y));
                prim = (SPRT*)(draw_mode + 1);
                width_left -= strip_width;
                addPrim(ot, draw_mode);
                if (width_left == 0)
                {
                    break;
                }
                /* Next strip: 4-bit pages hold two strips (32 VRAM columns each). */
                u ^= WSEL_SPRITE_STRIP_WIDTH;
                if (sprite->tpage_mode == 0)
                {
                    tpage_x += 32;
                }
                else
                {
                    /* 8-bit: one strip fills a page, so start the next one at u = 0. */
                    tpage_x += 64;
                    u = 0;
                }
                strip_width = WSEL_SPRITE_STRIP_WIDTH;
                if (width_left <= WSEL_SPRITE_STRIP_WIDTH)
                {
                    strip_width = width_left;
                }
                strip_x += WSEL_SPRITE_STRIP_WIDTH;
            }
            height_left -= band_height;
            tpage_y += WSEL_SPRITE_BAND_HEIGHT;
            if (height_left != 0)
            {
                v = 0;
                band_height = WSEL_SPRITE_BAND_HEIGHT;
                if (height_left <= WSEL_SPRITE_BAND_HEIGHT)
                {
                    band_height = height_left;
                }
                band_y += WSEL_SPRITE_BAND_HEIGHT;
            }
        } while (height_left != 0);
        x += WSEL_SHADOW_OFFSET;
        y += WSEL_SHADOW_OFFSET;
        brightness = 0;
    }
    return prim;
}

/**
 * @brief Handle pad input for the current WSEL screen state.
 * @note Confirm on the world map zooms in; on the land map it picks a free cell,
 *       and a second confirm stores that cell and leaves the overlay.
 */
static void wsel_update_input(void)
{
    s32 column;
    s32 row;

    wsel_update_pad_repeat();
    switch (g_wsel_state)
    {
    case WSEL_STATE_WORLD_MAP:
        if (g_debounced_input & WSEL_CONFIRM_BUTTONS)
        {
            play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);
            wsel_reset_scroll();
            g_wsel_transition_timer = WSEL_ZOOM_FRAMES;
            g_wsel_zoom_rect.corners[0].x = WSEL_ZOOM_SMALL_LEFT;
            g_wsel_zoom_rect.corners[1].x = WSEL_ZOOM_SMALL_RIGHT;
            g_wsel_zoom_target.corners[0].x = WSEL_ZOOM_LARGE_LEFT;
            g_wsel_zoom_target.corners[1].x = WSEL_ZOOM_LARGE_RIGHT;
            g_wsel_zoom_rect.corners[0].y = WSEL_ZOOM_SMALL_TOP;
            g_wsel_zoom_rect.corners[1].y = WSEL_ZOOM_SMALL_BOTTOM;
            g_wsel_zoom_target.corners[0].y = WSEL_ZOOM_LARGE_TOP;
            g_wsel_zoom_target.corners[1].y = WSEL_ZOOM_LARGE_BOTTOM;
            g_wsel_state = WSEL_STATE_ZOOM_IN;
            return;
        }
        if ((g_debounced_input & WSEL_CANCEL_BUTTONS) && !g_saved_game.layout.options.bits.flag_3)
        {
            play_sfx(SFX_CANCEL, AKAO_PAN_CENTER);
            g_wsel_exit_state = WSEL_EXIT_CANCELLED;
        }
        return;

    case WSEL_STATE_SELECT_CELL:
        if (g_wsel_cursor_frames != 0 || g_wsel_map_scroll_frames != 0)
        {
            break;
        }
        if (g_debounced_input & WSEL_CANCEL_BUTTONS)
        {
            play_sfx(SFX_CANCEL, AKAO_PAN_CENTER);
            g_wsel_transition_timer = WSEL_ZOOM_FRAMES;
            g_wsel_zoom_target.corners[0].x = WSEL_ZOOM_SMALL_LEFT;
            g_wsel_zoom_target.corners[1].x = WSEL_ZOOM_SMALL_RIGHT;
            g_wsel_zoom_rect.corners[0].x = WSEL_ZOOM_LARGE_LEFT;
            g_wsel_zoom_rect.corners[1].x = WSEL_ZOOM_LARGE_RIGHT;
            g_wsel_zoom_target.corners[0].y = WSEL_ZOOM_SMALL_TOP;
            g_wsel_zoom_target.corners[1].y = WSEL_ZOOM_SMALL_BOTTOM;
            g_wsel_zoom_rect.corners[0].y = WSEL_ZOOM_LARGE_TOP;
            g_wsel_zoom_rect.corners[1].y = WSEL_ZOOM_LARGE_BOTTOM;
            g_wsel_state = WSEL_STATE_ZOOM_OUT;
        }
        else if (g_debounced_input & WSEL_CONFIRM_BUTTONS)
        {
            g_wsel_transition_timer = 0;
            g_wsel_sprites[WSEL_SPRITE_CURSOR].brightness = GPU_TINT_NEUTRAL_LEVEL;
            g_wsel_sprites[WSEL_SPRITE_LAND_MAP].brightness = GPU_TINT_NEUTRAL_LEVEL;
            g_wsel_sprites[WSEL_SPRITE_LAND_MAP].semi_trans = 0;
            g_wsel_sprites[WSEL_SPRITE_CURSOR].semi_trans = 1;
            column = WSEL_CURSOR_CELL_COLUMN();
            row = WSEL_CURSOR_CELL_ROW();
            if (g_wsel_cell_occupied[WSEL_CELL_INDEX(column, row)] == 0)
            {
                play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);
                g_wsel_state = WSEL_STATE_CONFIRM;
                g_wsel_mask_shade = 0;
                return;
            }
            play_sfx(SFX_ERROR, AKAO_PAN_CENTER);
            return;
        }

        /* The cursor stays near the middle while the map can still scroll. */
        if (g_wsel_buttons_held & PAD_BTN_UP)
        {
            if (g_wsel_cursor_target.y <= WSEL_CURSOR_START_Y && g_wsel_map_scroll_target.y < 0)
            {
                g_wsel_map_scroll_target.y += WSEL_CELL_SIZE;
                g_wsel_map_scroll_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
            else if (g_wsel_cursor_target.y > WSEL_CURSOR_MIN)
            {
                g_wsel_cursor_target.y -= WSEL_CELL_SIZE;
                g_wsel_cursor_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
        }
        if (g_wsel_buttons_held & PAD_BTN_DOWN)
        {
            if (g_wsel_cursor_target.y >= WSEL_CURSOR_START_Y && g_wsel_map_scroll_target.y > WSEL_MAP_SCROLL_MIN_Y)
            {
                g_wsel_map_scroll_target.y -= WSEL_CELL_SIZE;
                g_wsel_map_scroll_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
            else if (g_wsel_cursor_target.y < WSEL_CURSOR_MAX_Y)
            {
                g_wsel_cursor_target.y += WSEL_CELL_SIZE;
                g_wsel_cursor_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
        }
        if (g_wsel_buttons_held & PAD_BTN_LEFT)
        {
            if (g_wsel_cursor_target.x <= WSEL_CURSOR_START_X && g_wsel_map_scroll_target.x < 0)
            {
                g_wsel_map_scroll_target.x += WSEL_CELL_SIZE;
                g_wsel_map_scroll_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
            else if (g_wsel_cursor_target.x > WSEL_CURSOR_MIN)
            {
                g_wsel_cursor_target.x -= WSEL_CELL_SIZE;
                g_wsel_cursor_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
        }
        if (g_wsel_buttons_held & PAD_BTN_RIGHT)
        {
            if (g_wsel_cursor_target.x >= WSEL_CURSOR_START_X && g_wsel_map_scroll_target.x > WSEL_MAP_SCROLL_MIN_X)
            {
                g_wsel_map_scroll_target.x -= WSEL_CELL_SIZE;
                g_wsel_map_scroll_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
            else if (g_wsel_cursor_target.x < WSEL_CURSOR_MAX_X)
            {
                g_wsel_cursor_target.x += WSEL_CELL_SIZE;
                g_wsel_cursor_frames = WSEL_CURSOR_MOVE_FRAMES;
            }
        }
        if ((g_wsel_buttons_held & PAD_BTN_DPAD) && (g_wsel_cursor_frames != 0 || g_wsel_map_scroll_frames != 0))
        {
            play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
            return;
        }
        break;

    case WSEL_STATE_CONFIRM:
        if (g_debounced_input & WSEL_FINAL_CONFIRM_BUTTONS)
        {
            play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);
            column = WSEL_CURSOR_CELL_COLUMN();
            row = WSEL_CURSOR_CELL_ROW();
            g_saved_game.layout.world_map_cell = WSEL_CELL_INDEX(column, row);
            g_wsel_exit_state = WSEL_EXIT_CELL_CHOSEN;
            g_saved_game.layout.options.word &= ~SAVED_OPTION_FLAG_3;
            return;
        }
        if (g_debounced_input & WSEL_CANCEL_BUTTONS)
        {
            play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
            g_wsel_state = WSEL_STATE_SELECT_CELL;
        }
        break;
    }
}

/**
 * @brief Queue the hero's standing pose and its drop shadow as two textured quads.
 * @param poly Next free POLY_FT4 in the packet buffer.
 * @param ot Ordering-table entry the quads are linked into.
 * @param alternate Nonzero selects the alternate hero sheet (layer 5), zero the default (layer 4).
 * @return The POLY_FT4 following the two queued quads.
 */
static POLY_FT4* wsel_draw_hero(POLY_FT4* poly, u_long* ot, s32 alternate)
{
    WselHeroPose* frame;
    WselSprite* sprite;

    if (alternate)
    {
        frame = &g_wsel_hero_poses_alternate[0];
        sprite = &g_wsel_sprites[WSEL_SPRITE_HERO_ALTERNATE];
    }
    else
    {
        frame = &g_wsel_hero_poses_default[0];
        sprite = &g_wsel_sprites[WSEL_SPRITE_HERO_DEFAULT];
    }

    /* Standing pose. */
    SET_BGR0_PACKED(poly, GPU_TINT_NEUTRAL);
    setPolyFT4(poly);
    setSemiTrans(poly, sprite->semi_trans);

    poly->x2 = poly->x0 = sprite->x + WSEL_HERO_ANCHOR_X - frame->x_offset * WSEL_HERO_UNIT;
    poly->y1 = poly->y0 = sprite->y + WSEL_HERO_ANCHOR_Y - frame->y_offset * WSEL_HERO_UNIT;
    poly->x1 = poly->x3 = poly->x0 + frame->width * WSEL_HERO_UNIT - 1;
    poly->y2 = poly->y3 = poly->y0 + frame->height * WSEL_HERO_UNIT - 1;

    poly->u0 = poly->u2 = frame->u * WSEL_HERO_UNIT;
    poly->v1 = poly->v0 = frame->v * WSEL_HERO_UNIT;
    poly->u1 = poly->u3 = poly->u0 + frame->width * WSEL_HERO_UNIT - 1;
    poly->v2 = poly->v3 = poly->v0 + frame->height * WSEL_HERO_UNIT - 1;

    setClut(poly, sprite->clut_x, sprite->clut_y);
    setTPage(poly, sprite->tpage_mode, sprite->blend_mode, sprite->tpage_x, sprite->tpage_y);
    addPrim(ot, poly);
    poly++;

    /* Shadow: a fixed 32x10 cell of layer 6, placed 32 pixels below the hero layer's position. */
    poly->x2 = poly->x0 = sprite->x;
    poly->y1 = poly->y0 = sprite->y + WSEL_HERO_SHADOW_Y;
    sprite = &g_wsel_sprites[WSEL_SPRITE_HERO_SHADOW];
    SET_BGR0_PACKED(poly, GPU_TINT_NEUTRAL);
    setPolyFT4(poly);
    setSemiTrans(poly, sprite->semi_trans);
    poly->u0 = poly->u2 = WSEL_HERO_SHADOW_U;
    poly->v1 = poly->v0 = WSEL_HERO_SHADOW_V;
    poly->x1 = poly->x3 = poly->x0 + WSEL_HERO_SHADOW_WIDTH;
    poly->y2 = poly->y3 = poly->y0 + WSEL_HERO_SHADOW_HEIGHT;
    poly->u1 = poly->u3 = poly->u0 + WSEL_HERO_SHADOW_WIDTH;
    poly->v2 = poly->v3 = poly->v0 + WSEL_HERO_SHADOW_HEIGHT;
    setClut(poly, sprite->clut_x, sprite->clut_y);
    setTPage(poly, sprite->tpage_mode, sprite->blend_mode, sprite->tpage_x, sprite->tpage_y);
    addPrim(ot, poly);
    return poly + 1;
}

/**
 * @brief Reset the selection state and scroll, seed the pad repeat state, and upload all TIMs.
 */
static void wsel_load_resources(void)
{
    g_wsel_state = WSEL_STATE_WORLD_MAP;
    wsel_reset_scroll();
    init_pad_repeat();
    wsel_upload_tim(g_wsel_land_map_tim, WSEL_SPRITE_LAND_MAP);
    wsel_upload_tim(g_wsel_world_map_tim, WSEL_SPRITE_WORLD_MAP);
    wsel_upload_tim(g_wsel_cursor_tim, WSEL_SPRITE_CURSOR);
    wsel_upload_tim(g_wsel_world_overlay_tim, WSEL_SPRITE_WORLD_OVERLAY);
    wsel_upload_tim(g_wsel_hero_tim_default, WSEL_SPRITE_HERO_DEFAULT);
    wsel_upload_tim(g_wsel_hero_tim_alternate, WSEL_SPRITE_HERO_ALTERNATE);
    wsel_upload_tim(g_wsel_hero_shadow_tim, WSEL_SPRITE_HERO_SHADOW);
    wsel_upload_tim(g_wsel_prompt_tim, WSEL_SPRITE_PROMPT);
}

/**
 * @brief Put the land map scroll and the cursor back at their starting positions.
 */
static void wsel_reset_scroll(void)
{
    g_wsel_map_scroll_target.x = g_wsel_map_scroll.x = WSEL_MAP_SCROLL_START_X;
    g_wsel_map_scroll_target.y = g_wsel_map_scroll.y = WSEL_MAP_SCROLL_START_Y;
    g_wsel_map_scroll_frames = 0;
    g_wsel_cursor_target.x = g_wsel_cursor.x = WSEL_CURSOR_START_X;
    g_wsel_cursor_target.y = g_wsel_cursor.y = WSEL_CURSOR_START_Y;
    g_wsel_cursor_frames = 0;
    wsel_update_scroll();
}

/**
 * @brief Upload a TIM image (and its CLUT, if present) to the VRAM slots of a sprite layer.
 * @param tim_data TIM file data.
 * @param index Sprite layer whose tpage and CLUT coordinates receive the image.
 */
static void wsel_upload_tim(u8* tim_data, s32 index)
{
    RECT rect;
    TimPrefix* tim;
    TimBlock* pixel_block;
    TimBlock* clut_block;
    s16 image_x;
    s16 image_y;
    s16 clut_x;
    s16 clut_y;
    u32 clut_block_size;

    image_x = g_wsel_sprites[index].tpage_x;
    image_y = g_wsel_sprites[index].tpage_y;
    clut_x = g_wsel_sprites[index].clut_x;
    clut_y = g_wsel_sprites[index].clut_y;
    tim = (TimPrefix*)tim_data;
    if ((u8)tim->flags & TIM_FLAG_HAS_CLUT)
    {
        /* The CLUT block's length locates the pixel block that follows it. */
        clut_block = &tim->clut_block;
        clut_block_size = clut_block->bnum;
        rect.w = tim->clut_block.dimensions.width * tim->clut_block.dimensions.height;
        rect.x = clut_x;
        rect.y = clut_y;
        rect.h = 1;
        LoadImage(&rect, (u_long*)tim->clut_data);
        tim_data = (u8*)clut_block + clut_block_size;
    }
    else
    {
        /* Without a CLUT the pixel block directly follows the file header. */
        tim_data += TIM_HEADER_SIZE;
    }
    pixel_block = (TimBlock*)tim_data;
    setRECT(&rect, image_x, image_y, pixel_block->dimensions.width, pixel_block->dimensions.height);
    LoadImage(&rect, (u_long*)(pixel_block + 1));
}

#include "../../common/pad_repeat/read_pad_buttons.inc.c"

#include "../../common/pad_repeat/sample_pad_buttons.inc.c"

/**
 * @brief Sample the controller and update the held, pressed, and key-repeat state.
 */
static void wsel_update_pad_repeat(void)
{
    s32 input_state;

    g_wsel_buttons_held = sample_pad_buttons();
    input_state = g_wsel_buttons_held;
    g_debounced_input = 0;
    if (((input_state == g_last_input_state) || ((g_last_input_state != 0) && (input_state & (g_last_input_state | PAD_REPEAT_HOLD_BUTTONS)))) &&
        (input_state != 0))
    {
        /* Held input repeats directional buttons only. */
        if ((input_state & PAD_BTN_DPAD) != 0)
        {
            input_state &= PAD_BTN_DPAD;
        }
        if (g_input_repeat_timer == 0)
        {
            g_debounced_input = input_state;
            g_input_repeat_timer = PAD_REPEAT_INTERVAL;
        }
        else
        {
            g_input_repeat_timer--;
            g_debounced_input = 0;
        }
    }
    else if (input_state == 0)
    {
        g_input_repeat_timer = 0;
        g_last_input_state = 0;
    }
    else
    {
        g_debounced_input = input_state;
        g_last_input_state = input_state;
        g_input_repeat_timer = PAD_REPEAT_INITIAL_DELAY;
    }
}

#include "../../common/pad_repeat/init_pad_repeat.inc.c"
