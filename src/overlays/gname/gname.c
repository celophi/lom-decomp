#include "gname.h"
#include "gname_types.h"

#include "cdrom.h"
#include "common.h"
#include "controller.h"
#include "display.h"
#include "gpu_packet.h"
#include "main.h"
#include "pad.h"
#include "tim.h"
#include "psyq/libetc.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"
#include "psyq/memory.h"

/* Ordering-table slots, from frontmost overlay to background. */
enum
{
    GNAME_OT_FRONT = 0x00,
    GNAME_OT_TEXT_CURSOR = 0x08,
    GNAME_OT_PANEL_LABEL = 0x09,
    GNAME_OT_CHAR_PANEL = 0x0A,
    GNAME_OT_CHAR_GRID = 0x0B,
    GNAME_OT_GLYPH_APPEND_ANIM = 0x0C,
    GNAME_OT_GLYPH_APPEND = 0x0D,
    GNAME_OT_NAME_STRIP = 0x0E,
    GNAME_OT_LAYOUT_BACKGROUND = 0x0F,
    GNAME_OT_ENTRY_COUNT
};

/* Null-terminated variable-width name encoding. */
#define NAME_BYTE_SPACE 0x20
#define NAME_BYTE_ALT_BLANK 0x80
#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))
#define MAKE_DBCS_GLYPH(lead_byte, trail_byte) \
    (u16)(((u16)(trail_byte) << 8) | (u16)(lead_byte))
#define LOW_BYTE(value) ((value) & 0xFFU)
#define HIGH_BYTE(value) ((value) >> 8)
#define NAME_GLYPH_SIZE_SINGLE 1
#define NAME_GLYPH_SIZE_DOUBLE 2
#define NAME_GLYPH_VALUE_MASK 0xFFFFU
#define NAME_GLYPH_LEAD_INDEX 0
#define NAME_GLYPH_TRAIL_INDEX 1
#define NAME_GLYPH_TERMINATOR_INDEX 2
#define NAME_GLYPH_BUFFER_SIZE (NAME_GLYPH_TERMINATOR_INDEX + 1)

/* Input groups. */
#define GNAME_BTN_CONFIRM (PAD_BTN_CROSS | PAD_BTN_L3)
#define GNAME_BTN_UNDO PAD_BTN_L2
#define GNAME_BTN_REDO PAD_BTN_R2
#define GNAME_BTN_CANCEL PAD_BTN_CIRCLE
#define GNAME_BTN_KANJI_PREV PAD_BTN_L1
#define GNAME_BTN_KANJI_NEXT PAD_BTN_R1
#define GNAME_BTN_KANJI_NAV (GNAME_BTN_KANJI_PREV | GNAME_BTN_KANJI_NEXT)
#define GNAME_BTN_NAV_MASK (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT | GNAME_BTN_CONFIRM)

/* Navigation regions and action entries. */
enum
{
    GNAME_MODE_ACTION_OK = 0,
    GNAME_MODE_ACTION_DELETE,
    GNAME_MODE_ACTION_RANDOM,
    GNAME_MODE_ACTION_DEFAULT,
    GNAME_MODE_PANEL_BASE,
    GNAME_MODE_PANEL_NAV_LAST = GNAME_MODE_PANEL_BASE + 2,
    GNAME_MODE_PANEL_LAST = GNAME_MODE_PANEL_BASE + 3,
    GNAME_MODE_GRID = 0x10
};

#define GNAME_BUTTONS_NONE 0
#define GNAME_REDISPATCH_DONE 0
#define GNAME_REDISPATCH_PENDING 0xFF
#define GNAME_NAVIGATION_STEP 1
#define GNAME_CURSOR_POS_TABLE_OFFSET 2
#define GNAME_TAB_CURSOR_X_BIAS 8
#define GNAME_CURSOR_LERP_STEPS 5
#define GNAME_GRID_LERP_STEPS 4

#define GNAME_ENTRY_NONE 0xFF

/* Overlay completion results. */
enum
{
    GNAME_RESULT_PENDING = 0,
    GNAME_RESULT_CANCEL = 2,
    GNAME_RESULT_CONFIRM = 5
};

#define NAME_STRIP_LERP_STEPS 5
#define GLYPH_APPEND_ANIM_TIMER_START 2

/* Random-action name sources. */
enum
{
    GNAME_SRC_CUSTOM = 1,
    GNAME_SRC_HISTORY = 3,
    GNAME_SRC_RAND_PRIMARY,
    GNAME_SRC_RAND_ALT
};

enum
{
    GNAME_RENDER_BUFFER_A = 0,
    GNAME_RENDER_BUFFER_B
};

/* Run-loop timing and history layout. */
#define GNAME_FRAME_VSYNC_INTERVAL 2U
#define GNAME_INIT_STACK_PAD_WORDS 2
#define GNAME_FADE_IN_FRAMES 20
#define GNAME_STARTUP_DELAY_FRAMES 40
#define GNAME_NAME_BUFFER_SIZE 48
#define GNAME_HISTORY_LAYOUT_MASK 0x7F
#define GNAME_HISTORY_LAYOUT_LARGE 4
#define GNAME_HISTORY_COPY_SIZE 0x15
#define GNAME_LARGE_HISTORY_STRIDE sizeof(((PadContext*)0)->large_history_names[0])
#define GNAME_SMALL_HISTORY_STRIDE sizeof(((PadContext*)0)->small_history_names[0])
#define GNAME_LARGE_HISTORY_OFFSET ((u32)&((PadContext*)0)->large_history_names)
#define GNAME_SMALL_HISTORY_OFFSET ((u32)&((PadContext*)0)->small_history_names)
#define GNAME_USES_LARGE_HISTORY(ctx) (((ctx)->unkAA8 & GNAME_HISTORY_LAYOUT_MASK) == GNAME_HISTORY_LAYOUT_LARGE)

#define NAME_MAX_GLYPHS 10

/* Menu sound effects. */
enum
{
    GNAME_SFX_ERROR = 0x78,
    GNAME_SFX_MOVE = 0x7D,
    GNAME_SFX_CONFIRM,
    GNAME_SFX_CANCEL
};

#define GNAME_SFX_VOLUME 0x80

/* Character selection grid layout constants. */
#define NAME_GRID_COLUMNS 10
#define NAME_GRID_LAST_COL (NAME_GRID_COLUMNS - 1)
#define NAME_GRID_CELL_SIZE 16
#define NAME_GRID_X_BASE 84
#define NAME_GRID_Y_TOP 104
#define NAME_GRID_Y_BOTTOM 168
#define NAME_GRID_Y_EXIT_BOUND (NAME_GRID_Y_BOTTOM + 1)
#define NAME_GRID_SCROLL_STEP 64
#define NAME_GRID_CELL_SHIFT 4
#define NAME_GRID_DIV_BIAS (NAME_GRID_CELL_SIZE - 1)
#define NAME_GRID_VISIBLE_ROWS (NAME_GRID_VIS_HEIGHT / NAME_GRID_CELL_SIZE)
#define NAME_GRID_BACKING_X 0x60
#define NAME_GRID_BACKING_W 0xA0
#define NAME_GRID_VIS_HEIGHT 0x50
#define NAME_GRID_BACKING_PAGE0_Y NAME_GRID_Y_TOP
#define NAME_GRID_BACKING_PAGE1_Y 0x150
#define NAME_GRID_OVERSCAN 0x0B

enum
{
    CHAR_PANEL_STANDARD_FIRST = 0,
    CHAR_PANEL_STANDARD_COUNT = 3,
    CHAR_PANEL_KANJI_CATEGORY = 3,
    CHAR_PANEL_KANJI
};

#define KANJI_CATEGORY_EMPTY 0xFF
#define CHAR_PANEL_GLYPH_COLOR 1
#define CHAR_PANEL_GLYPH_MODE 0

#define RANDOM_NAME_COUNT 128
#define HISTORY_NAME_INDEX_LIMIT 0x81
#define HISTORY_SUFFIX_INDEX_BASE 130
#define NAME_CLIPBOARD_MAX_GLYPHS 11

#define KANJI_CATEGORY_STEP 10
#define KANJI_CATEGORY_COUNT 50
#define KANJI_CATEGORY_WRAP_OFFSET 41
#define KANJI_CATEGORY_PREV_EDGE -1
#define KANJI_CATEGORY_FIRST 0
#define KANJI_CATEGORY_NEXT_EDGE 9

/* The biased unsigned comparison implements both grid-window bounds. */
#define NAME_GRID_ROW_VISIBLE(y) (((u32)((y) + NAME_GRID_OVERSCAN)) <= (NAME_GRID_VIS_HEIGHT + NAME_GRID_OVERSCAN - 1))

/* Fade channels above neutral use additive blending. */
#define FADE_CHAN_NEUTRAL 0x100
#define FADE_CHAN_ADDITIVE 0x101
#define FADE_ADDITIVE_BIAS (FADE_CHAN_ADDITIVE - FADE_CHAN_NEUTRAL)

/* Full-screen fade blend pages. */
#define FADE_TPAGE_ADD 0x25
#define FADE_TPAGE_SUB 0x45

/* 4-bit glyph texture page at VRAM x=320. */
#define GNAME_GLYPH_TPAGE 5

#define GNAME_LAYOUT_SPRITE_COUNT 20
#define GNAME_TEXT_CURSOR_GLYPH_ID 20
#define GNAME_FULL_TEX_WINDOW_SIZE 0xFF

/* Selection entries rendered from g_tab_cursor_entries by gname_render. */
#define GNAME_SELECTION_ENTRY_FIRST 2
#define GNAME_SELECTION_ENTRY_END_EXCLUSIVE 13
#define GNAME_SELECTION_ENTRY_HIDDEN 9
#define GNAME_SELECTION_ENTRY_Y_BIAS 8
#define GNAME_SELECTION_SHADOW_OFFSET 1
enum
{
    GNAME_SCROLL_UP_ENTRY = 0,
    GNAME_SCROLL_DOWN_ENTRY
};

/* Static append indicator rendered before the append animation. */
#define GNAME_APPEND_GLYPH 3
#define GNAME_APPEND_X 0xE8
#define GNAME_APPEND_Y 4

/* Panel-tab sprite selection and placement. */
#define GNAME_PANEL_TAB_DEFAULT_RECORD 12
#define GNAME_PANEL_TAB_KANJI_RECORD_OFFSET 10
#define GNAME_PANEL_TAB_X 0xB0
#define GNAME_PANEL_TAB_Y 0xC8
#define GNAME_PANEL_LABEL_X 0x23
#define GNAME_PANEL_LABEL_Y 0x47
#define GNAME_PANEL_SPRITE_COLOR 1
#define GNAME_PANEL_SPRITE_MODE 2

/* Entered-name strip placement and backing-region dimensions. */
#define NAME_STRIP_TEXT_COLOR 1
#define NAME_STRIP_TEXT_X 0x10
#define NAME_STRIP_TEXT_Y 8
#define NAME_STRIP_TEXT_MODE 0
#define NAME_STRIP_DECOR_GLYPH 2
#define NAME_STRIP_BACKING_RIGHT 0xF0
#define NAME_STRIP_BACKING_PAGE0_Y 0x18
#define NAME_STRIP_BACKING_PAGE1_Y 0x100
#define NAME_STRIP_BACKING_HEIGHT 0x20
#define NAME_STRIP_HORIZONTAL_PADDING 0x18
#define NAME_MEASURE_CAPACITY 16
#define NAME_MEASURE_TEXT_COLOR 0

/* Resolve a glyph-table entry while retaining offset-plus-base evaluation order. */
#define GLYPH_TABLE_ENTRY(table, index) \
    ((const GlyphInfo*)(((index) * sizeof(*(table))) + (u32)(table)))

