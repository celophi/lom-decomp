#include "wsel.h"
#include "akao.h"
#include "akao_cmd.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "controller.h"
#include "saved_game.h"
#include "display.h"
#include "gpu_packet.h"
#include "pad.h"
#include "scene_state.h"
#include "sdk/libetc.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/memory.h"
#include "tim.h"

#define WSEL_FADE_NEUTRAL 0x100
#define WSEL_FADE_ADDITIVE_THRESHOLD (WSEL_FADE_NEUTRAL + 1)
#define WSEL_FADE_ADDITIVE_DRAW_MODE 0x25
#define WSEL_FADE_SUBTRACTIVE_DRAW_MODE 0x45
/** Fixed RAM buffer that CD resources are staged into before being unpacked. */
#define WSEL_LOAD_BUFFER ((u8*)0x80180000)
/** Offset table at the head of a staged music file: [0] sequence, [1] instrument bank. */
#define WSEL_LOAD_BUFFER_OFFSETS ((u32*)0x80180004)
#define WSEL_PAD_UNAVAILABLE 0xFE
#define WSEL_INDICATOR_ANCHOR_X 32
#define WSEL_INDICATOR_ANCHOR_Y 40
#define WSEL_INDICATOR_LABEL_Y 32
#define WSEL_INDICATOR_LABEL_U 184
#define WSEL_INDICATOR_LABEL_V 6
#define WSEL_INDICATOR_LABEL_WIDTH 32
#define WSEL_INDICATOR_LABEL_HEIGHT 10
#define WSEL_REPEAT_DELAY 2
#define WSEL_INITIAL_REPEAT_DELAY 15
#define WSEL_NON_REPEAT_BUTTON_MASK                                                                                                                            \
    (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1 | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_SELECT | PAD_BTN_L3 | PAD_BTN_START)
#define WSEL_NEXT_FADE_PRIMITIVE(primitive, type) ((WselFadePrimitive*)((u8*)(primitive) + sizeof(type)))
#define WSEL_MASK_SIZE 96
#define WSEL_MASK_MARGIN 11
#define WSEL_MASK_END (WSEL_MASK_MARGIN + WSEL_MASK_SIZE)
#define WSEL_MASK_MAX_SHADE 64
#define WSEL_MASK_FADE_STEP 4
#define WSEL_SPRITE_COUNT 8
#define WSEL_SHADOWED_SPRITE 2
#define WSEL_SPRITE_STRIP_WIDTH 128
#define WSEL_SPRITE_BAND_HEIGHT 256

#define WSEL_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3)
#define WSEL_FINAL_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3 | PAD_BTN_START)
#define WSEL_CANCEL_BUTTONS PAD_BTN_CIRCLE
#define WSEL_DPAD_BUTTONS (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT)

#define WSEL_SFX_ERROR 0x78
#define WSEL_SFX_CURSOR 0x7D
#define WSEL_SFX_CONFIRM 0x7E
#define WSEL_SFX_CANCEL 0x7F
#define WSEL_SFX_PAN_CENTER 0x80

/* Sprite layers in g_wsel_sprites. */
#define WSEL_SPRITE_LAND_MAP 0
#define WSEL_SPRITE_WORLD_MAP 1
#define WSEL_SPRITE_CURSOR 2
#define WSEL_SPRITE_WORLD_OVERLAY 3
#define WSEL_SPRITE_ZOOM_OVERLAY 7

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
#define WSEL_ZOOM_QUAD_COLOR 128
#define WSEL_LAND_FADE_FRAMES 16
#define WSEL_FULL_BRIGHTNESS 128
#define WSEL_LAND_FADE_STEP (WSEL_FULL_BRIGHTNESS / WSEL_LAND_FADE_FRAMES)

/** @brief Bit number of SAVED_OPTION_FLAG_3; the cancel test shifts rather than masks. */
#define WSEL_OPTION_FLAG_3_BIT 3

/** @brief Build an axis-aligned quad from the two corners of a rectangle. */
#define WSEL_QUAD_FROM_RECT(quad, rect)                                                                                                                        \
    ((quad).x0 = (quad).x2 = (rect).corners[0].x, (quad).x1 = (quad).x3 = (rect).corners[1].x, (quad).y0 = (quad).y1 = (rect).corners[0].y,                    \
     (quad).y2 = (quad).y3 = (rect).corners[1].y)

/**
 * @brief Column and row of the land grid cell under the cursor.
 * @note The map scroll must be read first (negated) to reproduce the original load order.
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

/** @brief Value returned by wsel_main in g_wsel_exit_state (0 keeps running). */
typedef enum
{
    WSEL_EXIT_CELL_CHOSEN = 1,
    WSEL_EXIT_CANCELLED = 3
} WselExit;

typedef struct
{
    s16 x;
    s16 y;
} WselPoint;

/** @brief Rectangle given by its top-left and bottom-right corners. */
typedef struct
{
    WselPoint corners[2];
} WselRect;

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

