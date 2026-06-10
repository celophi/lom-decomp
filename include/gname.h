#ifndef _GNAME_H
#define _GNAME_H

#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "main.h"
#include "pad.h"
#include "tim.h"
#include "render_context.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

/**
 * @brief Name-buffer character encoding.
 *
 * A "name" is a null-terminated byte buffer (`u8*`) used by the name-entry
 * UI. It uses a small DBCS-style variable-width encoding:
 *
 *  - Most bytes are single-byte glyphs (1 byte each).
 *  - A byte in the range [0x19, 0x20) is the *lead* byte of a 2-byte glyph;
 *    the following byte is its trail byte. There are 7 lead-byte values
 *    (0x19..0x1F), giving up to 7 "pages" of wide glyphs.
 *  - 0x00 terminates the string.
 *
 * The `name_*` helpers in gname.c walk the buffer respecting this encoding:
 * `name_byte_length` returns raw bytes, `name_char_count` returns logical
 * glyphs, `name_pop_first_char` / `name_pop_last_char` strip and return one
 * glyph (packing a 2-byte glyph as `lead | (trail << 8)`), and
 * `name_prepend_char` inserts one glyph at the front.
 *
 * `name_is_blank` is a special case: it walks byte-by-byte (not
 * glyph-by-glyph) and treats both ASCII space (0x20) and wide-space
 * sentinel (0x80) as blank.
 */
#define CHAR_SPACE      0x20 /**< ASCII space; blank glyph in name buffers. */
#define CHAR_WIDE_SPACE 0x80 /**< Wide-space sentinel byte; also blank. */

/* True if byte is a custom 2-byte DBCS-style lead byte */
#define IS_DBSC_LEAD_BYTE(c) ((c) >= 0x19 && (c) <= 0x1F)

/* Pack two bytes into a single 16-bit DBCS-style glyph */
#define MAKE_DBCS_GLYPH(lo, hi) (u16)(((u16)(hi) << 8) | (u16)(lo))

/**
 * Button mask for confirming a character selection in the name-entry grid.
 * Combines PAD_BTN_CROSS (physical Circle/O = confirm on Japanese PSX) with
 * 0x200 (R2 shoulder button as a secondary confirm input).
 */
#define GNAME_BTN_CONFIRM (PAD_BTN_CROSS | 0x200)

/**
 * Full input mask passed to handle_char_set_input each frame: all four
 * D-pad directions plus the confirm pair.
 */
#define GNAME_BTN_NAV_MASK \
    (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT | GNAME_BTN_CONFIRM)

/*
 * Character-set navigation mode values stored in g_char_set_mode and
 * passed to / returned from handle_char_set_input.
 *
 *   0-3  : action tab bar (OK, Delete, Random, Default)
 *   4-7  : character-panel selector tabs (panel N at mode 4+N)
 *   0x10 : in-grid character cursor
 */
#define GNAME_MODE_ACTION_OK       0    /* action bar: commit the name */
#define GNAME_MODE_ACTION_DELETE   1    /* action bar: delete last character */
#define GNAME_MODE_ACTION_RANDOM   2    /* action bar: fill with random name */
#define GNAME_MODE_ACTION_DEFAULT  3    /* action bar: reset to default name */
#define GNAME_MODE_PANEL_BASE      4    /* first char-panel tab; panel N is at 4+N */
#define GNAME_MODE_GRID            0x10 /* in-grid character cursor mode */

/* g_name_source_mode values: selects which name is pasted on Random/Default action. */
#define GNAME_SRC_CUSTOM       1  /* use g_custom_name_buf */
#define GNAME_SRC_HISTORY      3  /* pick from g_history_names via g_history_name_idx */
#define GNAME_SRC_RAND_PRIMARY 4  /* random entry from g_random_names primary index table */
#define GNAME_SRC_RAND_ALT     5  /* random entry from g_random_names alternate offset table */

/* Maximum number of logical characters allowed in a name (distinct from
 * NAME_GRID_CHARS_PER_ROW, which is the grid display width). */
#define NAME_MAX_CHARS  10

/* Sound effect IDs passed as the first argument to play_menu_sfx. */
#define GNAME_SFX_ERROR   0x78  /* error: name is full, blank, or action is invalid */
#define GNAME_SFX_MOVE    0x7D  /* cursor movement / navigation */
#define GNAME_SFX_CONFIRM 0x7E  /* confirm / OK action */
#define GNAME_SFX_CANCEL  0x7F  /* cancel / back action */
#define GNAME_SFX_VOLUME  0x80  /* default volume argument for play_menu_sfx */