/* Glyph CLUT encoding. */
#define GLYPH_CLUT_X_MASK 0x3F
#define GLYPH_CLUT_X_SHIFT 4
#define GLYPH_CLUT_PAGE_BITS 0x7C80
#define GLYPH_SECONDARY_OFFSET_SCALE 2
#define GLYPH_SECONDARY_BLACK_TINT GPU_COLOR_WORD(0, 0, 0)
#define GLYPH_SECONDARY_BLUE_TINT GPU_COLOR_WORD(0, 0, 0xA0)

#define GLYPH_APPEND_ANIM_FRAME_COUNT 7

#define GLYPH_APPEND_ANIM_X_BIAS 0xE8
#define GLYPH_APPEND_ANIM_Y_BIAS 4

/** @brief Header of the serialized name-entry resource. */
typedef struct
{
    u32 unknown_0x00;
    u32 panel_records_offset;
    u32 kanji_records_offset;
    u32 history_names_offset;
    u32 random_names_offset;
} GnameDataHeader;

/** @brief Table of self-relative offsets to packed name-entry records. */
typedef struct
{
    u16 offsets[1];
} GnameRecordTable;

/* Typed views over the serialized name-entry resource. */
#define GNAME_HEADER_OFFSET(member) ((u32)&((GnameDataHeader*)0)->member)
#define GNAME_HEADER_FROM_FIELD(symbol, member) \
    ((const GnameDataHeader*)((u8*)&(symbol) - GNAME_HEADER_OFFSET(member)))
#define GNAME_RANDOM_NAMES_FIELD_OFFSET GNAME_HEADER_OFFSET(random_names_offset)
#define PANEL_DATA_HEADER GNAME_HEADER_FROM_FIELD(g_panel_tbl_off, panel_records_offset)
#define KANJI_DATA_HEADER GNAME_HEADER_FROM_FIELD(g_kanji_panel_offset, kanji_records_offset)
#define PANEL_RECORD_TABLE \
    ((const GnameRecordTable*)((u8*)PANEL_DATA_HEADER + g_panel_tbl_off))
#define KANJI_RECORD_TABLE \
    ((const GnameRecordTable*)((u32)KANJI_DATA_HEADER + (u32)g_kanji_panel_offset))
#define PANEL_CHARACTER_TABLE \
    ((GnameRecordTable*)((g_random_names_off - GNAME_RANDOM_NAMES_FIELD_OFFSET) + g_panel_tbl_off))
#define KANJI_CHARACTER_TABLE \
    ((GnameRecordTable*)((g_random_names_off - GNAME_RANDOM_NAMES_FIELD_OFFSET) + (u32)g_kanji_panel_offset))
#define RANDOM_NAME_TABLE \
    ((GnameRecordTable*)((g_random_names_off - GNAME_RANDOM_NAMES_FIELD_OFFSET) + \
                         (*((u32*)g_random_names_off))))
#define HISTORY_NAME_TABLE \
    ((GnameRecordTable*)((g_random_names_off - GNAME_RANDOM_NAMES_FIELD_OFFSET) + \
                         (*((u32*)g_history_names_off))))
/* Raw u16 indexing preserves GCC's required address evaluation. */
#define GNAME_RECORD(table, index) ((u8*)(table) + ((u16*)(table))[(index)])
#define GNAME_RECORD_IN_RANGE(table, range_start, index) \
    ((u8*)(table) + (&(table)->offsets[(range_start)])[(index)])

#define RANDOM_NAME(index) \
    ((u8*)RANDOM_NAME_TABLE + (&RANDOM_NAME_TABLE->offsets[0])[(index)])
#define HISTORY_NAME(index) \
    ((u8*)HISTORY_NAME_TABLE + (&HISTORY_NAME_TABLE->offsets[0])[(index)])
#define HISTORY_SUFFIX_TABLE \
    ((GnameRecordTable*)((g_history_names_off - GNAME_RANDOM_NAMES_FIELD_OFFSET) + \
                         (*((u32*)g_history_names_off))))
#define HISTORY_SUFFIX(index) \
    ((u8*)HISTORY_NAME_TABLE + (&HISTORY_SUFFIX_TABLE->offsets[0])[(index)])

/** @brief RGB fade color with an optional remaining interpolation count. */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps_remaining;
} FadeState;

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} FadePrimitive;

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define NEXT_FADE_PACKET(packet, type) ((FadePrimitive*)((u8*)(packet) + sizeof(type)))

/** @brief Pixel and CLUT destinations for a TIM upload. */
typedef struct
{
    s16 pixel_x;
    s16 pixel_y;
    s16 clut_x;
    s16 clut_y;
} TimUploadCoords;

/** @brief Partially mapped glyph measurement with width at offset 0x10. */
typedef struct
{
    u8 unknown_0x00[0x10];
    s16 width;
    u8 unknown_0x12[2];
} GlyphMeasure;

/** @brief DRAWENV scratch with the required leading stack padding. */
typedef struct
{
    u32 padding[2];
    DRAWENV draw_env;
} DrawEnvScratch;

/** @brief DRAWENV scratch with one trailing stack word. */
typedef union
{
    DRAWENV draw_env;
    u8 bytes[sizeof(DRAWENV) + sizeof(u32)];
} GridDrawEnvScratch;

/* Typed views over packed GNAME data. */
extern GlyphAppendAnimFrame g_glyph_append_anim_frames[];
extern Tim g_name_entry_tim;
extern GlyphSeqEntry g_layout_sprite_sequence[];

/* Overlay BSS layout is address-sensitive; do not reorder these definitions. */

/** Custom name source. */
u8 g_custom_name_buf[GNAME_NAME_BUFFER_SIZE];
/** Random-action source selector. */
s32 g_name_source_mode;
/** Pending, cancel, or confirm result. */
s32 g_overlay_result;
/** Name restored when the session resets. */
u8 g_initial_name[GNAME_NAME_BUFFER_SIZE];
/** RGB fade target color plus remaining lerp step count. */
FadeState g_fade_target;
/** RGB fade current interpolated color. */
FadeState g_fade_current;
/** If non-zero, pressing cancel while the name is empty triggers an overlay exit. */
s32 g_allow_empty_cancel;
/** Index into the saved-name history list (used when g_name_source_mode == 3). */
s32 g_history_name_idx;
/** Base of the two frame render buffers. */
RenderContext* g_render_buf_base;
/** Active name buffer the UI edits in place. */
u8* g_active_name;
/** Active standard or kanji character panel. */
s32 g_char_panel;
/** Current kanji category display data. */
void* g_kanji_cat_name;
/** Undo clipboard; removed glyphs are prepended here. */
u8 g_name_clipboard[GNAME_NAME_BUFFER_SIZE];
/** Frames remaining before name-entry input is accepted at startup. */
s32 g_startup_delay;
/** Frames remaining in the cursor-position lerp animation. */
s32 g_cursor_lerp_steps;
/** Action/panel entry activated this frame (0xFF = none). */
s32 g_activated_entry;
/** Cursor current X position (being lerped toward g_cursor_x_target). */
s32 g_cursor_x;
/** Cursor current Y position (being lerped toward g_cursor_y_target). */
s32 g_cursor_y;
/** Cursor target X position for the lerp animation. */
s32 g_cursor_x_target;
/** Last column index of the rightmost character in the current grid panel. */
s32 g_char_last_col;
/** Cursor target Y position for the lerp animation. */
s32 g_cursor_y_target;
/** Row index of the last glyph in the current grid panel (used for scroll bounds). */
s32 g_char_last_row;
/** Frames remaining in the name-strip width lerp animation. */
s32 g_strip_width_steps;
/** Current name-strip width in pixels (being lerped toward g_strip_width_target). */
s32 g_strip_width;
/** Current navigation region: 0-7 = action/panel tabs, 0x10 = active panel grid. */
s32 g_navigation_mode;
/** Current frame index into g_glyph_append_anim_frames. */
u8 g_glyph_append_anim_frame;
/** Explicit GCC 2.7.2 BSS alignment padding. */
u8 pad_8014F8B1[3];
/** Current horizontal scroll position of the character grid in pixels. */
s32 g_scroll_pos;
/** Render ticks until the next append-animation frame. */
u8 g_glyph_append_anim_timer;
/** Explicit GCC 2.7.2 BSS alignment padding. */
u8 pad_8014F8B9[3];
/** Target name-strip width in pixels for the width lerp. */
s32 g_strip_width_target;
/** Target horizontal scroll position for the scroll lerp. */
s32 g_scroll_target;
/** Frames remaining in the scroll lerp animation. */
s32 g_scroll_steps;
/** Currently selected kanji category index. */
s32 g_kanji_cat;
/** Rendered pixel width of the current name. */
s32 g_name_pixel_width;
/** Linearized character cursor position in the grid: row * 10 + col. */
s32 g_char_cursor;
/** Unknown trailing BSS word retained for layout. */
s32 D_8014F8D4;

/** Overlay header identifier stored immediately before @ref gname_run. */
const s32 g_gname_overlay_id = 5;

/* Packed table symbols defined by gname_data.c. */
extern u32 g_panel_char_offsets[];
extern s32 g_kanji_cat_names_offset;
extern u32 g_kanji_cat_entries[];
extern GlyphInfo g_glyph_table[];
extern TabCursorEntry g_tab_cursor_pos[];
extern TabCursorEntry g_tab_cursor_entries[];
extern u32 g_kanji_entry_offsets[];

/* Serialized panel-blob header fields. Types preserve matched address math. */
extern u8 g_panel_data_base[];
extern u32 g_panel_tbl_off;
extern u8* g_kanji_panel_offset;
extern u8 g_history_names_off[];
extern u8 g_random_names_off[];

/* Cross-module helpers without shared headers. */
/**
 * @brief Play a one-shot menu sound effect.
 * @param sfx_id Sound-effect identifier.
 * @param volume Playback volume.
 */
void play_menu_sfx(s32 sfx_id, s32 volume);
void func_80063194(void);
void func_8006441C(void);
void field_update_audio_timer(void);
void func_800A9E78(void);
void func_800AA02C(void);

/* Static forward declarations retain the original function order. */
static void reset_fade_state(void);
static void render_fade_overlay(RenderContext* render_ctx);
static void set_fade_target(s32 red, s32 green, s32 blue, s32 step_count);
static void load_name_entry_tim(void);
static void load_tim_to_vram(const TimUploadCoords* upload_coords);
static void gname_update_state(void);
static void reset_run_state(void);
static s32 handle_navigation_input(s32 mode, s32 buttons);
static void gname_process_input(void);
static u_long* emit_cursor_glyph(
    u_long* packet_cursor,
    u_long* ot_entry,
    s16 x,
    s16 y);
static void gname_render(RenderContext* render_ctx);
static void* emit_panel_tab_sprite(void* packet_cursor, u_long* ot_entry);
static void* emit_panel_label(void* packet_cursor, u_long* ot_entry);
static void render_name_strip(RenderContext* render_ctx, u8* name, s32 strip_width);
static void render_char_panel(RenderContext* render_ctx, s32 panel_index);
static void* emit_draw_mode_prim(DR_TPAGE* packet, u_long* ot_entry);
static void* emit_glyph_sprt(
    void* packet_start,
    u_long* ot_entry,
    s32 glyph_id,
    s32 base_x,
    s32 base_y,
    s32 shadow_offset,
    s32 activation_adjust,
    s32 use_blue_overlay);