/** @brief Source cell and screen offset of an indicator frame, all in 8-pixel units. */
typedef struct
{
    u8 u;
    u8 v;
    u8 width;
    u8 height;
    u8 x_offset;
    u8 y_offset;
} WselIndicatorFrame;

typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
} WselFadeCurrent;

typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} WselFadeTarget;

typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} WselFadePrimitive;

extern s32 D_80042FB4;
extern u32 D_80043000;
extern u8 D_800435E0;
extern u8 g_wsel_tims_0[];
extern u8 g_wsel_tims_1[];
extern u8 g_wsel_tims_2[];
extern u8 g_wsel_tims_3[];
extern u8 g_wsel_tims_4[];
extern u8 g_wsel_tims_5[];
extern u8 g_wsel_tims_6[];
extern u8 g_wsel_tims_7[];
extern u8 g_wsel_cell_occupied[];
extern u8 g_wsel_cell_edges[WSEL_MAP_CELLS * WSEL_MAP_CELLS][WSEL_SUBCELLS * WSEL_SUBCELLS];
extern WselSprite g_wsel_sprites[WSEL_SPRITE_COUNT];
extern WselIndicatorFrame g_wsel_indicator_frame_default;
extern WselIndicatorFrame g_wsel_indicator_frame_alternate;
extern WselRenderBuffer* g_wsel_render_context;
extern u8 g_wsel_sound_bank;
extern WselFadeTarget g_wsel_fade_target;
extern WselFadeCurrent g_wsel_fade_current;
extern s32 g_wsel_buffer_index;
extern s32 g_wsel_exit_state;
extern s32 g_wsel_repeat_buttons;
extern s32 g_wsel_repeat_timer;
extern WselRect g_wsel_zoom_rect;
extern s32 g_wsel_buttons_held;
extern s32 g_wsel_transition_timer;
extern WselPoint g_wsel_map_scroll;
extern WselPoint g_wsel_cursor;
extern WselRect g_wsel_zoom_target;
extern WselPoint g_wsel_map_scroll_target;
extern WselPoint g_wsel_cursor_target;
extern s32 g_wsel_mask_shade;
extern s32 g_wsel_buttons_pressed;
extern s32 g_wsel_state;
extern s32 g_wsel_map_scroll_frames;
extern s32 g_wsel_cursor_frames;

static void wsel_run_loop(WselRenderBuffer* buffers);
/* Declared without a prototype: wsel_main calls it without an argument (see there). */
static void wsel_init();
static void wsel_load_sound_bank(s32 seq_variant);
static void wsel_stop_music(void);
static void wsel_start_music(void);
static void wsel_play_sfx(s32 sound_id, s32 pan);
static void wsel_reset_fade(void);
static void wsel_draw_fade(WselRenderBuffer* buffer);
static void wsel_set_fade_target(s32 red, s32 green, s32 blue, s32 steps);
static void wsel_draw_frame(WselRenderBuffer* buffer);
static void wsel_update_scroll(void);
static void* wsel_draw_selection_mask(TILE* tile, u_long* ot);
static void* wsel_draw_zoom_quad(POLY_FT4* poly, u_long* ot, WselQuadCoords* coords, s32 semi, s32 color);
static void* wsel_draw_cell_shading(void* prim, u_long* ot);
static void* wsel_draw_shade_tile(TILE* tile, u_long* ot, s32 x, s32 y, s32 intensity);
static void* wsel_draw_sprite(SPRT* prim, u_long* ot, s32 index);
static void wsel_update_input(void);
static POLY_FT4* wsel_draw_indicator(POLY_FT4* poly, u_long* ot, s32 which);
static void wsel_load_resources(void);
static void wsel_reset_scroll(void);
static void wsel_upload_tim(u8* tim_data, s32 index);
static s32 wsel_read_pad(void);
static void wsel_update_pad_repeat(void);
static void wsel_init_pad_repeat(void);

/**
 * @brief Initialize and run the WSEL overlay until an exit state is selected.
 * @param buffers The two display buffers, back to back.
 * @return Selected WSEL exit state.
 */