/* Character selection grid layout constants. */
#define NAME_GRID_CHARS_PER_ROW  10  /**< Characters per row in the grid. */
#define NAME_GRID_CELL_SIZE      16  /**< Pixel width and height of each grid cell (0x10). */
#define NAME_GRID_X_BASE         84  /**< Pixel X of the leftmost grid column (0x54). */
#define NAME_GRID_Y_TOP         104  /**< Pixel Y of the top of the visible grid area (0x68). */
#define NAME_GRID_Y_BOTTOM      168  /**< Pixel Y of the bottom clamp (0xA8). */
#define NAME_GRID_SCROLL_STEP    64  /**< Scroll delta per step: 4 rows * 16 px/row (0x40). */

/**
 * @brief RGB lerp state.
 *
 * Used as a pair: `g_fade_target` is the *target* (final color + remaining
 * step count), `g_fade_current` is the *current* interpolated value (its
 * `steps` field is unused). Each tick @ref render_fade_overlay advances the
 * current toward the target by `(target - current) / steps` and decrements
 * `steps`. Channels are 0..FADE_CHAN_NEUTRAL (identity = no tint); values
 * above FADE_CHAN_NEUTRAL trigger additive blend mode.
 */
typedef struct
{
    s32 r;     /* 0x0 - red channel,   0..FADE_CHAN_NEUTRAL normal, >FADE_CHAN_NEUTRAL = additive */
    s32 g;     /* 0x4 - green channel, 0..FADE_CHAN_NEUTRAL normal, >FADE_CHAN_NEUTRAL = additive */
    s32 b;     /* 0x8 - blue channel,  0..FADE_CHAN_NEUTRAL normal, >FADE_CHAN_NEUTRAL = additive */
    s32 steps; /* 0xC - frames remaining in the lerp (target struct only) */
} FadeState;

/* FadeState channel sentinels. Channels run 0 (fully dark) up to
 * FADE_CHAN_NEUTRAL (identity, no tint). Values >= FADE_CHAN_ADDITIVE
 * select additive blend; values below select subtractive. */
#define FADE_CHAN_NEUTRAL   0x100
#define FADE_CHAN_ADDITIVE  0x101

/* tpage arguments for the blend-mode DR_TPAGE emitted by render_fade_overlay.
 * The tile is flat-colored so only the abr bits matter; x=320 is the
 * right-half VRAM column used as the tpage base. */
#define FADE_TPAGE_ADD  0x25 /* getTPage(0, 1, 320, 0) - abr=1: Back + Front */
#define FADE_TPAGE_SUB  0x45 /* getTPage(0, 2, 320, 0) - abr=2: Back - Front */

/**
 * @brief TIM upload destination coordinates for @ref load_tim_to_vram.
 *
 * Holds two VRAM destination points: one for the pixel data and one for the
 * CLUT row. Not a libgpu RECT (which carries width/height); this is just
 * two (x, y) pairs packed as four consecutive s16s.
 *
 * @see load_name_entry_tim for typical values.
 */
typedef struct
{
    s16 pixel_x; /* 0x0 - VRAM x of pixel-data destination */
    s16 pixel_y; /* 0x2 - VRAM y of pixel-data destination */
    s16 clut_x;  /* 0x4 - VRAM x of CLUT destination */
    s16 clut_y;  /* 0x6 - VRAM y of CLUT destination */
} TimDstCoords;

/**
 * @brief Glyph metrics entry: how to draw one glyph from VRAM.
 *
 * Used as @c g_glyph_table, indexed by character ID. The
 * fields are written directly into a sprite (tag 0x64) primitive by
 * @ref func_80142274: `u`/`v` at byte offsets 12/13, the CLUT ID at u16
 * offset 14, and `w`/`h` at u16 offsets 16/18.
 */
typedef struct
{
    u8 u;     /* 0x0 - texture U in VRAM */
    u8 v;     /* 0x1 - texture V in VRAM */
    u8 w;     /* 0x2 - sprite width */
    u8 h;     /* 0x3 - sprite height */
    u32 clut; /* 0x4 - CLUT id (low 6 bits used; combined with 0x7C80) */
} GlyphInfo;

/**
 * @brief One entry of the name-entry cursor glyph table at
 *        @c g_name_cursor_glyphs.
 *
 * 20 of these are walked by @ref draw_name_cursor_row each frame to emit
 * the cursor's textured-sprite row.
 */
typedef struct
{
    u32 id; /* 0x0 - index into g_glyph_table (selects which glyph to draw) */
    u32 xy; /* 0x4 - packed s16 x,y screen position (low half = x, high = y) */
} GlyphSeqEntry;

/** Number of glyph cells in the name-entry cursor row drawn by
 *  @ref draw_name_cursor_row. */
#define NAME_CURSOR_GLYPH_COUNT 20

