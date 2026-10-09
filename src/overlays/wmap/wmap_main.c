#include "internal/wmap_land_selection.h"
#include "internal/wmap_land_layout.h"
#include "internal/wmap_land_transition.h"
#include "internal/wmap_party_travel.h"
#include "internal/wmap_map_display.h"
#include "internal/wmap_view_effects.h"
#include "internal/wmap_resource_support.h"
#include "internal/wmap_main.h"
#include "internal/wmap_land_preview.h"
#include "internal/wmap_land_preview_lines.h"
#include "common.h"
#include "common/gpu_packet.h"
#include <libgpu.h>
#include <libgte.h>
#include <libetc.h>
#include <spadstk.h>
#include "internal/wmap_map_labels.h"
#include "internal/wmap_sequence_runtime.h"
#include "internal/wmap_effect_backdrop.h"
#include "internal/wmap_frame_render.h"
#include "main/display.h"
#include "common/pad.h"
#include "common/tim.h"
#include "main/controller_internal.h"
#include "main/audio/akao_cmd.h"
#include "internal/wmap_sprite_render.h"
#include "main/controller.h"
#include "main/cdrom.h"
#include "main/game_state.h"
#include "internal/wmap_pathfinding.h"
#include <rand.h>
#include "internal/wmap_map_events.h"
#include "internal/wmap_cells.h"
#include "main/audio/akao.h"

#define WMAP_MAP_CELL_SIZE 48
#define WMAP_TRAVEL_CELL_SIZE 160
#define WMAP_ACTOR_COUNT 256
#define WMAP_MENU_ITEM_HEIGHT 16
#define WMAP_MENU_CURSOR_STEP 4
#define WMAP_MENU_OT_INDEX 1
#define WMAP_HELP_IMAGE_RESOURCE_BASE 0x114B
#define WMAP_HELP_IMAGE_TPAGE 0xD5
#define WMAP_HELP_CURSOR_TPAGE 0xB5
#define WMAP_MENU_TRIANGLE_COUNT 28
#define WMAP_BACKDROP_WRAP_WIDTH 640
#define WMAP_BACKDROP_QUAD_WIDTH 160
#define WMAP_BACKDROP_COLOR_STEP 8
#define WMAP_FADE_COLOR_STEP 4
#define WMAP_SCREEN_FADE_OT_INDEX 176
#define WMAP_ANALOG_REPEAT_BASE_FRAMES 512
#define WMAP_ANALOG_REPEAT_MAX_SHIFT 7
#define WMAP_INPUT_GRACE_FRAMES 40
#define WMAP_EVENT_START_DELAY_FRAMES 51
#define WMAP_EXIT_CAPTURE_FRAME 11
#define WMAP_EXIT_END_FRAME 120
#define WMAP_SHADOW_RETRY_MASK 0x1F
#define WMAP_SHADOW_VSYNC_LIMIT 526
#define WMAP_RESIDENT_LAND_SLOT 16
#define WMAP_RESIDENT_LAND_DISPLAY 31
#define WMAP_CD_ERROR_TEXTURE_HEIGHT 32
#define WMAP_ENTRY_FADE_MAX 255
#define WMAP_INITIAL_MAP_SHADOW_LEVEL 64
#define WMAP_INITIAL_VIEW_DEPTH 24576
#define WMAP_INITIAL_CAMERA_PITCH 736
#define WMAP_INITIAL_CAMERA_ROLL 432
#define WMAP_INITIAL_CAMERA_X 8
#define WMAP_INITIAL_CAMERA_Y (-24)
#define WMAP_INITIAL_CAMERA_DEPTH 28000

/** @brief CD resources loaded while entering the world map. */
enum WmapEntryResource
{
    WMAP_INTERFACE_IMAGE_RESOURCE = 0x10C4,
    WMAP_LAND_IMAGE_PACK_RESOURCE = 0x10C5,
    WMAP_RESIDENT_LAND_IMAGE_RESOURCE = 0x10C6,
    WMAP_RESIDENT_LAND_ANIMATION_RESOURCE = 0x10C7,
    WMAP_MAP_OVERLAY_IMAGE_RESOURCE = 0x10C8,
    WMAP_LOADING_IMAGE_RESOURCE_BASE = 0x1453,
    WMAP_NORMAL_SOUND_BANK_RESOURCE = 0x145E,
    WMAP_EVENT_SOUND_BANK_RESOURCE = 0x145F,
    WMAP_SHARED_SOUND_BANK_RESOURCE = 0x1460,
    WMAP_MUSIC_RESOURCE = 0x1461,
    WMAP_LAYOUT_IMAGE_RESOURCE_BASE = 0x14DE
};

/** @brief Help pages stored consecutively after WM/WHLP/HELPMENU.TIM. */
enum WmapHelpPage
{
    WMAP_HELP_NOT_LOADED = -1,
    WMAP_HELP_MENU = 0,
    WMAP_HELP_DIRECTIONS = 1,
    WMAP_HELP_MANA = 2,
    WMAP_HELP_MANA_SPIRITS = 3,
    WMAP_HELP_ELEMENTAL_PROPERTIES = 4,
    WMAP_HELP_ENEMY_STRENGTH = 5,
    WMAP_HELP_LAND_PLACEMENT = 6
};

/** @brief Soft-reset chord (select, start and all four shoulder buttons) and the D-pad bits. */
#define WMAP_RESET_CHORD (PADselect | PADstart | PADL1 | PADL2 | PADR1 | PADR2)
#define WMAP_DPAD_MASK (PADLup | PADLright | PADLdown | PADLleft)
/** @brief 15-bit pixel semi-transparency (mask) bit. */
#define WMAP_PIXEL_MASK_BIT 0x8000

/** @brief Scratchpad word holding the world-map caller's saved stack pointer. */
#define WMAP_SCRATCH_STACK_SAVE_SLOT SCRATCHPAD_AT(0x3F0)

/** @brief Backdrop color: three channels. */
typedef struct
{
    u8 r;
    u8 g;
    u8 b;
} WmapColor3;

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

/** @brief Direction in which the fade quad's RGB intensity changes. */
typedef enum
{
    WMAP_FADE_HOLD = 0,
    WMAP_FADE_INCREASE = 1,
    WMAP_FADE_DECREASE = 2,
    WMAP_FADE_DISABLED = 3
} WmapFadeMode;

/** @brief Land display IDs used by the world-map appearance overrides. */
typedef enum
{
    WMAP_LAND_DISPLAY_DOMINA = 1,
    WMAP_LAND_DISPLAY_GATO = 5,
    WMAP_LAND_DISPLAY_GATO_OVERRIDE = 35,
    WMAP_LAND_DISPLAY_DOMINA_OVERRIDE = 36
} WmapLandDisplayId;

/** @brief Per-tile display state. */
typedef struct
{
    s16 unknown_00;
    s16 frame;
    u8 pad_04[24];
} WmapTileDisplay;

/** @brief Motion state for a world-map actor. */
typedef struct
{
    s16 state;
    u8 pad_02[0x12];
} WmapMotion;

extern s32 wmap_special_effect_34_run(s32 initialize);

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

extern s32 D_800DCEDC;

extern RECT D_80051A80;
/** @brief World-map loop result before attract movie requests are applied. */
extern s32 g_wmap_loop_result;
/** @brief Nonzero to play attract movie one when the world map returns. */
extern s32 g_wmap_attract_1_requested;
/** @brief Nonzero to play attract movies two through four after the map returns. */
extern s32 g_wmap_attract_2_requested;
extern s32 D_800D923C;
extern s32 D_800DBE6C;
extern s32 D_800DBE74;
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 D_800DCEFC;
extern s32 g_wmap_sequence_count;
extern s32 g_wmap_view_scroll_enabled;
extern s32 g_wmap_selected_artifact;
extern CVECTOR g_wmap_tint;
extern s32 D_80139218;
extern SVECTOR g_wmap_camera_rotation;
extern s32* g_wmap_effect_params;
extern s32 g_wmap_tint_blending;
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801398F4;
extern VECTOR g_wmap_view;
extern DVECTOR D_8013B260;
extern s32 D_8013B290;
extern s32 D_8013B27C;
extern s32 D_80182D88;
extern VECTOR g_wmap_camera_translation;
extern s32 g_wmap_map_shadow_level;
/** @brief Base RGB tint of the scrolling textured backdrop. */
extern CVECTOR g_wmap_backdrop_tint;
/** @brief Nonzero to start the normal world-map song after loading. */
extern s32 g_wmap_entry_music_enabled;
extern POLY_FT4 g_wmap_backdrop_front_quads[4];
extern POLY_FT4 g_wmap_backdrop_back_quads[4];

extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
/** @brief Textured fade quad drawn in the upper-left corner of the map. */
extern POLY_FT4 g_wmap_screen_fade_quad;
/** @brief Sprite template shared by menu and scripted input prompts. */
extern SPRT g_wmap_prompt_sprite_template;
extern WmapMenuTriangle g_wmap_menu_triangles[WMAP_MENU_TRIANGLE_COUNT];
/** @brief TIM workspace shared by help pages and scripted prompt images. */
extern TimPrefix g_wmap_help_image;
extern s16* g_wmap_input_scripts[];
/** @brief Textured quad shaded at each stage of world-map loading. */
extern POLY_FT4 g_wmap_loading_quad;
/** @brief Loaded layout selector used to choose the entry image. */
extern s32 g_wmap_layout_id;
extern s32 D_800DBE7C;
extern u8 g_wmap_load_buffer[];
/** @brief Sixteen-color CLUT of the image loaded on map entry. */
extern u16 g_wmap_entry_palette[16];
/** @brief Raw L1 held state from the first controller. */
extern s32 g_wmap_controller_1_l1_held;
/** @brief Raw L2 repeat state from the second controller. */
extern s32 g_wmap_controller_2_l2_repeat;
extern s32 g_wmap_view_mode;
/** @brief Last land-event flag read from the queue, or the end sentinel. */
extern u32 g_wmap_last_land_event;
/** @brief Entry palette expanded into five-bit RGB channels. */
extern CVECTOR g_wmap_entry_palette_channels[16];
/** @brief Sixteen-color palette uploaded when its first entry is nonzero. */
extern u16 g_wmap_pending_palette[16];
/** @brief Layout selector recorded when world-map entry begins. */
extern s32 g_wmap_entry_layout_id;
extern s32 g_wmap_tint_speed;
/** @brief World-map song loaded from the entry music resource. */
extern AkaoHeader g_wmap_music_buffer;
extern s32 D_801ADB08;
/** @brief Animation data for land display 31, kept in cache slot 16. */
extern u8 g_wmap_resident_land_animation_data[];
extern RECT D_80051A88;
extern WmapTileDisplay D_8011D108[6][6];
/** @brief Nonzero to use display resource 35 for Gato's map appearance. */
extern s32 g_wmap_gato_appearance_override;
/** @brief Nonzero to use display resource 36 for Domina's map appearance. */
extern s32 g_wmap_domina_appearance_override;
extern WmapMotion g_wmap_actor_motions[];

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
    akao_stop_all_songs();
    akao_release_all_sfx();
    wmap_init_frame_buffers();
    g_wmap_frames[0].draw_env.isbg = 0;
    g_wmap_frames[1].draw_env.isbg = 0;
    g_wmap_loop_result = wmap_run_loop();
    akao_stop_all_songs();
    akao_release_all_sfx();
    if (g_wmap_loop_result == 2)
    {
        DrawSync(0);
        VSync(0);
        SetDispMask(0);
        ResetGraph(0);
        ClearImage(&clear_rect, 0, 0, 0);
        return 2;
    }
    if (g_wmap_attract_1_requested != 0)
    {
        return GAME_STATE_ATTRACT_1;
    }
    if (g_wmap_attract_2_requested != 0)
    {
        return GAME_STATE_ATTRACT_2;
    }
    return g_wmap_loop_result;
}

/** @brief Reset map-entry state, display records, and rendering templates. */
void wmap_init_state(void)
{
    s32 land_display_mode;

    /* Reset pending events and the normal map-entry display policy. */
    g_wmap_special_land_event = 0;
    g_wmap_vehicle_flap_sound_enabled = 1;
    g_wmap_attract_2_requested = 0;
    g_wmap_attract_1_requested = 0;
    g_wmap_forced_animated_land_id = -1;
    g_wmap_land_event_24_pending = 0;
    g_wmap_special_return_pending = 0;
    D_800DBE6C = -1;
    D_800DBE74 = 0;
    g_wmap_land_event_10_active = 0;
    g_wmap_land_event_09_pending = 0;
    D_800D923C = 0;
    g_wmap_backdrop_gradient = g_wmap_backdrop_gradient_template;
    g_wmap_exit_mode = WMAP_EXIT_SCREEN_PANELS;
    g_wmap_land_image_load_locked = 0;
    g_wmap_cd_error = 0;
    g_wmap_last_effect_resource_set = -1;
    g_wmap_land_event_11_pending = 0;
    g_wmap_land_event_16_pending = 0;
    g_wmap_land_event_21_pending = 0;
    g_wmap_land_event_15_pending = 0;
    g_wmap_entry_music_enabled = 1;
    g_wmap_tint_blending = 0;
    g_wmap_third_traveler_enabled = 0;
    g_wmap_second_traveler_enabled = 0;
    g_wmap_entry_fade_level = WMAP_ENTRY_FADE_MAX;
    g_wmap_land_label_updates_enabled = 1;
    g_wmap_pending_event_count = 0;
    g_wmap_information_groups = 0;
    g_wmap_effect_params = (s32*)getScratchAddr(0);
    g_wmap_party_visible = 1;
    g_wmap_artifact_shadows_enabled = 1;
    g_wmap_view_scroll_enabled = 1;
    g_wmap_script_word = 0;
    g_wmap_input_script = 0;
    g_wmap_preview_bob_frame = 0;
    g_wmap_script_delay = 1;
    g_wmap_map_button_mask = -1;
    g_wmap_script_button_mask = -1;
    g_wmap_game_start_delay = WMAP_GAME_RETRY_DELAY;
    g_wmap_auxiliary_label_mode = 0;
    g_wmap_auxiliary_label_draw_mode = 0;
    g_wmap_auxiliary_labels_hidden = 0;
    g_wmap_frames[0].packet_cursor = g_wmap_packet_buffer_0;
    g_wmap_frames[1].packet_cursor = g_wmap_packet_buffer_1;
    g_wmap_map_shadow_level = WMAP_INITIAL_MAP_SHADOW_LEVEL;
    g_wmap_input_locked = 1;
    g_wmap_sequence_count = 0;
    D_800DCEFC = 0;
    D_80182D88 = 0;
    /* Prepare actor, land, and label display records. */
    wmap_select_mesh_motion(0);
    wmap_init_actor_display_states();
    wmap_init_spirit_animation();
    wmap_init_map_packets();
    g_wmap_frame_count = 0;
    D_80139218 = 0;
    g_wmap_party_cell_dirty = -1;
    D_8013B290 = -1;
    wmap_init_land_image_cache();
    func_80058298();
    wmap_init_land_display();
    wmap_init_sequences();
    wmap_reset_artifact_carousel();
    g_wmap_spirit_brightness = 0;
    g_wmap_spirit_target_brightness = WMAP_FULL_BRIGHTNESS;
    g_wmap_map_controls_active = 1;
    g_wmap_script_prompt_visible = 0;
    g_wmap_menu_selection = WMAP_HELP_DIRECTIONS;
    g_wmap_menu_cursor_y = 0;
    g_wmap_menu_page = WMAP_HELP_MENU;
    g_wmap_loaded_menu_page = WMAP_HELP_NOT_LOADED;
    wmap_init_label_sprites();
    wmap_start_mesh_transition(0);
    g_wmap_backdrop_tint.r = WMAP_FULL_BRIGHTNESS;
    g_wmap_backdrop_tint.g = WMAP_FULL_BRIGHTNESS;
    g_wmap_backdrop_tint.b = WMAP_FULL_BRIGHTNESS;
    g_wmap_backdrop_level = 1;
    g_wmap_backdrop_target_level = 0;
    g_wmap_exit_frame = 0;
    /* Restore the camera pose and map-selection defaults. */
    g_wmap_view.vx = 0;
    g_wmap_view.vy = 0;
    g_wmap_view.vz = WMAP_INITIAL_VIEW_DEPTH;
    g_wmap_camera_rotation.vx = WMAP_INITIAL_CAMERA_PITCH;
    g_wmap_camera_rotation.vz = WMAP_INITIAL_CAMERA_ROLL;
    g_wmap_camera_rotation.vy = 0;
    g_wmap_camera_translation.vx = WMAP_INITIAL_CAMERA_X;
    g_wmap_camera_translation.vy = WMAP_INITIAL_CAMERA_Y;
    g_wmap_camera_translation.vz = WMAP_INITIAL_CAMERA_DEPTH;
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_mesh_transition_frames = 0;
    g_wmap_mesh_previous_selection = 0;
    g_wmap_mesh_transition_selection = 0;
    g_wmap_mesh_transition_direction = 0;
    g_wmap_effect_camera_translation_offset = g_wmap_zero_translation;
    g_wmap_effect_camera_rotation_offset = g_wmap_zero_rotation;
    g_wmap_selection_phase = WMAP_SELECTION_MAP;
    g_wmap_event_active = 0;
    g_wmap_placement_overlay_hidden = 0;
    g_wmap_view_scroll_mode = 0;
    g_wmap_cursor_column = 1;
    g_wmap_cursor_row = 1;
    g_wmap_preview_travel_end = g_wmap_preview_travel_frame_bounds[WMAP_PREVIEW_CENTER_CELL_INDEX];
    g_wmap_preview_travel_frame = g_wmap_preview_travel_frame_bounds[WMAP_PREVIEW_CENTER_CELL_INDEX];
    g_wmap_screen_fade_mode = WMAP_FADE_HOLD;
    land_display_mode = WMAP_LAND_DISPLAY_ANIMATED;
    g_wmap_carousel_mode = WMAP_CAROUSEL_VISIBLE;
    g_wmap_land_display_limit = land_display_mode;
    D_801398F4 = 0;
    g_wmap_selected_artifact = -1;
    g_wmap_preview_artifact_visible = 0;
    g_wmap_artifact_transfer_end = 0;
    g_wmap_artifact_transfer_frame = 0;
    g_wmap_preview_shape = 0;
    g_wmap_preview_texture = 0;
    g_wmap_preview_draw_y = 0;
    g_wmap_preview_draw_x = 0;
    g_wmap_preview_y = 0;
    g_wmap_preview_x = 0;
    g_wmap_status_panel_mode = 0;
    D_8013B260.vy = 0;
    g_wmap_tint.r = 0;
    D_8013B260.vx = 0;
    g_wmap_tint.g = 0;
    g_wmap_tint.b = 0;
    SetPolyFT4(g_wmap_preview_saved_quads);
    /* Clear each packed (x, y) pair before copying the packet templates. */
    *(s32*)&g_wmap_preview_saved_quads[0].x2 = 0;
    *(s32*)&g_wmap_preview_saved_quads[0].x1 = 0;
    *(s32*)&g_wmap_preview_saved_quads[0].x0 = 0;
    g_wmap_preview_saved_quads[1] = g_wmap_preview_saved_quads[2] =
        g_wmap_preview_saved_quads[3] = g_wmap_preview_saved_quads[0];
    g_wmap_travel_day = wmap_get_day();
    wmap_refresh_cells();
    wmap_init_preview_lines();
}