s32 wsel_main(WselRenderBuffer* buffers)
{
    SceneState* scene_state = SCENE_STATE;
    WselRenderBuffer* context; /* chained assignment through it sets the target schedule */

    g_wsel_render_context = context = buffers;
    g_wsel_buffer_index = 0;
    /* The original passes no argument; @p buffers is still in the first argument register. */
    wsel_init();

    scene_state->map_id = 0;
    scene_state->object_index = 0;
    scene_state->camera_x = 0;
    scene_state->camera_y = 0;
    scene_state->camera_z = 0;

    do
    {
        wsel_run_loop(context);
    } while (g_wsel_exit_state == 0);

    D_80042FB4 = VSync(-1);
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
    RECT unused_rect; /* never used, but the compiled frame size depends on it */

    buffer = buffers;
    ClearOTagR(buffers[0].ot, WSEL_OT_LENGTH);
    ClearOTagR(buffers[1].ot, WSEL_OT_LENGTH);
    VSync(0);
    PutDispEnv(&buffers[0].disp_env);
    update_controllers();
    SetDispMask(1);
    do
    {
        ot = buffer->ot;
        ClearOTagR(ot, WSEL_OT_LENGTH);
        buffer->prim_cursor = buffer->packets;
        VSync(1);
        wsel_draw_fade(buffer);
        wsel_draw_frame(buffer);
        wsel_update_input();
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        ClearImage(&buffer->clear_rect, 0, 0, 0);
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
        PutDispEnv(&buffer->disp_env);
        PutDrawEnv(&buffer->draw_env);
        DrawOTag(&ot[WSEL_OT_LENGTH - 1]);
        update_controllers();
        cdrom_process_state();
    } while (g_wsel_exit_state == 0);
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

    SetGeomScreen(1500);
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    buffers[0].clear_rect.x = 0;
    buffers[0].clear_rect.y = 0;
    buffers[0].clear_rect.w = SCREEN_WIDTH;
    buffers[0].clear_rect.h = SCREEN_HEIGHT;
    buffers[1].clear_rect.y = 232;
    buffers[1].clear_rect.x = 0;
    buffers[1].clear_rect.w = SCREEN_WIDTH;
    buffers[1].clear_rect.h = SCREEN_HEIGHT;

    setRECT(&vram_rect, 0, 0, VRAM_WIDTH, VRAM_HEIGHT);
    ClearImage(&vram_rect, 0, 0, 0);

    wsel_reset_fade();
    wsel_set_fade_target(WSEL_FADE_NEUTRAL, WSEL_FADE_NEUTRAL, WSEL_FADE_NEUTRAL, 20);
    SetDefDispEnv(&buffers[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&buffers[1].disp_env, 0, 232, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&buffers[0].draw_env, 0, 240, SCREEN_WIDTH, 224);
    SetDefDrawEnv(&buffers[1].draw_env, 0, 8, SCREEN_WIDTH, 224);

    buffers[1].draw_env.dtd = 0;
    buffers[0].draw_env.dtd = 0;
    wsel_load_resources();
    g_wsel_exit_state = 0;
}

/**
 * @brief Load a music file from CD and upload its sequence and instrument bank.
 * @param seq_variant Music-file index; 0 selects MSC_DATA.DAT.
 * @see TITLE load_title_seq
 */
static void wsel_load_sound_bank(s32 seq_variant)
{
    u32* offsets;
    u8* base;

    cdrom_queue_read(CD_RES_MUSIC_FILE(seq_variant), WSEL_LOAD_BUFFER);
    cdrom_wait_queue_empty();

    offsets = WSEL_LOAD_BUFFER_OFFSETS;
    base = WSEL_LOAD_BUFFER;

    bcopy(base + offsets[0], (u8*)&g_wsel_sound_bank, (s32)(offsets[1] - offsets[0]));
    akao_upload_bank_blocking((AkaoBankHeader*)(base + offsets[1]), 1);
}

/**
 * @brief Stop the WSEL background music.
 * @see TITLE stop_title_music
 */
static void wsel_stop_music(void)
{
    akao_stop_song(0);
}

/**
 * @brief Start the sequence loaded by wsel_load_sound_bank at full volume.
 * @see TITLE start_title_music
 */
static void wsel_start_music(void)
{
    akao_play_song((AkaoHeader*)&g_wsel_sound_bank);
    akao_set_song_volume(0, AKAO_VOLUME_MAX);
}

/**
 * @brief Play a WSEL sound effect at full volume.
 * @param sound_id Sound effect ID.
 * @param pan Stereo pan (0x80 is center).
 * @see TITLE play_title_sfx
 */
static void wsel_play_sfx(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, AKAO_VOLUME_MAX);
}

/**
 * @brief Reset the current and target fade colors to black with no fade in progress.
 * @see TITLE reset_fade_state
 */
static void wsel_reset_fade(void)
{
    g_wsel_fade_current.red = 0;
    g_wsel_fade_current.green = 0;
    g_wsel_fade_current.blue = 0;

    g_wsel_fade_target.red = 0;
    g_wsel_fade_target.green = 0;
    g_wsel_fade_target.blue = 0;
    g_wsel_fade_target.steps = 0;
}

/**
 * @brief Step the screen fade toward its target and queue the full-screen tint tile.
 * @param buffer Render buffer whose packet cursor and ordering table receive the tile.
 * @note Levels above WSEL_FADE_NEUTRAL brighten additively, levels below darken subtractively.
 * @see TITLE render_fade_overlay
 */