static void render_layout_sprite_batch(RenderContext* render_ctx);
static s32 name_byte_length(const u8* name_buf);
static s32 name_glyph_count(const u8* name_buf);
static void name_append(u8* destination, const u8* source);
static s32 name_pop_last_glyph(u8* name_buf);
static void name_copy(u8* destination, const u8* source);
static void recalc_name_width(void);
static void name_prepend_glyph(u8* name_buf, u16 new_glyph);
static s32 name_pop_first_glyph(u8* name_buf);
static void* render_glyph_append_anim(void* packet_cursor, RenderContext* render_ctx);
static s32 name_is_blank(const u8* name_buf);

/**
 * @brief Run the name-entry UI until the user confirms or cancels.
 * @param render_buffers Double-buffered frame render contexts.
 * @param initial_name Name restored when the session resets.
 * @param active_name Name buffer edited by the UI.
 * @param source_mode Random-name source selector.
 * @param history_index History-list entry selector.
 * @param custom_name Custom random-name source.
 * @param allow_empty_cancel Whether cancel may exit with an empty name.
 * @return Final cancel or confirmation result.
 * @see https://decomp.me/scratch/FAyP7 (100%)
 */
s32 gname_run(
    RenderContext* render_buffers,
    const u8* initial_name,
    u8* active_name,
    s32 source_mode,
    s32 history_index,
    const u8* custom_name,
    s32 allow_empty_cancel)
{
    s32 history_byte_index;
    RenderContext* draw_buffer;
    RenderContext* next_buffer;
    RenderContext* other_buffer;
    RenderContext* draw_env_buffers;

    /* Install the caller's buffers and seed the persistent name-entry state. */
    g_render_buf_base = render_buffers;
    bcopy(initial_name, g_initial_name, sizeof(g_initial_name));
    bcopy(custom_name, g_custom_name_buf, sizeof(g_custom_name_buf));
    g_allow_empty_cancel = allow_empty_cancel;
    g_active_name = active_name;
    g_name_source_mode = source_mode;
    g_history_name_idx = history_index;

    /* Configure the two overlapping VRAM display/draw pages. */
    g_render_buf_base[GNAME_RENDER_BUFFER_A].clear_rect.x = 0;
    g_render_buf_base[GNAME_RENDER_BUFFER_A].clear_rect.y = VRAM_BACK_DRAW_Y;
    g_render_buf_base[GNAME_RENDER_BUFFER_A].clear_rect.w = SCREEN_WIDTH;
    g_render_buf_base[GNAME_RENDER_BUFFER_A].clear_rect.h = VRAM_DRAW_HEIGHT;
    g_render_buf_base[GNAME_RENDER_BUFFER_B].clear_rect.x = 0;
    g_render_buf_base[GNAME_RENDER_BUFFER_B].clear_rect.y = SCREEN_HEIGHT;
    g_render_buf_base[GNAME_RENDER_BUFFER_B].clear_rect.w = SCREEN_WIDTH;
    g_render_buf_base[GNAME_RENDER_BUFFER_B].clear_rect.h = VRAM_DRAW_HEIGHT;

    VSync(0);
    DrawSync(0);
    SetDefDispEnv(&g_render_buf_base[GNAME_RENDER_BUFFER_A].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_render_buf_base[GNAME_RENDER_BUFFER_B].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_render_buf_base[GNAME_RENDER_BUFFER_A].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_render_buf_base[GNAME_RENDER_BUFFER_B].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    draw_env_buffers = g_render_buf_base;
    draw_env_buffers[GNAME_RENDER_BUFFER_B].draw_env.dtd = FALSE;
    draw_env_buffers[GNAME_RENDER_BUFFER_A].draw_env.dtd = FALSE;
    g_overlay_result = GNAME_RESULT_PENDING;
    g_render_buf_base[GNAME_RENDER_BUFFER_A].frame_parity = GNAME_RENDER_BUFFER_A;
    g_render_buf_base[GNAME_RENDER_BUFFER_B].frame_parity = GNAME_RENDER_BUFFER_B;

    reset_fade_state();
    set_fade_target(FADE_CHAN_NEUTRAL, FADE_CHAN_NEUTRAL, FADE_CHAN_NEUTRAL, GNAME_FADE_IN_FRAMES);
    gname_init();

    /* Clear both ordering tables before enabling display output. */
    next_buffer = g_render_buf_base;
    ClearOTagR(next_buffer->ot, GNAME_OT_ENTRY_COUNT);
    ClearOTagR(g_render_buf_base[GNAME_RENDER_BUFFER_B].ot, GNAME_OT_ENTRY_COUNT);
    VSync(0);
    PutDispEnv(&next_buffer->disp_env);
    update_controllers();
    SetDispMask(TRUE);
    func_800AA02C();

    /* Render and submit frames until the overlay reports a final result. */
    while (TRUE)
    {
        draw_buffer = next_buffer;
        ClearOTagR(draw_buffer->ot, GNAME_OT_ENTRY_COUNT);
        draw_buffer->prim_cursor = &draw_buffer->ot[GNAME_OT_ENTRY_COUNT];
        func_8006441C();
        func_800A9E78();
        render_fade_overlay(draw_buffer);
        gname_tick(draw_buffer);
        func_80063194();

        if (g_overlay_result != GNAME_RESULT_PENDING)
        {
            break;
        }

        field_update_audio_timer();
        DrawSync(0);
        set_controller_vsync_interval(GNAME_FRAME_VSYNC_INTERVAL);
        VSync(GNAME_FRAME_VSYNC_INTERVAL);

        if (g_overlay_result != GNAME_RESULT_PENDING)
        {
            break;
        }

        ClearImage(&draw_buffer->clear_rect, 0U, 0U, 0U);

        /* Select the other half of the double buffer for the next frame. */
        other_buffer = g_render_buf_base;

        if (draw_buffer == g_render_buf_base)
        {
            other_buffer = &g_render_buf_base[GNAME_RENDER_BUFFER_B];
        }

        next_buffer = other_buffer;
        PutDispEnv(&other_buffer->disp_env);
        PutDrawEnv(&next_buffer->draw_env);
        DrawOTag(&draw_buffer->ot[GNAME_OT_LAYOUT_BACKGROUND]);
        draw_buffer = other_buffer;
        update_controllers();
        cdrom_process_state();
    }

    DrawSync(0);
    VSync(0);
    func_800AA02C();

    /* Persist edits only when this run targeted the pad context's history name. */
    if ((source_mode == GNAME_SRC_HISTORY) && (active_name == g_pad_ctx->gname_name))
    {
        history_byte_index = 0;
        if (GNAME_USES_LARGE_HISTORY(g_pad_ctx))
        {
            while (history_byte_index < GNAME_HISTORY_COPY_SIZE)
            {
                s32 history_slot_offset = g_pad_ctx->large_history_index * GNAME_LARGE_HISTORY_STRIDE;
                u8* history_byte = (u8*)g_pad_ctx + history_slot_offset + history_byte_index;

                history_byte[GNAME_LARGE_HISTORY_OFFSET] = active_name[history_byte_index];
                history_byte_index++;
            }
        }
        else
        {
            while (history_byte_index < GNAME_HISTORY_COPY_SIZE)
            {
                s32 history_slot_offset = g_pad_ctx->small_history_index * GNAME_SMALL_HISTORY_STRIDE;
                u8* history_byte = (u8*)g_pad_ctx + history_slot_offset + history_byte_index;

                history_byte[GNAME_SMALL_HISTORY_OFFSET] = active_name[history_byte_index];
                history_byte_index++;
            }
        }
    }

    return g_overlay_result;
}

/**
 * @brief Clear the RGB fade state to all zeros.
 *
 * Zeros @c g_fade_current (current color) and @c g_fade_target (target color
 * plus step count), so the fade starts from black with no animation pending.
 *
 * @see https://decomp.me/scratch/ld2aW (100%)
 */
static void reset_fade_state(void)
{
    g_fade_current.red = 0;
    g_fade_current.green = 0;
    g_fade_current.blue = 0;
    g_fade_target.red = 0;
    g_fade_target.green = 0;
    g_fade_target.blue = 0;
    g_fade_target.steps_remaining = 0;
}

/**
 * @brief Advance and render the full-screen fade overlay.
 * @param render_ctx Frame render context.
 * @see https://decomp.me/scratch/NvocJ (100%)
 */
static void render_fade_overlay(RenderContext* render_ctx)
{
    FadePrimitive* fade_packet = render_ctx->prim_cursor;
    RenderContext* ordering_table_ctx = render_ctx;
    s32 red_step;
    s32 green_step;
    s32 blue_step;
    s32 blend_mode_tpage;

    /* Interpolate toward the target, or snap when the fade is complete. */
    if (g_fade_target.steps_remaining != 0)
    {
        red_step = (g_fade_target.red - g_fade_current.red) / g_fade_target.steps_remaining;
        green_step = (g_fade_target.green - g_fade_current.green) / g_fade_target.steps_remaining;
        blue_step = (g_fade_target.blue - g_fade_current.blue) / g_fade_target.steps_remaining;
        g_fade_target.steps_remaining--;
        g_fade_current.red += red_step;
        g_fade_current.green += green_step;
        g_fade_current.blue += blue_step;
    }
    else
    {
        g_fade_current.red = g_fade_target.red;
        g_fade_current.green = g_fade_target.green;
        g_fade_current.blue = g_fade_target.blue;
    }

    /* A neutral fade requires no GPU packets. */
    if ((g_fade_current.red == FADE_CHAN_NEUTRAL) &&
        (g_fade_current.green == FADE_CHAN_NEUTRAL) &&
        (g_fade_current.blue == FADE_CHAN_NEUTRAL))
    {
        render_ctx->prim_cursor = fade_packet;
        return;
    }

    if (g_fade_current.red >= FADE_CHAN_ADDITIVE)
    {
        /* Decode additive channels with FADE_CHAN_ADDITIVE as zero intensity. */
        fade_packet->tile.r0 = (u8)g_fade_current.red - FADE_ADDITIVE_BIAS;
        fade_packet->tile.g0 = (u8)g_fade_current.green - FADE_ADDITIVE_BIAS;
        fade_packet->tile.b0 = (u8)g_fade_current.blue - FADE_ADDITIVE_BIAS;
    }
    else
    {
        /* Decode subtractive channels while preserving neutral as zero intensity. */
        if (g_fade_current.red == FADE_CHAN_NEUTRAL)
        {
            fade_packet->tile.r0 = 0;
        }
        else
        {
            fade_packet->tile.r0 = ~g_fade_current.red;
        }

        if (g_fade_current.green == FADE_CHAN_NEUTRAL)
        {
            fade_packet->tile.g0 = 0;
        }
        else
        {
            fade_packet->tile.g0 = ~g_fade_current.green;
        }

        if (g_fade_current.blue == FADE_CHAN_NEUTRAL)
        {
            fade_packet->tile.b0 = 0;
        }
        else
        {
            fade_packet->tile.b0 = ~g_fade_current.blue;
        }
    }

    setTile(&fade_packet->tile);
    setSemiTrans(&fade_packet->tile, 1);
    SET_YX0(&fade_packet->tile, 0, 0);
    setWH(&fade_packet->tile, SCREEN_WIDTH, SCREEN_HEIGHT);
    addPrim(&ordering_table_ctx->ot[GNAME_OT_FRONT], &fade_packet->tile);
    fade_packet = NEXT_FADE_PACKET(fade_packet, TILE);

    blend_mode_tpage = g_fade_current.red < FADE_CHAN_ADDITIVE ? FADE_TPAGE_SUB : FADE_TPAGE_ADD;

    setDrawTPage(&fade_packet->draw_mode, 0, 0, blend_mode_tpage);
    addPrim(&ordering_table_ctx->ot[GNAME_OT_FRONT], &fade_packet->draw_mode);
    fade_packet = NEXT_FADE_PACKET(fade_packet, DR_TPAGE);

    render_ctx->prim_cursor = fade_packet;
}

/**
 * @brief Set the fade target color and interpolation duration.
 * @param red Target red value and blend-mode selector.
 * @param green Target green value.
 * @param blue Target blue value.
 * @param step_count Frames to reach the target, or 0 to snap.
 * @see https://decomp.me/scratch/jq3uD (100%)
 */
static void set_fade_target(s32 red, s32 green, s32 blue, s32 step_count)
{
    g_fade_target.red = red;
    g_fade_target.green = green;
    g_fade_target.blue = blue;
    g_fade_target.steps_remaining = step_count;
}

/**
 * @brief Initialize name-entry resources and session state.
 * @see https://decomp.me/scratch/pnzC1 (100%)
 */
void gname_init(void)
{
    volatile s32 frame_padding[GNAME_INIT_STACK_PAD_WORDS];

    load_name_entry_tim();
    func_800AA02C();
    g_startup_delay = GNAME_STARTUP_DELAY_FRAMES;
    func_8006441C();
    reset_run_state();
    func_80063194();
}

/**
 * @brief Upload the name-entry TIM to its fixed VRAM destinations.
 * @see https://decomp.me/scratch/EWwJI (100%)
 */
static void load_name_entry_tim(void)
{
    TimUploadCoords upload_coords;

    upload_coords.pixel_x = SCREEN_WIDTH;
    upload_coords.pixel_y = 0;
    upload_coords.clut_x = 0;
    upload_coords.clut_y = VRAM_CLUT_Y;
    load_tim_to_vram(&upload_coords);
}

/**
 * @brief Apply CLUT transparency and upload the name-entry TIM to VRAM.
 * @param upload_coords Pixel and CLUT upload destinations.
 * @see https://decomp.me/scratch/P3W9C (100%)
 */
static void load_tim_to_vram(const TimUploadCoords* upload_coords)
{
    RECT upload_rect;
    TimBlock* pixel_block;
    s32 clut_index;

    Tim* name_tim = &g_name_entry_tim;
    s32 clut_block_size = name_tim->clut_block.bnum;
    u16* clut_entry = name_tim->clut_data;

    upload_rect.x = upload_coords->clut_x;
    upload_rect.y = upload_coords->clut_y;
    upload_rect.w = CLUT_ENTRY_COUNT;
    upload_rect.h = 1;

    /* Mark every non-zero CLUT entry semi-transparent. */
    for (clut_index = 0; clut_index < CLUT_ENTRY_COUNT; clut_index++)
    {
        if (*clut_entry != 0)
        {
            *clut_entry |= GPU_STP_BIT;
        }
        clut_entry++;
    }

    LoadImage(&upload_rect, name_tim->clut_data);
    pixel_block = TIM_PIXEL_BLOCK(name_tim, clut_block_size);

    upload_rect.x = upload_coords->pixel_x;
    upload_rect.y = upload_coords->pixel_y;
    upload_rect.w = pixel_block->w;
    upload_rect.h = pixel_block->h;

    LoadImage(&upload_rect, pixel_block + 1);

    /* Leave the rectangle positioned below the uploaded CLUT. */
    upload_rect.x = upload_coords->clut_x;
    upload_rect.y = upload_coords->clut_y + 1;
    upload_rect.w = CLUT_ENTRY_COUNT;
    upload_rect.h = 1;
}

/**
 * @brief Render and update one name-entry frame.
 * @param render_ctx Frame render context.
 * @see https://decomp.me/scratch/yYkTM (100%)
 */
void gname_tick(RenderContext* render_ctx)
{
    render_layout_sprite_batch(render_ctx);
    gname_render(render_ctx);
    g_frame_counter++;
    gname_update_state();
}

/**
 * @brief Advance input gating, strip width, and START-button confirmation.
 * @see https://decomp.me/scratch/g5Rx3 (100%)
 */
static void gname_update_state(void)
{
    s32 remaining_width_steps;

    /* Gate input until the startup delay expires. */
    if (g_startup_delay == 0)
    {
        gname_process_input();
    }
    else
    {
        g_startup_delay--;
    }

    /* Interpolate the backing strip to its target width. */
    remaining_width_steps = g_strip_width_steps;
    if (remaining_width_steps != 0)
    {
        g_strip_width_steps--;
        g_strip_width += (g_strip_width_target - g_strip_width) / remaining_width_steps;
    }
    else
    {
        g_strip_width = g_strip_width_target;
    }

    if (g_pad_input != PAD_BTN_START)
    {
        return;
    }

    if ((name_glyph_count(g_active_name) != 0) && (name_is_blank(g_active_name) == FALSE))
    {
        play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
        g_overlay_result = GNAME_RESULT_CONFIRM;
        return;
    }

    play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
}

/**
 * @brief Reset the name-entry state for a new session.
 * @see https://decomp.me/scratch/FboaU (100%)
 */
static void reset_run_state(void)
{
    g_activated_entry = GNAME_ENTRY_NONE;
    g_navigation_mode = handle_navigation_input(GNAME_MODE_ACTION_OK, GNAME_BUTTONS_NONE);
    g_cursor_lerp_steps = 0;
    g_scroll_pos = 0;
    g_scroll_target = 0;
    g_scroll_steps = 0;
    g_char_cursor = 0;
    g_name_clipboard[NAME_GLYPH_LEAD_INDEX] = 0;
    g_cursor_x = g_cursor_x_target;
    g_cursor_y = g_cursor_y_target;
    name_copy(g_active_name, g_initial_name);
    g_strip_width = 0;
    recalc_name_width();
    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
    g_glyph_append_anim_frame = 0;
    g_glyph_append_anim_timer = GLYPH_APPEND_ANIM_TIMER_START;
    g_char_panel = CHAR_PANEL_STANDARD_FIRST;
}

/**
 * @brief Update the focused name-entry region from navigation input.
 * @param mode Current navigation mode.
 * @param buttons Filtered directional and confirm buttons.
 * @return Updated navigation mode.
 * @see decomp.me (100%) https://decomp.me/scratch/jAuWs
 */
static s32 handle_navigation_input(s32 mode, s32 buttons)
{
    s32 repeat_dispatch = GNAME_REDISPATCH_PENDING;
    /* GCC carries this action-path step value across the later switch arms. */
    s32 navigation_step;

    while (repeat_dispatch == GNAME_REDISPATCH_PENDING)
    {
        switch (mode)
        {
        /* Top-row action buttons: OK, Delete, Random, and Default. */
        case GNAME_MODE_ACTION_OK:
        case GNAME_MODE_ACTION_DELETE:
        case GNAME_MODE_ACTION_RANDOM:
        case GNAME_MODE_ACTION_DEFAULT:
            if (buttons & GNAME_BTN_CONFIRM)
            {
                g_activated_entry = mode;
                navigation_step = GNAME_NAVIGATION_STEP;
                switch (mode)
                {
                case GNAME_MODE_ACTION_OK:
                    if ((name_glyph_count(g_active_name) != 0) && (!name_is_blank(g_active_name)))
                    {
                        play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                        g_overlay_result = GNAME_RESULT_CONFIRM;
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                    repeat_dispatch = GNAME_REDISPATCH_DONE;
                    continue;

                case GNAME_MODE_ACTION_DELETE:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    name_pop_last_glyph(g_active_name);
                    recalc_name_width();
                    /* Preserve the delete-action branch boundary. */
                    do
                    {
                    } while (FALSE);
                    repeat_dispatch = GNAME_REDISPATCH_DONE;
                    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                    continue;

                case GNAME_MODE_ACTION_RANDOM:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    if (g_name_source_mode == GNAME_SRC_RAND_PRIMARY)
                    {
                        g_name_clipboard[0] = 0;
                        name_copy(g_active_name, RANDOM_NAME(rand() % RANDOM_NAME_COUNT));
                    }
                    else if (g_name_source_mode == GNAME_SRC_RAND_ALT)
                    {
                        g_name_clipboard[0] = 0;
                        name_copy(g_active_name, RANDOM_NAME((rand() % RANDOM_NAME_COUNT) + RANDOM_NAME_COUNT));
                    }
                    else if (g_name_source_mode == GNAME_SRC_HISTORY)
                    {
                        g_name_clipboard[0] = 0;
                        if (g_history_name_idx >= HISTORY_NAME_INDEX_LIMIT)
                        {
                            name_copy(g_active_name, g_initial_name);
                        }
                        else
                        {
                            name_copy(g_active_name, HISTORY_NAME(g_history_name_idx));
                            name_append(g_active_name, HISTORY_SUFFIX((rand() % RANDOM_NAME_COUNT) + HISTORY_SUFFIX_INDEX_BASE));
                        }
                    }
                    else if (g_name_source_mode == navigation_step)
                    {
                        g_name_clipboard[0] = 0;
                        name_copy(g_active_name, g_custom_name_buf);
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                        g_name_clipboard[0] = 0;
                        name_copy(g_active_name, g_initial_name);
                    }
                    recalc_name_width();
                    repeat_dispatch = GNAME_REDISPATCH_DONE;
                    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                    continue;

                case GNAME_MODE_ACTION_DEFAULT:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    g_name_clipboard[0] = 0;
                    name_copy(g_active_name, g_initial_name);
                    break;

                default:
                    repeat_dispatch = GNAME_REDISPATCH_DONE;
                    continue;
                }

                recalc_name_width();
                repeat_dispatch = GNAME_REDISPATCH_DONE;
                g_strip_width_steps = NAME_STRIP_LERP_STEPS;
            }
            else
            {
                if (buttons != 0)
                {
                    if (buttons & PAD_BTN_DOWN)
                    {
                        mode = GNAME_MODE_GRID;
                        buttons = 0;
                        continue;
                    }
                    if (buttons & PAD_BTN_LEFT)
                    {
                        mode = (mode == GNAME_MODE_ACTION_OK) ? GNAME_MODE_ACTION_DEFAULT : (mode - navigation_step);
                    }
                    else if (buttons & PAD_BTN_RIGHT)
                    {
                        mode = (mode < GNAME_MODE_ACTION_DEFAULT) ? (mode + GNAME_NAVIGATION_STEP) : GNAME_MODE_ACTION_OK;
                    }
                }
                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].x - GNAME_TAB_CURSOR_X_BIAS;
                g_cursor_y_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].y;
                g_cursor_lerp_steps = GNAME_CURSOR_LERP_STEPS;
                repeat_dispatch = GNAME_REDISPATCH_DONE;
            }
            break;

        /* Left-column character-panel selector tabs. */
        case GNAME_MODE_PANEL_BASE:
        case GNAME_MODE_PANEL_BASE + GNAME_NAVIGATION_STEP:
        case GNAME_MODE_PANEL_NAV_LAST:
        case GNAME_MODE_PANEL_LAST:
            /* Confirm changes panels only when the selected tab is not already active. */
            if (((buttons & GNAME_BTN_CONFIRM) && ((g_activated_entry = mode, g_char_panel != (mode - GNAME_MODE_PANEL_BASE)))) != 0)
            {
                g_char_panel = g_activated_entry - GNAME_MODE_PANEL_BASE;
                mode = GNAME_MODE_GRID;
                buttons = 0;
                g_scroll_target = 0;
                g_scroll_pos = 0;
                g_scroll_steps = 0;
                g_char_cursor = 0;
                play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                continue;
            }

            if (buttons != 0)
            {
                if (buttons & PAD_BTN_RIGHT)
                {
                    mode = GNAME_MODE_GRID;
                    buttons = 0;
                    continue;
                }
                if (buttons & PAD_BTN_UP)
                {
                    mode = (mode == GNAME_MODE_PANEL_BASE) ? GNAME_MODE_PANEL_NAV_LAST : (mode - navigation_step);
                }
                else if (buttons & PAD_BTN_DOWN)
                {
                    mode = (mode < GNAME_MODE_PANEL_NAV_LAST) ? (mode + navigation_step) : GNAME_MODE_PANEL_BASE;
                }
            }
            play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
            g_cursor_x_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].x - GNAME_TAB_CURSOR_X_BIAS;
            g_cursor_y_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].y;
            g_cursor_lerp_steps = GNAME_CURSOR_LERP_STEPS;
            repeat_dispatch = GNAME_REDISPATCH_DONE;
            break;

        default:
            /* Other mode values are handled as the active character grid. */
            if (((buttons & GNAME_BTN_CONFIRM) && (((g_char_last_row * NAME_GRID_COLUMNS) + g_char_last_col) >= g_char_cursor)) != 0U)
            {
                if (g_char_panel < CHAR_PANEL_STANDARD_COUNT)
                {
                    if (name_glyph_count(g_active_name) < NAME_MAX_GLYPHS)
                    {
                        u8* selected_glyph;
                        g_glyph_append_anim_timer = GLYPH_APPEND_ANIM_TIMER_START;
                        selected_glyph = GNAME_RECORD_IN_RANGE(
                            PANEL_CHARACTER_TABLE,
                            g_panel_char_offsets[g_char_panel],
                            g_char_cursor);
                        g_glyph_append_anim_frame = 0;
                        name_append(g_active_name, selected_glyph);
                        recalc_name_width();
                        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                }
                else if (g_char_panel == CHAR_PANEL_KANJI_CATEGORY)
                {
                    if (g_kanji_cat_entries[g_char_cursor] == KANJI_CATEGORY_EMPTY)
                    {
                        repeat_dispatch = GNAME_REDISPATCH_DONE;
                        continue;
                    }
                    g_kanji_cat = g_char_cursor;
                    g_char_panel = CHAR_PANEL_KANJI;
                    g_scroll_target = 0;
                    g_scroll_pos = 0;
                    g_scroll_steps = 0;
                    g_cursor_x_target = NAME_GRID_X_BASE;
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_cursor_lerp_steps = GNAME_GRID_LERP_STEPS;
                    g_char_cursor = 0;
                    g_kanji_cat_name = GNAME_RECORD_IN_RANGE(
                        PANEL_CHARACTER_TABLE,
                        g_kanji_cat_names_offset,
                        g_kanji_cat);
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                }
                else if (g_char_panel == CHAR_PANEL_KANJI)
                {
                    if (name_glyph_count(g_active_name) < NAME_MAX_GLYPHS)
                    {
                        u8* selected_glyph;
                        g_glyph_append_anim_timer = GLYPH_APPEND_ANIM_TIMER_START;
                        selected_glyph = GNAME_RECORD_IN_RANGE(
                            KANJI_CHARACTER_TABLE,
                            g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]],
                            g_char_cursor);
                        g_glyph_append_anim_frame = 0;
                        name_append(g_active_name, selected_glyph);
                        recalc_name_width();
                        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                }
                repeat_dispatch = GNAME_REDISPATCH_DONE;
            }
            else
            {
                if (buttons != 0)
                {
                    if ((buttons & PAD_BTN_UP) && ((g_char_cursor / NAME_GRID_COLUMNS) == 0))
                    {
                        mode = GNAME_MODE_ACTION_OK;
                        buttons = 0;
                        continue;
                    }
                    if ((buttons & PAD_BTN_LEFT) && ((g_char_cursor % NAME_GRID_COLUMNS) == 0))
                    {
                        mode = GNAME_MODE_PANEL_BASE;
                        buttons = 0;
                        continue;
                    }
                    if ((buttons & PAD_BTN_UP) && ((g_char_cursor / NAME_GRID_COLUMNS) != 0))
                    {
                        g_char_cursor -= NAME_GRID_COLUMNS;
                    }
                    else if ((buttons & PAD_BTN_DOWN) && ((g_char_cursor / NAME_GRID_COLUMNS) != g_char_last_row))
                    {
                        g_char_cursor += NAME_GRID_COLUMNS;
                    }
                    else if ((buttons & PAD_BTN_LEFT) && ((g_char_cursor % NAME_GRID_COLUMNS) != 0))
                    {
                        g_char_cursor -= GNAME_NAVIGATION_STEP;
                    }
                    else if ((buttons & PAD_BTN_RIGHT) && ((g_char_cursor % NAME_GRID_COLUMNS) != NAME_GRID_LAST_COL))
                    {
                        g_char_cursor += navigation_step;
                    }
                    else
                    {
                        repeat_dispatch = GNAME_REDISPATCH_DONE;
                        continue;
                    }
                }

                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = ((g_char_cursor % NAME_GRID_COLUMNS) * NAME_GRID_CELL_SIZE) + NAME_GRID_X_BASE;
                g_cursor_y_target = ((g_char_cursor / NAME_GRID_COLUMNS) * NAME_GRID_CELL_SIZE) + NAME_GRID_Y_TOP - g_scroll_pos;

                if (g_cursor_y_target < NAME_GRID_Y_TOP)
                {
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_scroll_target = (g_char_cursor / NAME_GRID_COLUMNS) * NAME_GRID_CELL_SIZE;
                    g_scroll_steps = GNAME_GRID_LERP_STEPS;
                }

                if (g_cursor_y_target >= NAME_GRID_Y_EXIT_BOUND)
                {
                    g_cursor_y_target = NAME_GRID_Y_BOTTOM;
                    g_scroll_target = ((g_char_cursor / NAME_GRID_COLUMNS) * NAME_GRID_CELL_SIZE) - NAME_GRID_SCROLL_STEP;
                    g_scroll_steps = GNAME_GRID_LERP_STEPS;
                }

                g_cursor_lerp_steps = GNAME_GRID_LERP_STEPS;
                repeat_dispatch = GNAME_REDISPATCH_DONE;
            }
            break;
        }
    }

    return mode;
}