/**
 * @brief One entry in the tab-cursor and scroll-indicator position table.
 *
 * @c g_tab_cursor_pos[0..1] are the scroll-up and scroll-down indicator
 * glyphs. @c g_tab_cursor_entries[0..10] are the cursor target positions for
 * the 11 character-panel tabs.
 *
 * The @c x field is read whole (LW) by gname_render for use with @c & 0x1FF;
 * only the low 9 bits represent the screen X coordinate.
 */
typedef struct {
    unsigned int x : 9;
    unsigned int pad : 7;
    u8 y;
    u8 glyph;
} TabCursorEntry;

/** Mask for the CLUT X-column index stored in @c GlyphInfo::clut.
 *  Bits [5:0] hold CLUT_X/16; upper bits carry unrelated data and must be
 *  discarded before writing the CLUT id into a sprite primitive. */
#define GLYPH_CLUT_X_MASK 0x3F

/** CLUT-page bit pattern OR'd over the low 6 bits of @c GlyphInfo::clut
 *  before writing it into a sprite primitive (see @ref draw_name_cursor_row,
 *  @ref func_80142274). Encodes the fixed VRAM Y row (498) shared by all
 *  name-entry palettes; bits [5:0] are zero and supplied by @c GLYPH_CLUT_X_MASK. */
#define GLYPH_CLUT_PAGE_BITS 0x7C80

/**
 * @brief One glyph slot inside an @ref AppendAnimFrame.
 *
 * @ref draw_char_append_anim emits a textured-glyph SPRT for every slot
 * whose @c glyph id is non-zero, at screen position (@c x + 0xE8,
 * @c y + 4).
 */
typedef struct
{
    u8 x;     /* 0x0 - X position (biased by 0xE8 when drawn) */
    u8 y;     /* 0x1 - Y position (biased by 4 when drawn) */
    u8 glyph; /* 0x2 - glyph id (index into g_glyph_table); 0 = empty slot */
    u8 pad;   /* 0x3 - unused; slot 0 only: frame duration in render ticks */
} AppendAnimSlot;

/** Number of glyph slots per @ref AppendAnimFrame. */
#define APPEND_ANIM_SLOT_COUNT 3

/**
 * @brief One frame of the character-append animation played by
 *        @ref draw_char_append_anim.
 *
 * @c g_char_append_anim holds @ref APPEND_ANIM_FRAME_COUNT of these. Frame 0
 * is the idle (empty) frame; frames 1.. are the animation. The frame is
 * shown for @c slots[0].pad render ticks before advancing.
 */
typedef struct
{
    AppendAnimSlot slots[APPEND_ANIM_SLOT_COUNT];
} AppendAnimFrame; /* 0xC bytes */

/** Number of frames in @c g_char_append_anim; reaching this index wraps the
 *  animation back to the idle frame and stops it. */
#define APPEND_ANIM_FRAME_COUNT 7

/** Byte stride of one @ref AppendAnimFrame record in @c g_char_append_anim. */
#define APPEND_ANIM_FRAME_STRIDE 12

/**
 * @brief Per-glyph measurement record produced by @c func_800644FC.
 *
 * @ref recalc_name_width passes an array of these to @c func_800644FC,
 * which fills one entry per glyph of a name buffer. @c width is then
 * summed to obtain the name's rendered pixel width. Only @c width is
 * currently understood; the surrounding bytes are unknown.
 */
typedef struct
{
    u8 pad0[0x10];
    s16 width; /* 0x10 - glyph advance / pixel width */
    u8 pad1[2];
} GlyphMeasure;

extern void* func_80142274(void* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7);

/* --- Named data globals --- */
extern s32 g_name_pixel_width;
extern u8 g_char_append_anim[]; /* AppendAnimFrame[APPEND_ANIM_FRAME_COUNT]; declared as u8[] for byte-level accesses */
extern FadeState g_fade_target;
extern FadeState g_fade_current;
extern s32 g_startup_delay;
extern Tim g_name_entry_tim; /* glyph TIM blob; Tim covers the fixed header + CLUT, pixel block follows */
extern s32 g_strip_width_target;
extern s32 g_strip_width;
extern u8* g_active_name;
extern u8 g_append_anim_timer; /* render ticks until the next animation frame */
extern u8 g_append_anim_frame; /* current frame index into g_char_append_anim */
extern s32 g_strip_width_steps;
extern GlyphInfo g_glyph_table[];
/**
 * Tab cursor and scroll-indicator position table (@ref TabCursorEntry).
 * Entry [0] = scroll-up indicator; entry [1] = scroll-down indicator.
 * The table is contiguous with @ref g_tab_cursor_entries at 0x80142E14.
 */