static void wsel_draw_fade(WselRenderBuffer* buffer)
{
    WselFadePrimitive* primitive = (WselFadePrimitive*)buffer->prim_cursor;
    u_long* ot = buffer->ot;
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 draw_mode;

    if (g_wsel_fade_target.steps != 0)
    {
        red_step = (g_wsel_fade_target.red - g_wsel_fade_current.red) / g_wsel_fade_target.steps;
        green_step = (g_wsel_fade_target.green - g_wsel_fade_current.green) / g_wsel_fade_target.steps;
        blue_step = (g_wsel_fade_target.blue - g_wsel_fade_current.blue) / g_wsel_fade_target.steps;
        g_wsel_fade_target.steps--;
        g_wsel_fade_current.red += red_step;
        g_wsel_fade_current.green += green_step;
        g_wsel_fade_current.blue += blue_step;
    }
    else
    {
        g_wsel_fade_current.red = g_wsel_fade_target.red;
        g_wsel_fade_current.green = g_wsel_fade_target.green;
        g_wsel_fade_current.blue = g_wsel_fade_target.blue;
    }
    if (!((g_wsel_fade_current.red == WSEL_FADE_NEUTRAL) && (g_wsel_fade_current.green == WSEL_FADE_NEUTRAL) &&
          (g_wsel_fade_current.blue == WSEL_FADE_NEUTRAL)))
    {
        if (g_wsel_fade_current.red >= WSEL_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = g_wsel_fade_current.red - 1;
            primitive->tile.g0 = g_wsel_fade_current.green - 1;
            primitive->tile.b0 = g_wsel_fade_current.blue - 1;
        }
        else
        {
            if (g_wsel_fade_current.red == WSEL_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~g_wsel_fade_current.red;
            }
            if (g_wsel_fade_current.green == WSEL_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~g_wsel_fade_current.green;
            }
            if (g_wsel_fade_current.blue == WSEL_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~g_wsel_fade_current.blue;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        SET_YX0(&primitive->tile, 0, 0);
        setWH(&primitive->tile, SCREEN_WIDTH, SCREEN_HEIGHT);
        addPrim(ot, &primitive->tile);

        draw_mode = WSEL_FADE_ADDITIVE_DRAW_MODE;
        primitive = WSEL_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (g_wsel_fade_current.red < WSEL_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = WSEL_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ot, &primitive->draw_mode);

        primitive = WSEL_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    buffer->prim_cursor = (u_long*)primitive;
}

/**
 * @brief Start a fade toward the given color levels.
 * @param red Target red level (WSEL_FADE_NEUTRAL is unchanged).
 * @param green Target green level.
 * @param blue Target blue level.
 * @param steps Number of frames over which to interpolate.
 * @see TITLE set_fade_target
 */
static void wsel_set_fade_target(s32 red, s32 green, s32 blue, s32 steps)
{
    g_wsel_fade_target.red = red;
    g_wsel_fade_target.green = green;
    g_wsel_fade_target.blue = blue;
    g_wsel_fade_target.steps = steps;
}

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

    prim = (u8*)buffer->prim_cursor;
    ot = buffer->ot;

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
                g_wsel_sprites[WSEL_SPRITE_LAND_MAP].blend_mode = 1;
                g_wsel_sprites[WSEL_SPRITE_CURSOR].semi_trans = 1;
                g_wsel_sprites[WSEL_SPRITE_CURSOR].blend_mode = 1;
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
        prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 0, WSEL_ZOOM_QUAD_COLOR);
        if (--g_wsel_transition_timer == 0)
        {
            g_wsel_state = WSEL_STATE_ZOOMED;
            g_wsel_transition_timer = WSEL_LAND_FADE_FRAMES;
        }
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_OVERLAY);
        prim = (u8*)wsel_draw_indicator((POLY_FT4*)prim, ot, D_800435E0 & 0x7F);
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
        prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 0, WSEL_ZOOM_QUAD_COLOR);
        if (--g_wsel_transition_timer == 0)
        {
            g_wsel_state = WSEL_STATE_WORLD_MAP;
            g_wsel_transition_timer = 0;
        }
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_OVERLAY);
        prim = (u8*)wsel_draw_indicator((POLY_FT4*)prim, ot, D_800435E0 & 0x7F);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_MAP);
        break;

    case WSEL_STATE_ZOOMED:
        for (i = 0; i < WSEL_RECT_CORNERS; i++)
        {
            g_wsel_zoom_rect.corners[i].x = g_wsel_zoom_target.corners[i].x;
            g_wsel_zoom_rect.corners[i].y = g_wsel_zoom_target.corners[i].y;
        }
        WSEL_QUAD_FROM_RECT(quad, g_wsel_zoom_rect);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_ZOOM_OVERLAY);
        prim = wsel_draw_zoom_quad((POLY_FT4*)prim, ot, &quad, 0, WSEL_ZOOM_QUAD_COLOR);
        if (g_wsel_buttons_held & WSEL_CONFIRM_BUTTONS)
        {
            wsel_play_sfx(WSEL_SFX_CONFIRM, WSEL_SFX_PAN_CENTER);
            g_wsel_state = WSEL_STATE_SELECT_CELL;
            g_wsel_transition_timer = WSEL_LAND_FADE_FRAMES;
        }
        /* fall through */
    case WSEL_STATE_WORLD_MAP:
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_OVERLAY);
        prim = (u8*)wsel_draw_indicator((POLY_FT4*)prim, ot, D_800435E0 & 0x7F);
        prim = wsel_draw_sprite((SPRT*)prim, ot, WSEL_SPRITE_WORLD_MAP);
        break;
    }
    buffer->prim_cursor = (u_long*)prim;
}

