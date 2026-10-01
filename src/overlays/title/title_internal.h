#ifndef LOM_TITLE_INTERNAL_H
#define LOM_TITLE_INTERNAL_H

#include "title.h"
#include "cd_resources.h"
#include "display.h"
#include "gpu_packet.h"
#include "main.h"
#include "akao.h"
#include "pad.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "scene_state.h"
#include "saved_game.h"

/**
 * @brief Current screen-fade colour. RGB only; the fade-target struct carries the
 * step counter. Counterpart to g_fade_current in the CHECKPS overlay.
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
} FadeCurrent;

/**
 * @brief Target colour and remaining frames for the screen-fade interpolation.
 * Counterpart to g_fade_target in the CHECKPS overlay (which uses a single
 * FadeColor struct for both; here current/target have distinct sizes
 * because g_fade_current is followed immediately by another global).
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeTarget;

/** @brief Ordering-table entries of each TITLE frame buffer. */
#define TITLE_OT_LENGTH 0x1000

/** @brief Fade colour component that leaves the screen unchanged, and the length of a fade. */
#define TITLE_FADE_NEUTRAL 0x100
#define TITLE_FADE_FRAMES 0x14

/** @brief Shift applied to the second rand() value when two are combined into the game id. */
#define TITLE_RNG_HIGH_SHIFT 15

/** @brief Display environments and GPU command buffers used by TITLE. */
struct TitleMenuContext
{
    char first_buffer_header[0x40];
    u_long otag_buffer[TITLE_OT_LENGTH];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT front_screen;
    u_long prim_buffer[0x1000];
    u_long* next_prim_ptr;
    char unknown_0x80BC[0x3C10];

    char second_buffer_header[0x40];
    u_long otag_buffer2[TITLE_OT_LENGTH];
    DISPENV disp_env2;
    DRAWENV draw_env2;
    RECT back_screen;
};

extern s32 g_playtime_vsync_origin;
extern u8 g_title_selected_item;
extern s32 g_title_menu_exit_state;
/**
 * Base address of the AKAO instrument/sample bank loaded by load_title_audio_bank
 * (always 0x8013C000). Passed to akao_register_bank to register it with the
 * audio driver.
 */
extern u8* g_title_audio_bank_base;
extern unsigned char D_8003ECA0;
extern s32 g_title_idle_countdown;
/** @brief g_controller_device_type values at or above this mean no controller is connected. */
#define TITLE_PAD_UNAVAILABLE 0xFE
#define TITLE_REPEAT_DELAY 2
#define TITLE_INITIAL_REPEAT_DELAY 15
#define TITLE_ANALOG_LOW_THRESHOLD -1
#define TITLE_ANALOG_HIGH_THRESHOLD 2
#define TITLE_DPAD_BUTTONS (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT)
/** @brief Buttons other than the face buttons that still count as the same held input for repeat. */
#define TITLE_NON_REPEAT_BUTTON_MASK                                                                                                                           \
    (PAD_BTN_L2 | PAD_BTN_R2 | PAD_BTN_L1 | PAD_BTN_R1 | PAD_BTN_CROSS | PAD_BTN_CIRCLE | PAD_BTN_SELECT | PAD_BTN_L3 | PAD_BTN_START)

extern s32 g_debounced_input;
extern u8 g_title_menu_item_flags[];
extern u8 g_title_visible_item_rank;
extern u8 g_title_anim_frame;
/**
 * Four-frame texture-U animation for the title-menu cursor; indexed by
 * (g_title_anim_frame >> 2) & 3 in render_title_menu_items. The values select
 * adjacent 16-pixel-wide cursor images in the title-menu texture.
 */
extern u8 g_cursor_blink_u_offsets[];

/**
 * Timer used to implement input repeating (auto-repeat).
 * Controls the delay before a held button begins triggering actions rapidly.
 * Same semantics as the identically-named symbol in the CHECKPS overlay
 * (separate copy, different address).
 */
extern s32 g_input_repeat_timer;
extern s32 g_last_input_state;
/**
 * Self-relative offset table for the two title-menu TIMs uploaded by
 * init_title_menu_state: entries [1] and [2] are byte offsets from the table's
 * own base to each TIM. Entry [0] stores the asset count.
 */