/**
 * @brief Process name editing, kanji navigation, and cursor interpolation.
 * @see decomp.me (100%) https://decomp.me/scratch/pCzH6
 */
static void gname_process_input(void)
{
    s32 next_category;
    s32 category_before_step;
    u8 moved_glyph_bytes[NAME_GLYPH_BUFFER_SIZE];
    s32 move_sfx_id;
    s32 previous_category;
    u8* clipboard;
    s32 navigation_input;
    s32 previous_page_category;
    s32 moved_glyph;
    s32 scroll_delta;
    s32* scroll_position;
    s32 category_record_base_index;
    u16 glyph_value;
    u8* panel_data_base;
    u32 category_index;
    u32 category_entry;
    u32 category_name_table_offset;
    u16* category_name_offset_entry;
    u16 category_name_offset;
    s32 category_record_base_index_copy;
    s32 volume_or_nav_mask;
    void** category_name_ptr;
    s32 remaining_scroll_steps;

    g_activated_entry = GNAME_ENTRY_NONE;
    navigation_input = g_pad_input & GNAME_BTN_NAV_MASK;

    if (navigation_input != 0)
    {
        g_navigation_mode = handle_navigation_input(g_navigation_mode, navigation_input);
    }
    /* Undo: move the last name glyph to the front of the clipboard. */
    else if (g_pad_input & GNAME_BTN_UNDO)
    {
        moved_glyph = name_pop_last_glyph(g_active_name);
        while (name_glyph_count(g_name_clipboard) >= NAME_CLIPBOARD_MAX_GLYPHS)
        {
            name_pop_last_glyph(g_name_clipboard);
        }

        name_prepend_glyph(g_name_clipboard, moved_glyph);
        recalc_name_width();
        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
        move_sfx_id = GNAME_SFX_MOVE;
        volume_or_nav_mask = GNAME_SFX_VOLUME;
        play_menu_sfx(move_sfx_id, volume_or_nav_mask);
    }
    /* Redo: move the first clipboard glyph back into the active name. */
    else if (g_pad_input & GNAME_BTN_REDO)
    {
        if (name_glyph_count(g_active_name) < NAME_MAX_GLYPHS)
        {
            clipboard = g_name_clipboard;
            moved_glyph = name_pop_first_glyph(clipboard);
            glyph_value = moved_glyph;
            if (glyph_value != 0)
            {
                moved_glyph_bytes[NAME_GLYPH_LEAD_INDEX] = moved_glyph;
                moved_glyph_bytes[NAME_GLYPH_TRAIL_INDEX] = HIGH_BYTE(glyph_value);
                moved_glyph_bytes[NAME_GLYPH_TERMINATOR_INDEX] = 0;
                name_append(g_active_name, moved_glyph_bytes);
                recalc_name_width();
                g_strip_width_steps = NAME_STRIP_LERP_STEPS;
            }
            move_sfx_id = GNAME_SFX_MOVE;
            play_menu_sfx(move_sfx_id, GNAME_SFX_VOLUME);
        }
        else
        {
            play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
        }
    }
    else if (g_pad_input & GNAME_BTN_CANCEL)
    {
        if ((g_allow_empty_cancel != 0) && (name_glyph_count(g_active_name) == 0))
        {
            g_overlay_result = GNAME_RESULT_CANCEL;
            play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
            return;
        }
        play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
        name_pop_last_glyph(g_active_name);
        recalc_name_width();
        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
    }
    /* Dormant in the shipped data: cycle kanji categories, skipping empty entries. */
    if (((g_navigation_mode == GNAME_MODE_GRID) && (g_char_panel == CHAR_PANEL_KANJI)) && (g_pad_input & GNAME_BTN_KANJI_NAV))
    {
        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
        if (g_pad_input & GNAME_BTN_KANJI_NAV)
        {
            while (g_pad_input & GNAME_BTN_KANJI_NAV)
            {
                if (g_pad_input & GNAME_BTN_KANJI_PREV)
                {
                    previous_category = g_kanji_cat;
                    category_before_step = previous_category;
                    previous_page_category = category_before_step - KANJI_CATEGORY_STEP;
                    g_kanji_cat = previous_page_category;
                    if (previous_page_category == KANJI_CATEGORY_PREV_EDGE)
                    {
                        g_kanji_cat = KANJI_CATEGORY_FIRST;
                    }
                    else if (previous_page_category < 0)
                    {
                        g_kanji_cat = previous_category + KANJI_CATEGORY_WRAP_OFFSET;
                    }
                }
                else
                {
                    category_before_step = g_kanji_cat;
                    next_category = category_before_step + KANJI_CATEGORY_STEP;
                    g_kanji_cat = next_category;
                    if (next_category == KANJI_CATEGORY_COUNT)
                    {
                        g_kanji_cat = KANJI_CATEGORY_NEXT_EDGE;
                    }
                    else if (next_category >= KANJI_CATEGORY_COUNT)
                    {
                        g_kanji_cat = category_before_step - KANJI_CATEGORY_WRAP_OFFSET;
                    }
                }

                category_entry = g_kanji_cat;
                if (g_kanji_cat_entries[category_entry] == KANJI_CATEGORY_EMPTY)
                {
                    continue;
                }

                category_name_ptr = &g_kanji_cat_name;
                g_scroll_target = 0;
                g_scroll_pos = 0;
                category_record_base_index = g_panel_char_offsets[CHAR_PANEL_KANJI_CATEGORY];
                volume_or_nav_mask = ~GNAME_BTN_KANJI_NAV;
                g_scroll_steps = 0;
                g_char_cursor = 0;
                g_cursor_x_target = NAME_GRID_X_BASE;
                g_cursor_y_target = NAME_GRID_Y_TOP;
                g_cursor_lerp_steps = GNAME_GRID_LERP_STEPS;
                category_record_base_index_copy = category_record_base_index;
                category_index = g_kanji_cat;
                category_name_table_offset = (category_index * sizeof(u16)) +
                                             ((category_record_base_index_copy * sizeof(u16)) + g_panel_tbl_off);
                panel_data_base = g_panel_data_base;
                category_name_offset_entry = (u16*)(panel_data_base + category_name_table_offset);
                category_name_offset = *category_name_offset_entry;
                g_pad_input &= volume_or_nav_mask;
                *category_name_ptr = (void*)(g_panel_tbl_off + (category_name_offset + ((unsigned long)panel_data_base)));
            }
        }
    }
    /* Advance the cursor and panel-scroll interpolations. */
    if (g_cursor_lerp_steps != 0)
    {
        g_cursor_x += (g_cursor_x_target - g_cursor_x) / g_cursor_lerp_steps;
        g_cursor_y += (g_cursor_y_target - g_cursor_y) / g_cursor_lerp_steps;
        g_cursor_lerp_steps -= 1;
    }
    else
    {
        g_cursor_x = g_cursor_x_target;
        g_cursor_y = g_cursor_y_target;
    }
    remaining_scroll_steps = g_scroll_steps;
    if (remaining_scroll_steps != 0)
    {
        scroll_position = &g_scroll_pos;
        scroll_delta = (g_scroll_target - *scroll_position) / remaining_scroll_steps;
        g_scroll_steps -= 1;
        g_scroll_pos += scroll_delta;
        return;
    }
    g_scroll_pos = g_scroll_target;
}