/**
 * @brief Step the map scroll and cursor toward their targets and position their sprites.
 * @note Each point covers the remaining distance evenly over its remaining frame count.
 */
static void wsel_update_scroll(void)
{
    WselPoint* scroll;
    WselPoint* cursor;
    s32 scroll_step_x;
    s32 scroll_step_y;
    s32 cursor_step_x;
    s32 cursor_step_y;

    if (g_wsel_map_scroll_frames != 0)
    {
        scroll = &g_wsel_map_scroll;
        scroll_step_x = (g_wsel_map_scroll_target.x - scroll->x) / g_wsel_map_scroll_frames;
        scroll_step_y = (g_wsel_map_scroll_target.y - scroll->y) / g_wsel_map_scroll_frames;
        g_wsel_map_scroll_frames--;
        scroll->x += scroll_step_x;
        scroll->y += scroll_step_y;
    }
    else
    {
        g_wsel_map_scroll.x = g_wsel_map_scroll_target.x;
        g_wsel_map_scroll.y = g_wsel_map_scroll_target.y;
    }

    if (g_wsel_cursor_frames != 0)
    {
        cursor = &g_wsel_cursor;
        cursor_step_x = (g_wsel_cursor_target.x - cursor->x) / g_wsel_cursor_frames;
        cursor_step_y = (g_wsel_cursor_target.y - cursor->y) / g_wsel_cursor_frames;
        g_wsel_cursor_frames--;
        cursor->x += cursor_step_x;
        cursor->y += cursor_step_y;
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
 * @see decomp.me (100%) https://decomp.me/scratch/f3i65
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
    setWH(tile, SCREEN_WIDTH, g_wsel_sprites[2].y + WSEL_MASK_MARGIN);
    addPrim(ot, tile);
    tile++;

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, 0, g_wsel_sprites[2].y + WSEL_MASK_END);
    setWH(tile, SCREEN_WIDTH, VRAM_DRAW_HEIGHT - tile->y0);
    addPrim(ot, tile);
    tile++;

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, 0, g_wsel_sprites[2].y + WSEL_MASK_MARGIN);
    setWH(tile, g_wsel_sprites[2].x + WSEL_MASK_MARGIN, WSEL_MASK_SIZE);
    addPrim(ot, tile);
    tile++;

    setTile(tile);
    tile->r0 = tile->g0 = tile->b0 = g_wsel_mask_shade;
    setSemiTrans(tile, 1);
    setXY0(tile, g_wsel_sprites[2].x + WSEL_MASK_END, g_wsel_sprites[2].y + WSEL_MASK_MARGIN);
    setWH(tile, SCREEN_WIDTH - tile->x0, WSEL_MASK_SIZE);
    addPrim(ot, tile);
    tile++;

    draw_mode = (DR_TPAGE*)tile;
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 2, 0, 0));
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
    poly->r0 = poly->g0 = poly->b0 = color;
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
    setClut(poly, g_wsel_sprites[1].clut_x, g_wsel_sprites[1].clut_y);
    setTPage(poly, 1, 1, g_wsel_sprites[1].tpage_x, g_wsel_sprites[1].tpage_y);
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
 *       bit 0x10 held the sub-cells without an edge (plus the neighbouring cells'
 *       shared border sub-cells) are brightened.
 * @see decomp.me (100%) TODO: no scratch link yet
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

    cell_x = (-g_wsel_map_scroll.x + g_wsel_cursor.x - WSEL_CELL_SIZE) / WSEL_CELL_SIZE;
    cell_y = (-g_wsel_map_scroll.y + g_wsel_cursor.y - WSEL_CELL_SIZE) / WSEL_CELL_SIZE;

    if (g_wsel_cell_occupied[WSEL_CELL_INDEX(cell_x, cell_y)] != 0)
    {
        for (row = 0; row < WSEL_SUBCELLS; row++)
        {
            for (col = 0; col < WSEL_SUBCELLS; col++)
            {
                p = wsel_draw_shade_tile((TILE*)p, ot, g_wsel_sprites[2].x + col * WSEL_CELL_SIZE + WSEL_MASK_MARGIN,
                                         g_wsel_sprites[2].y + row * WSEL_CELL_SIZE + WSEL_MASK_MARGIN, WSEL_SHADE_DARKEN);
            }
        }
    }
    else
    {
        SetDrawEnv((DR_ENV*)p, &g_wsel_render_context[g_wsel_buffer_index ^ 1].draw_env);
        addPrim(ot, p);
        p += sizeof(DR_ENV);

        scroll_x = (-g_wsel_map_scroll.x + g_wsel_cursor.x - WSEL_CELL_SIZE) % WSEL_CELL_SIZE;
        scroll_y = (-g_wsel_map_scroll.y + g_wsel_cursor.y - WSEL_CELL_SIZE) % WSEL_CELL_SIZE;

        if (g_wsel_buttons_held & 0x10)
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

        draw_x = g_wsel_sprites[2].x + WSEL_CLIP_INSET;
        draw_y = g_wsel_sprites[2].y + WSEL_CLIP_INSET + VRAM_BACK_DRAW_Y;
        if (g_wsel_buffer_index != 0)
        {
            draw_y = g_wsel_sprites[2].y + WSEL_CLIP_INSET + SCREEN_HEIGHT;
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
        tile->r0 = tile->g0 = tile->b0 = -intensity;
    }
    else
    {
        tile->r0 = tile->g0 = tile->b0 = intensity;
    }
    setXY0(tile, x, y);
    setWH(tile, WSEL_CELL_SIZE, WSEL_CELL_SIZE);
    setSemiTrans(tile, 1);
    addPrim(ot, tile);

    draw_mode = (DR_TPAGE*)(tile + 1);
    if (intensity < WSEL_SHADE_ADDITIVE)
    {
        /* Subtractive blending (B - F). */
        setDrawTPage(draw_mode, 0, 0, getTPage(0, 2, 0, 0));
    }
    else
    {
        /* Additive blending (B + F). */
        setDrawTPage(draw_mode, 0, 0, getTPage(0, 1, 0, 0));
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
    if (index == WSEL_SHADOWED_SPRITE)
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

    while (passes != 0)
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
            for (;;)
            {
                setSprt(prim);
                prim->r0 = prim->g0 = prim->b0 = brightness;
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
                             getTPage(sprite->tpage_mode, (index == WSEL_SHADOWED_SPRITE && passes == 1) ? 0 : sprite->blend_mode, tpage_x, tpage_y));
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
        x += 2;
        y += 2;
        passes--;
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
        if (g_wsel_buttons_pressed & WSEL_CONFIRM_BUTTONS)
        {
            wsel_play_sfx(WSEL_SFX_CONFIRM, WSEL_SFX_PAN_CENTER);
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
        /* D_80043000 is g_saved_game.layout.option_flags, which the original addresses directly. */
        if ((g_wsel_buttons_pressed & WSEL_CANCEL_BUTTONS) && !((D_80043000 >> WSEL_OPTION_FLAG_3_BIT) & 1))
        {
            wsel_play_sfx(WSEL_SFX_CANCEL, WSEL_SFX_PAN_CENTER);
            g_wsel_exit_state = WSEL_EXIT_CANCELLED;
        }
        return;

    case WSEL_STATE_SELECT_CELL:
        if (g_wsel_cursor_frames != 0 || g_wsel_map_scroll_frames != 0)
        {
            break;
        }
        if (g_wsel_buttons_pressed & WSEL_CANCEL_BUTTONS)
        {
            wsel_play_sfx(WSEL_SFX_CANCEL, WSEL_SFX_PAN_CENTER);
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
        else if (g_wsel_buttons_pressed & WSEL_CONFIRM_BUTTONS)
        {
            g_wsel_transition_timer = 0;
            g_wsel_sprites[WSEL_SPRITE_CURSOR].brightness = WSEL_FULL_BRIGHTNESS;
            g_wsel_sprites[WSEL_SPRITE_LAND_MAP].brightness = WSEL_FULL_BRIGHTNESS;
            g_wsel_sprites[WSEL_SPRITE_LAND_MAP].semi_trans = 0;
            g_wsel_sprites[WSEL_SPRITE_CURSOR].semi_trans = 1;
            column = WSEL_CURSOR_CELL_COLUMN();
            row = WSEL_CURSOR_CELL_ROW();
            if (g_wsel_cell_occupied[row * WSEL_MAP_CELLS + column] == 0)
            {
                wsel_play_sfx(WSEL_SFX_CONFIRM, WSEL_SFX_PAN_CENTER);
                g_wsel_state = WSEL_STATE_CONFIRM;
                g_wsel_mask_shade = 0;
                return;
            }
            wsel_play_sfx(WSEL_SFX_ERROR, WSEL_SFX_PAN_CENTER);
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
        if ((g_wsel_buttons_held & WSEL_DPAD_BUTTONS) && (g_wsel_cursor_frames != 0 || g_wsel_map_scroll_frames != 0))
        {
            wsel_play_sfx(WSEL_SFX_CURSOR, WSEL_SFX_PAN_CENTER);
            return;
        }
        break;

    case WSEL_STATE_CONFIRM:
        if (g_wsel_buttons_pressed & WSEL_FINAL_CONFIRM_BUTTONS)
        {
            wsel_play_sfx(WSEL_SFX_CONFIRM, WSEL_SFX_PAN_CENTER);
            column = WSEL_CURSOR_CELL_COLUMN();
            row = WSEL_CURSOR_CELL_ROW();
            g_saved_game.layout.world_map_cell = column + row * WSEL_MAP_CELLS;
            g_wsel_exit_state = WSEL_EXIT_CELL_CHOSEN;
            g_saved_game.layout.option_flags &= ~SAVED_OPTION_FLAG_3;
            return;
        }
        if (g_wsel_buttons_pressed & WSEL_CANCEL_BUTTONS)
        {
            wsel_play_sfx(WSEL_SFX_CURSOR, WSEL_SFX_PAN_CENTER);
            g_wsel_state = WSEL_STATE_SELECT_CELL;
        }
        break;
    }
}

/**
 * @brief Queue the selection indicator frame and its label as two textured quads.
 * @param poly Next free POLY_FT4 in the packet buffer.
 * @param ot Ordering-table entry the quads are linked into.
 * @param which Nonzero selects the alternate frame (sprite 5), zero the default (sprite 4).
 * @return The POLY_FT4 following the two queued quads.
 */
static POLY_FT4* wsel_draw_indicator(POLY_FT4* poly, u_long* ot, s32 which)
{
    WselIndicatorFrame* frame;
    WselSprite* sprite;

    if (which)
    {
        frame = &g_wsel_indicator_frame_alternate;
        sprite = &g_wsel_sprites[5];
    }
    else
    {
        frame = &g_wsel_indicator_frame_default;
        sprite = &g_wsel_sprites[4];
    }

    /* Indicator frame: source cell and offset are in 8-pixel units. */
    SET_BGR0_PACKED(poly, GPU_TINT_NEUTRAL);
    setPolyFT4(poly);
    setSemiTrans(poly, sprite->semi_trans);

    poly->x2 = poly->x0 = sprite->x + WSEL_INDICATOR_ANCHOR_X - frame->x_offset * 8;
    poly->y1 = poly->y0 = sprite->y + WSEL_INDICATOR_ANCHOR_Y - frame->y_offset * 8;
    poly->x1 = poly->x3 = poly->x0 + frame->width * 8 - 1;
    poly->y2 = poly->y3 = poly->y0 + frame->height * 8 - 1;

    poly->u0 = poly->u2 = frame->u * 8;
    poly->v1 = poly->v0 = frame->v * 8;
    poly->u1 = poly->u3 = poly->u0 + frame->width * 8 - 1;
    poly->v2 = poly->v3 = poly->v0 + frame->height * 8 - 1;

    setClut(poly, sprite->clut_x, sprite->clut_y);
    setTPage(poly, sprite->tpage_mode, sprite->blend_mode, sprite->tpage_x, sprite->tpage_y);
    addPrim(ot, poly);
    poly++;

    /* Label: a fixed 32x10 cell of sprite 6, placed 32 pixels below the frame sprite. */
    poly->x2 = poly->x0 = sprite->x;
    poly->y1 = poly->y0 = sprite->y + WSEL_INDICATOR_LABEL_Y;
    sprite = &g_wsel_sprites[6];
    /* setPolyFT4 split in two: the original stores the color word between setlen and setcode. */
    setlen(poly, 9);
    SET_BGR0_PACKED(poly, GPU_TINT_NEUTRAL);
    setcode(poly, 0x2C);
    setSemiTrans(poly, sprite->semi_trans);
    poly->u0 = poly->u2 = WSEL_INDICATOR_LABEL_U;
    poly->v1 = poly->v0 = WSEL_INDICATOR_LABEL_V;
    poly->x1 = poly->x3 = poly->x0 + WSEL_INDICATOR_LABEL_WIDTH;
    poly->y2 = poly->y3 = poly->y0 + WSEL_INDICATOR_LABEL_HEIGHT;
    poly->u1 = poly->u3 = poly->u0 + WSEL_INDICATOR_LABEL_WIDTH;
    poly->v2 = poly->v3 = poly->v0 + WSEL_INDICATOR_LABEL_HEIGHT;
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
    g_wsel_state = 0;
    wsel_reset_scroll();
    wsel_init_pad_repeat();
    wsel_upload_tim(g_wsel_tims_0, 0);
    wsel_upload_tim(g_wsel_tims_1, 1);
    wsel_upload_tim(g_wsel_tims_2, 2);
    wsel_upload_tim(g_wsel_tims_3, 3);
    wsel_upload_tim(g_wsel_tims_4, 4);
    wsel_upload_tim(g_wsel_tims_5, 5);
    wsel_upload_tim(g_wsel_tims_6, 6);
    wsel_upload_tim(g_wsel_tims_7, 7);
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
    WselSprite* sprites = g_wsel_sprites; /* separate base local keeps the target address order */
    WselSprite* sprite;
    Tim* tim;
    TimBlock* pixel_block;
    s16 image_x;
    s16 image_y;
    s16 clut_x;
    s16 clut_y;
    u32 clut_block_size;
    s32 header_size;

    sprite = &sprites[index];
    image_x = sprite->tpage_x;
    image_y = sprite->tpage_y;
    clut_x = sprite->clut_x;
    clut_y = sprite->clut_y;
    tim = (Tim*)tim_data;
    header_size = TIM_HEADER_SIZE;
    if ((u8)tim->flags & TIM_FLAG_HAS_CLUT)
    {
        /* The CLUT block address is formed from a runtime header size so it is
         * computed once and shared by the bnum read and the pixel-block advance. */
        clut_block_size = ((TimBlock*)(tim_data + header_size))->bnum;
        rect.w = tim->clut_block.dimensions.width * tim->clut_block.dimensions.height;
        rect.x = clut_x;
        rect.y = clut_y;
        rect.h = 1;
        LoadImage(&rect, (u_long*)tim->clut_data);
        tim_data = (tim_data + header_size) + clut_block_size;
    }
    else
    {
        /* Without a CLUT the pixel block directly follows the file header. */
        tim_data = tim_data + TIM_HEADER_SIZE;
    }
    pixel_block = (TimBlock*)tim_data;
    setRECT(&rect, image_x, image_y, pixel_block->dimensions.width, pixel_block->dimensions.height);
    LoadImage(&rect, (u_long*)(pixel_block + 1));
}

s32 wsel_read_pad(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s16 axis_x;
    s16 axis_y;
    u16 hi_read;
    u16 lo_read;

    if (regs->device_type >= WSEL_PAD_UNAVAILABLE)
    {
        return 0;
    }

    /* Read twice because the controller register may change asynchronously. */
    hi_read = regs->held_buttons;
    lo_read = regs->held_buttons;
    buttons = (hi_read >> 8) | (lo_read << 8);
    buttons = PAD_REMAP_FACE_BITS(buttons);
    if (regs->device_type != 0)
    {
        /* Convert signed analog-axis thresholds to digital directions. */
        axis_x = regs->axis_x.signed_value;
        if (axis_x < -1)
        {
            buttons |= PAD_BTN_LEFT;
        }
        else if (axis_x >= 2)
        {
            buttons |= PAD_BTN_RIGHT;
        }

        axis_y = regs->axis_y.signed_value;
        if (axis_y < -1)
        {
            buttons |= PAD_BTN_UP;
        }
        else if (axis_y >= 2)
        {
            buttons |= PAD_BTN_DOWN;
        }
    }
    return buttons;
}

/**
 * @brief Sample the controller and update the held, pressed, and key-repeat state.
 */
static void wsel_update_pad_repeat(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s16 axis_x;
    s16 axis_y;
    s32 sampled_buttons;
    s32 input_state;

    if (g_controller_device_type >= WSEL_PAD_UNAVAILABLE)
    {
        sampled_buttons = 0;
    }
    else
    {
        buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (regs->device_type != 0)
        {
            axis_x = regs->axis_x.signed_value;
            if (axis_x < -1)
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }

            axis_y = regs->axis_y.signed_value;
            if (axis_y < -1)
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis_y >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        sampled_buttons = buttons;
    }
    /* A separate variable from sampled_buttons; merging them drops a register copy. */
    input_state = sampled_buttons;

    g_wsel_buttons_held = input_state;
    g_wsel_buttons_pressed = 0;
    if (((input_state == g_wsel_repeat_buttons) || ((g_wsel_repeat_buttons != 0) && (input_state & (g_wsel_repeat_buttons | WSEL_NON_REPEAT_BUTTON_MASK)))) &&
        (input_state != 0))
    {
        /* Held input repeats directional buttons only. */
        if ((input_state & WSEL_DPAD_BUTTONS) != 0)
        {
            input_state &= WSEL_DPAD_BUTTONS;
        }
        if (g_wsel_repeat_timer == 0)
        {
            g_wsel_buttons_pressed = input_state;
            g_wsel_repeat_timer = WSEL_REPEAT_DELAY;
        }
        else
        {
            g_wsel_repeat_timer--;
            g_wsel_buttons_pressed = 0;
        }
    }
    else if (input_state == 0)
    {
        g_wsel_repeat_timer = 0;
        g_wsel_repeat_buttons = 0;
    }
    else
    {
        g_wsel_buttons_pressed = input_state;
        g_wsel_repeat_buttons = input_state;
        g_wsel_repeat_timer = WSEL_INITIAL_REPEAT_DELAY;
    }
}

/**
 * @brief Seed the key-repeat state from the current controller sample.
 */
static void wsel_init_pad_repeat(void)
{
    SCDRegs* regs = SCD_REGS;
    u32 buttons;
    s16 axis_x;
    s16 axis_y;
    s32 input_state;

    g_wsel_buttons_pressed = 0;
    if (g_controller_device_type >= WSEL_PAD_UNAVAILABLE)
    {
        input_state = 0;
    }
    else
    {
        buttons = (regs->held_buttons >> 8) | (regs->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (regs->device_type != 0)
        {
            axis_x = regs->axis_x.signed_value;
            if (axis_x < -1)
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis_x >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }

            axis_y = regs->axis_y.signed_value;
            if (axis_y < -1)
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis_y >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        input_state = buttons;
    }
    g_wsel_repeat_buttons = input_state;
    g_wsel_repeat_timer = WSEL_INITIAL_REPEAT_DELAY;
}