/**
 * @brief A CVECTOR read back as the packed word a packet's color slot holds.
 */
typedef union
{
    CVECTOR rgb;
    u_long word;
} WmapPackedColor;

#if defined(VERSION_JP)
#define WMAP_BACKDROP_COLOR_WORD(color) (*(u_long*)&(color))
#else
#define WMAP_BACKDROP_COLOR_WORD(color) (((WmapPackedColor*)&(color))->word)
#endif

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
            color.r = g_wmap_backdrop_level * g_wmap_backdrop_tint.r / 16;
            color.g = g_wmap_backdrop_level * g_wmap_backdrop_tint.g / 16;
            color.b = g_wmap_backdrop_level * g_wmap_backdrop_tint.b / 16;
        }
        else
        {
            color.r = g_wmap_backdrop_level * g_wmap_backdrop_tint.r / 24;
            color.g = g_wmap_backdrop_level * g_wmap_backdrop_tint.g / 24;
            color.b = g_wmap_backdrop_level * g_wmap_backdrop_tint.b / 24;
        }
        color.cd = 0x2C;
        /* Each strip has two copies so it wraps across the screen. */
        if ((g_wmap_sequence_count == 0) && (g_wmap_map_controls_active != 0))
        {
            for (strip = 0; strip < 4; strip++)
            {
                packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
                *packet = g_wmap_backdrop_front_quads[strip];
                front_x = packet->x0 + SCREEN_WIDTH;
                wrapped_x = (front_x + g_wmap_backdrop_scroll) % WMAP_BACKDROP_WRAP_WIDTH;
                packet->x0 = packet->x2 = wrapped_x - SCREEN_WIDTH;
                packet->x1 = packet->x3 = wrapped_x - WMAP_BACKDROP_QUAD_WIDTH;
                SET_BGR0_PACKED(packet, WMAP_BACKDROP_COLOR_WORD(color));
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
                SET_BGR0_PACKED(packet, WMAP_BACKDROP_COLOR_WORD(color));
                packet->code |= 2;
                addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
                if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
                {
                    g_wmap_packet_bytes += sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
            if (!(g_wmap_frame_count & 3))
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
            SET_BGR0_PACKED(packet, WMAP_BACKDROP_COLOR_WORD(color));
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
            SET_BGR0_PACKED(packet, WMAP_BACKDROP_COLOR_WORD(color));
            addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(POLY_FT4);
                g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
            }
        }
#if !defined(VERSION_JP)
        return 1;
#endif
    }