/**
 * @brief Emit the editable-name cursor sprite and draw-mode packet.
 * @param packet_cursor Next free primitive-buffer address.
 * @param ot_entry Ordering-table entry that receives both packets.
 * @param x Cursor X coordinate.
 * @param y Cursor Y coordinate.
 * @return Next free primitive-buffer address.
 * @note No current call sites; @ref gname_render emits the same packet pair inline.
 * @see decomp.me (100%) https://decomp.me/scratch/oXGkF
 */
static u_long* emit_cursor_glyph(u_long* packet_cursor, u_long* ot_entry, s16 x, s16 y)
{
    u32 clut_id;
    SPRT* cursor_sprite = (SPRT*)packet_cursor;

    SET_BGR0_PACKED(cursor_sprite, GPU_TINT_NEUTRAL);
    setSprt(cursor_sprite);
    setXY0(cursor_sprite, x, y);

    setUV0(cursor_sprite, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].u, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].v);
    setWH(cursor_sprite, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].w, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].h);

    clut_id = g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].clut & GLYPH_CLUT_X_MASK;
    cursor_sprite->clut = clut_id | GLYPH_CLUT_PAGE_BITS;
    addPrim(ot_entry, cursor_sprite);

    packet_cursor += PRIM_WORDS(SPRT);
    setDrawTPage(packet_cursor, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(ot_entry, packet_cursor);

    return packet_cursor + PRIM_WORDS(DR_TPAGE);
}

