#include "wmap_land_layout.h"
#include "wmap_land_transition.h"
#include "wmap_party_travel.h"
#include "wmap_map_display.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_land_preview.h"
#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/libetc.h"
#include "sdk/spadstk.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_backdrop.h"
#include "wmap_frame_render.h"
#include "display.h"
#include "pad.h"
#include "tim.h"
#include "controller_internal.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))

#define WMAP_GRID_SIZE 6
#define WMAP_MAP_CELL_SIZE 48
#define WMAP_TRAVEL_CELL_SIZE 160
#define WMAP_ACTOR_COUNT 256
#define WMAP_MENU_ITEM_COUNT 6
#define WMAP_MENU_ITEM_HEIGHT 16
#define WMAP_MENU_TRIANGLE_COUNT 28
#define WMAP_BACKDROP_WRAP_WIDTH 640
#define WMAP_BACKDROP_QUAD_WIDTH 160
#define WMAP_BACKDROP_COLOR_STEP 8
#define WMAP_FADE_COLOR_STEP 4

/** @brief Scratchpad word holding the world-map caller's saved stack pointer. */
#define WMAP_SCRATCH_STACK_SAVE_SLOT 0x1F8003F0

/** @brief Gouraud triangle used by the world-map menu cursor. */
typedef struct
{
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, pad1;
    short x1, y1;
    u_char r2, g2, b2, pad2;
    short x2, y2;
} WmapMenuTriangle;

/** @brief A transfer rectangle with the trailing words present in the image table. */
typedef struct
{
    RECT rect;
    s16 reserved[2];
} WmapTransferRect;

/** @brief Script words: duration/button pairs, or a command preceded by -2. */
typedef enum
{
    WMAP_SCRIPT_END = -1,
    WMAP_SCRIPT_COMMAND = -2,
    WMAP_SCRIPT_IMAGE = 0,
    WMAP_SCRIPT_BUTTON_MASK = 1,
    WMAP_SCRIPT_VISIBILITY = 2,
    WMAP_SCRIPT_WAIT_ARTIFACT = 3,
    WMAP_SCRIPT_WAIT_PLACEMENT = 4,
    WMAP_SCRIPT_WAIT_PREVIEW = 5,
    WMAP_SCRIPT_WAIT_TRAVEL = 6,
    WMAP_SCRIPT_WAIT_MAP_STATE = 7,
    WMAP_SCRIPT_FIND_HOME = 8
} WmapInputCommand;

/** @brief Direction in which the screen-covering polygon changes intensity. */
typedef enum
{
    WMAP_FADE_HOLD = 0,
    WMAP_FADE_INCREASE = 1,
    WMAP_FADE_DECREASE = 2,
    WMAP_FADE_DISABLED = 3
} WmapFadeMode;

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 land_id;
    s16 placement_allowed;
    s16 other_land;
    s32 spirit_sprites[8];
} WmapTile;

/** @brief Per-tile display state. */
typedef struct
{
    s16 unknown_00;
    s16 frame;
    u8 pad_04[24];
} WmapTileDisplay;

/** @brief Actor storage containing the map display mode. */
typedef struct
{
    u8 pad_00[0x26];
    s16 display_mode;
    u8 pad_28[4];
} WmapActor;

/** @brief Motion state for a world-map actor. */
typedef struct
{
    s16 state;
    u8 pad_02[0x12];
} WmapMotion;

s32 akao_cmd_f0();
s32 akao_cmd_f1();
extern void func_8005909C(void);
s32 func_8005B548();
s32 akao_play_sfx_from_buffer(s32, s32, s32, s32);
void cdrom_queue_read();
s32 cdrom_wait_queue_empty();
extern s32 func_800BFD18(s32 initialize);
extern void akao_cmd_c2(s32, s32, s32, s32);

extern s32 g_wmap_script_button_mask;
extern s32 g_wmap_script_word;
extern s32 g_wmap_script_delay;
extern s32 g_wmap_script_wait;
extern s32 g_wmap_input_script;
extern s32 g_wmap_cd_error;
extern s32 g_wmap_script_prompt_visible;
extern s32 g_wmap_menu_selection;
extern s32 g_wmap_menu_page;
extern s32 g_wmap_menu_cursor_y;
extern s32 g_wmap_map_controls_active;
extern s32 g_wmap_loaded_menu_page;
extern s32 g_wmap_backdrop_level;
extern s32 g_wmap_backdrop_scroll;
extern s32 g_wmap_small_motor;
extern s32 g_wmap_large_motor;

extern ControllerPortState* D_800D0454;
extern s32 D_800DCEDC;

extern RECT D_80051A80;
extern s32 D_800D0550;
extern s32 D_80182228;
extern s32 D_80182240;
extern u8 D_80139258;
extern s32 D_8005136C;
extern POLY_G4 D_800D06BC;
extern s32 D_800D9224;
extern s32 D_800D923C;
extern POLY_G4 D_800D9240;
extern s32 D_800DBE6C;
extern s32 D_800DBE70;
extern s32 D_800DBE74;
extern s32 D_800DBE78;
extern s32 D_800DCEC0;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCEFC;
extern u8 D_8010CF18[];
extern u8 D_80114F18[];
extern s32 D_8011CF18;
extern s32 D_8011CF20;
extern s32 D_8011CF44;
extern s32 D_8011CF58;
extern VECTOR D_8011CF60;
extern s32 D_8011CF70;
extern s32 D_8011CF74;
extern s32 D_8011CF7C;
extern s32 D_8011D4F8;
extern s32 D_8011D4FC;
extern s32 D_80129540;
extern CVECTOR D_80129548;
extern s32 D_8012954C;
extern s32 D_80139218;
extern s32 D_80139224;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_80139248;
extern SVECTOR D_80139278;
extern s32 D_80139280;
extern s32 D_801398B0;
extern s32 D_801398B8;
extern s32 D_801398BC;
extern u8 D_801398C8;
extern s32 D_801398D0;
extern s32 D_801398F4;
extern s32 D_80139900;
extern VECTOR D_80139950;
extern s32 D_80139960;
extern s32 D_80139978;
extern s32 D_8013997C;
extern s32 D_8013B208;
extern s32 D_8013B258;
extern DVECTOR D_8013B260;
extern s32 D_8013B28C;
extern s32 D_8013B290;
extern s32 D_8013B294;
extern s32 D_8013B27C;
extern VECTOR D_80182D48;
extern s32 D_80182D70;
extern s32 D_80182D88;
extern VECTOR D_80182DC0;
extern u32 D_80182DD8;
extern s32 D_80182E00;
extern s32 D_80182E1C;
extern s32 D_80182E34;
extern s32 D_80182E3C;
extern s32 D_8019D6D8;
extern s32 D_801ADAE0;
extern s32 D_801ADAE8;
extern s32 D_801ADAF0;
extern s32 D_801ADAFC;
extern CVECTOR D_801ADB04;
extern s32 D_801ADB90;
extern POLY_FT4 g_wmap_backdrop_front_quads[4];
extern POLY_FT4 g_wmap_backdrop_back_quads[4];

extern u8 D_80182D74;
extern u8 D_80182D75;
extern u8 D_80182D76;
extern u8 D_80182D80;
extern u8 D_80182D81;
extern u8 D_80182D82;
extern u8 D_80182D8C;
extern u8 D_80182D8D;
extern u8 D_80182D8E;
extern u8 D_80182D94;
extern u8 D_80182D95;
extern u8 D_80182D96;
extern POLY_FT4 D_800D0694;
extern u8 D_800D0698;
extern s32 D_800CB204;
extern s32 D_800CB224;
extern s32 D_800CB23C;
extern s32 D_800CB254;
extern SPRT D_800D06E4;
extern WmapMenuTriangle g_wmap_menu_triangles[WMAP_MENU_TRIANGLE_COUNT];
extern u8 D_8019D6E0;
extern s32 D_80182DE0;
extern s16* g_wmap_input_scripts[];
extern WmapTile D_80139290[6][6];
extern s32 D_800CB248;
extern u8 D_800D0A08[];
extern s32 D_800D9228;
extern s32 D_800D9238;
extern s32 D_800DBE68;
extern s32 D_800DBE7C;
extern u8 D_800DCF18[];
extern u16 D_800DCF2C[];
extern s32 D_8011D0E0;
extern s32 D_8011D0E4;
extern s32 D_8011D500;
extern s8 D_80129538[5];
extern u8 D_801295BC[];
extern s32 D_80139238;
extern s32 D_80139834;
extern s32 D_8013986C;
extern VECTOR D_80139870;
extern VECTOR D_80139888;
extern u32 D_801398F8;
extern u8 D_80139908[];
extern u16 D_8013B210;
extern s32 D_8013B234;
extern SVECTOR D_8013B238;
extern SVECTOR D_8013B240;
extern s32 D_8013B24C;
extern u16 D_8013B2A0;
extern u16 D_8013C628;
extern s32 D_80182D5C;
extern s32 D_80182DD4;
extern s32 D_80182E38;
extern s32 D_801ADAF8;
extern s32 D_801ADB08;
extern u16 D_801ADBA0;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;
extern RECT D_80051A88;
extern WmapTileDisplay D_8011D108[6][6];
extern s32 D_80139830;
extern s32 D_80182E20;
extern WmapActor D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern s32 D_8011D52C;
extern s32 D_80129550;