#if defined(VERSION_JP)
    else
    {
#endif
        /* The gradient takes over once the textured backdrop has faded away. */
        if (D_80182D74.r != g_wmap_backdrop_gradient.r0)
        {
            if (g_wmap_backdrop_gradient.r0 >= D_80182D74.r)
            {
                g_wmap_backdrop_gradient.r0 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.r0 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D74.g != g_wmap_backdrop_gradient.g0)
        {
            if (g_wmap_backdrop_gradient.g0 >= D_80182D74.g)
            {
                g_wmap_backdrop_gradient.g0 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.g0 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D74.b != g_wmap_backdrop_gradient.b0)
        {
            if (g_wmap_backdrop_gradient.b0 >= D_80182D74.b)
            {
                g_wmap_backdrop_gradient.b0 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.b0 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D80.r != g_wmap_backdrop_gradient.r1)
        {
            if (g_wmap_backdrop_gradient.r1 >= D_80182D80.r)
            {
                g_wmap_backdrop_gradient.r1 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.r1 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D80.g != g_wmap_backdrop_gradient.g1)
        {
            if (g_wmap_backdrop_gradient.g1 >= D_80182D80.g)
            {
                g_wmap_backdrop_gradient.g1 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.g1 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D80.b != g_wmap_backdrop_gradient.b1)
        {
            if (g_wmap_backdrop_gradient.b1 >= D_80182D80.b)
            {
                g_wmap_backdrop_gradient.b1 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.b1 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D8C.r != g_wmap_backdrop_gradient.r2)
        {
            if (g_wmap_backdrop_gradient.r2 >= D_80182D8C.r)
            {
                g_wmap_backdrop_gradient.r2 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.r2 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D8C.g != g_wmap_backdrop_gradient.g2)
        {
            if (g_wmap_backdrop_gradient.g2 >= D_80182D8C.g)
            {
                g_wmap_backdrop_gradient.g2 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.g2 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D8C.b != g_wmap_backdrop_gradient.b2)
        {
            if (g_wmap_backdrop_gradient.b2 >= D_80182D8C.b)
            {
                g_wmap_backdrop_gradient.b2 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.b2 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D94.r != g_wmap_backdrop_gradient.r3)
        {
            if (g_wmap_backdrop_gradient.r3 >= D_80182D94.r)
            {
                g_wmap_backdrop_gradient.r3 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.r3 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D94.g != g_wmap_backdrop_gradient.g3)
        {
            if (g_wmap_backdrop_gradient.g3 >= D_80182D94.g)
            {
                g_wmap_backdrop_gradient.g3 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.g3 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        if (D_80182D94.b != g_wmap_backdrop_gradient.b3)
        {
            if (g_wmap_backdrop_gradient.b3 >= D_80182D94.b)
            {
                g_wmap_backdrop_gradient.b3 -= WMAP_BACKDROP_COLOR_STEP;
            }
            else
            {
                g_wmap_backdrop_gradient.b3 += WMAP_BACKDROP_COLOR_STEP;
            }
        }
        gradient = (POLY_G4*)g_wmap_current_frame->packet_cursor;
        *gradient = g_wmap_backdrop_gradient;
        addPrim(&g_wmap_current_frame->ordering_table[177], g_wmap_current_frame->packet_cursor);
        if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
        {
            g_wmap_packet_bytes += sizeof(POLY_G4);
            g_wmap_current_frame->packet_cursor += sizeof(POLY_G4);
        }
#if defined(VERSION_JP)
    }
#endif
    return 1;
}

/**
 * @brief Step the map fade quad's intensity and draw it while nonzero.
 * @param initialize Callback initialization flag; unused.
 * @return Zero when disabled, otherwise one to keep the callback active.
 */
s32 wmap_update_screen_fade(s32 initialize)
{
    POLY_FT4* packet;

    switch (g_wmap_screen_fade_mode)
    {
    case WMAP_FADE_DECREASE:
        g_wmap_screen_fade_quad.b0 -= WMAP_FADE_COLOR_STEP;
        g_wmap_screen_fade_quad.g0 = g_wmap_screen_fade_quad.b0;
        g_wmap_screen_fade_quad.r0 = g_wmap_screen_fade_quad.b0;
        if (g_wmap_screen_fade_quad.r0 == 0)
        {
            g_wmap_screen_fade_mode = WMAP_FADE_HOLD;
        }
        break;
    case WMAP_FADE_INCREASE:
        g_wmap_screen_fade_quad.b0 += WMAP_FADE_COLOR_STEP;
        g_wmap_screen_fade_quad.g0 = g_wmap_screen_fade_quad.b0;
        g_wmap_screen_fade_quad.r0 = g_wmap_screen_fade_quad.b0;
        if (g_wmap_screen_fade_quad.r0 == WMAP_FULL_BRIGHTNESS)
        {
            g_wmap_screen_fade_mode = WMAP_FADE_HOLD;
        }
        break;
    case WMAP_FADE_DISABLED:
        return 0;
    }
    if (g_wmap_screen_fade_quad.r0 != 0)
    {
        packet = (POLY_FT4*)g_wmap_current_frame->packet_cursor;
        *packet = g_wmap_screen_fade_quad;
        /* The full-intensity quad is opaque; intermediate levels blend. */
        if (g_wmap_screen_fade_quad.r0 != WMAP_FULL_BRIGHTNESS)
        {
            packet->code |= GPU_CODE_SEMI_TRANS;
        }
        addPrim(&g_wmap_current_frame->ordering_table[WMAP_SCREEN_FADE_OT_INDEX], g_wmap_current_frame->packet_cursor);
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
 * @brief Update the Select help menu and draw its cursor and current page.
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
        if (g_wmap_menu_page == WMAP_HELP_MENU)
        {
            map_controls_active = g_wmap_map_controls_active == 0;
            g_wmap_map_controls_active = map_controls_active;
            if (map_controls_active != 0)
            {
                akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_BACK - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
            }
            else
            {
                akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_OPEN_MENU - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
            }
        }
        else
        {
            g_wmap_menu_page = WMAP_HELP_MENU;
            akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_BACK - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
        }
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
    }
    if (g_wmap_map_controls_active != 0)
    {
        return;
    }
    previous_item = g_wmap_menu_selection - 1;
    cursor_target_y = previous_item * WMAP_MENU_ITEM_HEIGHT;
    if (cursor_target_y != g_wmap_menu_cursor_y)
    {
        if (cursor_target_y < g_wmap_menu_cursor_y)
        {
            g_wmap_menu_cursor_y -= WMAP_MENU_CURSOR_STEP;
        }
        else
        {
            g_wmap_menu_cursor_y += WMAP_MENU_CURSOR_STEP;
        }
    }
    else
    {
        /* Navigation resumes after the cursor reaches the selected row. */
        if (g_wmap_menu_page == WMAP_HELP_MENU)
        {
            if (g_wmap_buttons_repeat & PADLup)
            {
                g_wmap_menu_selection = previous_item;
                if (previous_item < WMAP_HELP_DIRECTIONS)
                {
                    g_wmap_menu_selection = WMAP_HELP_LAND_PLACEMENT;
                }
                akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_CURSOR - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
            }
            if (g_wmap_buttons_repeat & PADLdown)
            {
                next_item = g_wmap_menu_selection + 1;
                g_wmap_menu_selection = next_item;
                if (next_item > WMAP_HELP_LAND_PLACEMENT)
                {
                    g_wmap_menu_selection = WMAP_HELP_DIRECTIONS;
                }
                akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_CURSOR - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
            }
        }
        if ((g_wmap_buttons_repeat & WMAP_PAD_CONFIRM) && g_wmap_menu_page == WMAP_HELP_MENU)
        {
            g_wmap_menu_page = g_wmap_menu_selection;
            akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_CONFIRM - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
        }
        if (g_wmap_buttons_repeat & WMAP_PAD_CANCEL)
        {
            if (g_wmap_menu_page == WMAP_HELP_MENU)
            {
                g_wmap_map_controls_active = g_wmap_map_controls_active == 0;
                akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_BACK - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
                g_wmap_buttons_held = 0;
                g_wmap_buttons_repeat = 0;
                return;
            }
            g_wmap_menu_page = WMAP_HELP_MENU;
            akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_BACK - 1], 0, AKAO_PAN_CENTER, AKAO_VOLUME_MAX);
        }
    }
    if (g_wmap_loaded_menu_page != g_wmap_menu_page)
    {
        image = &g_wmap_help_image;
        g_wmap_loaded_menu_page = g_wmap_menu_page;
        cdrom_queue_read((u16)(g_wmap_menu_page + WMAP_HELP_IMAGE_RESOURCE_BASE), image);
        image_block = &image->clut_block;
        cdrom_wait_queue_empty();
        wmap_load_image_block(&image->clut_block);
        image_block = TIM_NEXT_BLOCK(image_block);
        wmap_load_image_block(image_block);
        DrawSync(0);
        g_wmap_artifact_shadows_enabled = 1;
    }
    if (g_wmap_menu_page == WMAP_HELP_MENU)
    {
        for (triangle_index = 0; triangle_index < WMAP_MENU_TRIANGLE_COUNT; triangle_index++)
        {
            triangle_template = &g_wmap_menu_triangles[triangle_index];
            triangle = (WmapMenuTriangle*)g_wmap_current_frame->packet_cursor;
            *triangle = *triangle_template;
            triangle->y0 += g_wmap_menu_cursor_y;
            triangle->y1 += g_wmap_menu_cursor_y;
            triangle->y2 += g_wmap_menu_cursor_y;
            addPrim(&g_wmap_current_frame->ordering_table[WMAP_MENU_OT_INDEX], triangle);
            if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
            {
                g_wmap_packet_bytes += sizeof(WmapMenuTriangle);
                g_wmap_current_frame->packet_cursor += sizeof(WmapMenuTriangle);
            }
        }
    }
    /* Prepending packets draws the subtractive image before the additive cursor. */
    wmap_queue_texture_page(WMAP_HELP_CURSOR_TPAGE, WMAP_MENU_OT_INDEX);
    sprite = (SPRT*)g_wmap_current_frame->packet_cursor;
    *sprite = g_wmap_prompt_sprite_template;
    addPrim(&g_wmap_current_frame->ordering_table[WMAP_MENU_OT_INDEX], sprite);
    if (g_wmap_packet_bytes < WMAP_PACKET_LIMIT)
    {
        g_wmap_packet_bytes += sizeof(SPRT);
        g_wmap_current_frame->packet_cursor += sizeof(SPRT);
    }
    wmap_queue_texture_page(WMAP_HELP_IMAGE_TPAGE, WMAP_MENU_OT_INDEX);
    g_wmap_buttons_held = 0;
    g_wmap_buttons_repeat = 0;
}

/**
 * @brief Wait for a scripted map event while allowing live controller input.
 */
void wmap_wait_for_script_event(void)
{
    switch (g_wmap_script_wait)
    {
    case WMAP_SCRIPT_WAIT_ARTIFACT:
        if (g_wmap_selected_artifact != -1)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    case WMAP_SCRIPT_WAIT_PLACEMENT:
        if (g_wmap_artifact_placement_frame != 0)
        {
            g_wmap_script_wait = 0;
        }
        else
        {
            wmap_read_controller();
        }
        break;
    case WMAP_SCRIPT_WAIT_PREVIEW:
        if (g_wmap_sequence_count == 0)
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
        if (g_wmap_selection_phase == 3)
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
    s32 buttons;
    s32 row;
    s32 column;
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
                image = &g_wmap_help_image;
                cdrom_queue_read((u16)script[0], image);
                image_block = &image->clut_block;
                cdrom_wait_queue_empty();
                wmap_load_image_block(&image->clut_block);
                image_block = (TimBlock*)((u8*)image_block + image->clut_block.bnum);
                wmap_load_image_block(image_block);
                DrawSync(0);
                g_wmap_artifact_shadows_enabled = 1;
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
                g_wmap_auxiliary_labels_hidden = 0;
            }
            else
            {
                g_wmap_auxiliary_labels_hidden = 1;
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
                for (column = 0; column < WMAP_GRID_SIZE; column++)
                {
                    if (g_wmap_cells[column][row].land_id == 0)
                    {
                        g_wmap_travelers[0].position_x = g_wmap_travelers[0].target_x = column * WMAP_MAP_CELL_SIZE;
                        g_wmap_travelers[0].next_cell_x = column;
                        g_wmap_travelers[0].next_cell_y = row;
                        g_wmap_travelers[0].position_y = g_wmap_travelers[0].target_y = row * WMAP_MAP_CELL_SIZE;
                        g_wmap_travelers[0].cell_x = g_wmap_travelers[0].destination_x = g_wmap_travelers[0].next_cell_x;
                        g_wmap_travelers[0].cell_y = g_wmap_travelers[0].destination_y = g_wmap_travelers[0].next_cell_y;
                        g_wmap_travelers[0].moving = 0;
                        column = WMAP_GRID_SIZE;
                        row = WMAP_GRID_SIZE;
                    }
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
        button_result = wmap_get_controller_repeat_buttons();
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
        button_result = wmap_get_controller_repeat_buttons() & (s16)((u16)script[0] & 0xF7FF);
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

/**
 * @brief Copy the destination rectangle from a TIM block header.
 * @param rectangle Receives the destination rectangle.
 * @param block TIM block whose header contains the rectangle.
 */
static inline void wmap_copy_rectangle(RECT* rectangle, TimBlock* block)
{
    *rectangle = *(RECT*)&block->dx;
}

/**
 * @brief Select the CD-error message strip in the error texture.
 * @param quad Packet receiving the texture coordinates and command.
 * @param error Message number, from one through five.
 */
static inline void wmap_set_error_uv(POLY_FT4* quad, s32 error)
{
    s32 top;
    setlen(quad, 9);
    setcode(quad, 0x2E);
    top = (error - 1) * WMAP_CD_ERROR_TEXTURE_HEIGHT;
    quad->v1 = top;
    quad->v0 = top;
    top += WMAP_CD_ERROR_TEXTURE_HEIGHT;
    quad->v3 = top;
    quad->v2 = top;
}

/**
 * @brief Copy a TIM block's destination rectangle and upload the block's pixels.
 * @param rect Receives the block's destination rectangle.
 * @param block TIM block header (length word, rectangle, then pixel data).
 */
static inline void wmap_upload_tim_block(RECT* rect, TimBlock* block)
{
    *rect = *(RECT*)&block->dx;
    LoadImage(rect, (u_long*)(block + 1));
}

/** @brief Clear the map image destination to black. */
static inline void wmap_clear_map_surface(RECT* rectangle)
{
    *rectangle = D_80051A88;
    ClearImage(rectangle, 0, 0, 0);
}

/**
 * @brief Upload a TIM image block unless its destination rectangle is unused.
 * @param rect Destination rectangle copied from the block header.
 * @param pixels Pixel data following the block header.
 */
static inline void wmap_upload_image_rect(RECT* rect, u_long* pixels)
{
    if (rect->x != -1)
    {
        LoadImage(rect, pixels);
        DrawSync(0);
        g_wmap_artifact_shadows_enabled = 1;
    }
}

/** @brief Upload and synchronize a TIM pixel block with a valid destination. */
static inline void wmap_load_pixel_block(TimBlock* block)
{
    RECT rect;

    rect = *(RECT*)&block->dx;
    if (rect.x != -1)
    {
        LoadImage(&rect, (u_long*)(block + 1));
        DrawSync(0);
        g_wmap_artifact_shadows_enabled = 1;
    }
}

/**
 * @brief Load world-map resources, process events, and render frames until exit.
 * @return Two for the controller reset chord, or zero after the exit effect.
 * @note JP handles land events 4-8 inside the event switch and keeps reading the
 *       queue; US stops at the first of them and drains the rest.
 */
s32 wmap_run_loop(void)
{
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
    s32 color_index;
    s32 show_loading_image;

    s32 scroll_cell_x;
    s32 scroll_cell_y;
    s32 palette_index;
    s32 pixel_index;
    u16* palette_entry;
    u16* screen_pixel;
    s32 controller_buttons;
    u16 packed_color;
    u32 event;
    SPRT* prompt_sprite;
    POLY_FT4* error_quad;

    layout_id = wmap_load_land_layout();
    show_loading_image = 1;
    g_wmap_layout_id = layout_id;
    g_wmap_entry_layout_id = layout_id;
    D_801ADB08 = -1;
    wmap_init_state();
    wmap_init_transition_mesh();
    /* Pending land events may replace the normal map entry sequence. */
    while (1)
    {
        event = wmap_next_land_event();
        g_wmap_last_land_event = event;
        if (event == -1U)
        {
            break;
        }
        g_wmap_pending_event_count += 1;
#if !defined(VERSION_JP)
        if ((event - 4) < 5U)
        {
            g_wmap_special_land_event = event;
            g_wmap_input_locked = 1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            g_wmap_pending_event_count = 1;
            {
                s32 drain_event;

                do
                {
                    drain_event = wmap_next_land_event();
                } while (drain_event != -1);
            }
            wmap_install_callback(&wmap_special_effect_34_run);
            break;
        }
#endif
        switch (event)
        {
        case 0:
            g_wmap_input_script = 1;
            g_wmap_party_visible = 0;
            g_wmap_attract_1_requested = 1;
            break;
        case 1:
            g_wmap_input_script = 2;
            break;
        case 28:
            g_wmap_input_script = 3;
            g_wmap_party_visible = 0;
            g_wmap_attract_1_requested = 1;
            break;
        case 25:
            g_wmap_pending_vehicle_events[0] = 1;
            break;
        case 17:
            g_wmap_pending_vehicle_events[1] = 1;
            break;
        case 18:
            g_wmap_pending_vehicle_events[2] = 1;
            break;
        case 19:
            g_wmap_pending_vehicle_events[3] = 1;
            break;
        case 20:
            g_wmap_pending_vehicle_events[4] = 1;
            break;
        case 22:
            g_wmap_third_traveler_enabled = 1;
            break;
        case 23:
            g_wmap_second_traveler_enabled = 1;
            break;
        case 21:
            g_wmap_land_event_21_pending = 1;
            break;
        case 16:
            g_wmap_land_event_16_pending = 1;
            break;
        case 11:
            show_loading_image = 0;
            g_wmap_entry_music_enabled = 0;
            g_wmap_land_event_11_pending = 1;
            break;
        case 15:
            g_wmap_land_event_15_pending = 1;
            break;
        case 12:
            show_loading_image = 0;
            wmap_select_mesh_motion(1);
            wmap_start_mesh_transition(1);
            g_wmap_entry_music_enabled = 0;
            g_wmap_party_visible = 0;
            g_wmap_status_panel_mode = 3;
            g_wmap_carousel_mode = WMAP_CAROUSEL_HIDDEN;
            g_wmap_screen_fade_mode = WMAP_FADE_DISABLED;
            g_wmap_spirit_brightness = g_wmap_spirit_target_brightness = 0;
            g_wmap_event_active = 1;
            g_wmap_entry_music_enabled = 0;
            g_wmap_entry_day = wmap_get_starting_cell(&g_wmap_travelers[0].cell_x, &g_wmap_travelers[0].cell_y);
            g_wmap_land_event_12_pending = 1;
            g_wmap_event_active = 1;
            break;
        case 13:
            show_loading_image = 0;
            wmap_select_mesh_motion(1);
            wmap_start_mesh_transition(1);
            g_wmap_entry_music_enabled = 0;
            g_wmap_party_visible = 0;
            g_wmap_status_panel_mode = 3;
            g_wmap_carousel_mode = WMAP_CAROUSEL_HIDDEN;
            g_wmap_screen_fade_mode = WMAP_FADE_DISABLED;
            g_wmap_spirit_brightness = g_wmap_spirit_target_brightness = 0;
            g_wmap_event_active = 1;
            g_wmap_entry_day = wmap_get_starting_cell(&g_wmap_travelers[0].cell_x, &g_wmap_travelers[0].cell_y);
            g_wmap_land_event_13_pending = 1;
            g_wmap_event_active = 1;
            break;
        case 9:
            show_loading_image = 0;
            wmap_select_mesh_motion(1);
            wmap_start_mesh_transition(1);
            g_wmap_overlay_fade_target = 0xFF;
            g_wmap_overlay_fade_level = 0xFF;
            g_wmap_overlay_fade_depth = 1;
            g_wmap_entry_music_enabled = 0;
            g_wmap_entry_fade_level = 0;
            wmap_install_callback(&func_80064D64);
            g_wmap_transition_mesh_hidden = 1;
            g_wmap_event_active = 1;
            g_wmap_party_visible = 0;
            g_wmap_carousel_mode = WMAP_CAROUSEL_HIDDEN;
            g_wmap_status_panel_mode = 3;
            g_wmap_spirit_brightness = g_wmap_spirit_target_brightness = 0;
            g_wmap_screen_fade_mode = WMAP_FADE_DISABLED;
            g_wmap_land_event_09_pending = 1;
            break;
        case 10:
            show_loading_image = 0;
            wmap_select_mesh_motion(1);
            wmap_start_mesh_transition(1);
            g_wmap_overlay_fade_target = 0xFF;
            g_wmap_overlay_fade_level = 0xFF;
            g_wmap_overlay_fade_depth = 1;
            g_wmap_entry_music_enabled = 0;
            g_wmap_party_visible = 0;
            g_wmap_event_active = 1;
            g_wmap_entry_fade_level = 0;
            wmap_install_callback(&func_80064D64);
            g_wmap_transition_mesh_hidden = 1;
            g_wmap_status_panel_mode = 3;
            g_wmap_carousel_mode = WMAP_CAROUSEL_HIDDEN;
            g_wmap_spirit_brightness = g_wmap_spirit_target_brightness = 0;
            g_wmap_screen_fade_mode = WMAP_FADE_DISABLED;
            g_wmap_land_event_10_active = 1;
            break;
        case 24:
            show_loading_image = 0;
            wmap_select_mesh_motion(1);
            wmap_start_mesh_transition(1);
            g_wmap_entry_music_enabled = 0;
            g_wmap_party_visible = 0;
            g_wmap_status_panel_mode = 3;
            g_wmap_carousel_mode = WMAP_CAROUSEL_HIDDEN;
            g_wmap_screen_fade_mode = WMAP_FADE_DISABLED;
            g_wmap_spirit_brightness = g_wmap_spirit_target_brightness = 0;
            g_wmap_event_active = 1;
            g_wmap_land_event_24_pending = 1;
            break;
#if defined(VERSION_JP)
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            g_wmap_input_locked = 1;
            g_wmap_buttons_held = 0;
            g_wmap_buttons_repeat = 0;
            g_wmap_special_land_event = g_wmap_last_land_event;
            wmap_install_callback(&wmap_special_effect_34_run);
            break;
#endif
        case 27:
            show_loading_image = 0;
            wmap_select_mesh_motion(1);
            wmap_start_mesh_transition(1);
            g_wmap_overlay_fade_target = 0;
            g_wmap_overlay_fade_level = 0;
            wmap_install_callback(&func_80064D64);
            g_wmap_land_event_27_pending = 1;
            g_wmap_entry_music_enabled = 0;
            g_wmap_party_visible = 0;
            g_wmap_status_panel_mode = 3;
            g_wmap_carousel_mode = WMAP_CAROUSEL_HIDDEN;
            g_wmap_screen_fade_mode = WMAP_FADE_DISABLED;
            g_wmap_spirit_brightness = g_wmap_spirit_target_brightness = 0;
            g_wmap_event_active = 1;
            g_wmap_attract_2_requested = 1;
            break;
        }
    }
    if (show_loading_image != 0)
    {
        {
            TimBlock* block;
            TimPrefix* image;
            s32 resource = wmap_get_land_count_tier() + WMAP_LOADING_IMAGE_RESOURCE_BASE;
            block = (TimBlock*)g_wmap_load_buffer;
            cdrom_queue_read(resource & 0xFFFF, block);
            cdrom_wait_queue_empty();
            block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
            image = (TimPrefix*)g_wmap_load_buffer;
            if (*(u8*)&image->flags & TIM_FLAG_HAS_CLUT)
            {
                wmap_load_image_block(block);
                block = TIM_NEXT_BLOCK(block);
                wmap_load_pixel_block(block);
            }
            else
            {
                wmap_load_pixel_block(block);
            }
        }
        g_wmap_loading_quad.b0 = 0x20;
        g_wmap_loading_quad.g0 = 0x20;
        g_wmap_loading_quad.r0 = 0x20;
        PutDispEnv(&g_wmap_frames[0].disp_env);
        PutDrawEnv(&g_wmap_frames[0].draw_env);
        SetDispMask(1);
        DrawPrim(&g_wmap_loading_quad);
        PutDispEnv(&g_wmap_frames[1].disp_env);
        PutDrawEnv(&g_wmap_frames[1].draw_env);
        DrawPrim(&g_wmap_loading_quad);
    }
    /* Load the sound banks before starting the world-map song. */
    if (g_wmap_land_event_11_pending != 0)
    {
        cdrom_queue_read(WMAP_EVENT_SOUND_BANK_RESOURCE, g_wmap_load_buffer);
    }
    else
    {
        cdrom_queue_read(WMAP_NORMAL_SOUND_BANK_RESOURCE, g_wmap_load_buffer);
    }
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking((AkaoBankHeader*)g_wmap_load_buffer, 1);
    cdrom_queue_read(WMAP_SHARED_SOUND_BANK_RESOURCE, g_wmap_load_buffer);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking((AkaoBankHeader*)g_wmap_load_buffer, 1);
    cdrom_queue_read(WMAP_MUSIC_RESOURCE, &g_wmap_music_buffer);
    cdrom_wait_queue_empty();
    if (show_loading_image != 0)
    {
        g_wmap_loading_quad.b0 = 0x40;
        g_wmap_loading_quad.g0 = 0x40;
        g_wmap_loading_quad.r0 = 0x40;
        PutDispEnv(&g_wmap_frames[0].disp_env);
        PutDrawEnv(&g_wmap_frames[0].draw_env);
        DrawPrim(&g_wmap_loading_quad);
        PutDispEnv(&g_wmap_frames[1].disp_env);
        PutDrawEnv(&g_wmap_frames[1].draw_env);
        DrawPrim(&g_wmap_loading_quad);
    }
    if (g_wmap_entry_music_enabled != 0)
    {
        akao_play_song(&g_wmap_music_buffer);
        akao_set_master_pan(0);
        akao_fade_song_volume_from(0, 30, 1, AKAO_VOLUME_MAX);
    }
    {
        TimBlock* block;
        TimPrefix* image;
        block = (TimBlock*)g_wmap_load_buffer;
        cdrom_stream((g_wmap_layout_id + WMAP_LAYOUT_IMAGE_RESOURCE_BASE) & 0xFFFF, (u8*)block);
        cdrom_wait_queue_empty();
        block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
        image = (TimPrefix*)g_wmap_load_buffer;
        if (*(u8*)&image->flags & TIM_FLAG_HAS_CLUT)
        {
            wmap_load_image_block(block);
            block = TIM_NEXT_BLOCK(block);
            {
                RECT rect;

                rect = *(RECT*)&block->dx;
                if (rect.x != -1)
                {
                    LoadImage(&rect, (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
        }
        else
        {
            RECT rect;

            rect = *(RECT*)&block->dx;
            wmap_upload_image_rect(&rect, (u_long*)(block + 1));
        }
    }
    /* Expand the entry image palette into five-bit RGB channels. */
    for (color_index = 0; color_index < GPU_CLUT_4BIT_COLORS; color_index++)
    {
        g_wmap_entry_palette_channels[color_index].r = g_wmap_entry_palette[color_index] & 0x1F;
        g_wmap_entry_palette_channels[color_index].g = (g_wmap_entry_palette[color_index] >> 5) & 0x1F;
        packed_color = g_wmap_entry_palette[color_index];
        g_wmap_entry_palette_channels[color_index].b = (packed_color >> 10) & 0x1F;
    }
    {
        RECT rect;

        rect.x = 0x278;
        rect.y = 0;
        rect.w = 0x10;
        rect.h = 0x180;
        MoveImage(&rect, 0x2B0, 0);
        rect.x = 0x240;
        rect.y = 0xE0;
        rect.w = 0x60;
        rect.h = 0x40;
        MoveImage(&rect, 0x240, 0x1C0);
        rect.x = 0x278;
        rect.y = 0xE0;
        rect.w = 0x10;
        rect.h = 0x40;
        MoveImage(&rect, 0x2B0, 0x1C0);
    }
    {
        RECT rects[3];

        {
            TimBlock* block;
            TimPrefix* image;
            block = (TimBlock*)g_wmap_load_buffer;
            cdrom_stream(WMAP_INTERFACE_IMAGE_RESOURCE, (u8*)block);
            cdrom_wait_queue_empty();
            block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
            image = (TimPrefix*)g_wmap_load_buffer;
            if (*(u8*)&image->flags & TIM_FLAG_HAS_CLUT)
            {
                wmap_upload_tim_block(&rects[0], block);
                block = TIM_NEXT_BLOCK(block);
                wmap_copy_rectangle(&rects[0], block);
                if (rects[0].x != -1)
                {
                    LoadImage(&rects[0], (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
            else
            {
                wmap_copy_rectangle(&rects[0], block);
                if (rects[0].x != -1)
                {
                    LoadImage(&rects[0], (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
        }
        g_wmap_backdrop_target_level = 8;
        wmap_install_callback(&wmap_draw_backdrop);
        cdrom_stream(WMAP_LAND_IMAGE_PACK_RESOURCE, (u8*)g_wmap_land_image_pack);
        if (show_loading_image != 0)
        {
            g_wmap_loading_quad.b0 = 0x80;
            g_wmap_loading_quad.g0 = 0x80;
            g_wmap_loading_quad.r0 = 0x80;
            PutDispEnv(&g_wmap_frames[0].disp_env);
            PutDrawEnv(&g_wmap_frames[0].draw_env);
            DrawPrim(&g_wmap_loading_quad);
            PutDispEnv(&g_wmap_frames[1].disp_env);
            PutDrawEnv(&g_wmap_frames[1].draw_env);
            DrawPrim(&g_wmap_loading_quad);
        }
        wmap_install_callback(&wmap_update_map_view);
        g_wmap_tint_blending = 1;
        g_wmap_tint_speed = 16;
        wmap_start_map_tint(GPU_TINT_NEUTRAL);
        {
            TimBlock* block;
            TimPrefix* image;
            cdrom_stream(WMAP_RESIDENT_LAND_IMAGE_RESOURCE, g_wmap_load_buffer);
            cdrom_wait_queue_empty();
            block = (TimBlock*)g_wmap_load_buffer;
            image = (TimPrefix*)g_wmap_load_buffer;
            if (*(u8*)&image->flags & TIM_FLAG_HAS_CLUT)
            {
                block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
                wmap_upload_tim_block(&rects[0], block);
                block = TIM_NEXT_BLOCK(block);
                rects[0] = *(RECT*)&block->dx;
                if (rects[0].x != -1)
                {
                    LoadImage(&rects[0], (u_long*)(block + 1));
                    DrawSync(0);
                }
            }
            else
            {
                block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
                rects[0] = *(RECT*)&block->dx;
                if (rects[0].x != -1)
                {
                    LoadImage(&rects[0], (u_long*)(block + 1));
                    DrawSync(0);
                }
            }
        }
        cdrom_queue_read(WMAP_RESIDENT_LAND_ANIMATION_RESOURCE, g_wmap_resident_land_animation_data);
        /* Slot 16 holds the resident land 31 image read above. */
        g_wmap_land_image_cache[WMAP_RESIDENT_LAND_SLOT].animation_data = g_wmap_resident_land_animation_data;
        g_wmap_land_image_cache[WMAP_RESIDENT_LAND_SLOT].slot_index = WMAP_RESIDENT_LAND_SLOT;
        g_wmap_land_image_cache[WMAP_RESIDENT_LAND_SLOT].resource_id = WMAP_RESIDENT_LAND_DISPLAY;
        g_wmap_land_image_cache[WMAP_RESIDENT_LAND_SLOT].loaded_frame = 0xFFFF;
        g_wmap_land_image_cache[WMAP_RESIDENT_LAND_SLOT].busy = 0;
        g_wmap_land_image_cache[WMAP_RESIDENT_LAND_SLOT].slot_index = WMAP_RESIDENT_LAND_SLOT;
        {
            TimBlock* block;
            TimPrefix* image;
            block = (TimBlock*)g_wmap_load_buffer;
            cdrom_queue_read(WMAP_MAP_OVERLAY_IMAGE_RESOURCE, block);
            cdrom_wait_queue_empty();
            block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
            image = (TimPrefix*)g_wmap_load_buffer;
            if (*(u8*)&image->flags & TIM_FLAG_HAS_CLUT)
            {
                wmap_upload_tim_block(&rects[0], block);
                block = TIM_NEXT_BLOCK(block);
                wmap_copy_rectangle(&rects[0], block);
                if (rects[0].x != -1)
                {
                    LoadImage(&rects[0], (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
            else
            {
                wmap_copy_rectangle(&rects[0], block);
                if (rects[0].x != -1)
                {
                    LoadImage(&rects[0], (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
        }
        g_wmap_land_display[WMAP_RESIDENT_LAND_DISPLAY].previous_animation_index = -1;
        g_wmap_land_display[WMAP_RESIDENT_LAND_DISPLAY].animation_index = 0;
        /* Clamp the map origin so the party stays within the scrolling area. */
        wmap_init_party_travel();
        scroll_cell_x = g_wmap_travelers[0].cell_x - 1;
        scroll_cell_y = g_wmap_travelers[0].cell_y - 1;
        if (scroll_cell_x < 0)
        {
            scroll_cell_x = 0;
            g_wmap_cursor_column = 0;
        }
        else if (scroll_cell_x >= 4)
        {
            scroll_cell_x = 3;
            g_wmap_cursor_column = 2;
        }
        if (scroll_cell_y < 0)
        {
            scroll_cell_y = 0;
            g_wmap_cursor_row = 0;
        }
        else if (scroll_cell_y >= 4)
        {
            scroll_cell_y = 3;
            g_wmap_cursor_row = 2;
        }
        g_wmap_view.vx = scroll_cell_x * WMAP_MAP_CELL_SIZE;
        g_wmap_view.vy = scroll_cell_y * WMAP_MAP_CELL_SIZE;
        g_wmap_travelers[0].next_cell_x = g_wmap_travelers[0].cell_x;
        g_wmap_travelers[0].next_cell_y = g_wmap_travelers[0].cell_y;
        traveler_x = (g_wmap_travelers[0].cell_x - 1) * WMAP_TRAVEL_CELL_SIZE;
        g_wmap_travelers[0].position_x = traveler_x;
        g_wmap_travelers[0].target_x = traveler_x;
        traveler_y = (g_wmap_travelers[0].cell_y - 1) * WMAP_TRAVEL_CELL_SIZE;
        g_wmap_travelers[0].position_y = traveler_y;
        g_wmap_travelers[0].target_y = traveler_y;
        wmap_clear_map_surface(&rects[0]);
        rects[0].x = 0x2C0;
        rects[0].y = 0x1FF;
        rects[0].w = 0x100;
        rects[0].h = 1;
        StoreImage(&rects[0], (u_long*)g_wmap_load_buffer);
        DrawSync(0);
        /* Leave entry zero unchanged and enable blending for the other colors. */
        {
            s16 blend_mask;
            palette_entry = (u16*)g_wmap_load_buffer;
            for (palette_index = 1, blend_mask = GPU_STP_BIT;
                 palette_index < CLUT_ENTRY_COUNT; palette_index++)
            {
                palette_entry[palette_index] |= blend_mask;
            }
        }
        rects[0].x = 0;
        rects[0].y = 0x1FF;
        LoadImage(&rects[0], (u_long*)g_wmap_load_buffer);
        DrawSync(0);
        {
            TimBlock* block;
            TimPrefix* image;
            block = (TimBlock*)g_wmap_load_buffer;
            cdrom_queue_read((wmap_get_land_count_tier() + WMAP_PLACEMENT_RESOURCE_BASE) & 0xFFFF, block);
            cdrom_wait_queue_empty();
            block = (TimBlock*)((u8*)block + TIM_HEADER_SIZE);
            image = (TimPrefix*)g_wmap_load_buffer;
            if (*(u8*)&image->flags & TIM_FLAG_HAS_CLUT)
            {
                wmap_upload_tim_block(&rects[1], block);
                block = TIM_NEXT_BLOCK(block);
                wmap_copy_rectangle(&rects[1], block);
                if (rects[1].x != -1)
                {
                    LoadImage(&rects[1], (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
            else
            {
                wmap_copy_rectangle(&rects[1], block);
                if (rects[1].x != -1)
                {
                    LoadImage(&rects[1], (u_long*)(block + 1));
                    DrawSync(0);
                    g_wmap_artifact_shadows_enabled = 1;
                }
            }
        }
        wmap_install_callback(&wmap_update_screen_fade);
        g_wmap_backdrop_target_level = 16;
        g_wmap_map_shadow_level = 1;
        if (g_wmap_pending_event_count == 0)
        {
            g_wmap_input_locked = 0;
        }
        update_controllers();
        g_wmap_frame_count = 0;
        /* Build one display list per frame, alternating the two packet buffers. */
        while (1)
        {
            if (g_wmap_frame_count & 1)
            {
                g_wmap_frames[0].packet_cursor = g_wmap_packet_buffer_0;
                g_wmap_current_frame = &g_wmap_frames[0];
            }
            else
            {
                g_wmap_frames[1].packet_cursor = g_wmap_packet_buffer_1;
                g_wmap_current_frame = &g_wmap_frames[1];
            }
            g_wmap_packet_bytes = 0;
            ClearOTagR(g_wmap_current_frame->ordering_table, WMAP_OT_COUNT);
            D_800DBE7C = 0;
            g_wmap_frame_count = g_wmap_frame_count + 1;
            if ((g_wmap_input_locked != 0) || (g_wmap_selection_phase >= WMAP_SELECTION_SHOW_ARTIFACTS) ||
                (g_wmap_mesh_transition_direction != 0) || (g_wmap_view_scroll_mode != 0) || (g_wmap_sequence_count != 0))
            {
                g_wmap_buttons_repeat = 0;
                g_wmap_buttons_held = 0;
            }
            else
            {
                wmap_read_controller();
            }
            if (g_wmap_frame_count < WMAP_INPUT_GRACE_FRAMES)
            {
                g_wmap_buttons_repeat &= WMAP_DPAD_MASK;
                g_wmap_buttons_held &= WMAP_DPAD_MASK;
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
            controller_buttons = g_wmap_controller_ports[1].published_sample.repeat_buttons;
            if ((controller_buttons >> 8) & PADL2)
            {
                g_wmap_controller_2_l2_repeat = 1;
            }
            else
            {
                g_wmap_controller_2_l2_repeat = 0;
            }
            raw_buttons = g_wmap_controller_ports->published_sample.held_buttons;
            raw_buttons = (raw_buttons >> 8) | (raw_buttons << 8);
            if ((raw_buttons & WMAP_RESET_CHORD) == WMAP_RESET_CHORD)
            {
                return 2;
            }
            if (raw_buttons & PADL1)
            {
                g_wmap_controller_1_l1_held = 1;
            }
            else
            {
                g_wmap_controller_1_l1_held = 0;
            }
            exit_frame = g_wmap_exit_frame;
            if (exit_frame != 0)
            {
                g_wmap_input_locked = 1;
                g_wmap_buttons_held = 0;
                g_wmap_buttons_repeat = 0;
                if ((exit_frame == 1) && (g_wmap_frame_count & 1))
                {
                    g_wmap_exit_frame = 2;
                }
                else
                {
                    g_wmap_exit_frame += 1;
                }
            }
            if (g_wmap_exit_frame >= WMAP_EXIT_CAPTURE_FRAME)
            {
                if (g_wmap_exit_frame == WMAP_EXIT_CAPTURE_FRAME)
                {
                    akao_fade_song_volume_from(0, 90, AKAO_VOLUME_MAX, 0);
                    if (g_wmap_exit_mode == WMAP_EXIT_SCREEN_PANELS)
                    {
                        akao_play_sfx_from_buffer(g_wmap_sfx_buffers[WMAP_SOUND_EXIT - 1], 0, 0x80, 0x7F);
                    }
                    func_8005DF50(g_wmap_travelers[0].cell_x, g_wmap_travelers[0].cell_y);
                    DrawSync(0);
                    /* Save the displayed frame for the exit transition. */
                    screen_pixel = (u16*)g_wmap_load_buffer;
                    StoreImage(&g_wmap_current_frame->disp_env.disp, (u_long*)g_wmap_load_buffer);
                    MoveImage(&g_wmap_current_frame->disp_env.disp, g_wmap_current_frame->draw_env.tw.x, g_wmap_current_frame->draw_env.tw.y);
                    DrawSync(0);
                    for (pixel_index = 0; pixel_index < SCREEN_WIDTH * SCREEN_HEIGHT; pixel_index++)
                    {
                        screen_pixel[pixel_index] |= WMAP_PIXEL_MASK_BIT;
                    }
                    /* Reload the captured frame into the 320x240 buffer at x = 320. */
                    rects[1] = (RECT){SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    LoadImage(&rects[1], (u_long*)g_wmap_load_buffer);
                    g_wmap_effect_model_c_rotation.vx = 0;
                    g_wmap_effect_model_c_rotation.vy = 0;
                    g_wmap_effect_model_c_rotation.vz = 0;
                    g_wmap_effect_model_c_position.vx = SCREEN_WIDTH / 2;
                    g_wmap_effect_model_c_position.vy = SCREEN_HEIGHT / 2;
                    g_wmap_effect_model_c_position.vz = 0;
                    g_wmap_effect_model_d_rotation.vx = 0;
                    g_wmap_effect_model_d_rotation.vy = 0;
                    g_wmap_effect_model_d_rotation.vz = 0;
                    g_wmap_effect_model_d_position.vx = SCREEN_WIDTH / 2;
                    g_wmap_effect_model_d_position.vy = SCREEN_HEIGHT / 2;
                    g_wmap_effect_model_d_position.vz = 0;
                    g_wmap_frames[1].draw_env.dtd = 0;
                    g_wmap_frames[0].draw_env.dtd = 0;
                    g_wmap_effect_model_b_position.vx = 3;
                    g_wmap_effect_model_b_position.vy = 0;
                    g_wmap_effect_model_b_position.vz = 0;
                    g_wmap_effect_model_b_rotation.vx = 8;
                    g_wmap_effect_model_b_rotation.vy = 12;
                    g_wmap_effect_model_b_rotation.vz = 0;
                    backdrop_choice = rand() & 3;
                    switch (backdrop_choice)
                    {
                    case 0:
                        g_wmap_exit_panel_color = 0x201010;
                        break;
                    case 1:
                        g_wmap_exit_panel_color = 0x101810;
                        break;
                    case 2:
                        g_wmap_exit_panel_color = 0x201020;
                        break;
                    case 3:
                        g_wmap_exit_panel_color = 0x202010;
                        break;
                    }
                    DrawSync(0);
                }
                if (g_wmap_exit_frame == WMAP_EXIT_END_FRAME)
                {
                    break;
                }
                switch (g_wmap_exit_mode)
                {
                case WMAP_EXIT_FADE_OVERLAY:
                    func_800641DC();
                    break;
                case WMAP_EXIT_SCREEN_PANELS:
                    func_8006454C();
                    break;
                case WMAP_EXIT_DIRECT:
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
                    *prompt_sprite = g_wmap_prompt_sprite_template;
                    addPrim(&g_wmap_current_frame->ordering_table[1], prompt_sprite);
                    packet_bytes = g_wmap_packet_bytes;
                    if (packet_bytes < WMAP_PACKET_LIMIT)
                    {
                        g_wmap_packet_bytes = packet_bytes + sizeof(SPRT);
                        g_wmap_current_frame->packet_cursor += sizeof(SPRT);
                    }
                    wmap_queue_texture_page(0xD5, 1);
                }
                if (g_wmap_view_mode == WMAP_VIEW_MODE_MAP)
                {
                    if (g_wmap_selected_artifact != -1)
                    {
                        g_wmap_auxiliary_label_mode = 3;
                    }
                    else if (g_wmap_selection_phase == WMAP_SELECTION_ARTIFACTS)
                    {
                        g_wmap_auxiliary_label_mode = g_wmap_selection_phase;
                    }
                    else
                    {
                        g_wmap_auxiliary_label_mode = 0;
                    }
                }
                else if (g_wmap_view_mode == WMAP_VIEW_MODE_SPIRITS)
                {
                    g_wmap_auxiliary_label_mode = 1;
                }
                if ((g_wmap_view_mode == WMAP_VIEW_MODE_MAP) && (g_wmap_game_start_delay != 0))
                {
                    wmap_update_artifact_selection();
                }
                wmap_update_view_zoom();
                SetSpadStack(WMAP_SCRATCH_STACK_SAVE_SLOT);
                wmap_update_callbacks();
                wmap_update_sequences();
                ResetSpadStack();
                wmap_update_map_game();
                if (g_wmap_game_start_delay != 0)
                {
                    wmap_draw_lands();
                }
                if (g_wmap_pending_event_count != 0)
                {
                    g_wmap_input_locked = 1;
                    if (g_wmap_frame_count >= WMAP_EVENT_START_DELAY_FRAMES)
                    {
                        func_800A5DFC();
                    }
                }
                if (g_wmap_view_mode != 0)
                {
                    func_8005FF88(-1);
                }
                func_8005F9BC();
                SetSpadStack(WMAP_SCRATCH_STACK_SAVE_SLOT);
                wmap_update_party_travel();
                wmap_update_map_display();
                wmap_update_land_preview();
                wmap_draw_artifact_carousel();
                wmap_draw_transition_mesh();
                ResetSpadStack();
            }
            func_8005D46C();
            g_wmap_land_image_cache_missed = 0;
            vsync_count = VSync(1);
            DrawSync(0);
            set_controller_vsync_interval(2);
            VSync(2);
            if (vsync_count >= WMAP_SHADOW_VSYNC_LIMIT)
            {
                g_wmap_artifact_shadows_enabled = 0;
            }
            if (!(g_wmap_frame_count & WMAP_SHADOW_RETRY_MASK))
            {
                g_wmap_artifact_shadows_enabled = 1;
            }
            PutDispEnv(&g_wmap_current_frame->disp_env);
            PutDrawEnv(&g_wmap_current_frame->draw_env);
            if (g_wmap_pending_palette[0] != 0)
            {
                rects[1].x = 0x240;
                rects[1].y = 0x151;
                g_wmap_pending_palette[0] = 0;
                rects[1].w = 0x10;
                rects[1].h = 1;
                LoadImage(&rects[1], (u_long*)g_wmap_pending_palette);
            }
            func_80064BF8();
            DrawOTag(&g_wmap_current_frame->ordering_table[WMAP_OT_COUNT - 1]);
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
                SET_BGR0_PACKED(error_quad, GPU_TINT_NEUTRAL);
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
                    g_wmap_packet_bytes = error_packet_bytes + sizeof(POLY_FT4);
                    g_wmap_current_frame->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
        return 0;
    }
}

/**
 * @brief Read and filter world-map input and publish vibration commands.
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
    g_wmap_buttons_held = g_wmap_controller_ports->published_sample.held_buttons;
    g_wmap_buttons_repeat = g_wmap_controller_ports->published_sample.repeat_buttons;
    g_wmap_buttons_held = ((u32)g_wmap_buttons_held >> 8) | (g_wmap_buttons_held << 8);
    repeat_buttons = ((u32)g_wmap_buttons_repeat >> 8) | (g_wmap_buttons_repeat << 8);
    g_wmap_buttons_repeat = repeat_buttons;
    switch (g_wmap_controller_ports->published_sample.device_type)
    {
    case CONTROLLER_DEVICE_DIGITAL:
        g_wmap_large_motor = 0;
        g_wmap_small_motor = 0;
        break;
    case CONTROLLER_DEVICE_ANALOG_JOYSTICK:
    case CONTROLLER_DEVICE_ANALOG:
        /* Stronger stick deflections shorten the directional repeat interval. */
        stick_x = g_wmap_controller_ports->published_sample.left_stick_x;
        stick_negative = stick_x < 0;
        repeat_shift_x = stick_x;
        if (stick_negative)
        {
            repeat_shift_x = -repeat_shift_x;
        }
        if (repeat_shift_x > WMAP_ANALOG_REPEAT_MAX_SHIFT)
        {
            repeat_shift_x = WMAP_ANALOG_REPEAT_MAX_SHIFT;
        }
        if (stick_x < 0)
        {
            left_repeat = repeat_buttons;
            if ((g_wmap_frame_count % (WMAP_ANALOG_REPEAT_BASE_FRAMES >> repeat_shift_x)) == 0)
            {
                left_repeat |= PADLleft;
            }
            g_wmap_buttons_repeat = left_repeat;
        }
        if (g_wmap_controller_ports->published_sample.left_stick_x > 0)
        {
            right_repeat = g_wmap_buttons_repeat;
            if ((g_wmap_frame_count % (WMAP_ANALOG_REPEAT_BASE_FRAMES >> repeat_shift_x)) == 0)
            {
                right_repeat |= PADLright;
            }
            g_wmap_buttons_repeat = right_repeat;
        }
        stick_y = g_wmap_controller_ports->published_sample.left_stick_y;
        stick_negative = stick_y < 0;
        repeat_shift_y = stick_y;
        if (stick_negative)
        {
            repeat_shift_y = -repeat_shift_y;
        }
        if (repeat_shift_y > WMAP_ANALOG_REPEAT_MAX_SHIFT)
        {
            repeat_shift_y = WMAP_ANALOG_REPEAT_MAX_SHIFT;
        }
        if (stick_y < 0)
        {
            up_repeat = g_wmap_buttons_repeat;
            if ((g_wmap_frame_count % (WMAP_ANALOG_REPEAT_BASE_FRAMES >> repeat_shift_y)) == 0)
            {
                up_repeat |= PADLup;
            }
            g_wmap_buttons_repeat = up_repeat;
        }
        if (g_wmap_controller_ports->published_sample.left_stick_y > 0)
        {
            down_repeat = g_wmap_buttons_repeat;
            if ((g_wmap_frame_count % (WMAP_ANALOG_REPEAT_BASE_FRAMES >> repeat_shift_y)) == 0)
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
        g_wmap_buttons_held |= WMAP_PAD_CONFIRM;
    }
    if (g_wmap_buttons_repeat & PAD_BTN_L3)
    {
        g_wmap_buttons_repeat |= WMAP_PAD_CONFIRM;
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
    g_wmap_controller_ports->small_motor_command = g_wmap_small_motor;
    g_wmap_controller_ports->actuator_control.fields.large_motor_command = g_wmap_large_motor;
    g_wmap_land_image_load_locked = 0;
}

/**
 * @brief Cache land display IDs, travel and placement checks, and spirit sprites.
 * @note Appearance overrides change only the cached display ID. Travel, placement,
 * and spirit calculations still use the original map cell.
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
            if (g_wmap_gato_appearance_override != 0)
            {
                if (land_id == WMAP_LAND_DISPLAY_GATO)
                {
                    land_id = WMAP_LAND_DISPLAY_GATO_OVERRIDE;
                }
            }
            if (g_wmap_domina_appearance_override != 0)
            {
                if (land_id == WMAP_LAND_DISPLAY_DOMINA)
                {
                    land_id = WMAP_LAND_DISPLAY_DOMINA_OVERRIDE;
                }
            }
            g_wmap_cells[x][y].land_id = land_id;
            g_wmap_cells[x][y].travel_allowed = wmap_is_other_land_cell(x, y);
            wmap_get_cell_spirit_sprites(x, y, g_wmap_cells[x][y].spirit_sprites);
            g_wmap_cells[x][y].placement_allowed = wmap_can_place_land(x, y, g_wmap_selected_artifact);
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

    g_wmap_forced_animated_land_id = -1;
    for (actor_index = 0; actor_index < WMAP_ACTOR_COUNT; actor_index++)
    {
        g_wmap_sprite_actors[actor_index].shade_step = 16;
        g_wmap_actor_motions[actor_index].state = 0;
    }
    wmap_refresh_cells();
    g_wmap_input_locked = 0;
    g_wmap_auxiliary_labels_hidden = 0;
    g_wmap_preview_artifact_visible = 0;
    g_wmap_artifact_placement_active = 0;
    g_wmap_selected_artifact = -1;
    g_wmap_preview_texture = 0;
    g_wmap_preview_shape = 0;
    wmap_reset_artifact_carousel();
    g_wmap_placement_preview_locked = 0;
    wmap_start_mesh_transition(0);
    g_wmap_land_display_limit = 2;
    g_wmap_placement_overlay_hidden = 0;
    g_wmap_spirit_target_brightness = 0x80;
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_artifact_placement_frame = 0;
    g_wmap_menu_page = WMAP_HELP_MENU;
    g_wmap_menu_cursor_y = 0;
    g_wmap_menu_selection = WMAP_HELP_DIRECTIONS;
    g_wmap_loaded_menu_page = WMAP_HELP_NOT_LOADED;
    g_wmap_map_button_mask = -1;
    akao_fade_song_volume_from(0, 0x1E, 1, 0x7F);
    g_wmap_artifact_shadows_enabled = 1;
    g_wmap_tint_speed = 4;
    g_wmap_last_effect_resource_set = -1;
}