/**
 * @brief Render the interactive name-entry elements for one frame.
 * @param render_ctx Render context whose ordering table and packet cursor are updated.
 * @see decomp.me (100%) https://decomp.me/scratch/a0Oye
 */
static void gname_render(RenderContext* render_ctx)
{
    s32 selection_index;
    s32 scroll_offset;
    const TabCursorEntry* selection_entry;
    void* packet_cursor;
    SPRT* cursor_sprite;
    DR_TPAGE* cursor_draw_mode;
    RenderContext* ordering_ctx;
    s32 cursor_x;
    s32 cursor_y;

    ordering_ctx = render_ctx;
    packet_cursor = render_ctx->prim_cursor;
    selection_entry = g_tab_cursor_entries;

    /* Emit the action and panel-selection glyphs. */
    for (selection_index = GNAME_SELECTION_ENTRY_FIRST; selection_index < GNAME_SELECTION_ENTRY_END_EXCLUSIVE;
         selection_index++, selection_entry++)
    {
        if (selection_index != GNAME_SELECTION_ENTRY_HIDDEN)
        {
            packet_cursor = emit_glyph_sprt(packet_cursor, &ordering_ctx->ot[GNAME_OT_CHAR_GRID], selection_entry->glyph,
                                            selection_entry->x, selection_entry->y - GNAME_SELECTION_ENTRY_Y_BIAS,
                                            GNAME_SELECTION_SHADOW_OFFSET,
                                            (selection_index - GNAME_SELECTION_ENTRY_FIRST) == g_activated_entry, FALSE);
        }
    }

    /* Render the append indicator, its animation, and the current panel tab. */
    packet_cursor = emit_draw_mode_prim(packet_cursor, &ordering_ctx->ot[GNAME_OT_CHAR_GRID]);
    packet_cursor = emit_glyph_sprt(packet_cursor, &ordering_ctx->ot[GNAME_OT_GLYPH_APPEND], GNAME_APPEND_GLYPH, GNAME_APPEND_X,
                                    GNAME_APPEND_Y, 0, 0, FALSE);
    packet_cursor = render_glyph_append_anim(packet_cursor, ordering_ctx);
    packet_cursor = emit_draw_mode_prim(packet_cursor, &ordering_ctx->ot[GNAME_OT_GLYPH_APPEND]);
    packet_cursor = emit_panel_tab_sprite(packet_cursor, &ordering_ctx->ot[GNAME_OT_FRONT]);

    /* Emit the editable-name cursor and its texture-page packet. */
    cursor_x = g_cursor_x;
    cursor_y = g_cursor_y;
    cursor_sprite = (SPRT*)packet_cursor;
    SET_BGR0_PACKED(cursor_sprite, GPU_TINT_NEUTRAL);
    setSprt(cursor_sprite);
    setXY0(cursor_sprite, cursor_x, cursor_y);
    setUV0(cursor_sprite, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].u, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].v);
    setWH(cursor_sprite, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].w, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].h);
    setClut(cursor_sprite, (g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].clut & GLYPH_CLUT_X_MASK) << GLYPH_CLUT_X_SHIFT, VRAM_CLUT_Y);
    addPrim(&ordering_ctx->ot[GNAME_OT_TEXT_CURSOR], cursor_sprite);
    cursor_draw_mode = (DR_TPAGE*)(cursor_sprite + 1);
    setDrawTPage(cursor_draw_mode, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(&ordering_ctx->ot[GNAME_OT_TEXT_CURSOR], cursor_draw_mode);
    packet_cursor = cursor_draw_mode + 1;

    /* Show scroll indicators for content outside the visible grid window. */
    if (g_scroll_pos != 0)
    {
        packet_cursor = emit_glyph_sprt(packet_cursor, &ordering_ctx->ot[GNAME_OT_FRONT], g_tab_cursor_pos[GNAME_SCROLL_UP_ENTRY].glyph,
                                        g_tab_cursor_pos[GNAME_SCROLL_UP_ENTRY].x, g_tab_cursor_pos[GNAME_SCROLL_UP_ENTRY].y, 0, 0, FALSE);
    }

    if (g_char_last_row >= NAME_GRID_VISIBLE_ROWS)
    {
        /* Divide the pixel scroll by one row, rounding negative values toward zero. */
        scroll_offset = g_scroll_pos;
        if (scroll_offset < 0)
        {
            scroll_offset += NAME_GRID_DIV_BIAS;
        }

        if ((scroll_offset >> NAME_GRID_CELL_SHIFT) != (g_char_last_row - (NAME_GRID_VISIBLE_ROWS - 1)))
        {
            packet_cursor = emit_glyph_sprt(packet_cursor, &ordering_ctx->ot[GNAME_OT_FRONT],
                                            g_tab_cursor_pos[GNAME_SCROLL_DOWN_ENTRY].glyph,
                                            g_tab_cursor_pos[GNAME_SCROLL_DOWN_ENTRY].x, g_tab_cursor_pos[GNAME_SCROLL_DOWN_ENTRY].y,
                                            0, 0, FALSE);
        }
    }

    /* Finish with the panel label, character grid, and active-name strip. */
    packet_cursor = emit_draw_mode_prim(packet_cursor, &ordering_ctx->ot[GNAME_OT_FRONT]);
    render_ctx->prim_cursor = emit_panel_label(packet_cursor, &ordering_ctx->ot[GNAME_OT_PANEL_LABEL]);
    render_char_panel(render_ctx, g_char_panel);
    render_name_strip(render_ctx, g_active_name, g_strip_width);
}

/**
 * @brief Emit the tab sprite for the current navigation mode.
 * @param packet_cursor Next free primitive-buffer address.
 * @param ot_entry Ordering-table entry that receives the sprite.
 * @return Next free primitive-buffer address.
 * @see decomp.me (100%) https://decomp.me/scratch/RnoNS
 */