extern TabCursorEntry g_tab_cursor_pos[];
/**
 * Cursor target positions for the 11 character-panel tabs (@ref TabCursorEntry).
 * Indexed 0..10; tab index 7 is unused (no valid mode maps to it).
 * Immediately follows @ref g_tab_cursor_pos in ROM; accessed together via
 * @c g_tab_cursor_pos base + @c (cur_mode + 2) * 4 in @ref handle_char_set_input.
 */
extern TabCursorEntry g_tab_cursor_entries[];
extern GlyphSeqEntry g_name_cursor_glyphs[];

/* --- Globals named during decomp --- */

/** 48-byte name buffer holding the custom preset name (used when g_name_source_mode == 1). */
extern u8 g_custom_name_buf;
/** Which preset name source to paste: 1 = custom (g_custom_name_buf), 3 = history
 *  (g_history_name_idx), 4/5 = timer-seeded random name table. */
extern s32 g_name_source_mode;
/** Overlay exit code written when the session ends: 2 = cancel, 5 = confirm. */
extern s32 g_overlay_result;
/** 48-byte name buffer; initial content copied into g_active_name at reset. */
extern u8 g_initial_name;
/** If non-zero, pressing cancel while the name is empty triggers an overlay exit. */
extern s32 g_allow_empty_cancel;
/** Index into the saved-name history list (used when g_name_source_mode == 3). */
extern s32 g_history_name_idx;
/** Base address for the double-buffered render/primitive scratch buffers. */
extern s32 g_render_buf_base;
/** Active character panel index: 0-2 = character-set tabs, 3 = kanji category
 *  picker, 4 = kanji character picker within a selected category. */
extern s32 g_char_panel;
/** Pointer to the current kanji category's display data (set when g_char_panel == 4). */
extern void* g_kanji_cat_name;
/** 48-byte clipboard buffer; deleted chars are prepended here and can be re-pasted. */
extern u8 g_name_clipboard;
/** Frames remaining in the cursor-position lerp animation. */
extern s32 g_cursor_lerp_steps;
/** Index of the highlighted tab in the left selection grid (0xFF = none highlighted). */
extern s32 g_cursor_tab;
/** Cursor current X position (being lerped toward g_cursor_x_target). */
extern s32 g_cursor_x;
/** Cursor current Y position (being lerped toward g_cursor_y_target). */
extern s32 g_cursor_y;
/** Cursor target X position for the lerp animation. */
extern s32 g_cursor_x_target;
/** Last column index of the rightmost character in the current grid panel. */
extern s32 g_char_last_col;
/** Cursor target Y position for the lerp animation. */
extern s32 g_cursor_y_target;
/** Row index of the last character in the current grid panel (used for scroll bounds). */
extern s32 g_char_last_row;
/** Current character-set navigation state: 0-7 = character set tabs, 0x10 = kanji picker. */
extern s32 g_char_set_mode;
/** Current horizontal scroll position of the character grid in pixels. */
extern s32 g_scroll_pos;
/** Target horizontal scroll position for the scroll lerp. */
extern s32 g_scroll_target;
/** Frames remaining in the scroll lerp animation. */
extern s32 g_scroll_steps;
/** Currently selected kanji category index. */
extern s32 g_kanji_cat;
/** Linearized character cursor position in the grid: row * 10 + col. */
extern s32 g_char_cursor;

/* --- ROM data tables --- */
/** Random name pool used when g_name_source_mode == 4 or 5. */
extern u8* g_random_names;
/** History name list used when g_name_source_mode == 3. */
extern u8* g_history_names;
/** Kanji character panel glyph data base pointer. */
extern u8* g_kanji_panel_data;
/** Kanji category entry index table: [cat] -> sub-index into g_kanji_entry_offsets, or 0xFF. */
extern u32 g_kanji_cat_entries[];
/** Kanji sub-index to glyph offset lookup table. */
extern u32 g_kanji_entry_offsets[];
/** Character panel glyph data: element[0] is the self-referential base offset. */
extern u8* g_char_panel_data;
/** Per-panel character set base offsets (u32 per panel; low u16 = row count). */
extern u32 g_panel_char_offsets[];
/** u16 offset within g_char_panel_data where kanji category name offsets begin. */
extern s32 g_kanji_cat_names_offset;
extern u8 D_80142EF4[];

/**
 * @brief Play a one-shot UI sound effect via the AKAO driver.
 * @param sfx_id Sound effect index (GNAME_SFX_* constants).
 * @param volume Playback volume; use GNAME_SFX_VOLUME (0x80) for default.
 */
extern void play_menu_sfx(int sfx_id, int volume);
extern void func_8014139C(void);
extern s32 name_char_count(u8*);
extern s32 name_is_blank(u8*);
extern s32 handle_char_set_input(s32 mode, s32 buttons);
extern void name_copy(u8*, u8*);

#endif