void wmap_init_frame_buffers(void);
void wmap_init_state(void);
s32 wmap_draw_backdrop(s32 initialize);
s32 wmap_update_screen_fade(s32 initialize);
void wmap_update_menu(s32 buttons);
void wmap_wait_for_script_event(void);
void wmap_step_input_script(void);
s32 wmap_run_loop(void);
void wmap_read_controller(void);
void wmap_refresh_cells(void);

/** @brief Configure the two world-map frame buffers and clear their ordering tables. */
void wmap_init_frame_buffers(void)
{
    SetGeomOffset(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    D_800DCEDC = 0x4000;
    SetGeomScreen(0x4000);
    SetDispMask(1);
    SetDefDrawEnv(&g_wmap_frames[0].draw_env, 0, 8, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_wmap_frames[1].draw_env, 0, 248, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDispEnv(&g_wmap_frames[0].disp_env, 0, 240, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_wmap_frames[1].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    g_wmap_frames[1].draw_env.clip.x = 8;
    g_wmap_frames[0].draw_env.clip.x = 8;
    g_wmap_frames[1].draw_env.clip.w = 312;
    g_wmap_frames[0].draw_env.clip.w = 312;
    ClearOTagR(g_wmap_frames[0].ordering_table, WMAP_OT_COUNT);
    ClearOTagR(g_wmap_frames[1].ordering_table, WMAP_OT_COUNT);
}

/** @brief Run the world-map overlay and return its next game state.
 * @return Next top-level game state.
 */
s32 run_world_map(void)
{
    RECT clear_rect;

    clear_rect = D_80051A80;
    akao_cmd_f0();
    akao_cmd_f1();
    wmap_init_frame_buffers();
    g_wmap_frames[0].draw_env.isbg = 0;
    g_wmap_frames[1].draw_env.isbg = 0;
    D_800D0550 = wmap_run_loop();
    akao_cmd_f0();
    akao_cmd_f1();
    if (D_800D0550 == 2)
    {
        DrawSync(0);
        VSync(0);
        SetDispMask(0);
        ResetGraph(0);
        ClearImage(&clear_rect, 0, 0, 0);
        return 2;
    }
    if (D_80182228 != 0)
    {
        return 9;
    }
    if (D_80182240 != 0)
    {
        return 10;
    }
    return D_800D0550;
}

/** @brief Initialize world-map state, resource slots, and packet templates. */
void wmap_init_state(void)
{
    s32 transition_mode;
    D_80182DD8 = 0;
    D_80139224 = 1;
    D_80182240 = 0;
    D_80182228 = 0;
    D_80139978 = -1;
    D_8013997C = 0;
    D_8011CF20 = 0;
    D_800DBE6C = -1;
    D_800DBE74 = 0;
    D_801ADAF0 = 0;
    D_8012954C = 0;
    D_800D923C = 0;
    D_800D9240 = D_800D06BC;
    D_80139228 = 0;
    D_8019D6D8 = 0;
    g_wmap_cd_error = 0;
    D_80182E3C = -1;
    D_8011D4F8 = 0;
    D_80139248 = 0;
    D_80129540 = 0;
    D_80139900 = 0;
    D_801ADB90 = 1;
    D_801398B0 = 0;
    D_80182E1C = 0;
    D_801398B8 = 0;
    D_80182E00 = 0xFF;
    D_800DCEC0 = 1;
    D_800D9224 = 0;
    g_wmap_information_groups = 0;
    D_80139280 = 0x1F800000;
    g_wmap_party_visible = 1;
    D_801ADAFC = 1;
    D_8011CF7C = 1;
    g_wmap_script_word = 0;
    g_wmap_input_script = 0;
    g_wmap_preview_bob_frame = 0;
    g_wmap_script_delay = 1;
    g_wmap_map_button_mask = -1;
    g_wmap_script_button_mask = -1;
    g_wmap_game_start_delay = 60;
    D_801398BC = 0;
    D_8013B28C = 0;
    D_8013B258 = 0;
    g_wmap_frames[0].packet_cursor = D_8010CF18;
    g_wmap_frames[1].packet_cursor = D_80114F18;
    D_801ADAE8 = 0x40;
    g_wmap_input_locked = 1;
    D_8011CF44 = 0;
    D_800DCEFC = 0;
    D_80182D88 = 0;
    func_8006D8F0(0);
    func_800653EC();
    wmap_init_spirit_animation();
    func_800654F8();
    D_8011CF74 = 0;
    D_80139218 = 0;
    g_wmap_party_cell_dirty = -1;
    D_8013B290 = -1;
    wmap_init_land_image_cache();
    func_80058298();
    wmap_init_land_display();
    func_8006CD18();
    func_8005909C();
    g_wmap_spirit_brightness = 0;
    g_wmap_spirit_target_brightness = 0x80;
    g_wmap_map_controls_active = 1;
    g_wmap_script_prompt_visible = 0;
    g_wmap_menu_selection = 1;
    g_wmap_menu_cursor_y = 0;
    g_wmap_menu_page = 0;
    g_wmap_loaded_menu_page = -1;
    func_800605B4();
    func_8006D870(0);
    D_801ADB04.r = 0x80;
    D_801ADB04.g = 0x80;
    D_801ADB04.b = 0x80;
    g_wmap_backdrop_level = 1;
    g_wmap_backdrop_target_level = 0;
    D_8013B294 = 0;
    D_80139950.vx = 0;
    D_80139950.vy = 0;
    D_80139950.vz = 0x6000;
    D_80139278.vx = 0x2E0;
    D_80139278.vz = 0x1B0;
    D_80139278.vy = 0;
    D_80182DC0.vx = 8;
    D_80182DC0.vy = -0x18;
    D_80182DC0.vz = 0x6D60;
    D_80139244 = 0;
    D_8011CF70 = 0;
    D_80182D70 = 0;
    D_8011CF58 = 0;
    D_80139960 = 0;
    D_80182D48 = D_8011CF60;
    *(SVECTOR*)&D_801398C8 = *(SVECTOR*)&D_80139258;
    D_8011CF18 = 0;
    D_8013B208 = 0;
    D_801ADAE0 = 0;
    D_801398D0 = 0;
    D_800DCEEC = 1;
    D_800DCEF0 = 1;
    g_wmap_preview_travel_end = D_8005136C;
    g_wmap_preview_travel_frame = D_8005136C;
    g_wmap_screen_fade_mode = 0;
    transition_mode = 2;
    D_800DBE78 = 0;
    D_800DBE70 = transition_mode;
    D_801398F4 = 0;
    D_8011D4FC = -1;
    g_wmap_preview_artifact_visible = 0;
    g_wmap_artifact_transfer_end = 0;
    g_wmap_artifact_transfer_frame = 0;
    g_wmap_preview_shape = 0;
    g_wmap_preview_texture = 0;
    g_wmap_preview_draw_y = 0;
    g_wmap_preview_draw_x = 0;
    g_wmap_preview_y = 0;
    g_wmap_preview_x = 0;
    D_80182E34 = 0;
    D_8013B260.vy = 0;
    D_80129548.r = 0;
    D_8013B260.vx = 0;
    D_80129548.g = 0;
    D_80129548.b = 0;
    SetPolyFT4(g_wmap_preview_saved_quads);
    /* Clear each packed (x, y) pair before copying the packet templates. */
    *(s32*)&g_wmap_preview_saved_quads[0].x2 = 0;
    *(s32*)&g_wmap_preview_saved_quads[0].x1 = 0;
    *(s32*)&g_wmap_preview_saved_quads[0].x0 = 0;
    g_wmap_preview_saved_quads[1] = g_wmap_preview_saved_quads[2] =
        g_wmap_preview_saved_quads[3] = g_wmap_preview_saved_quads[0];
    g_wmap_travel_day = wmap_get_day();
    wmap_refresh_cells();
    func_8005B548();
}

/**
 * @brief Fade and scroll the textured backdrop, or draw its color gradient.
 * @param initialize Callback initialization flag; unused.
 * @return One to keep the callback active.
 */
s32 wmap_draw_backdrop(s32 initialize)
{
    CVECTOR color;
    POLY_FT4* packet;
    POLY_G4* gradient;
    s16 wrapped_x;
    s32 front_x;
    s32 back_x;
    s32 wrapped_back_x;
    s32 strip;

    if (g_wmap_backdrop_level != g_wmap_backdrop_target_level)
    {
        if (g_wmap_backdrop_target_level < g_wmap_backdrop_level)
        {
            g_wmap_backdrop_level--;
        }
        else
        {
            g_wmap_backdrop_level++;
        }
    }
    if (g_wmap_backdrop_level != 0)
    {
        if (g_wmap_map_controls_active != 0)
        {
            color.r = g_wmap_backdrop_level * D_801ADB04.r / 16;
            color.g = g_wmap_backdrop_level * D_801ADB04.g / 16;
            color.b = g_wmap_backdrop_level * D_801ADB04.b / 16;
        }
        else
        {
            color.r = g_wmap_backdrop_level * D_801ADB04.r / 24;
            color.g = g_wmap_backdrop_level * D_801ADB04.g / 24;
            color.b = g_wmap_backdrop_level * D_801ADB04.b / 24;
        }
        color.cd = 0x2C;
        /* Each strip has two copies so it wraps across the screen. */
        if ((D_8011CF44 == 0) && (g_wmap_map_controls_active != 0))
        {
            for (strip = 0; strip < 4; strip++)
            {
                packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
                *packet = g_wmap_backdrop_front_quads[strip];
                front_x = packet->x0 + SCREEN_WIDTH;
                wrapped_x = (front_x + g_wmap_backdrop_scroll) % WMAP_BACKDROP_WRAP_WIDTH;
                packet->x0 = packet->x2 = wrapped_x - SCREEN_WIDTH;
                packet->x1 = packet->x3 = wrapped_x - WMAP_BACKDROP_QUAD_WIDTH;
                do
            {
                *(u_long*)&packet->r0 = *(u_long*)&color;
            } while (0);
                packet->code |= 2;
                addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
                packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
                *packet = g_wmap_backdrop_front_quads[strip];
                wrapped_x = (packet->x0 + g_wmap_backdrop_scroll) % WMAP_BACKDROP_WRAP_WIDTH;
                packet->x0 = packet->x2 = wrapped_x - SCREEN_WIDTH;
                packet->x1 = packet->x3 = wrapped_x - WMAP_BACKDROP_QUAD_WIDTH;
                do
            {
                *(u_long*)&packet->r0 = *(u_long*)&color;
            } while (0);
                packet->code |= 2;
                addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
            if (!(D_8011CF74 & 3))
            {
                g_wmap_backdrop_scroll = (g_wmap_backdrop_scroll + 1) & 0x7FFF;
            }
        }
        for (strip = 0; strip < 4; strip++)
        {
            packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
            *packet = g_wmap_backdrop_back_quads[strip];
            back_x = packet->x0 + 0x8140;
            wrapped_x = (back_x - g_wmap_backdrop_scroll) % WMAP_BACKDROP_WRAP_WIDTH;
            packet->x0 = packet->x2 = wrapped_x - SCREEN_WIDTH;
            packet->x1 = packet->x3 = wrapped_x - WMAP_BACKDROP_QUAD_WIDTH;
            do
            {
                *(u_long*)&packet->r0 = *(u_long*)&color;
            } while (0);
            addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_FT4);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
            }
            packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
            *packet = g_wmap_backdrop_back_quads[strip];
            wrapped_back_x = packet->x0 + 0x8000;
            wrapped_x = (wrapped_back_x - g_wmap_backdrop_scroll) % WMAP_BACKDROP_WRAP_WIDTH;
            packet->x0 = packet->x2 = wrapped_x - SCREEN_WIDTH;
            packet->x1 = packet->x3 = wrapped_x - WMAP_BACKDROP_QUAD_WIDTH;
            do
            {
                *(u_long*)&packet->r0 = *(u_long*)&color;
            } while (0);
            addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_FT4);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
            }
        }
        return 1;
    }
    /* The gradient takes over once the textured backdrop has faded away. */
    if (D_80182D74 != (D_800D9240.r0 & 0xFF))
    {
        if ((D_800D9240.r0 & 0xFF) >= (u8)D_80182D74)
        {
            D_800D9240.r0 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.r0 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D75 != (D_800D9240.g0 & 0xFF))
    {
        if ((D_800D9240.g0 & 0xFF) >= (u8)D_80182D75)
        {
            D_800D9240.g0 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.g0 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D76 != (D_800D9240.b0 & 0xFF))
    {
        if ((D_800D9240.b0 & 0xFF) >= (u8)D_80182D76)
        {
            D_800D9240.b0 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.b0 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D80 != (D_800D9240.r1 & 0xFF))
    {
        if ((D_800D9240.r1 & 0xFF) >= (u8)D_80182D80)
        {
            D_800D9240.r1 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.r1 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D81 != (D_800D9240.g1 & 0xFF))
    {
        if ((D_800D9240.g1 & 0xFF) >= (u8)D_80182D81)
        {
            D_800D9240.g1 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.g1 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D82 != (D_800D9240.b1 & 0xFF))
    {
        if ((D_800D9240.b1 & 0xFF) >= (u8)D_80182D82)
        {
            D_800D9240.b1 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.b1 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D8C != (D_800D9240.r2 & 0xFF))
    {
        if ((D_800D9240.r2 & 0xFF) >= (u8)D_80182D8C)
        {
            D_800D9240.r2 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.r2 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D8D != (D_800D9240.g2 & 0xFF))
    {
        if ((D_800D9240.g2 & 0xFF) >= (u8)D_80182D8D)
        {
            D_800D9240.g2 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.g2 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D8E != (D_800D9240.b2 & 0xFF))
    {
        if ((D_800D9240.b2 & 0xFF) >= (u8)D_80182D8E)
        {
            D_800D9240.b2 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.b2 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D94 != (D_800D9240.r3 & 0xFF))
    {
        if ((D_800D9240.r3 & 0xFF) >= (u8)D_80182D94)
        {
            D_800D9240.r3 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.r3 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D95 != (D_800D9240.g3 & 0xFF))
    {
        if ((D_800D9240.g3 & 0xFF) >= (u8)D_80182D95)
        {
            D_800D9240.g3 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.g3 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    if (D_80182D96 != (D_800D9240.b3 & 0xFF))
    {
        if ((D_800D9240.b3 & 0xFF) >= (u8)D_80182D96)
        {
            D_800D9240.b3 -= WMAP_BACKDROP_COLOR_STEP;
        }
        else
        {
            D_800D9240.b3 += WMAP_BACKDROP_COLOR_STEP;
        }
    }
    gradient = (POLY_G4*)g_wmap_current_frame->packet_cursor;
    *gradient = D_800D9240;
    addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
    if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
    {
        g_wmap_packet_bytes += sizeof(POLY_G4);
        g_wmap_current_frame->packet_cursor += sizeof(POLY_G4);
    }
    return 1;
}

/**
 * @brief Advance the screen-covering fade and submit its polygon.
 * @param initialize Callback initialization flag; unused.
 * @return Zero when disabled, otherwise one to keep the callback active.
 */
s32 wmap_update_screen_fade(s32 initialize)
{
    POLY_FT4* packet;
    u8 intensity;

    switch (g_wmap_screen_fade_mode)
    {
    case WMAP_FADE_DECREASE:
        intensity = D_800D0694.b0 - WMAP_FADE_COLOR_STEP;
        D_800D0694.b0 = intensity;
        D_800D0694.g0 = intensity;
        D_800D0694.r0 = intensity;
        if (intensity == 0)
        {
            g_wmap_screen_fade_mode = WMAP_FADE_HOLD;
        }
        else
        {
            goto draw;
        }
        break;
    case WMAP_FADE_INCREASE:
        intensity = D_800D0694.b0 + WMAP_FADE_COLOR_STEP;
        D_800D0694.b0 = intensity;
        D_800D0694.g0 = intensity;
        D_800D0694.r0 = intensity;
        if (intensity == 128)
        {
            g_wmap_screen_fade_mode = WMAP_FADE_HOLD;
        }
        break;
    case WMAP_FADE_DISABLED:
        return 0;
    }
    if (D_800D0698 != 0)
    {
    draw:
        packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
        *packet = D_800D0694;
        if (D_800D0698 != 128)
        {
            packet->code |= 2;
        }
        addPrim(&g_wmap_current_frame->ordering_table[176], g_wmap_current_frame->packet_cursor);
        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(POLY_FT4);
            g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
        }
    }
    return 1;
}

/**
 * @brief Upload one TIM palette or pixel block to its VRAM rectangle.
 * @param block TIM block header followed by its image data.
 */
static inline void wmap_load_image_block(TimBlock* block)
{
    RECT rect;

    rect = *(RECT*)&block->dx;
    LoadImage(&rect, (u_long*)(block + 1));
}

/**
 * @brief Handle the Select menu and draw its cursor and current page.
 * @param buttons Raw controller buttons; unused. Input comes from the map button globals.
 */
void wmap_update_menu(s32 buttons)
{
    TimBlock* image_block;
    TimPrefix* image;
    WmapMenuTriangle* triangle_template;
    s32 previous_item;
    s32 map_controls_active;
    s32 cursor_target_y;
    s32 next_item;
    s32 triangle_index;
    WmapMenuTriangle* triangle;
    SPRT* sprite;

    if (g_wmap_buttons_repeat & PADselect)
    {
        if (g_wmap_menu_page == 0)
        {
            map_controls_active = g_wmap_map_controls_active == 0;
            g_wmap_map_controls_active = map_controls_active;
            if (map_controls_active != 0)
            {
                akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
            }
            else
            {
                akao_play_sfx_from_buffer(D_800CB23C, 0, 0x80, 0x7F);
            }
        }
        else
        {
            g_wmap_menu_page = 0;
            akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
        }
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
    }
    if (g_wmap_map_controls_active == 0)
    {
        previous_item = g_wmap_menu_selection - 1;
        cursor_target_y = previous_item * WMAP_MENU_ITEM_HEIGHT;
        if (cursor_target_y != g_wmap_menu_cursor_y)
        {
            if (cursor_target_y < g_wmap_menu_cursor_y)
            {
                g_wmap_menu_cursor_y -= 4;
            }
            else
            {
                g_wmap_menu_cursor_y += 4;
            }
            goto draw_menu;
        }
        if (g_wmap_menu_page == 0)
        {
            if (g_wmap_buttons_repeat & PADLup)
            {
                g_wmap_menu_selection = previous_item;
                if (previous_item <= 0)
                {
                    g_wmap_menu_selection = WMAP_MENU_ITEM_COUNT;
                }
                akao_play_sfx_from_buffer(D_800CB204, 0, 0x80, 0x7F);
            }
            if (g_wmap_buttons_repeat & PADLdown)
            {
                next_item = g_wmap_menu_selection + 1;
                g_wmap_menu_selection = next_item;
                if (next_item > WMAP_MENU_ITEM_COUNT)
                {
                    g_wmap_menu_selection = 1;
                }
                akao_play_sfx_from_buffer(D_800CB204, 0, 0x80, 0x7F);
            }
        }
        if (g_wmap_buttons_repeat & PADRdown)
        {
            if (g_wmap_menu_page == 0)
            {
                g_wmap_menu_page = g_wmap_menu_selection;
                akao_play_sfx_from_buffer(D_800CB254, 0, 0x80, 0x7F);
            }
        }
        if (g_wmap_buttons_repeat & PADRright)
        {
            if (g_wmap_menu_page == 0)
            {
                g_wmap_map_controls_active = g_wmap_map_controls_active == 0;
                akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
                g_wmap_buttons_held = 0;
                g_wmap_buttons_repeat = 0;
                return;
            }
            g_wmap_menu_page = 0;
            akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
            goto draw_menu;
        }
    draw_menu:
        if (g_wmap_loaded_menu_page != g_wmap_menu_page)
        {
            image = (TimPrefix*)&D_8019D6E0;
            g_wmap_loaded_menu_page = g_wmap_menu_page;
            cdrom_queue_read(((u16)g_wmap_menu_page + 0x114B) & 0xFFFF, image);
            image_block = &image->clut_block;
            cdrom_wait_queue_empty();
            wmap_load_image_block(&image->clut_block);
            image_block = (TimBlock*)((u8*)image_block + image->clut_block.bnum);
            wmap_load_image_block(image_block);
            DrawSync(0);
            D_801ADAFC = 1;
        }
        if (g_wmap_menu_page == 0)
        {
            triangle_index = 0;
            do
            {
                triangle_template = &g_wmap_menu_triangles[triangle_index];
                triangle = (WmapMenuTriangle*)g_wmap_current_frame->packet_cursor;
                *triangle = *triangle_template;
                triangle->y0 = (u16)(triangle->y0 + (u16)g_wmap_menu_cursor_y);
                triangle->y1 = (u16)(triangle->y1 + (u16)g_wmap_menu_cursor_y);
                triangle->y2 = (u16)(triangle->y2 + (u16)g_wmap_menu_cursor_y);
                addPrim(&g_wmap_current_frame->ordering_table[1], triangle);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(WmapMenuTriangle);
                    g_wmap_current_frame->packet_cursor = g_wmap_current_frame->packet_cursor + sizeof(WmapMenuTriangle);
                }
                triangle_index += 1;
            } while (triangle_index < WMAP_MENU_TRIANGLE_COUNT);
        }
        func_8006534C(0xB5, 1);
        sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
        *sprite = D_800D06E4;
        addPrim(&g_wmap_current_frame->ordering_table[1], sprite);
        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(SPRT);
            g_wmap_current_frame->packet_cursor = g_wmap_current_frame->packet_cursor + sizeof(SPRT);
        }
        func_8006534C(0xD5, 1);
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
    }
}

/**
 * @brief Wait for a scripted map event while allowing live controller input.
 */
void wmap_wait_for_script_event(void)
{
    switch (g_wmap_script_wait)
    {
    case WMAP_SCRIPT_WAIT_ARTIFACT:
        if (D_8011D4FC != -1)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    case WMAP_SCRIPT_WAIT_PLACEMENT:
        if (D_80182DE0 != 0)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    case WMAP_SCRIPT_WAIT_PREVIEW:
        if (D_8011CF44 == 0)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    case WMAP_SCRIPT_WAIT_TRAVEL:
        if (g_wmap_party_moving != 0)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    case WMAP_SCRIPT_WAIT_MAP_STATE:
        if (D_8011CF18 == 3)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    }
}

/**
 * @brief Consume a scripted-input command or inject a timed button press.
 * @note Commands and button output use the shared script state.
 */
void wmap_step_input_script(void)
{
    TimPrefix* image;
    TimBlock* image_block;
    s32 duration;
    s32 command;
    s16 home_y;
    s32 buttons;
    s32 row;
    s32 column;
    s32 map_x;
    s32 packed_row;
    s32 home_row;
    s32 packed_column;
    s32 home_column;
    s32 row_scale;
    s32 button_result;
    s16* script;

    script = g_wmap_input_scripts[g_wmap_input_script - 1];
    script += g_wmap_script_word;
    duration = *script++;
    g_wmap_script_delay = (s32)duration;
    if (duration == WMAP_SCRIPT_END)
    {
        g_wmap_input_locked = 0;
        g_wmap_input_script = 0;
        g_wmap_script_delay = 1;
        return;
    }
    if (duration == WMAP_SCRIPT_COMMAND)
    {
        command = *script++;
        switch (command)
        {
        case WMAP_SCRIPT_IMAGE:
            if (script[0] > 0)
            {
                image = (TimPrefix*)&D_8019D6E0;
                cdrom_queue_read((u16)script[0], image);
                image_block = &image->clut_block;
                cdrom_wait_queue_empty();
                wmap_load_image_block(&image->clut_block);
                image_block = (TimBlock*)((u8*)image_block + image->clut_block.bnum);
                wmap_load_image_block(image_block);
                DrawSync(0);
                D_801ADAFC = 1;
                g_wmap_script_prompt_visible = 1;
            }
            else
            {
                g_wmap_script_prompt_visible = 0;
            }
            g_wmap_script_word += 3;
            break;
        case WMAP_SCRIPT_BUTTON_MASK:
            g_wmap_script_button_mask = (s32)script[0];
            g_wmap_script_word += 3;
            break;
        case WMAP_SCRIPT_VISIBILITY:
            if (script[0] != 0)
            {
                D_8013B258 = 0;
            }
            else
            {
                D_8013B258 = 1;
            }
            g_wmap_script_word += 3;
            break;
        case WMAP_SCRIPT_WAIT_ARTIFACT:
            g_wmap_script_wait = WMAP_SCRIPT_WAIT_ARTIFACT;
            g_wmap_script_word += 2;
            break;
        case WMAP_SCRIPT_WAIT_PLACEMENT:
            g_wmap_script_wait = WMAP_SCRIPT_WAIT_PLACEMENT;
            g_wmap_script_word += 2;
            break;
        case WMAP_SCRIPT_WAIT_PREVIEW:
            g_wmap_script_wait = WMAP_SCRIPT_WAIT_PREVIEW;
            g_wmap_script_word += 2;
            break;
        case WMAP_SCRIPT_WAIT_TRAVEL:
            g_wmap_script_wait = WMAP_SCRIPT_WAIT_TRAVEL;
            g_wmap_script_word += 2;
            break;
        case WMAP_SCRIPT_WAIT_MAP_STATE:
            g_wmap_script_wait = WMAP_SCRIPT_WAIT_MAP_STATE;
            g_wmap_script_word += 2;
            break;
        case WMAP_SCRIPT_FIND_HOME:
            for (row = 0; row < WMAP_GRID_SIZE; row++)
            {
                for (map_x = column = 0; column < WMAP_GRID_SIZE; column++)
                {
                    if (D_80139290[column][row].land_id == 0)
                    {
                        packed_column = column << 0x10;
                        g_wmap_travelers[0].target_x = map_x;
                        g_wmap_travelers[0].position_x = map_x;
                        map_x = WMAP_GRID_SIZE * WMAP_MAP_CELL_SIZE;
                        g_wmap_travelers[0].next_cell_x = column;
                        column = WMAP_GRID_SIZE;
                        row_scale = row * 3;
                        packed_row = row << 0x10;
                        g_wmap_travelers[0].next_cell_y = row;
                        row = WMAP_GRID_SIZE;
                        home_y = row_scale * 0x10;
                        home_column = packed_column >> 0x10;
                        home_row = packed_row >> 0x10;
                        g_wmap_travelers[0].target_y = home_y;
                        g_wmap_travelers[0].position_y = home_y;
                        g_wmap_travelers[0].destination_x = home_column;
                        g_wmap_travelers[0].cell_x = home_column;
                        g_wmap_travelers[0].destination_y = home_row;
                        g_wmap_travelers[0].cell_y = home_row;
                        g_wmap_travelers[0].moving = 0;
                    }
                    map_x += WMAP_MAP_CELL_SIZE;
                }
            }
            g_wmap_script_word += 2;
            break;
        }
        g_wmap_script_delay = 1;
        return;
    }
    if (duration != 0)
    {
        buttons = script[0];
        g_wmap_script_word += 2;
        g_wmap_buttons_held = (s32)buttons;
        g_wmap_buttons_repeat = (s32)buttons;
        return;
    }
    if (script[0] == 0)
    {
        button_result = func_80065428();
        if (button_result != 0)
        {
            g_wmap_script_word += 2;
        }
        g_wmap_script_delay = 1;
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
        return;
    }
    if ((u16)script[0] & 0x800)
    {
        button_result = func_80065428() & (s16)((u16)script[0] & 0xF7FF);
        if (button_result != 0)
        {
            g_wmap_script_word += 2;
        }
        g_wmap_script_delay = 1;
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
        return;
    }
    g_wmap_input_script = 0;
    g_wmap_script_delay = 1;
}

/** @brief Rectangle template and trailing zero used during display transfers. */
const WmapTransferRect g_wmap_exit_capture_rect = {{SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, {0, 0}};

/** @brief Copy the destination rectangle from a TIM block header. */
static inline void wmap_copy_rectangle(RECT* rectangle, u8* block)
{
    *rectangle = *(RECT*)(block + 4);
}

static inline void wmap_set_error_uv(POLY_FT4* quad, s32 error)
{
    s32 top;
    setlen(quad, 9);
    setcode(quad, 0x2E);
    top = (error - 1) << 5;
    quad->v1 = top;
    quad->v0 = top;
    top += 0x20;
    quad->v3 = top;
    quad->v2 = top;
}

/**
 * @brief Load world-map resources and run frames until the map exits.
 * @return Two for the controller reset chord, or zero after the exit effect.
 */
s32 wmap_run_loop(void)
{
    RECT rects[4];
    WmapFrame* render_base;
    WmapFrame* render_alt;
    u8* palette_color;
    s16 traveler_x;
    s16 traveler_y;
    s32 raw_buttons;
    s32 error_packet_bytes;
    s32 packet_bytes;
    s32 vsync_count;
    s32 layout_id;
    s32 script_delay;
    s32 cd_error;
    s32 exit_frame;
    s32 backdrop_choice;
    s32 sound_bank;
    s32 color_index;
    s32 show_loading_image;

    s32 scroll_cell_x;
    s32 scroll_cell_y;
    s32 next_exit_frame;
    s32 palette_index;
    s32 pixel_index;
    s32 backdrop_color;
    s8 error_texture_top;
    s8 error_texture_bottom;
    u8* image_header;
    u8* interface_block;
    u8* backdrop_block;
    u8* pixels;
    u16* palette_entry;
    u16* screen_pixel;
    u8* image_data;
    u8* map_block;
    u8* marker_block;
    u8* marker_header;
    s32 controller_buttons;
    u16 packed_color;
    u32 event;
    u16* palette_source;
    SPRT* prompt_sprite;
    POLY_FT4* error_quad;
    u8* pixel_block;

    layout_id = wmap_load_land_layout();
    show_loading_image = 1;
    D_800DBE68 = layout_id;
    D_8013B234 = layout_id;
    D_801ADB08 = -1;
    wmap_init_state();
    func_8006D520();
    /* Pending land events may replace the normal map entry sequence. */
    while (1)
    {
        event = wmap_next_land_event();
        D_801398F8 = event;
        if (event != -1U)
        {
            D_800D9224 += 1;
            if ((u32)(event - 4) < 5U)
            {
                D_80182DD8 = event;
                g_wmap_input_locked = 1;
                g_wmap_buttons_held = 0;
                g_wmap_buttons_repeat = 0;
                D_800D9224 = 1;
                {
                    s32 drain_event;

                    do
                    {
                        drain_event = wmap_next_land_event();
                    } while (drain_event != -1);
                }
                func_8006CBD8(&func_800BFD18);
                break;
            }
            else
            {
                switch (event)
                {
                case 0:
                    g_wmap_input_script = 1;
                    g_wmap_party_visible = 0;
                    D_80182228 = 1;
                    break;
                case 1:
                    g_wmap_input_script = 2;
                    break;
                case 28:
                    g_wmap_input_script = 3;
                    g_wmap_party_visible = 0;
                    D_80182228 = 1;
                    break;
                case 25:
                    D_80129538[0] = 1;
                    break;
                case 17:
                    D_80129538[1] = 1;
                    break;
                case 18:
                    D_80129538[2] = 1;
                    break;
                case 19:
                    D_80129538[3] = 1;
                    break;
                case 20:
                    D_80129538[4] = 1;
                    break;
                case 22:
                    D_80182E1C = 1;
                    break;
                case 23:
                    D_801398B8 = 1;
                    break;
                case 21:
                    D_80129540 = 1;
                    break;
                case 16:
                    D_80139248 = 1;
                    break;
                case 11:
                    show_loading_image = 0;
                    D_801ADB90 = 0;
                    D_8011D4F8 = 1;
                    break;
                case 15:
                    D_80139900 = 1;
                    break;
                case 12:
                    show_loading_image = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_80182E34 = 3;
                    D_800DBE78 = 3;
                    g_wmap_screen_fade_mode = 3;
                    g_wmap_spirit_target_brightness = 0;
                    D_801ADB90 = 0;
                    g_wmap_party_visible = 0;
                    g_wmap_spirit_brightness = 0;
                    D_8013B208 = 1;
                    D_801ADB90 = 0;
                    D_80182D5C = wmap_get_starting_cell(&g_wmap_travelers[0].cell_x, &g_wmap_travelers[0].cell_y);
                    D_80139834 = 1;
                    D_8013B208 = 1;
                    break;
                case 13:
                    show_loading_image = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_80182E34 = 3;
                    D_800DBE78 = 3;
                    g_wmap_screen_fade_mode = 3;
                    g_wmap_spirit_target_brightness = 0;
                    D_801ADB90 = 0;
                    g_wmap_party_visible = 0;
                    g_wmap_spirit_brightness = 0;
                    D_8013B208 = 1;
                    D_80182D5C = wmap_get_starting_cell(&g_wmap_travelers[0].cell_x, &g_wmap_travelers[0].cell_y);
                    D_80139238 = 1;
                    D_8013B208 = 1;
                    break;
                case 9:
                    show_loading_image = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_8011D500 = 0xFF;
                    D_800D9228 = 0xFF;
                    D_80182E38 = 1;
                    D_801ADB90 = 0;
                    D_80182E00 = 0;
                    func_8006CBD8(&func_80064D64);
                    D_80139244 = 1;
                    D_800DBE78 = 3;
                    D_80182E34 = 3;
                    g_wmap_spirit_target_brightness = 0;
                    g_wmap_spirit_brightness = 0;
                    g_wmap_screen_fade_mode = 3;
                    D_8013B208 = 1;
                    g_wmap_party_visible = 0;
                    D_8012954C = 1;
                    break;
                case 10:
                    show_loading_image = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_8011D500 = 0xFF;
                    D_800D9228 = 0xFF;
                    D_80182E38 = 1;
                    D_801ADB90 = 0;
                    g_wmap_party_visible = 0;
                    D_8013B208 = 1;
                    D_80182E00 = 0;
                    func_8006CBD8(&func_80064D64);
                    D_80139244 = 1;
                    D_80182E34 = 3;
                    D_800DBE78 = 3;
                    g_wmap_spirit_target_brightness = 0;
                    g_wmap_spirit_brightness = 0;
                    g_wmap_screen_fade_mode = 3;
                    D_801ADAF0 = 1;
                    break;
                case 24:
                    show_loading_image = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_80182E34 = 3;
                    D_800DBE78 = 3;
                    g_wmap_screen_fade_mode = 3;
                    g_wmap_spirit_target_brightness = 0;
                    g_wmap_spirit_brightness = 0;
                    D_801ADB90 = 0;
                    g_wmap_party_visible = 0;
                    D_8013B208 = 1;
                    D_8013997C = 1;
                    break;
                case 27:
                    show_loading_image = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_8011D500 = 0;
                    D_800D9228 = 0;
                    func_8006CBD8(&func_80064D64);
                    D_80182DD4 = 1;
                    D_80182E34 = 3;
                    D_800DBE78 = 3;
                    g_wmap_screen_fade_mode = 3;
                    g_wmap_spirit_target_brightness = 0;
                    g_wmap_spirit_brightness = 0;
                    D_801ADB90 = 0;
                    g_wmap_party_visible = 0;
                    D_8013B208 = 1;
                    D_80182240 = 1;
                    break;
                }
                continue;
            }
        }
        else
        {
            break;
        }
    }
    if (show_loading_image != 0)
    {
        {
            u8* data;
            u8* header;
            data = D_800DCF18;
            cdrom_queue_read((wmap_get_land_count_tier() + 0x1453) & 0xFFFF, data);
            cdrom_wait_queue_empty();
            data += 8;
            header = D_800DCF18;
            if (header[4] & 8)
            {
                wmap_copy_rectangle(&rects[0], data);
                LoadImage(&rects[0], data + 12);
                data += *(s32*)data;
            }
            wmap_copy_rectangle(&rects[0], data);
            if (rects[0].x != -1)
            {
                LoadImage(&rects[0], data + 12);
                DrawSync(0);
                D_801ADAFC = 1;
            }
        }
        D_800D0A08[6] = 0x20;
        D_800D0A08[5] = 0x20;
        D_800D0A08[4] = 0x20;
        PutDispEnv(D_801295BC);
        PutDrawEnv(D_801295BC - 0x5C);
        SetDispMask(1);
        DrawPrim(D_800D0A08);
        PutDispEnv(D_801295BC + 0x7E40);
        PutDrawEnv(D_801295BC + 0x7DE4);
        DrawPrim(D_800D0A08);
    }
    /* Load the sound banks before starting the world-map song. */
    sound_bank = 0x145E;
    if (D_8011D4F8 != 0)
    {
        sound_bank = 0x145F;
    }
    cdrom_queue_read(sound_bank, D_800DCF18);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking(D_800DCF18, 1);
    cdrom_queue_read(0x1460, D_800DCF18);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking(D_800DCF18, 1);
    cdrom_queue_read(0x1461, &D_8013B2A0);
    cdrom_wait_queue_empty();
    if (show_loading_image != 0)
    {
        D_800D0A08[6] = 0x40;
        D_800D0A08[5] = 0x40;
        D_800D0A08[4] = 0x40;
        PutDispEnv(D_801295BC);
        PutDrawEnv(D_801295BC - 0x5C);
        DrawPrim(D_800D0A08);
        PutDispEnv(D_801295BC + 0x7E40);
        PutDrawEnv(D_801295BC + 0x7DE4);
        DrawPrim(D_800D0A08);
    }
    if (D_801ADB90 != 0)
    {
        akao_play_song(&D_8013B2A0);
        akao_cmd_d0(0);
        akao_cmd_c2(0, 0x1E, 1, 0x7F);
    }
    {
        u8* data;
        u8* header;
        data = D_800DCF18;
        cdrom_stream((D_800DBE68 + 0x14DE) & 0xFFFF, data);
        cdrom_wait_queue_empty();
        data += 8;
        header = D_800DCF18;
        if (header[4] & 8)
        {
            wmap_copy_rectangle(&rects[0], data);
            LoadImage(&rects[0], header + 20);
            data += *(s32*)(header + 8);
            wmap_copy_rectangle(&rects[0], data);
            if (rects[0].x != -1)
            {
                LoadImage(&rects[0], data + 12);
                DrawSync(0);
                D_801ADAFC = 1;
            }
        }
        else
        {
            rects[0] = *(RECT*)(header + 12);
            if (rects[0].x != -1)
            {
                LoadImage(&rects[0], header + 20);
                DrawSync(0);
                D_801ADAFC = 1;
            }
        }
    }
    /* Keep the source palette in five-bit channels for later color effects. */
    palette_source = D_800DCF2C;
    for (color_index = 0; color_index < 16; color_index++)
    {
        ((CVECTOR*)D_80139908)[color_index].r = D_800DCF2C[color_index] & 0x1F;
        ((CVECTOR*)D_80139908)[color_index].g = (D_800DCF2C[color_index] >> 5) & 0x1F;
        packed_color = D_800DCF2C[color_index];
        ((CVECTOR*)D_80139908)[color_index].b = (packed_color >> 10) & 0x1F;
    }
    rects[0].x = 0x278;
    rects[0].y = 0;
    rects[0].w = 0x10;
    rects[0].h = 0x180;
    MoveImage(&rects[0], 0x2B0, 0);
    rects[0].x = 0x240;
    rects[0].y = 0xE0;
    rects[0].w = 0x60;
    rects[0].h = 0x40;
    MoveImage(&rects[0], 0x240, 0x1C0);
    rects[0].x = 0x278;
    rects[0].y = 0xE0;
    rects[0].w = 0x10;
    rects[0].h = 0x40;
    MoveImage(&rects[0], 0x2B0, 0x1C0);
    {
        u8* data;
        u8* header;
        data = D_800DCF18;
        cdrom_stream((0x10C4) & 0xFFFF, data);
        cdrom_wait_queue_empty();
        data += 8;
        header = D_800DCF18;
        if (header[4] & 8)
        {
            wmap_copy_rectangle(&rects[1], data);
            LoadImage(&rects[1], data + 12);
            data += *(s32*)data;
        }
        wmap_copy_rectangle(&rects[1], data);
        if (rects[1].x != -1)
        {
            LoadImage(&rects[1], data + 12);
            DrawSync(0);
            D_801ADAFC = 1;
        }
    }
    g_wmap_backdrop_target_level = 8;
    func_8006CBD8(&wmap_draw_backdrop);
    cdrom_stream(0x10C5, &D_8013C628);
    if (show_loading_image != 0)
    {
        D_800D0A08[6] = 0x80;
        D_800D0A08[5] = 0x80;
        D_800D0A08[4] = 0x80;
        PutDispEnv(D_801295BC);
        PutDrawEnv(D_801295BC - 0x5C);
        DrawPrim(D_800D0A08);
        PutDispEnv(D_801295BC + 0x7E40);
        PutDrawEnv(D_801295BC + 0x7DE4);
        DrawPrim(D_800D0A08);
    }
    func_8006CBD8(&func_8006579C);
    D_801398B0 = 1;
    D_8013B24C = 0x10;
    func_8006683C(0x808080);
    {
        u8* data;
        u8* header;
        data = D_800DCF18;
        cdrom_stream((0x10C6) & 0xFFFF, data);
        cdrom_wait_queue_empty();
        data += 8;
        header = D_800DCF18;
        if (header[4] & 8)
        {
            wmap_copy_rectangle(&rects[1], data);
            LoadImage(&rects[1], header + 20);
            data += *(s32*)(header + 8);
            wmap_copy_rectangle(&rects[1], data);
            if (rects[1].x != -1)
            {
                LoadImage(&rects[1], data + 12);
                DrawSync(0);
                
            }
        }
        else
        {
            rects[1] = *(RECT*)(header + 12);
            if (rects[1].x != -1)
            {
                LoadImage(&rects[1], header + 20);
                DrawSync(0);
                
            }
        }
    }
    cdrom_queue_read(0x10C7, &D_801ADBA0);
    g_wmap_land_image_cache[16].resource_id = 0x1F;
    g_wmap_land_image_cache[16].animation_data = (u8*)&D_801ADBA0;
    g_wmap_land_image_cache[16].slot_index = 0x10;
    g_wmap_land_image_cache[16].loaded_frame = 0xFFFF;
    g_wmap_land_image_cache[16].busy = 0;
    {
        u8* data;
        u8* header;
        data = D_800DCF18;
        cdrom_queue_read((0x10C8) & 0xFFFF, data);
        cdrom_wait_queue_empty();
        data += 8;
        header = D_800DCF18;
        if (header[4] & 8)
        {
            wmap_copy_rectangle(&rects[1], data);
            LoadImage(&rects[1], data + 12);
            data += *(s32*)data;
        }
        wmap_copy_rectangle(&rects[1], data);
        if (rects[1].x != -1)
        {
            LoadImage(&rects[1], data + 12);
            DrawSync(0);
            D_801ADAFC = 1;
        }
    }
    g_wmap_land_display[31].previous_animation_index = -1;
    g_wmap_land_display[31].animation_index = 0;
    /* Clamp the map origin so the party stays within the scrolling area. */
    wmap_init_party_travel();
    scroll_cell_x = g_wmap_travelers[0].cell_x - 1;
    scroll_cell_y = g_wmap_travelers[0].cell_y - 1;
    if (scroll_cell_x < 0)
    {
        scroll_cell_x = 0;
        D_800DCEEC = 0;
    }
    else if (scroll_cell_x >= 4)
    {
        scroll_cell_x = 3;
        D_800DCEEC = 2;
    }
    if (scroll_cell_y < 0)
    {
        scroll_cell_y = 0;
        D_800DCEF0 = 0;
    }
    else if (scroll_cell_y >= 4)
    {
        scroll_cell_y = 3;
        D_800DCEF0 = 2;
    }
    D_80139950.vx = (s32)(scroll_cell_x * WMAP_MAP_CELL_SIZE);
    D_80139950.vy = (s32)(scroll_cell_y * WMAP_MAP_CELL_SIZE);
    rects[1] = D_80051A88;
    traveler_x = (g_wmap_travelers[0].cell_x - 1) * WMAP_TRAVEL_CELL_SIZE;
    g_wmap_travelers[0].next_cell_x = (u16)g_wmap_travelers[0].cell_x;
    g_wmap_travelers[0].next_cell_y = (u16)g_wmap_travelers[0].cell_y;
    g_wmap_travelers[0].position_x = traveler_x;
    g_wmap_travelers[0].target_x = traveler_x;
    traveler_y = (g_wmap_travelers[0].cell_y - 1) * WMAP_TRAVEL_CELL_SIZE;
    g_wmap_travelers[0].position_y = traveler_y;
    g_wmap_travelers[0].target_y = traveler_y;
    ClearImage(&rects[1], 0, 0, 0);
    rects[1].x = 0x2C0;
    rects[1].y = 0x1FF;
    rects[1].w = 0x100;
    rects[1].h = 1;
    StoreImage(&rects[1], D_800DCF18);
    DrawSync(0);
    {
    s32 opaque_mask = ~0x7FFF;
    palette_index = 1;
    palette_entry = (u16*)(D_800DCF18 + 2);
    do
    {
        palette_index += 1;
        *palette_entry |= opaque_mask;
        palette_entry++;
    } while (palette_index < 0x100);
    }
    rects[1].x = 0;
    rects[1].y = 0x1FF;
    LoadImage(&rects[1], D_800DCF18);
    DrawSync(0);
    {
        u8* data;
        u8* header;
        data = D_800DCF18;
        cdrom_queue_read((wmap_get_land_count_tier() + 0x10CE) & 0xFFFF, data);
        cdrom_wait_queue_empty();
        data += 8;
        header = D_800DCF18;
        if (header[4] & 8)
        {
            wmap_copy_rectangle(&rects[2], data);
            LoadImage(&rects[2], header + 20);
            data += *(s32*)(header + 8);
            wmap_copy_rectangle(&rects[2], data);
            if (rects[2].x != -1)
            {
                LoadImage(&rects[2], data + 12);
                DrawSync(0);
                D_801ADAFC = 1;
            }
        }
        else
        {
            rects[2] = *(RECT*)(header + 12);
            if (rects[2].x != -1)
            {
                LoadImage(&rects[2], header + 20);
                DrawSync(0);
                D_801ADAFC = 1;
            }
        }
    }
    func_8006CBD8(&wmap_update_screen_fade);
    g_wmap_backdrop_target_level = 0x10;
    D_801ADAE8 = 1;
    if (D_800D9224 == 0)
    {
        g_wmap_input_locked = 0;
    }
    update_controllers();
    D_8011CF74 = 0;
    render_base = &g_wmap_frames[0];
    render_alt = &g_wmap_frames[1];
    /* Build one display list per frame, alternating the two packet buffers. */
    while (1)
    {
        if (D_8011CF74 & 1)
        {
            render_base->packet_cursor = (u8*)D_8010CF18;
            g_wmap_current_frame = render_base;
        }
        else
        {
            render_base[1].packet_cursor = (u8*)D_80114F18;
            g_wmap_current_frame = render_alt;
        }
        g_wmap_packet_bytes = 0;
        ClearOTagR(g_wmap_current_frame->ordering_table, WMAP_OT_COUNT);
        D_800DBE7C = 0;
        D_8011CF74 = (s32)(D_8011CF74 + 1);
        if ((g_wmap_input_locked != 0) || (D_8011CF18 >= 3) ||
            (D_80139960 != 0) || (D_801398D0 != 0) || (D_8011CF44 != 0))
        {
            g_wmap_buttons_repeat = 0;
            g_wmap_buttons_held = 0;
        }
        else
        {
            wmap_read_controller();
        }
        if (D_8011CF74 < 0x28)
        {
            g_wmap_buttons_repeat &= 0xF000;
            g_wmap_buttons_held &= 0xF000;
        }
        if (g_wmap_input_script != 0)
        {
            if (g_wmap_script_wait != 0)
            {
                wmap_wait_for_script_event();
            }
            else
            {
                script_delay = g_wmap_script_delay - 1;
                g_wmap_script_delay = script_delay;
                if (script_delay != 0)
                {
                    g_wmap_buttons_held = 0;
                    g_wmap_buttons_repeat = 0;
                }
                else
                {
                    wmap_step_input_script();
                }
            }
        }
        controller_buttons = M2C_FIELD(D_800D0454, u16*, 0xB4);
        if ((controller_buttons >> 8) & 1)
        {
            D_8011D0E4 = 1;
        }
        else
        {
            D_8011D0E4 = 0;
        }
        raw_buttons = D_800D0454->published_sample.held_buttons;
        raw_buttons = (raw_buttons >> 8) | (raw_buttons << 8);
        if ((raw_buttons & 0x90F) == 0x90F)
        {
            return 2;
        }
        if (raw_buttons & 4)
        {
            D_8011D0E0 = 1;
        }
        else
        {
            D_8011D0E0 = 0;
        }
        exit_frame = D_8013B294;
        if (exit_frame != 0)
        {
            g_wmap_input_locked = 1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            if ((exit_frame == 1) && (D_8011CF74 & 1))
            {
                D_8013B294 = 2;
            }
            else
            {
                D_8013B294 += 1;
            }
        }
        if (D_8013B294 >= 0xB)
        {
            if (D_8013B294 == 0xB)
            {
                akao_cmd_c2(0, 0x5A, 0x7F, 0);
                if (D_80139228 == 0)
                {
                    akao_play_sfx_from_buffer(D_800CB248, 0, 0x80, 0x7F);
                }
                func_8005DF50(g_wmap_travelers[0].cell_x, g_wmap_travelers[0].cell_y);
                DrawSync(0);
                /* Save the displayed frame for the exit transition. */
                screen_pixel = (u16*)D_800DCF18;
                StoreImage(&g_wmap_current_frame->disp_env.disp, D_800DCF18);
                MoveImage(&g_wmap_current_frame->disp_env.disp, g_wmap_current_frame->draw_env.tw.x, g_wmap_current_frame->draw_env.tw.y);
                DrawSync(0);
                pixel_index = 0;
                do
                {
                    pixel_index += 1;
                    *screen_pixel |= 0x8000;
                    screen_pixel++;
                } while (pixel_index <= 0x12BFF);
                rects[2] = g_wmap_exit_capture_rect.rect;
                LoadImage(&rects[2], D_800DCF18);
                D_8013B238.vx = 0;
                D_8013B238.vy = 0;
                D_8013B238.vz = 0;
                D_80139870.vx = 0xA0;
                D_80139870.vy = 0x78;
                D_80139870.vz = 0;
                D_8013B240.vx = 0;
                D_8013B240.vy = 0;
                D_8013B240.vz = 0;
                D_80139888.vx = 0xA0;
                D_80139888.vy = 0x78;
                D_80139888.vz = 0;
                render_base[1].draw_env.dtd = 0;
                render_base->draw_env.dtd = 0;
                D_801B2478.vx = 3;
                D_801B2478.vy = 0;
                D_801B2478.vz = 0;
                D_801B24A8.vx = 8;
                D_801B24A8.vy = 0xC;
                D_801B24A8.vz = 0;
                backdrop_choice = rand() & 3;
                switch (backdrop_choice)
                {
                case 0:
                    D_800D9238 = 0x201010;
                    break;
                case 1:
                    D_800D9238 = 0x101810;
                    break;
                case 2:
                    D_800D9238 = 0x201020;
                    break;
                case 3:
                    D_800D9238 = 0x202010;
                    break;
                }
                DrawSync(0);
            }
            if (D_8013B294 == 0x78)
            {
                break;
            }
            switch (D_80139228)
            {
            case 1:
                func_800641DC();
                break;
            case 0:
                func_8006454C();
                break;
            case 2:
                return 0;
            default:
                break;
            }
        }
        else
        {
            wmap_update_menu(raw_buttons);
            if (g_wmap_script_prompt_visible != 0)
            {
                prompt_sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
                *prompt_sprite = D_800D06E4;
                addPrim(&g_wmap_current_frame->ordering_table[1], prompt_sprite);
                packet_bytes = g_wmap_packet_bytes;
                if (packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes = packet_bytes + sizeof(SPRT);
                    g_wmap_current_frame->packet_cursor += sizeof(SPRT);
                }
                func_8006534C(0xD5, 1);
            }
            if (D_8013986C == 0)
            {
                if (D_8011D4FC != -1)
                {
                    D_801398BC = 3;
                }
                else if (D_8011CF18 == 2)
                {
                    D_801398BC = D_8011CF18;
                }
                else
                {
                    D_801398BC = 0;
                }
            }
            else if (D_8013986C == 1)
            {
                D_801398BC = 1;
            }
            if ((D_8013986C == 0) && (g_wmap_game_start_delay != 0))
            {
                wmap_update_artifact_selection();
            }
            func_800664B8();
            SetSpadStack(WMAP_SCRATCH_STACK_SAVE_SLOT);
            func_8006CB60();
            func_8006CA28();
            ResetSpadStack();
            wmap_update_map_game();
            if (g_wmap_game_start_delay != 0)
            {
                wmap_draw_lands();
            }
            if (D_800D9224 != 0)
            {
                g_wmap_input_locked = 1;
                if (D_8011CF74 >= 0x33)
                {
                    func_800A5DFC();
                }
            }
            if (D_8013986C != 0)
            {
                func_8005FF88(-1);
            }
            func_8005F9BC();
            SetSpadStack(WMAP_SCRATCH_STACK_SAVE_SLOT);
            wmap_update_party_travel();
            wmap_update_map_display();
            wmap_update_land_preview();
            wmap_draw_artifact_carousel();
            func_8006D674();
            ResetSpadStack();
        }
        func_8005D46C();
        D_801ADAF8 = 0;
        vsync_count = VSync(1);
        DrawSync(0);
        set_controller_vsync_interval(2);
        VSync(2);
        if (vsync_count >= 0x20E)
        {
            D_801ADAFC = 0;
        }
        if (!(D_8011CF74 & 0x1F))
        {
            D_801ADAFC = 1;
        }
        PutDispEnv(&g_wmap_current_frame->disp_env);
        PutDrawEnv(&g_wmap_current_frame->draw_env);
        if (D_8013B210 != 0)
        {
            rects[2].x = 0x240;
            rects[2].y = 0x151;
            D_8013B210 = 0;
            rects[2].w = 0x10;
            rects[2].h = 1;
            LoadImage(&rects[2], &D_8013B210);
        }
        func_80064BF8();
        DrawOTag(&g_wmap_current_frame->ordering_table[0xB2]);
        update_controllers();
        cdrom_process_state();
        cd_error = cdrom_get_error_status();
        g_wmap_cd_error = cd_error;
        if (cd_error != 0)
        {
            error_quad = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
            if (cd_error < 0)
            {
                g_wmap_cd_error = 1;
            }
            if (g_wmap_cd_error >= 6)
            {
                g_wmap_cd_error = 5;
            }
            *(u32*)&error_quad->r0 = 0x808080;
            error_quad->x2 = 0x64;
            error_quad->x0 = 0x64;
            error_quad->x3 = 0xE4;
            error_quad->x1 = 0xE4;
            error_quad->y1 = 0x50;
            error_quad->y0 = 0x50;
            error_quad->y3 = 0x70;
            error_quad->y2 = 0x70;
            error_quad->u2 = 0x80;
            error_quad->u0 = 0x80;
            error_quad->u3 = 0xFF;
            error_quad->u1 = 0xFF;
            error_quad->tpage = 0x5E;
            error_quad->clut = 0x7F70;
            wmap_set_error_uv(error_quad, g_wmap_cd_error);
            setaddr(error_quad, getaddr(&g_wmap_current_frame->ordering_table[1]));
            error_packet_bytes = g_wmap_packet_bytes;
            setaddr(&g_wmap_current_frame->ordering_table[1], error_quad);
            if (error_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes = error_packet_bytes + 0x28;
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
            }
        }
    }
    return 0;
}

/**
 * @brief Read controller buttons, generate analog repeats, and apply input masks.
 * @note The controller words are byte-swapped but keep their hardware button order.
 */
void wmap_read_controller(void)
{
    s32 stick_x;
    s32 stick_negative;
    s32 stick_y;
    s32 repeat_shift_x;
    s32 repeat_shift_y;
    s32 masked_repeat;
    s32 repeat_buttons;
    s32 masked_held;
    s32 left_repeat;
    s32 right_repeat;
    s32 up_repeat;
    s32 down_repeat;

    if ((g_wmap_cd_error != 0) || (g_wmap_input_locked != 0))
    {
        g_wmap_buttons_repeat = 0;
        g_wmap_buttons_held = 0;
        return;
    }
    g_wmap_buttons_held = D_800D0454->published_sample.held_buttons;
    g_wmap_buttons_repeat = D_800D0454->published_sample.repeat_buttons;
    g_wmap_buttons_held = ((u32)g_wmap_buttons_held >> 8) | (g_wmap_buttons_held << 8);
    repeat_buttons = ((u32)g_wmap_buttons_repeat >> 8) | (g_wmap_buttons_repeat << 8);
    g_wmap_buttons_repeat = repeat_buttons;
    switch (D_800D0454->published_sample.device_type)
    {
    case CONTROLLER_DEVICE_DIGITAL:
        g_wmap_large_motor = 0;
        g_wmap_small_motor = 0;
        break;
    case CONTROLLER_DEVICE_ANALOG_JOYSTICK:
    case CONTROLLER_DEVICE_ANALOG:
        stick_x = D_800D0454->published_sample.left_stick_x;
        stick_negative = stick_x < 0;
        repeat_shift_x = stick_x;
        if (stick_negative)
        {
            repeat_shift_x = -repeat_shift_x;
        }
        if (repeat_shift_x >= 8)
        {
            repeat_shift_x = 7;
        }
        if (stick_x < 0)
        {
            left_repeat = repeat_buttons;
            if (((s32)D_8011CF74 % (s32)(0x200 >> repeat_shift_x)) == 0)
            {
                left_repeat |= PADLleft;
            }
            g_wmap_buttons_repeat = left_repeat;
        }
        if (D_800D0454->published_sample.left_stick_x > 0)
        {
            right_repeat = g_wmap_buttons_repeat;
            if (((s32)D_8011CF74 % (s32)(0x200 >> repeat_shift_x)) == 0)
            {
                right_repeat |= PADLright;
            }
            g_wmap_buttons_repeat = right_repeat;
        }
        stick_y = D_800D0454->published_sample.left_stick_y;
        stick_negative = stick_y < 0;
        repeat_shift_y = stick_y;
        if (stick_negative)
        {
            repeat_shift_y = -repeat_shift_y;
        }
        if (repeat_shift_y >= 8)
        {
            repeat_shift_y = 7;
        }
        if (stick_y < 0)
        {
            up_repeat = g_wmap_buttons_repeat;
            if (((s32)D_8011CF74 % (s32)(0x200 >> repeat_shift_y)) == 0)
            {
                up_repeat |= PADLup;
            }
            g_wmap_buttons_repeat = up_repeat;
        }
        if (D_800D0454->published_sample.left_stick_y > 0)
        {
            down_repeat = g_wmap_buttons_repeat;
            if (((s32)D_8011CF74 % (s32)(0x200 >> repeat_shift_y)) == 0)
            {
                down_repeat |= PADLdown;
            }
            g_wmap_buttons_repeat = down_repeat;
        }
        break;
    default:
        g_wmap_buttons_repeat = 0;
        g_wmap_buttons_held = 0;
        break;
    }
    if (g_wmap_buttons_held & PAD_BTN_L3)
    {
        g_wmap_buttons_held |= PADRdown;
    }
    if (g_wmap_buttons_repeat & PAD_BTN_L3)
    {
        g_wmap_buttons_repeat |= PADRdown;
    }
    masked_held = g_wmap_buttons_held & g_wmap_script_button_mask;
    g_wmap_buttons_held = masked_held;
    masked_repeat = g_wmap_buttons_repeat & g_wmap_script_button_mask;
    g_wmap_buttons_repeat = masked_repeat;
    if (g_wmap_map_controls_active != 0)
    {
        g_wmap_buttons_held = masked_held & g_wmap_map_button_mask;
        g_wmap_buttons_repeat = masked_repeat & g_wmap_map_button_mask;
    }
    D_800D0454->small_motor_command = (u8)g_wmap_small_motor;
    D_800D0454->actuator_control.fields.large_motor_command = (u8)g_wmap_large_motor;
    D_8019D6D8 = 0;
}

/**
 * @brief Cache land appearances, placement checks, and spirit sprites for every cell.
 */
void wmap_refresh_cells(void)
{
    s32 x;
    s32 y;
    s32 land_id;

    for (y = 0; y < WMAP_GRID_SIZE; y++)
    {
        for (x = 0; x < WMAP_GRID_SIZE; x++)
        {
            land_id = wmap_get_land_at_cell(x, y);
            if (D_80139830 != 0)
            {
                if (land_id == 5)
                {
                    land_id = 35;
                }
            }
            if (D_80182E20 != 0)
            {
                if (land_id == 1)
                {
                    land_id = 36;
                }
            }
            D_80139290[x][y].land_id = land_id;
            D_80139290[x][y].other_land = wmap_is_other_land_cell(x, y);
            wmap_get_cell_spirit_sprites(x, y, D_80139290[x][y].spirit_sprites);
            D_80139290[x][y].placement_allowed = wmap_can_place_land(x, y, D_8011D4FC);
            D_8011D108[x][y].frame = 0;
        }
    }
}

/**
 * @brief Reset map actors, land previews, and menu state after a transition.
 */
void wmap_reset_after_transition(void)
{
    s32 actor_index;

    D_80139978 = -1;
    for (actor_index = 0; actor_index < WMAP_ACTOR_COUNT; actor_index++)
    {
        D_800D9268[actor_index].display_mode = 16;
        D_801AFBD0[actor_index].state = 0;
    }
    wmap_refresh_cells();
    g_wmap_input_locked = 0;
    D_8013B258 = 0;
    g_wmap_preview_artifact_visible = 0;
    D_80129550 = 0;
    D_8011D4FC = -1;
    g_wmap_preview_texture = 0;
    g_wmap_preview_shape = 0;
    func_8005909C();
    D_8011D52C = 0;
    func_8006D870(0);
    D_800DBE70 = 2;
    D_801ADAE0 = 0;
    g_wmap_spirit_target_brightness = 0x80;
    g_wmap_backdrop_target_level = 0x10;
    D_80182DE0 = 0;
    g_wmap_menu_page = 0;
    g_wmap_menu_cursor_y = 0;
    g_wmap_menu_selection = 1;
    g_wmap_loaded_menu_page = -1;
    g_wmap_map_button_mask = -1;
    akao_cmd_c2(0, 0x1E, 1, 0x7F);
    D_801ADAFC = 1;
    D_8013B24C = 4;
    D_80182E3C = -1;
}