extern u32 g_title_menu_tim_table[3];
extern s32 g_slot_slide_x;
extern s32 g_slot_slide_y;
extern s32 g_slot_selected_index;
extern s32 g_slot_slide_frames;
extern s32 g_slot_highlight_x;
extern s32 g_slot_highlight_target_x;
extern s32 g_slot_slide_x_lerped;
extern s32 g_slot_slide_y_lerped;
extern s32 g_slot_highlight_frames;
/** @brief Starting weapon of each weapon category; the chosen one becomes the hero's weapon. */
extern FieldItemRecord g_starting_weapon_records[FIELD_WEAPON_CATEGORY_COUNT];
/**
 * @brief One entry in the 27-element save-slot UI layout table (g_save_layout_table).
 */
typedef struct
{
    u8 flags;    /**< bit0=apply_slide, bit1=semi_transparent, bits2-3=abr */
    u8 type;     /**< prim type: 0=skip, 2=TILE, 3=POLY_FT4, 4=SPRT, other=glyph */
    u8 tex_slot; /**< index into g_save_layout_tex_table[] tex table (stride 0x10) */
    u8 pad;
    s16 x;        /**< screen base X (POLY_FT4, SPRT, glyph) */
    s16 y;        /**< screen base Y */
    s16 tile_x;   /**< screen X for TILE (slideX always added) */
    s16 tile_y;   /**< screen Y for TILE */
    u16 u0;       /**< initial U texture coordinate (glyph strip) */
    u16 v0;       /**< initial V; animated by animate_save_slot_panel for highlight entries */
    u16 width;    /**< TILE.w / glyph total pixel width (chunked at 128 px) */
    u16 height;   /**< TILE.h / glyph per-chunk sprite height */
    u32 reserved; /**< zero in every initial table entry; not read at runtime */
} SaveLayoutEntry;

/* Home U/V texture coordinate the panel's primary entry resets to. */
#define SAVE_SLOT_HOME_V 0x10
/* Vertical span of the slot highlight bar (added to g_slot_highlight_x to get
 * the bottom V coordinate). */
#define SAVE_HIGHLIGHT_SPAN 0x20
/* Scroll-window width when the panel is re-homed (minimum value; see
 * animate_save_slot_panel which adds the pan offset to this base). */
#define SAVE_SCROLL_WIDTH_HOME 0x40

/** @brief Save-slot picker entries, including the mode selector at index 18. */
extern SaveLayoutEntry g_save_layout_table[27];
/** @brief TIM pixel mode and size of an uploaded save-layout texture. */
typedef struct
{
    u32 mode : 3;
    u32 width : 10;
    u32 height : 10;
} SaveLayoutTexControl;

/**
 * @brief One texture-descriptor entry in g_save_layout_tex_table (stride 0x10).
 *
 * @details Holds the destination VRAM coordinates plus the source TIM pointer
 * and a packed control word that upload_save_layout_textures fills in from the
 * uploaded image's dimensions.
 */
typedef struct
{
    s16 tex_x;  /**< pixel-block destination VRAM X */
    s16 tex_y;  /**< pixel-block destination VRAM Y */
    s16 clut_x; /**< CLUT destination VRAM X */
    s16 clut_y; /**< CLUT destination VRAM Y */
    u8* src;    /**< source TIM-style blob */
    SaveLayoutTexControl control;
} SaveLayoutTex;

/**
 * Texture-descriptor table for the save-slot layout: 11 entries of stride 0x10,
 * each holding VRAM coords plus the source TIM pointer/control word uploaded by
 * upload_save_layout_textures. Indexed by SaveLayoutEntry::tex_slot and cast to
 * SaveLayoutTex* by consumers.
 */
extern SaveLayoutTex g_save_layout_tex_table[];
/** @brief Saved game loaded for a new game. */
extern u32 g_new_game_template[];
/** @brief Saved game loaded when TITLE starts directly in FIELD. */
extern u32 g_field_start_template[];
/** @brief Starting FieldCharacterRecord of the hero with character type 0. */
extern s32 g_hero_template_type_0[SAVED_CHARACTER_SIZE / 4];
/** @brief Starting FieldCharacterRecord of the hero with character type 1. */
extern s32 g_hero_template_type_1[SAVED_CHARACTER_SIZE / 4];

extern FadeCurrent g_fade_current;
extern FadeTarget g_fade_target;

/* Private calls that cross the title/title_save translation-unit boundary. */
void play_title_sfx(s32 sound_id, s32 pan);
void update_menu_input(void);

#endif