static void* emit_panel_tab_sprite(void* packet_cursor, u_long* ot_entry)
{
    s32 navigation_mode = g_navigation_mode;

    if (navigation_mode <= GNAME_MODE_PANEL_LAST)
    {
        packet_cursor = func_800A88A0(packet_cursor, ot_entry,
                                     GNAME_RECORD(PANEL_RECORD_TABLE,
                                                  g_tab_cursor_pos[navigation_mode + GNAME_CURSOR_POS_TABLE_OFFSET].sprite_idx),
                                     GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
    }
    else if (navigation_mode == GNAME_MODE_GRID)
    {
        s32 panel_index = g_char_panel;

        /* Select specialized tabs only for the category and kanji panels. */
        if ((u32)(panel_index - CHAR_PANEL_KANJI_CATEGORY) < (CHAR_PANEL_KANJI - CHAR_PANEL_KANJI_CATEGORY + 1))
        {
            packet_cursor = func_800A88A0(packet_cursor, ot_entry,
                                         GNAME_RECORD(PANEL_RECORD_TABLE, panel_index + GNAME_PANEL_TAB_KANJI_RECORD_OFFSET),
                                         GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
        }
        else
        {
            packet_cursor = func_800A88A0(packet_cursor, ot_entry,
                                         GNAME_RECORD(PANEL_RECORD_TABLE, GNAME_PANEL_TAB_DEFAULT_RECORD), GNAME_PANEL_SPRITE_COLOR,
                                         GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
        }
    }
    return packet_cursor;
}

/**
 * @brief Emit the current character-panel label.
 * @param packet_cursor Next free primitive-buffer address.
 * @param ot_entry Ordering-table entry that receives the label.
 * @return Next free primitive-buffer address.
 * @see decomp.me (100%) https://decomp.me/scratch/jK7bc
 */
static void* emit_panel_label(void* packet_cursor, u_long* ot_entry)
{
    s32 panel_index = g_char_panel;

    if (panel_index < CHAR_PANEL_KANJI)
    {
        packet_cursor = func_800A88A0(packet_cursor, ot_entry, GNAME_RECORD(PANEL_RECORD_TABLE, panel_index), GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_LABEL_X,
                                     GNAME_PANEL_LABEL_Y, GNAME_PANEL_SPRITE_MODE);
    }
    else
    {
        packet_cursor = func_800A88A0(packet_cursor, ot_entry, g_kanji_cat_name, GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_LABEL_X,
                                     GNAME_PANEL_LABEL_Y, GNAME_PANEL_SPRITE_MODE);
    }

    return packet_cursor;
}

/**
 * @brief Render the active name into its backing strip.
 * @param render_ctx Render context whose ordering table and packet cursor are updated.
 * @param name Name buffer to render.
 * @param strip_width Backing-strip width in pixels.
 * @see https://decomp.me/scratch/LxujJ (100%)
 */
static void render_name_strip(RenderContext* render_ctx, u8* name, s32 strip_width)
{
    u_long* ot_entry;
    DR_ENV* restore_env_packet;
    DR_ENV* packet_cursor;
    s32 backing_y;
    s32 backing_x;
    DRAWENV* strip_draw_env;
    DrawEnvScratch strip_draw_scratch;

    ot_entry = &render_ctx->ot[GNAME_OT_NAME_STRIP];
    restore_env_packet = render_ctx->prim_cursor;
    /* The initial alias is required to preserve the target's register allocation. */
    packet_cursor = restore_env_packet;

    /* Restore the inactive frame's drawing environment after this OT pass. */
    SetDrawEnv(restore_env_packet, &g_render_buf_base[render_ctx->frame_parity ^ 1].draw_env);

    addPrim(ot_entry, restore_env_packet);

    /* Emit the name text, decorative glyph, and glyph draw mode. */
    packet_cursor = func_800A88A0(restore_env_packet + 1, ot_entry, name, NAME_STRIP_TEXT_COLOR, NAME_STRIP_TEXT_X, NAME_STRIP_TEXT_Y,
                                  NAME_STRIP_TEXT_MODE);
    packet_cursor = emit_glyph_sprt(packet_cursor, ot_entry, NAME_STRIP_DECOR_GLYPH, 0, 0, 0, 0, 0);
    packet_cursor = emit_draw_mode_prim(packet_cursor, ot_entry);

    /* Redirect rendering to the strip region on the current backing page. */
    strip_draw_env = &strip_draw_scratch.draw_env;
    backing_x = NAME_STRIP_BACKING_RIGHT - strip_width;
    backing_y = NAME_STRIP_BACKING_PAGE0_Y;
    if (render_ctx->frame_parity != 0)
    {
        backing_y = NAME_STRIP_BACKING_PAGE1_Y;
    }

    SetDefDrawEnv(strip_draw_env, backing_x, backing_y, strip_width, NAME_STRIP_BACKING_HEIGHT);
    SetDrawEnv(packet_cursor, strip_draw_env);

    addPrim(ot_entry, packet_cursor);
    packet_cursor++;
    render_ctx->prim_cursor = packet_cursor;
}

/**
 * @brief Render the visible character-panel glyphs.
 * @param render_ctx Render context whose ordering table and packet cursor are updated.
 * @param panel_index Active standard-panel index; ignored for the kanji panel.
 * @see decomp.me (100%) https://decomp.me/scratch/ckF2S
 */
static void render_char_panel(RenderContext* render_ctx, s32 panel_index)
{
    u_long* ot_entry;
    u8 stack_padding[8]; /* Preserve the packet scratch frame size. */
    GridDrawEnvScratch grid_draw_scratch;
    DR_ENV* packet_cursor;
    void* glyph_packet_cursor;
    const GnameRecordTable* glyph_table;
    s32 grid_column;
    s32 glyph_index;
    s32 glyph_end_copy;
    s32 grid_row;
    s32 glyph_y;
    s32 glyph_end;
    u32 category_entry;
    DRAWENV* grid_draw_env;
    s32 backing_y;

    ot_entry = &render_ctx->ot[GNAME_OT_CHAR_PANEL];
    packet_cursor = render_ctx->prim_cursor;
    SetDrawEnv(packet_cursor, &g_render_buf_base[render_ctx->frame_parity ^ 1].draw_env);
    addPrim(ot_entry, packet_cursor);
    glyph_packet_cursor = packet_cursor + 1;

    /* Select the glyph range from either the kanji or standard panel tables. */
    if (g_char_panel == CHAR_PANEL_KANJI)
    {
        glyph_table = KANJI_RECORD_TABLE;
        category_entry = g_kanji_cat_entries[g_kanji_cat];
        glyph_index = g_kanji_entry_offsets[category_entry];
        glyph_end = g_kanji_entry_offsets[category_entry + 1];
        grid_row = 0;
    }
    else
    {
        glyph_index = g_panel_char_offsets[panel_index];
        glyph_end = g_panel_char_offsets[panel_index + 1];
        glyph_table = PANEL_RECORD_TABLE;
        /* Preserve the standard-panel branch boundary. */
        do
        {
        } while (0);
        grid_row = 0;
    }

    /* Walk the range row-major, emitting only glyphs inside the visible window. */
    grid_column = grid_row;
    while (TRUE)
    {
        glyph_y = (grid_row * NAME_GRID_CELL_SIZE) - g_scroll_pos;
        glyph_end_copy = glyph_end;
        if (NAME_GRID_ROW_VISIBLE(glyph_y))
        {
            glyph_packet_cursor = func_800A88A0(glyph_packet_cursor, ot_entry, GNAME_RECORD(glyph_table, glyph_index), CHAR_PANEL_GLYPH_COLOR,
                                                grid_column * NAME_GRID_CELL_SIZE, glyph_y, CHAR_PANEL_GLYPH_MODE);
        }
        glyph_index++;
        if (glyph_end_copy == glyph_index)
        {
            break;
        }
        grid_column++;
        if (grid_column == NAME_GRID_COLUMNS)
        {
            grid_column = 0;
            grid_row++;
        }
    }

    /* Redirect this OT pass into the grid region on this frame's backing page. */
    grid_draw_env = &grid_draw_scratch.draw_env;
    g_char_last_row = grid_row;
    g_char_last_col = grid_column;
    packet_cursor = glyph_packet_cursor;
    backing_y = NAME_GRID_BACKING_PAGE0_Y;
    if (render_ctx->frame_parity != 0)
    {
        backing_y = NAME_GRID_BACKING_PAGE1_Y;
    }
    SetDefDrawEnv(grid_draw_env, NAME_GRID_BACKING_X, backing_y, NAME_GRID_BACKING_W, NAME_GRID_VIS_HEIGHT);
    SetDrawEnv(packet_cursor, grid_draw_env);
    addPrim(ot_entry, packet_cursor);
    packet_cursor++;
    render_ctx->prim_cursor = packet_cursor;
}

/**
 * @brief Emit the glyph texture-page draw-mode packet.
 * @param packet Destination draw-mode packet.
 * @param ot_entry Ordering-table entry that receives the packet.
 * @return Next free primitive-buffer address.
 * @see https://decomp.me/scratch/EyVeo (100%)
 */
static void* emit_draw_mode_prim(DR_TPAGE* packet, u_long* ot_entry)
{
    setDrawTPage(packet, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(ot_entry, packet);

    return packet + 1;
}

/**
 * @brief Emit a glyph sprite with an optional secondary sprite.
 * @param packet_start Next free primitive-buffer address.
 * @param ot_entry Ordering-table entry that receives the sprites.
 * @param glyph_id Index into @c g_glyph_table.
 * @param base_x Base screen X coordinate.
 * @param base_y Base screen Y coordinate.
 * @param shadow_offset Position offset; zero disables the secondary sprite.
 * @param activation_adjust Adjustment applied to the primary and secondary positions.
 * @param use_blue_overlay TRUE for a blue overlay; FALSE for a translucent black shadow.
 * @return Next free primitive-buffer address.
 * @see decomp.me (100%) https://decomp.me/scratch/Au2h5
 */
static void* emit_glyph_sprt(
    void* packet_start,
    u_long* ot_entry,
    s32 glyph_id,
    s32 base_x,
    s32 base_y,
    s32 shadow_offset,
    s32 activation_adjust,
    s32 use_blue_overlay)
{
    u8* packet_cursor = packet_start;
    SPRT* primary_sprite = packet_start;
    const GlyphInfo* primary_glyph_info = GLYPH_TABLE_ENTRY(g_glyph_table, glyph_id);
    s32 secondary_position_offset;

    SET_BGR0_PACKED(primary_sprite, GPU_TINT_NEUTRAL);
    setSprt(primary_sprite);
    setXY0(primary_sprite, base_x - shadow_offset + activation_adjust, base_y - shadow_offset + activation_adjust);
    setUV0(primary_sprite, primary_glyph_info->u, primary_glyph_info->v);
    setWH(primary_sprite, primary_glyph_info->w, primary_glyph_info->h);
    setClut(primary_sprite, primary_glyph_info->clut << GLYPH_CLUT_X_SHIFT, VRAM_CLUT_Y);
    addPrim(ot_entry, primary_sprite);
    packet_cursor += sizeof(SPRT);

    if (shadow_offset != 0)
    {
        const GlyphInfo* glyph_table;
        const GlyphInfo* secondary_glyph_info;

        SET_BGR0_PACKED((SPRT*)packet_cursor,
                        (use_blue_overlay != FALSE) ? GLYPH_SECONDARY_BLUE_TINT : GLYPH_SECONDARY_BLACK_TINT);

        setSprt((SPRT*)packet_cursor);

        if (use_blue_overlay == FALSE)
        {
            setSemiTrans((SPRT*)packet_cursor, TRUE);
        }

        secondary_position_offset = (shadow_offset - activation_adjust) * GLYPH_SECONDARY_OFFSET_SCALE;

        glyph_table = g_glyph_table;
        secondary_glyph_info = GLYPH_TABLE_ENTRY(glyph_table, glyph_id);

        setXY0((SPRT*)packet_cursor, base_x + secondary_position_offset, base_y + secondary_position_offset);
        setUV0((SPRT*)packet_cursor, secondary_glyph_info->u, secondary_glyph_info->v);
        setWH((SPRT*)packet_cursor, secondary_glyph_info->w, secondary_glyph_info->h);
        setClut((SPRT*)packet_cursor, secondary_glyph_info->clut << GLYPH_CLUT_X_SHIFT, VRAM_CLUT_Y);
        addPrim(ot_entry, packet_cursor);

        packet_cursor += sizeof(SPRT);
    }

    return packet_cursor;
}

/**
 * @brief Emit the fixed background layout sprites.
 * @param render_ctx Render context whose ordering table and packet cursor are updated.
 * @see decomp.me (100%) https://decomp.me/scratch/Q6WL2
 */
static void render_layout_sprite_batch(RenderContext* render_ctx)
{
    RECT texture_window_rect;

    s32 sprite_count;

    u8* packet_cursor;
    u8* sprite_cursor;
    DR_TWIN* texture_window_packet;
    SPRT* sprite;
    DR_TPAGE* draw_mode_packet;
    const GlyphSeqEntry* sequence_entry;

    RenderContext* opening_ctx = render_ctx;
    RenderContext* batch_ctx;
    const GlyphInfo* glyph_table;
    batch_ctx = opening_ctx;

    packet_cursor = opening_ctx->prim_cursor;

    /* Open with a full-size texture window. */
    texture_window_rect.h = GNAME_FULL_TEX_WINDOW_SIZE;
    texture_window_rect.w = GNAME_FULL_TEX_WINDOW_SIZE;
    texture_window_rect.y = 0;
    texture_window_rect.x = 0;

    texture_window_packet = (DR_TWIN*)packet_cursor;
    setTexWindow(texture_window_packet, &texture_window_rect);
    addPrim(&opening_ctx->ot[GNAME_OT_LAYOUT_BACKGROUND], texture_window_packet);

    sequence_entry = g_layout_sprite_sequence;
    sprite_count = 0;
    glyph_table = g_glyph_table;

    packet_cursor += sizeof(DR_TWIN);

    sprite_cursor = packet_cursor;
    while (sprite_count < GNAME_LAYOUT_SPRITE_COUNT)
    {
        u32 glyph_id = sequence_entry->id;
        u32 packed_xy;
        const GlyphInfo* glyph_info;
        u8 glyph_height;
        u32 clut_word;

        sprite = (SPRT*)sprite_cursor;
        /* setSprt replaces the code byte written with the packed tint. */
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);

        packed_xy = sequence_entry->xy;
        glyph_info = GLYPH_TABLE_ENTRY(glyph_table, glyph_id);
        SET_SPRT_XY0_WORD(sprite, packed_xy);

        sprite->u0 = glyph_info->u;
        sprite->v0 = glyph_info->v;
        sprite->w = glyph_info->w;
        glyph_height = glyph_info->h;
        sprite_count++;
        sprite->h = glyph_height;
        clut_word = glyph_info->clut;
        sequence_entry++;
        sprite->clut = (clut_word & GLYPH_CLUT_X_MASK) | GLYPH_CLUT_PAGE_BITS;

        addPrim(&batch_ctx->ot[GNAME_OT_LAYOUT_BACKGROUND], sprite);
        sprite_cursor += sizeof(SPRT);
    }
    packet_cursor = sprite_cursor;

    /* Restore the full-size texture window after the sprite batch. */
    texture_window_rect.w = GNAME_FULL_TEX_WINDOW_SIZE;
    texture_window_rect.h = GNAME_FULL_TEX_WINDOW_SIZE;
    texture_window_rect.x = 0;
    texture_window_rect.y = 0;
    texture_window_packet = (DR_TWIN*)packet_cursor;
    setTexWindow(texture_window_packet, &texture_window_rect);
    addPrim(&batch_ctx->ot[GNAME_OT_LAYOUT_BACKGROUND], texture_window_packet);
    packet_cursor += sizeof(DR_TWIN);

    draw_mode_packet = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_mode_packet, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(&batch_ctx->ot[GNAME_OT_LAYOUT_BACKGROUND], draw_mode_packet);

    render_ctx->prim_cursor = draw_mode_packet + 1;
}

/**
 * @brief Count the encoded bytes in a name buffer.
 * @param name_buf Null-terminated name buffer.
 * @return Number of bytes excluding the terminator.
 * @see https://decomp.me/scratch/2QgjW (100%)
 */
static s32 name_byte_length(const u8* name_buf)
{
    const u8* scan_cursor;
    s32 byte_count;

    scan_cursor = name_buf;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    return byte_count;
}

/**
 * @brief Count the glyphs in a name buffer.
 * @param name_buf Null-terminated name buffer.
 * @return Number of encoded glyphs.
 * @see https://decomp.me/scratch/c8fPe (100%)
 */
static s32 name_glyph_count(const u8* name_buf)
{
    s32 glyph_count = 0;

    while (*name_buf)
    {
        name_buf += IS_DBCS_LEAD_BYTE(*name_buf)
            ? NAME_GLYPH_SIZE_DOUBLE
            : NAME_GLYPH_SIZE_SINGLE;
        glyph_count++;
    }

    return glyph_count;
}

/**
 * @brief Append one name buffer to another.
 * @param destination Null-terminated buffer with sufficient capacity.
 * @param source Null-terminated buffer to append.
 * @see https://decomp.me/scratch/1lsbD (100%)
 */
static void name_append(u8* destination, const u8* source)
{
    const u8* scan_cursor;
    s32 destination_byte_count;
    s32 source_byte_count;
    s32 append_offset;
    s32 byte_index;

    scan_cursor = destination;
    destination_byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            destination_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            destination_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    scan_cursor = source;
    source_byte_count = 0;
    append_offset = destination_byte_count;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            source_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            source_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    for (byte_index = 0; byte_index < source_byte_count; byte_index++)
    {
        destination[append_offset + byte_index] = source[byte_index];
    }

    destination[append_offset + byte_index] = 0;
}

/**
 * @brief Remove and return the last glyph in a name buffer.
 * @param name_buf Null-terminated buffer to truncate.
 * @return Packed glyph; the low byte is zero when the buffer is empty.
 * @see https://decomp.me/scratch/agZ8y (100%)
 */
static s32 name_pop_last_glyph(u8* name_buf)
{
    u8* last_glyph_cursor;
    u8* scan_cursor;
    s32 packed_glyph;

    last_glyph_cursor = name_buf;
    scan_cursor = last_glyph_cursor;

    while (*scan_cursor)
    {
        last_glyph_cursor = scan_cursor;
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    packed_glyph = MAKE_DBCS_GLYPH(last_glyph_cursor[0], last_glyph_cursor[1]);

    if (last_glyph_cursor != scan_cursor)
    {
        *last_glyph_cursor = 0;
    }

    return packed_glyph;
}

/**
 * @brief Copy a null-terminated name buffer.
 * @param destination Destination buffer with sufficient capacity.
 * @param source Null-terminated source buffer.
 * @see https://decomp.me/scratch/UeYRe (100%)
 */
static void name_copy(u8* destination, const u8* source)
{
    const u8* scan_cursor;
    s32 byte_index;
    s32 byte_count;

    scan_cursor = source;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    for (byte_index = 0; byte_index < byte_count; byte_index++)
    {
        destination[byte_index] = source[byte_index];
    }

    destination[byte_index] = 0;
}

/**
 * @brief Recalculate the active name and strip widths.
 * @see https://decomp.me/scratch/y0CgJ (100%)
 */
static void recalc_name_width(void)
{
    GlyphMeasure glyphs[NAME_MEASURE_CAPACITY];
    s16 glyph_width;
    s32 glyph_count;
    s32 glyph_index;

    glyph_count = func_800644FC(glyphs, g_active_name, NAME_MEASURE_TEXT_COLOR);
    glyph_index = 0;
    g_name_pixel_width = 0;

    if (glyph_index < glyph_count)
    {
        while (glyph_index < glyph_count)
        {
            glyph_width = glyphs[glyph_index].width;
            g_name_pixel_width += glyph_width;
            glyph_index++;
        }
    }

    g_strip_width_target = g_name_pixel_width + NAME_STRIP_HORIZONTAL_PADDING;
}

/**
 * @brief Prepend a packed glyph to a name buffer.
 * @param name_buf Null-terminated buffer with room for the glyph.
 * @param new_glyph Packed glyph; a zero lead byte is ignored.
 * @see https://decomp.me/scratch/VOLcD (100%)
 */
static void name_prepend_glyph(u8* name_buf, u16 new_glyph)
{
    u8* scan_cursor;
    u32 byte_count;
    u32 glyph_size;
    u32 bytes_to_move;
    u32 byte_index;
    u16 glyph = new_glyph;

    if (LOW_BYTE(glyph) == 0)
    {
        return;
    }

    if (IS_DBCS_LEAD_BYTE(LOW_BYTE(glyph)))
    {
        glyph_size = NAME_GLYPH_SIZE_DOUBLE;
    }
    else
    {
        glyph_size = NAME_GLYPH_SIZE_SINGLE;
    }

    scan_cursor = name_buf;
    byte_count = 0;

    while (*scan_cursor != '\0')
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    /* Include the null terminator. */
    bytes_to_move = byte_count + 1;
    for (byte_index = bytes_to_move; byte_index > 0; byte_index--)
    {
        name_buf[(glyph_size + byte_index) - 1] = name_buf[byte_index - 1];
    }

    name_buf[0] = LOW_BYTE(glyph);
    if (glyph_size == NAME_GLYPH_SIZE_DOUBLE)
    {
        name_buf[1] = HIGH_BYTE(glyph);
    }
}

/**
 * @brief Remove and return the first glyph in a name buffer.
 * @param name_buf Null-terminated name buffer updated in place.
 * @return Removed glyph packed in the low 16 bits, or 0 if empty.
 * @see https://decomp.me/scratch/ArXXq (100%)
 */
static s32 name_pop_first_glyph(u8* name_buf)
{
    u8 first_byte;
    u32 glyph_size;
    u16 first_glyph;
    u8* tail_cursor;
    s32 tail_byte_count;
    s32 bytes_to_move;
    s32 byte_index;
    u32 glyph_value_mask;

    first_byte = name_buf[0];

    if (first_byte == '\0')
    {
        return 0;
    }

    if (IS_DBCS_LEAD_BYTE(first_byte))
    {
        first_glyph = MAKE_DBCS_GLYPH(name_buf[0], name_buf[1]);
        glyph_size = NAME_GLYPH_SIZE_DOUBLE;
    }
    else
    {
        first_glyph = name_buf[0];
        glyph_size = NAME_GLYPH_SIZE_SINGLE;
    }

    tail_byte_count = 0;
    tail_cursor = name_buf + glyph_size;

    while (*tail_cursor != '\0')
    {
        if (IS_DBCS_LEAD_BYTE(*tail_cursor))
        {
            tail_cursor += NAME_GLYPH_SIZE_DOUBLE;
            tail_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            tail_cursor += NAME_GLYPH_SIZE_SINGLE;
            tail_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    /* Include the null terminator. */
    bytes_to_move = tail_byte_count + 1;
    glyph_value_mask = NAME_GLYPH_VALUE_MASK;
    for (byte_index = 0; byte_index < bytes_to_move; byte_index++)
    {
        name_buf[byte_index] = name_buf[byte_index + glyph_size];
    }

    return first_glyph & glyph_value_mask;
}

/**
 * @brief Render and advance the glyph-append animation.
 * @param packet_cursor Next free primitive-buffer address.
 * @param render_ctx Current render context.
 * @return Updated primitive-buffer cursor.
 * @see decomp.me (100%) https://decomp.me/scratch/3TQG6
 */
static void* render_glyph_append_anim(void* packet_cursor, RenderContext* render_ctx)
{
    u8 frame_index = g_glyph_append_anim_frame;
    s32 slot_index;
    const GlyphAppendAnimSlot* slot = g_glyph_append_anim_frames[frame_index].slots;
    s16 glyph_id;

    for (slot_index = 0; slot_index < GLYPH_APPEND_ANIM_SLOT_COUNT; slot_index++, slot++)
    {
        s32 raw_glyph_id = slot->glyph;

        glyph_id = raw_glyph_id;

        if (glyph_id != 0)
        {
            packet_cursor = emit_glyph_sprt(packet_cursor, &render_ctx->ot[GNAME_OT_GLYPH_APPEND_ANIM], (u8)glyph_id,
                                            slot->x + GLYPH_APPEND_ANIM_X_BIAS, slot->y + GLYPH_APPEND_ANIM_Y_BIAS, 0, 0, FALSE);
        }
    }

    if (g_glyph_append_anim_timer == 0)
    {
        return packet_cursor;
    }

    g_glyph_append_anim_timer--;

    if (g_glyph_append_anim_timer == 0)
    {
        g_glyph_append_anim_frame++;

        if (g_glyph_append_anim_frame == GLYPH_APPEND_ANIM_FRAME_COUNT)
        {
            g_glyph_append_anim_frame = 0;
            g_glyph_append_anim_timer = 0;
            return packet_cursor;
        }

        g_glyph_append_anim_timer = g_glyph_append_anim_frames[g_glyph_append_anim_frame].slots[0].pad;
    }

    return packet_cursor;
}

/**
 * @brief Check whether a name contains only blank bytes.
 * @param name_buf Null-terminated name buffer.
 * @return TRUE if blank, otherwise FALSE.
 * @see https://decomp.me/scratch/rdbBA (100%)
 */
static s32 name_is_blank(const u8* name_buf)
{
    while (*name_buf != '\0')
    {
        if ((*name_buf != NAME_BYTE_SPACE) && (*name_buf != NAME_BYTE_ALT_BLANK))
        {
            return FALSE;
        }

        name_buf++;
    }

    return TRUE;
}
