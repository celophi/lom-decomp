#include "gname.h"

#include "cd.h"
#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "main.h"
#include "pad.h"
#include "tim.h"
#include "psyq/libetc.h"
#include "psyq/libgpu.h"
#include "psyq/libgte.h"
#include "psyq/memory.h"

/** Number of glyph slots per @ref AppendAnimFrame; defined here because it is
 *  the array dimension of the struct's @c slots[] member below. */
#define APPEND_ANIM_SLOT_COUNT 3

/**
 * @name GNAME ordering-table depth slots
 *
 * Hand-assigned indices into @ref RenderContext::ot that the name-entry
 * render passes chain their primitives into. The OT is a Z-priority bucket
 * list, so each constant just fixes one element's place in the draw order;
 * the numbers carry no meaning beyond their relative layering. Lower slots
 * sort in front of higher ones, so the text cursor (8) draws over the
 * character grid (11), which draws over the name strip (14).
 * @{
 */
#define GNAME_OT_FRONT          0x00 /* fade overlay, scroll indicators, panel-tab sprite, label draw-mode */
#define GNAME_OT_TEXT_CURSOR    0x08 /* text cursor glyph + DrawTPage */
#define GNAME_OT_PANEL_LABEL    0x09 /* category-label sprite */
#define GNAME_OT_CHAR_PANEL     0x0A /* scrolling character-panel grid */
#define GNAME_OT_CHAR_GRID      0x0B /* character grid glyphs */
#define GNAME_OT_CHAR_APPEND_ANIM 0x0C /* character-append animation glyphs */
#define GNAME_OT_CHAR_APPEND    0x0D /* static append glyph + draw-mode */
#define GNAME_OT_NAME_STRIP     0x0E /* name strip (entered-name display) */
#define GNAME_OT_NAME_CURSOR    0x0F /* name-entry cursor row */
/** @} */

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

/** L2: undo -- pop the last character from the active name back to the clipboard. */
#define GNAME_BTN_UNDO PAD_BTN_L2
/** R2: redo -- pop the first character from the clipboard and append to the active name. */
#define GNAME_BTN_REDO PAD_BTN_R2
/** L1: scroll/cycle to the previous kanji category (decrement by 10). */
#define GNAME_BTN_KANJI_PREV PAD_BTN_L1
/** R1: scroll/cycle to the next kanji category (increment by 10). */
#define GNAME_BTN_KANJI_NEXT PAD_BTN_R1
/** Combined mask: either kanji-category navigation button (L1 or R1). */
#define GNAME_BTN_KANJI_NAV (GNAME_BTN_KANJI_PREV | GNAME_BTN_KANJI_NEXT)

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

/* Sentinel for g_cursor_tab meaning "no tab/grid cell selected". */
#define GNAME_TAB_NONE 0xFF

/* g_overlay_result values: how the overlay finished, read by the caller. */
#define GNAME_RESULT_CANCEL  2  /* cancelled with an empty name (when allowed) */
#define GNAME_RESULT_CONFIRM 5  /* name committed; advance to the next overlay stage */

/* Frame count seeded into g_strip_width_steps to start a name-strip width lerp. */
#define NAME_STRIP_LERP_STEPS 5

/* Frame count seeded into g_append_anim_timer to start the append animation. */
#define APPEND_ANIM_TIMER_START 2

/* g_name_source_mode values: selects which name is pasted on Random/Default action. */
#define GNAME_SRC_CUSTOM       1  /* use g_custom_name_buf */
#define GNAME_SRC_HISTORY      3  /* pick from g_history_names_off via g_history_name_idx */
#define GNAME_SRC_RAND_PRIMARY 4  /* random entry from g_random_names_off primary index table */
#define GNAME_SRC_RAND_ALT     5  /* random entry from g_random_names_off alternate offset table */

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
#define NAME_GRID_VRAM_X        0x60 /**< VRAM X of the grid area upload rect (96 px). */
#define NAME_GRID_VIS_HEIGHT    0x50 /**< Visible grid height in pixels: 5 rows * 16 (80 px). */

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

/* tpage for the overlay's 4-bit glyph/font texture (cursor, text, DrawMode
 * packets). getTPage(0, 0, 320, 0): 4-bit CLUT, abr=0, VRAM page at x=320. */
#define GNAME_GLYPH_TPAGE 5

/** Number of glyph cells in the name-entry cursor row drawn by
 *  @ref draw_name_cursor_row. */
#define NAME_CURSOR_GLYPH_COUNT 20

/** Mask for the CLUT X-column index stored in @c GlyphInfo::clut.
 *  Bits [5:0] hold CLUT_X/16; upper bits carry unrelated data and must be
 *  discarded before writing the CLUT id into a sprite primitive. */
#define GLYPH_CLUT_X_MASK 0x3F

/** CLUT-page bit pattern OR'd over the low 6 bits of @c GlyphInfo::clut
 *  before writing it into a sprite primitive (see @ref draw_name_cursor_row,
 *  @ref emit_glyph_sprt). Encodes the fixed VRAM Y row (498) shared by all
 *  name-entry palettes; bits [5:0] are zero and supplied by @c GLYPH_CLUT_X_MASK. */
#define GLYPH_CLUT_PAGE_BITS 0x7C80

/** Number of frames in @c g_char_append_anim; reaching this index wraps the
 *  animation back to the idle frame and stops it. */
#define APPEND_ANIM_FRAME_COUNT 7

/** Byte stride of one @ref AppendAnimFrame record in @c g_char_append_anim. */
#define APPEND_ANIM_FRAME_STRIDE 12

/* The panel data blob is described by the PanelDataHeader struct above. Its
 * header fields and record-offset table
 * (g_random_names_off, g_history_names_off, g_kanji_panel_off,
 * g_panel_record_offsets, g_panel_tbl_off, g_panel_data_base) plus
 * g_kanji_cat_entries and g_kanji_entry_offsets are defined below in this file.
 *
 * g_panel_tbl_off (blob + 4) holds the byte offset (0x14) from the blob base
 * to the u16 record-offset table; a record pointer is therefore
 * PANEL_DATA_BLOB + g_panel_tbl_off + table[i]. The PANEL_* macros below
 * depend on g_panel_tbl_off being defined ahead of their use. */

/**
 * @brief Base address of the character panel data blob, derived from
 *        @ref g_panel_tbl_off (the header field at blob + 4).
 *
 * The base must be derived from @c &g_panel_tbl_off with a runtime
 * @c - 4 (not referenced via the @ref g_panel_data_base symbol): the
 * original code shares a single @c lui between loading the field's value
 * and forming the base address, leaving the @c -4 as a separate @c addiu
 * in the binary.
 */
#define PANEL_DATA_BLOB (((u8*)(&g_panel_tbl_off)) - 4)

/** The blob's u16 record-offset table. Must stay a macro: the original
 *  re-derives the table at every use (no CSE), which a named local would
 *  destroy. */
#define PANEL_REC_TBL ((u16*)(PANEL_DATA_BLOB + g_panel_tbl_off))

/** Pointer to record i: the table is self-relative, entries are byte
 *  offsets from the table itself (same idiom as FF8's string tables). */
#define PANEL_RECORD(i) ((u8*)PANEL_REC_TBL + PANEL_REC_TBL[(i)])

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
 * @ref emit_glyph_sprt: `u`/`v` at byte offsets 12/13, the CLUT ID at u16
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

/**
 * @brief One entry in the tab-cursor and scroll-indicator position table.
 *
 * @c g_tab_cursor_pos[0..1] are the scroll-up and scroll-down indicator
 * glyphs. @c g_tab_cursor_entries[0..10] are the cursor target positions for
 * the 11 character-panel tabs.
 *
 * The @c x bitfield occupies the low 9 bits of the first word; accessing it
 * directly generates the same @c lw + @c andi sequence as the raw LW+mask form.
 *
 * @note @c sprite_idx (bits 9..15) is the tab's entry index into the panel
 * data blob's u16 record-offset table (see @ref g_panel_record_offsets).
 * @ref emit_panel_tab_sprite must read it as the raw word via
 * @c (word >> 8) & 0xFE (= @c sprite_idx * 2); a bitfield read would compile
 * to @c srl 9 / @c andi 0x7F / @c sll 1 and break the match.
 */
typedef struct {
    unsigned int x : 9;
    unsigned int sprite_idx : 7;
    u8 y;
    u8 glyph;
} TabCursorEntry;

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

/**
 * @brief Header layout of the character-panel resource blob at
 *        @ref g_panel_data_base (0x80142EF4).
 *
 * The "tables" below are really one serialized data file whose header is a
 * run of u32 byte offsets; splat carved each header field into its own
 * symbol. A record inside the blob is always resolved as
 * @c blob_base + header_field + table_entry.
 *
 * This typedef is documentation only - do not re-type the accesses with it.
 * The matched code anchors each access at the individual field symbol and
 * subtracts the field's offset at runtime (e.g. @c &g_panel_tbl_off - 4,
 * @c g_random_names_off - 0x10), which shares one @c lui between the field load
 * and the base address. Anchoring at a blob-base symbol instead drops the
 * runtime @c addiu and breaks the match.
 */
typedef struct
{
    u32 unk0;        /* 0x00 - g_panel_data_base: stored value 4; purpose unknown */
    u32 tbl_off;     /* 0x04 - g_panel_tbl_off: offset of the u16 record-offset table (0x14) */
    u32 kanji_off;   /* 0x08 - g_kanji_panel_off: offset of the kanji panel glyph data (0x2A0) */
    u32 history_off; /* 0x0C - g_history_names_off: offset of the history name list (0x3754) */
    u32 random_off;  /* 0x10 - g_random_names_off: offset of the random name pool (0x3C9C) */
    /* g_panel_record_offsets (u16[]) follows at +0x14; each entry is a byte
     * offset from the table itself to one record (panel glyph lists, category
     * labels, tab sprites, kanji category names). */
} PanelDataHeader;

/* --- Data-blob globals (raw bytes in the gname_data databin) --- */
extern u8 g_char_append_anim[]; /* AppendAnimFrame[APPEND_ANIM_FRAME_COUNT]; declared as u8[] for byte-level accesses */
extern Tim g_name_entry_tim; /* glyph TIM blob; Tim covers the fixed header + CLUT, pixel block follows */
extern GlyphSeqEntry g_name_cursor_glyphs[];

/* --- Overlay .bss scratch globals -------------------------------------------
 *
 * Uninitialized run-state RAM owned by this translation unit (gname.o(.bss),
 * 0x8014F7B0..0x8014F8D8). Defined here in ascending address order so the
 * compiler lays them out matching the original; do not reorder. The single-byte
 * fields (g_append_anim_frame, g_append_anim_timer) are each followed by an
 * explicit 3-byte pad field: this compiler does not implicitly align the
 * following s32 global to a 4-byte boundary, so the gap must be spelled out
 * or the subsequent globals land 3 bytes early.
 */

/** 48-byte name buffer holding the custom preset name (used when g_name_source_mode == 1). */
u8 g_custom_name_buf[48];
/** Which preset name source to paste: 1 = custom (g_custom_name_buf), 3 = history
 *  (g_history_name_idx), 4/5 = timer-seeded random name table. */
s32 g_name_source_mode;
/** Overlay exit code written when the session ends: 2 = cancel, 5 = confirm. */
s32 g_overlay_result;
/** 48-byte name buffer; initial content copied into g_active_name at reset. */
u8 g_initial_name[48];
/** RGB fade target color plus remaining lerp step count. */
FadeState g_fade_target;
/** RGB fade current interpolated color. */
FadeState g_fade_current;
/** If non-zero, pressing cancel while the name is empty triggers an overlay exit. */
s32 g_allow_empty_cancel;
/** Index into the saved-name history list (used when g_name_source_mode == 3). */
s32 g_history_name_idx;
/** Base of the double-buffered render/primitive scratch buffers; the two frames
 *  are @c g_render_buf_base[0] and @c g_render_buf_base[1] (stride
 *  @ref DRAW_BUF_STRIDE == @c sizeof(RenderContext)). */
RenderContext* g_render_buf_base;
/** Active name buffer the UI edits in place. */
u8* g_active_name;
/** Active character panel index: 0-2 = character-set tabs, 3 = kanji category
 *  picker, 4 = kanji character picker within a selected category. */
s32 g_char_panel;
/** Pointer to the current kanji category's display data (set when g_char_panel == 4). */
void* g_kanji_cat_name;
/** 48-byte clipboard buffer; deleted chars are prepended here and can be re-pasted. */
u8 g_name_clipboard[48];
/** Frames remaining before name-entry input is accepted at startup. */
s32 g_startup_delay;
/** Frames remaining in the cursor-position lerp animation. */
s32 g_cursor_lerp_steps;
/** Index of the highlighted tab in the left selection grid (0xFF = none highlighted). */
s32 g_cursor_tab;
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
/** Row index of the last character in the current grid panel (used for scroll bounds). */
s32 g_char_last_row;
/** Frames remaining in the name-strip width lerp animation. */
s32 g_strip_width_steps;
/** Current name-strip width in pixels (being lerped toward g_strip_width_target). */
s32 g_strip_width;
/** Current character-set navigation state: 0-7 = character set tabs, 0x10 = kanji picker. */
s32 g_char_set_mode;
/** Current frame index into g_char_append_anim. */
u8 g_append_anim_frame;
/** Explicit alignment pad; see the block comment above. */
u8 pad_8014F8B1[3];
/** Current horizontal scroll position of the character grid in pixels. */
s32 g_scroll_pos;
/** Render ticks until the next append-animation frame. */
u8 g_append_anim_timer;
/** Explicit alignment pad; see the block comment above. */
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
/** Trailing 4 bytes of the overlay's .bss; unreferenced by name, purpose
 *  unknown. Kept so the overlay's total .bss size (and file length once
 *  linked) matches the original exactly. */
s32 D_8014F8D4;

/**
 * @brief Play a one-shot UI sound effect via the AKAO driver.
 * @param sfx_id Sound effect index (GNAME_SFX_* constants).
 * @param volume Playback volume; use GNAME_SFX_VOLUME (0x80) for default.
 * @note Defined in MENU.BIN; see config/symbols/gname_symbol_addrs.txt.
 */
void play_menu_sfx(int sfx_id, int volume);

/* --- Internal (file-local) function forward declarations --------------------
 *
 * Everything this overlay implements is private except gname_init and
 * gname_tick (declared in gname.h), the two entry points FIELD invokes. The
 * rest are static so they stay encapsulated in this translation unit.
 */
static void reset_fade_state(void);
static void render_fade_overlay(RenderContext* ctx);
static void set_fade_target(s32 r, s32 g, s32 b, s32 steps);
static void load_name_entry_tim(void);
static void load_tim_to_vram(TimDstCoords* dst_coords);
static void gname_update_state(void);
static void reset_run_state(void);
static s32 handle_char_set_input(s32 mode, s32 buttons);
static void gname_process_input(void);
static u_long* emit_cursor_glyph(u_long* prim, u_long* ot, s16 x, s16 y);
static void gname_render(RenderContext* context);
static void* emit_panel_tab_sprite(void* prim, u_long* ot_entry);
static void* emit_panel_label(void* prim, u_long* ot_entry);
static void render_name_strip(RenderContext* ctx, s32 name_buf, s32 strip_width);
static void render_char_panel(RenderContext* ctx, s32 panel_idx);
static void* emit_draw_mode_prim(DR_TPAGE* prim, u_long* ot_head);
static void* emit_glyph_sprt(void* prim_buf, u_long* ot_tag, s32 glyph_id, s32 x, s32 y, s32 shadow_dist, s32 primary_adj, s32 highlight);
static void draw_name_cursor_row(RenderContext* ctx);
static s32 name_byte_length(u8* name);
static s32 name_char_count(const u8* name);
static void name_append(u8* dst, const u8* src);
static s32 name_pop_last_char(u8* name);
static void name_copy(u8* dst, const u8* src);
static void recalc_name_width(void);
static void name_prepend_char(u8* buffer, u16 new_char);
static s32 name_pop_first_char(u8* name);
static void* draw_char_append_anim(void* prim, RenderContext* ctx);
static s32 name_is_blank(u8* name);

const s32 g_gname_overlay_id = 5;

/* --- Constant data tables ---------------------------------------------------
 *
 * These map onto the leading run of the overlay's .data section
 * (0x80142C98..0x8014301C), which is now packed into the databin asset blob
 * (see config/overlays/GNAME.BIN.yaml) instead of being emitted from this
 * file, since it is derived from copyrighted game data. The extern
 * declarations below just give the existing references in this file a
 * symbol and layout to resolve against; the bytes themselves come from the
 * linked databin.
 */

/** Per-panel character set base offsets (low u16 = row count). */
extern u32 g_panel_char_offsets[];

/** First record-offset entry index of the kanji category name records. */
extern s32 g_kanji_cat_names_offset;

/** Kanji category entry index table: [cat] -> sub-index into
 *  g_kanji_entry_offsets, or 0xFF when empty. */
extern u32 g_kanji_cat_entries[];

/** Glyph metrics table indexed by character id (see @ref GlyphInfo). */
extern GlyphInfo g_glyph_table[];

/** Scroll-up [0] and scroll-down [1] indicator glyphs, followed by cursor
 *  target positions for the 11 character-panel tabs. */
extern TabCursorEntry g_tab_cursor_pos[];
extern TabCursorEntry g_tab_cursor_entries[];

/** Kanji sub-index to glyph offset lookup table. */
extern u32 g_kanji_entry_offsets[];

/* --- Character panel data blob ----------------------------------------------
 *
 * The next run (0x80142EF4..0x8014301C) is the header of the serialized panel
 * data file (see @ref PanelDataHeader) followed by its u16 record-offset
 * table. These symbols stay contiguous and in this order inside the databin
 * blob: the code derives the blob base from individual field addresses at
 * runtime (e.g. PANEL_DATA_BLOB = &g_panel_tbl_off - 4,
 * g_random_names_off - 0x10).
 *
 * The header fields are u32 byte offsets; the u8[] ones are kept as byte
 * arrays so the pointer arithmetic in the referencing code is unchanged.
 */

/** Blob + 0x00: stored value 4; purpose unknown. */
extern u8 g_panel_data_base[];
/** Blob + 0x04: offset (0x14) of the u16 record-offset table. */
extern u32 g_panel_tbl_off;
/** Blob + 0x08: offset (0x2A0) of the kanji panel glyph data. */
extern u8* g_kanji_panel_off;
/** Blob + 0x0C: offset (0x3754) of the history name list. */
extern u8 g_history_names_off[];
/** Blob + 0x10: offset (0x3C9C) of the random name pool. */
extern u8 g_random_names_off[];

/** Blob + 0x14: u16 record-offset table (138 entries). Each entry is a byte
 *  offset from the table itself to one record. */
extern u16 g_panel_record_offsets[];

/* --- Cross-module helpers invoked by the GNAME run loop (defined in other
 *     overlays / the main executable). ------------------------------------- */
void func_800157B0(unsigned long arg0);
void func_800157DC(void);
void func_80063194(void);
void func_8006441C(void);
void func_80068440(void);
void func_800A9E78(void);
void func_800AA02C(void);

/**
 * @brief GNAME overlay entry point: run the name-entry UI to completion.
 *
 * Installs the caller's double-buffered render context, seeds the name-entry
 * state (initial/custom/clipboard buffers, source mode, history index), sets up
 * both frames' display/draw environments, then spins the per-frame loop:
 * rebuild the OT, draw the world and fade overlay, tick the name-entry UI, and
 * flip buffers - until @ref g_overlay_result becomes non-zero (confirm/cancel).
 *
 * On exit, when entering a history name (@c source_mode == 3) targeting the pad
 * context's history buffer, the entered name is copied back into the
 * appropriate per-slot history table (0x14C-stride when @c unk29D7's low 7 bits
 * are 4, otherwise the 0x60-stride table).
 *
 * @param buf_base Render context base; the two double buffers are @c buf_base[0] and @c buf_base[1].
 * @param initial_name Initial name buffer (0x30 bytes) copied into @ref g_initial_name.
 * @param active_name Active name buffer the UI edits in place (stored in @ref g_active_name).
 * @param source_mode Name source mode (stored in @ref g_name_source_mode).
 * @param history_idx History list index (stored in @ref g_history_name_idx).
 * @param custom_name Custom preset name buffer (0x30 bytes) copied into @ref g_custom_name_buf.
 * @param allow_empty_cancel Allow-empty-cancel flag (stored in @ref g_allow_empty_cancel).
 * @return The overlay result code (@ref g_overlay_result): cancel or confirm.
 * @see https://decomp.me/scratch/FAyP7 (100%)
 */
s32 gname_run(RenderContext* buf_base, u8* initial_name, u8* active_name, s32 source_mode, s32 history_idx, u8* custom_name, s32 allow_empty_cancel)
{
    s32 i;
    RenderContext* draw_buf;
    RenderContext* next_buf;
    RenderContext* other_buf;
    RenderContext* buf;

    g_render_buf_base = buf_base;
    bcopy(initial_name, g_initial_name, sizeof(g_initial_name));
    bcopy(custom_name, g_custom_name_buf, sizeof(g_custom_name_buf));
    g_allow_empty_cancel = allow_empty_cancel;
    g_active_name = active_name;
    g_name_source_mode = source_mode;
    g_history_name_idx = history_idx;
    g_render_buf_base[0].clear_rect.x = 0;
    g_render_buf_base[0].clear_rect.y = 8;
    g_render_buf_base[0].clear_rect.w = 0x140;
    g_render_buf_base[0].clear_rect.h = 0xE0;
    g_render_buf_base[1].clear_rect.x = 0;
    g_render_buf_base[1].clear_rect.y = 0xF0;
    g_render_buf_base[1].clear_rect.w = 0x140;
    g_render_buf_base[1].clear_rect.h = 0xE0;
    VSync(0);
    DrawSync(0);
    SetDefDispEnv(&g_render_buf_base[0].disp_env, 0, 0, 0x140, 0xF0);
    SetDefDispEnv(&g_render_buf_base[1].disp_env, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(&g_render_buf_base[0].draw_env, 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(&g_render_buf_base[1].draw_env, 0, 8, 0x140, 0xE0);
    buf = g_render_buf_base;
    buf[1].draw_env.dtd = 0;
    buf[0].draw_env.dtd = 0;
    g_overlay_result = 0;
    g_render_buf_base[0].frame_parity = 0;
    g_render_buf_base[1].frame_parity = 1;
    reset_fade_state();
    set_fade_target(0x100, 0x100, 0x100, 0x14);
    gname_init();
    next_buf = g_render_buf_base;
    ClearOTagR((u32*)next_buf, 0x10);
    ClearOTagR((u32*)(&g_render_buf_base[1]), 0x10);
    VSync(0);
    PutDispEnv(&next_buf->disp_env);
    func_800157DC();
    SetDispMask(1);
    func_800AA02C();
    while (1)
    {
        draw_buf = next_buf;
        ClearOTagR((u32*)draw_buf, 0x10);
        draw_buf->prim_cursor = &draw_buf->ot[0x10];
        func_8006441C();
        func_800A9E78();
        render_fade_overlay(draw_buf);
        gname_tick(draw_buf);
        func_80063194();
        if (g_overlay_result != 0)
        {
            break;
        }
        func_80068440();
        DrawSync(0);
        func_800157B0(2U);
        VSync(2);
        if (g_overlay_result != 0)
        {
            break;
        }
        ClearImage(&draw_buf->clear_rect, 0U, 0U, 0U);
        other_buf = g_render_buf_base;
        if (draw_buf == g_render_buf_base)
        {
            other_buf = &g_render_buf_base[1];
        }
        next_buf = other_buf;
        PutDispEnv(&other_buf->disp_env);
        PutDrawEnv(&next_buf->draw_env);
        DrawOTag(&draw_buf->ot[15]);
        draw_buf = other_buf;
        func_800157DC();
        cdrom_process_state();
    }

    DrawSync(0);
    VSync(0);
    func_800AA02C();
    if ((source_mode == 3) && (active_name == (&g_pad_ctx->pad85C[0x234])))
    {
        i = 0;
        if ((g_pad_ctx->unkAA8 & 0x7F) == 4)
        {
            while (i < 0x15)
            {
                s32 m = g_pad_ctx->unk29D7 * 0x14C;
                u8* p = (u8*)g_pad_ctx + m + i;
                p[0x2B0C] = active_name[i];
                i += 1;
            }
        }
        else
        {
            while (i < 0x15)
            {
                s32 m = g_pad_ctx->unk2EF0 * 0x60;
                u8* p = (u8*)g_pad_ctx + m + i;
                p[0x2EF4] = active_name[i];
                i += 1;
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
    g_fade_current.r = 0;
    g_fade_current.g = 0;
    g_fade_current.b = 0;
    g_fade_target.r = 0;
    g_fade_target.g = 0;
    g_fade_target.b = 0;
    g_fade_target.steps = 0;
}

/**
 * @brief Per-frame RGB fade tick: lerp the tint toward its target and emit
 *        the full-screen tint quad into the OT.
 *
 * Advances @c g_fade_current toward @c g_fade_target by one of the remaining
 * @c steps (snapping if none remain). Unless the tint is neutral, emits a
 * full-screen TILE plus a @c DR_TPAGE blend packet into @ref GNAME_OT_FRONT:
 * additive (@c FADE_TPAGE_ADD, brighten) when a channel is >=
 * @c FADE_CHAN_ADDITIVE, otherwise subtractive (@c FADE_TPAGE_SUB, darken).
 *
 * @param ctx Render context; primitives are written at @c prim_cursor (which is
 *            advanced past them) and linked into @ref GNAME_OT_FRONT.
 *
 * @note Equivalent to TITLE.BIN's render_fade_overlay.
 * @see https://decomp.me/scratch/NvocJ (100%)
 */
static void render_fade_overlay(RenderContext* ctx)
{
    void* prim = ctx->prim_cursor;
    RenderContext* p_ctx = ctx;
    s32 step_r;
    s32 step_g;
    s32 step_b;
    s32 tpage;

    /* Lerp current toward target, or snap if no steps remain. */
    if (g_fade_target.steps != 0)
    {
        step_r = (g_fade_target.r - g_fade_current.r) / g_fade_target.steps;
        step_g = (g_fade_target.g - g_fade_current.g) / g_fade_target.steps;
        step_b = (g_fade_target.b - g_fade_current.b) / g_fade_target.steps;
        g_fade_target.steps--;
        g_fade_current.r += step_r;
        g_fade_current.g += step_g;
        g_fade_current.b += step_b;
    }
    else
    {
        g_fade_current.r = g_fade_target.r;
        g_fade_current.g = g_fade_target.g;
        g_fade_current.b = g_fade_target.b;
    }

    /* Skip emit when all channels are neutral (identity tint). */
    if (!((g_fade_current.r != FADE_CHAN_NEUTRAL) || (g_fade_current.g != FADE_CHAN_NEUTRAL) || (g_fade_current.b != FADE_CHAN_NEUTRAL)))
    {
        ctx->prim_cursor = prim;
        return;
    }

    /* Write RGB into the flat-quad color bytes. */
    if (g_fade_current.r >= FADE_CHAN_ADDITIVE)
    {
        /* Additive bias: subtract 1 so FADE_CHAN_ADDITIVE maps to 0x00. */
        ((TILE*)prim)->r0 = (u8)g_fade_current.r - 1;
        ((TILE*)prim)->g0 = (u8)g_fade_current.g - 1;
        ((TILE*)prim)->b0 = (u8)g_fade_current.b - 1;
    }
    else
    {
        /* Subtractive bias: bitwise NOT so 0xFF->0x00, 0x00->0xFF.
         * FADE_CHAN_NEUTRAL (casts to 0 as u8) is clamped to 0 explicitly. */
        if (g_fade_current.r == FADE_CHAN_NEUTRAL)
        {
            ((TILE*)prim)->r0 = 0;
        }
        else
        {
            ((TILE*)prim)->r0 = ~g_fade_current.r;
        }

        if (g_fade_current.g == FADE_CHAN_NEUTRAL)
        {
            ((TILE*)prim)->g0 = 0;
        }
        else
        {
            ((TILE*)prim)->g0 = ~g_fade_current.g;
        }

        if (g_fade_current.b == FADE_CHAN_NEUTRAL)
        {
            ((TILE*)prim)->b0 = 0;
        }
        else
        {
            ((TILE*)prim)->b0 = ~g_fade_current.b;
        }
    }

    setTile(prim);
    setSemiTrans(prim, 1);
    SET_YX0((TILE*)prim, 0, 0);
    setWH((TILE*)prim, SCREEN_WIDTH, SCREEN_HEIGHT);
    addPrim(&p_ctx->ot[GNAME_OT_FRONT], prim);
    prim = (TILE*)prim + 1;

    /* Choose blend mode by direction of tint. */
    tpage = g_fade_current.r < FADE_CHAN_ADDITIVE ? FADE_TPAGE_SUB : FADE_TPAGE_ADD;

    setDrawTPage(prim, 0, 0, tpage);
    addPrim(&p_ctx->ot[GNAME_OT_FRONT], prim);
    prim = (DR_TPAGE*)prim + 1;

    ctx->prim_cursor = prim;
}

/**
 * @brief Set the RGB fade target and step count.
 *
 * The next @c steps ticks of @ref render_fade_overlay lerp the current color
 * toward (r, g, b), snapping on the final tick.
 *
 * @param r     Target red   (0..0x100 normal, >0x100 = additive).
 * @param g     Target green (0..0x100 normal, >0x100 = additive).
 * @param b     Target blue  (0..0x100 normal, >0x100 = additive).
 * @param steps Frames over which to interpolate. 0 means "snap immediately".
 *
 * @see https://decomp.me/scratch/jq3uD (100%)
 */
static void set_fade_target(s32 r, s32 g, s32 b, s32 steps)
{
    g_fade_target.r = r;
    g_fade_target.g = g;
    g_fade_target.b = b;
    g_fade_target.steps = steps;
}

/**
 * @brief Overlay boot entry: upload glyph TIM, init engine state, seed run state.
 *
 * @see https://decomp.me/scratch/pnzC1 (100%)
 */
void gname_init(void)
{
    volatile int stack_pad[2]; /* padding to force a 0x20 stack frame, ra at 0x18(sp) */
    load_name_entry_tim();
    func_800AA02C();
    g_startup_delay = 0x28;
    func_8006441C();
    reset_run_state();
    func_80063194();
}

/**
 * @brief Upload the name-entry overlay's glyph TIM to its fixed VRAM slots.
 *
 * Builds the destination-coordinate block consumed by @ref load_tim_to_vram
 * and loads @c g_name_entry_tim. The four packed s16 coordinates are:
 *   - [0],[1] = pixel-data destination, VRAM (@c SCREEN_WIDTH, 0).
 *   - [2],[3] = CLUT destination, VRAM (0, @c VRAM_CLUT_Y).
 *
 * @see https://decomp.me/scratch/EWwJI (100%)
 */
static void load_name_entry_tim(void)
{
    TimDstCoords dst_coords;
    dst_coords.pixel_x = SCREEN_WIDTH;
    dst_coords.pixel_y = 0;
    dst_coords.clut_x = 0;
    dst_coords.clut_y = VRAM_CLUT_Y;
    load_tim_to_vram(&dst_coords);
}

/**
 * @brief Upload the CLUT and pixel data of @c g_name_entry_tim to VRAM.
 *
 * ORs @c GPU_STP_BIT into every non-zero CLUT entry, then uploads the CLUT and
 * pixel blocks. The TIM's embedded destination coordinates are ignored;
 * @p dst_coords supplies them.
 *
 * @param dst_coords Pixel and CLUT VRAM destination coordinates.
 *
 * @note Uses LoadImage for VRAM upload.
 * @see https://decomp.me/scratch/P3W9C (100%)
 */
static void load_tim_to_vram(TimDstCoords* dst_coords)
{
    RECT rect;
    TimBlock* pixel_block;
    int i;

    Tim* tim = &g_name_entry_tim;
    s32 clut_len = tim->clut_block.bnum;
    u16* clut = tim->clut_data;

    rect.x = dst_coords->clut_x;
    rect.y = dst_coords->clut_y;
    rect.w = CLUT_ENTRY_COUNT;
    rect.h = 1;

    /* Mark every non-zero CLUT entry semi-transparent. */
    for (i = 0; i < CLUT_ENTRY_COUNT; i++)
    {
        if (*clut)
        {
            *clut |= GPU_STP_BIT;
        }
        clut++;
    }

    LoadImage(&rect, tim->clut_data);
    pixel_block = TIM_PIXEL_BLOCK(tim, clut_len);

    rect.x = dst_coords->pixel_x;
    rect.y = dst_coords->pixel_y;
    rect.w = pixel_block->w;
    rect.h = pixel_block->h;

    LoadImage(&rect, pixel_block + 1);

    /* Trailing rect writes are dead but required to match. */
    rect.x = dst_coords->clut_x;
    rect.y = dst_coords->clut_y + 1;
    rect.w = CLUT_ENTRY_COUNT;
    rect.h = 1;
}

/**
 * @brief Per-frame tick: render the frame, advance the frame counter, and
 *        update the overlay state machine.
 *
 * @param ctx Render context passed through to @ref gname_render.
 *
 * @see https://decomp.me/scratch/yYkTM (100%)
 */
void gname_tick(RenderContext* ctx)
{
    draw_name_cursor_row(ctx);
    gname_render(ctx);
    g_frame_counter++;
    gname_update_state();
}

/**
 * @brief Per-frame state-machine update: startup countdown, strip-width lerp,
 *        and confirm-button handling.
 *
 * Once the startup delay elapses, input is handled by @ref gname_process_input.
 * On confirm (START), a valid name plays the accept SFX and sets
 * @c g_overlay_result = @c GNAME_RESULT_CONFIRM to advance the overlay; an invalid one plays the
 * reject SFX.
 *
 * @see https://decomp.me/scratch/g5Rx3 (100%)
 */
static void gname_update_state(void)
{
    s32 steps;

    /* Startup delay countdown. */
    if (g_startup_delay == 0)
    {
        gname_process_input();
    }
    else
    {
        g_startup_delay--;
    }

    /* Lerp g_strip_width toward g_strip_width_target, snap when no steps remain. */
    steps = g_strip_width_steps;
    if (steps != 0)
    {
        g_strip_width_steps--;
        g_strip_width += (g_strip_width_target - g_strip_width) / steps;
    }
    else
    {
        g_strip_width = g_strip_width_target;
    }

    /* Confirm-button: play accept SFX on valid entry, reject SFX otherwise. */
    if (g_pad_input == PAD_BTN_START)
    {
        /* accept */
        if ((name_char_count(g_active_name) != 0) && (name_is_blank(g_active_name) == 0))
        {
            play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
            g_overlay_result = GNAME_RESULT_CONFIRM;
            return;
        }

        /* reject */
        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME); 
    }
}

/**
 * @brief Reset the overlay's run-state globals to their per-session defaults.
 *
 * Zeros the counters and indices, seeds the cursor from its frozen defaults,
 * computes the initial @c g_char_set_mode, and copies @c g_initial_name into
 * the active name buffer.
 *
 * @see https://decomp.me/scratch/FboaU (100%)
 */
static void reset_run_state(void)
{
    g_cursor_tab = GNAME_TAB_NONE;
    g_char_set_mode = handle_char_set_input(0, 0);
    g_cursor_lerp_steps = 0;
    g_scroll_pos = 0;
    g_scroll_target = 0;
    g_scroll_steps = 0;
    g_char_cursor = 0;
    g_name_clipboard[0] = 0;
    g_cursor_x = g_cursor_x_target;
    g_cursor_y = g_cursor_y_target;
    name_copy(g_active_name, g_initial_name);
    g_strip_width = 0;
    recalc_name_width();
    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
    g_append_anim_frame = 0;
    g_append_anim_timer = APPEND_ANIM_TIMER_START;
    g_char_panel = 0;
}

/**
 * @brief Process one frame of name-entry UI input and return the new char-set mode.
 *
 * State machine for the character-selection screen. @p mode names the focused
 * UI region:
 *   - GNAME_MODE_ACTION_OK..DEFAULT (0-3): action tab bar
 *   - GNAME_MODE_PANEL_BASE..+3 (4-7): character-panel selector tabs (panel N-4)
 *   - GNAME_MODE_GRID (0x10): in-grid character cursor
 *
 * Updates cursor position, panel, scroll state, and the name buffer as a side
 * effect, then returns the next mode.
 *
 * @param mode    Current char-set mode (see above).
 * @param buttons Filtered pad bitmask (e.g. @c g_pad_input & @c GNAME_BTN_NAV_MASK).
 * @return        New char-set mode after processing the input.
 * @see decomp.me (100%) https://decomp.me/scratch/jAuWs
 */
static s32 handle_char_set_input(s32 mode, s32 buttons)
{
    /* 0xFF: re-run the switch with the updated mode; 0: done */
    s32 redispatch = 0xFF;
    /*
     * Holds the constant 1, assigned only inside the case 0-3 confirm branch
     * but reused as an operand in later branches. Required to match the
     * original register allocation; do not fold back to literal 1.
     */
    int tmp;

    while (redispatch == 0xFF)
    {
        switch (mode)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if (buttons & 0x220)
            {
                g_cursor_tab = mode;
                tmp = 1;
                switch (mode)
                {
                case 0:
                    if ((name_char_count(g_active_name) != 0) && (!name_is_blank(g_active_name)))
                    {
                        play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                        g_overlay_result = GNAME_RESULT_CONFIRM;
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                    redispatch = 0;
                    continue;

                case 1:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    name_pop_last_char(g_active_name);
                    recalc_name_width();
                    /* empty statement required to match */
                    do
                    {
                    } while (0);
                    redispatch = 0;
                    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                    continue;

                case 2:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    if (g_name_source_mode == GNAME_SRC_RAND_PRIMARY)
                    {
                        g_name_clipboard[0] = 0;
                        name_copy(g_active_name, ((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) +
                                                     (*((u16*)(((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) + ((rand() % 128) * 2)))));
                    }
                    else if (g_name_source_mode == GNAME_SRC_RAND_ALT)
                    {
                        g_name_clipboard[0] = 0;
                        name_copy(g_active_name, ((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) +
                                                     (*((u16*)(((g_random_names_off - 0x10) + (*((u32*)g_random_names_off))) + (((rand() % 128) + 128) * 2)))));
                    }
                    else if (g_name_source_mode == GNAME_SRC_HISTORY)
                    {
                        g_name_clipboard[0] = 0;
                        if (g_history_name_idx >= 0x81)
                        {
                            name_copy(g_active_name, g_initial_name);
                        }
                        else
                        {
                            name_copy(g_active_name,
                                      ((g_random_names_off - 0x10) + (*((u32*)g_history_names_off))) +
                                          (*((u16*)(((g_random_names_off - 0x10) + (*((u32*)g_history_names_off))) + (g_history_name_idx * 2)))));
                            name_append(g_active_name,
                                        ((g_random_names_off - 0x10) + ((*((u32*)g_history_names_off)))) +
                                            (*((u16*)(((g_history_names_off - 0x10) + (*((u32*)g_history_names_off))) + (((rand() % 128) + 130) * 2)))));
                        }
                    }
                    else if (g_name_source_mode == tmp)
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
                    redispatch = 0;
                    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                    continue;

                case 3:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    g_name_clipboard[0] = 0;
                    name_copy(g_active_name, g_initial_name);
                    break;

                default:
                    redispatch = 0;
                    continue;
                }

                recalc_name_width();
                redispatch = 0;
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
                        mode = (mode == 0) ? (3) : (mode - tmp);
                    }
                    else if (buttons & PAD_BTN_RIGHT)
                    {
                        mode = (mode < 3) ? (mode + 1) : (0);
                    }
                }
                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = g_tab_cursor_pos[mode + 2].x - 8;
                g_cursor_y_target = g_tab_cursor_pos[mode + 2].y;
                g_cursor_lerp_steps = 5;
                redispatch = 0;
            }
            break;

        case 4:
        case 5:
        case 6:
        case 7:
            if (((buttons & 0x220) && ((g_cursor_tab = mode, g_char_panel != (mode - 4)))) != 0)
            {
                g_char_panel = g_cursor_tab - 4;
                mode = GNAME_MODE_GRID;
                buttons = 0;
                g_scroll_target = 0;
                g_scroll_pos = 0;
                g_scroll_steps = 0;
                g_char_cursor = 0;
                play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                continue;
            }
            else
            {
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
                        mode = (mode == 4) ? (6) : (mode - tmp);
                    }
                    else if (buttons & PAD_BTN_DOWN)
                    {
                        mode = (mode < 6) ? (mode + tmp) : (4);
                    }
                }
                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = g_tab_cursor_pos[mode + 2].x - 8;
                g_cursor_y_target = g_tab_cursor_pos[mode + 2].y;
                g_cursor_lerp_steps = 5;
                redispatch = 0;
            }
            break;

        default:
            if (((buttons & 0x220) && (((g_char_last_row * 10) + g_char_last_col) >= g_char_cursor)) != 0U)
            {
                if (g_char_panel < 3)
                {
                    if (name_char_count(g_active_name) < NAME_MAX_CHARS)
                    {
                        u8* argA;
                        g_append_anim_timer = APPEND_ANIM_TIMER_START;
                        argA = ((g_random_names_off - 0x10) + g_panel_tbl_off) +
                               (*((u16*)((((g_random_names_off - 0x10) + g_panel_tbl_off) + (g_panel_char_offsets[g_char_panel] * 2)) + (g_char_cursor * 2))));
                        g_append_anim_frame = 0;
                        name_append(g_active_name, argA);
                        recalc_name_width();
                        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                }
                else if (g_char_panel == 3)
                {
                    if (g_kanji_cat_entries[g_char_cursor] == 0xFF)
                    {
                        redispatch = 0;
                        continue;
                    }
                    g_kanji_cat = g_char_cursor;
                    g_char_panel = 4;
                    g_scroll_target = 0;
                    g_scroll_pos = 0;
                    g_scroll_steps = 0;
                    g_cursor_x_target = NAME_GRID_X_BASE;
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_cursor_lerp_steps = 4;
                    g_char_cursor = 0;
                    g_kanji_cat_name = ((g_random_names_off - 0x10) + g_panel_tbl_off) +
                                       (*((u16*)((((g_random_names_off - 0x10) + g_panel_tbl_off) + (g_kanji_cat_names_offset * 2)) + (g_kanji_cat * 2))));
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                }
                else if (g_char_panel == 4)
                {
                    if (name_char_count(g_active_name) < NAME_MAX_CHARS)
                    {
                        u8* argA;
                        g_append_anim_timer = APPEND_ANIM_TIMER_START;
                        argA = ((g_random_names_off - 0x10) + ((u32)g_kanji_panel_off)) +
                               (*((u16*)((((g_random_names_off - 0x10) + ((u32)g_kanji_panel_off)) +
                                          (g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]] * 2)) +
                                         (g_char_cursor * 2))));
                        g_append_anim_frame = 0;
                        name_append(g_active_name, argA);
                        recalc_name_width();
                        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                }
                redispatch = 0;
            }
            else
            {
                s32 scroll_off;

                if (buttons != 0)
                {
                    if ((buttons & PAD_BTN_UP) && ((g_char_cursor / 10) == 0))
                    {
                        mode = GNAME_MODE_ACTION_OK;
                        buttons = 0;
                        continue;
                    }
                    if ((buttons & PAD_BTN_LEFT) && ((g_char_cursor % 10) == 0))
                    {
                        mode = GNAME_MODE_PANEL_BASE;
                        buttons = 0;
                        continue;
                    }
                    if ((buttons & PAD_BTN_UP) && ((g_char_cursor / 10) != 0))
                    {
                        g_char_cursor -= 10;
                    }
                    else if ((buttons & PAD_BTN_DOWN) && ((g_char_cursor / 10) != g_char_last_row))
                    {
                        g_char_cursor += 10;
                    }
                    else if ((buttons & PAD_BTN_LEFT) && ((g_char_cursor % 10) != 0))
                    {
                        g_char_cursor -= 1;
                    }
                    else
                    {

                        if ((buttons & PAD_BTN_RIGHT) && ((g_char_cursor % 10) != 9))
                        {
                            g_char_cursor += tmp;
                        }
                        else
                        {
                            redispatch = 0;
                            continue;
                        }
                    }
                }

                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = ((g_char_cursor % 10) * NAME_GRID_CELL_SIZE) + NAME_GRID_X_BASE;
                g_cursor_y_target = ((g_char_cursor / 10) * NAME_GRID_CELL_SIZE) + NAME_GRID_Y_TOP - g_scroll_pos;

                if (g_cursor_y_target < NAME_GRID_Y_TOP)
                {
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_scroll_target = (g_char_cursor / 10) * NAME_GRID_CELL_SIZE;
                    g_scroll_steps = 4;
                }

                if (g_cursor_y_target >= 0xA9)
                {
                    g_cursor_y_target = NAME_GRID_Y_BOTTOM;
                    g_scroll_target = ((g_char_cursor / 10) * NAME_GRID_CELL_SIZE) - NAME_GRID_SCROLL_STEP;
                    g_scroll_steps = 4;
                }

                g_cursor_lerp_steps = 4;
                redispatch = 0;
            }
            break;
        }
    }

    return mode;
}

/**
 * @brief Per-frame input handler for the active name-entry phase.
 *
 * Called every frame by @ref gname_update_state once @c g_startup_delay
 * reaches zero. Dispatches pad input in priority order:
 *  - Nav/confirm (@c GNAME_BTN_NAV_MASK): delegated to
 *    @ref handle_char_set_input.
 *  - Undo (L2): move the last character from @c g_active_name back onto the
 *    clipboard @c g_name_clipboard.
 *  - Redo (R2): move the first clipboard character back into @c g_active_name.
 *  - Cancel (Circle): pop the last character, or finish with
 *    @c GNAME_RESULT_CANCEL when empty and @c g_allow_empty_cancel is set.
 *
 * In the kanji grid (@c g_char_set_mode == GNAME_MODE_GRID, @c g_char_panel ==
 * 4), L1/R1 cycle @c g_kanji_cat by 10 with wrap and reset the page. Finally,
 * advances the cursor and scroll lerp animations.
 *
 * @note The kanji-nav block re-tests @c GNAME_BTN_KANJI_NAV in a nested @c if
 *       that is always true (the outer @c if already guaranteed it). This
 *       redundant check is a codegen artifact and must be preserved.
 * @see decomp.me (100%) https://decomp.me/scratch/pCzH6
 */
static void gname_process_input(void)
{
    s32 cat_after_inc;
    s32 cat_prev_inc;
    s8 char_lo;
    s32 sfx_id;
    s32 cat_prev;
    u8(*clipboard_ptr)[];
    s32 nav_input;
    s32 cat_after_dec;
    s32 undo_char;
    s32 scroll_step;
    s32* scroll_ptr;
    s32 panel3_off;
    u16 clipboard_char_u16;
    u8* base;
    u32 idx;
    u32 offset;
    u32 byte_off;
    u16 kanji_name_tbl_off;
    s32 kanji_panel_offset;
    int sfx_vol;
    void** kanji_name_dst;
    s32 scroll_steps_v;

    g_cursor_tab = GNAME_TAB_NONE;
    nav_input = g_pad_input & GNAME_BTN_NAV_MASK;
    
    if (nav_input != 0)
    {
        g_char_set_mode = handle_char_set_input(g_char_set_mode, nav_input);
    }
    else if (g_pad_input & PAD_BTN_L2)
    {
        undo_char = name_pop_last_char(g_active_name);
        while (name_char_count(g_name_clipboard) >= 0xB)
        {
            name_pop_last_char(g_name_clipboard);
        }

        name_prepend_char(g_name_clipboard, (unsigned long)(undo_char & 0xFFFF));
        recalc_name_width();
        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
        sfx_id = GNAME_SFX_MOVE;
        sfx_vol = GNAME_SFX_VOLUME;
        play_menu_sfx(sfx_id, sfx_vol);
    }
    else if (g_pad_input & PAD_BTN_R2)
    {
        if (name_char_count(g_active_name) < NAME_MAX_CHARS)
        {
            clipboard_ptr = g_name_clipboard;
            undo_char = name_pop_first_char(clipboard_ptr);
            clipboard_char_u16 = (u16)undo_char;
            if (clipboard_char_u16 != 0)
            {
                char_lo = undo_char;
                (&char_lo)[1] = (s8)(clipboard_char_u16 >> 8);
                (&char_lo)[2] = 0;
                name_append(g_active_name, &char_lo);
                recalc_name_width();
                g_strip_width_steps = NAME_STRIP_LERP_STEPS;
            }
            sfx_id = GNAME_SFX_MOVE;
            play_menu_sfx(sfx_id, GNAME_SFX_VOLUME);
        }
        else
        {
            play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
        }
    }
    else if (g_pad_input & PAD_BTN_CIRCLE)
    {
        if (g_allow_empty_cancel != 0)
        {
            if (name_char_count(g_active_name) == 0)
            {
                g_overlay_result = GNAME_RESULT_CANCEL;
                play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
                return;
            }
        }
        play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
        name_pop_last_char(g_active_name);
        recalc_name_width();
        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
    }
    if (((g_char_set_mode == GNAME_MODE_GRID) && (g_char_panel == 4)) && (g_pad_input & GNAME_BTN_KANJI_NAV))
    {
        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
        if (g_pad_input & GNAME_BTN_KANJI_NAV)
        {
            while (g_pad_input & GNAME_BTN_KANJI_NAV)
            {
                if (g_pad_input & PAD_BTN_L1)
                {
                    cat_prev = g_kanji_cat;
                    cat_prev_inc = cat_prev;
                    cat_after_dec = cat_prev_inc - 0xA;
                    g_kanji_cat = cat_after_dec;
                    if (cat_after_dec == (-1))
                    {
                        g_kanji_cat = 0;
                    }
                    else if (cat_after_dec < 0)
                    {
                        g_kanji_cat = cat_prev + 0x29;
                    }
                }
                else
                {
                    cat_prev_inc = g_kanji_cat;
                    cat_after_inc = cat_prev_inc + 10;
                    g_kanji_cat = cat_after_inc;
                    if (cat_after_inc == 0x32)
                    {
                        g_kanji_cat = 9;
                    }
                    else if (cat_after_inc >= 0x32)
                    {
                        g_kanji_cat = cat_prev_inc - 0x29;
                    }
                }
                offset = g_kanji_cat;
                if (g_kanji_cat_entries[offset] != 0xFF)
                {
                    kanji_name_dst = &g_kanji_cat_name;
                    g_scroll_target = (long)0;
                    g_scroll_pos = 0;
                    panel3_off = g_panel_char_offsets[3];
                    sfx_vol = ~GNAME_BTN_KANJI_NAV;
                    g_scroll_steps = 0;
                    g_char_cursor = 0;
                    g_cursor_x_target = NAME_GRID_X_BASE;
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_cursor_lerp_steps = 4;
                    kanji_panel_offset = panel3_off;
                    idx = g_kanji_cat;
                    byte_off = (idx * 2) + ((kanji_panel_offset * 2) + g_panel_tbl_off);
                    base = g_panel_data_base;
                    kanji_name_tbl_off = *((u16*)(base + byte_off));
                    g_pad_input &= sfx_vol;
                    *kanji_name_dst = (void*)(g_panel_tbl_off + (kanji_name_tbl_off + ((unsigned long)base)));
                }
            }
        }
    }
    if (g_cursor_lerp_steps != 0)
    {
        g_cursor_x += ((s32)(g_cursor_x_target - g_cursor_x)) / ((s32)g_cursor_lerp_steps);
        g_cursor_y += ((s32)(g_cursor_y_target - g_cursor_y)) / ((s32)g_cursor_lerp_steps);
        g_cursor_lerp_steps -= 1;
    }
    else
    {
        g_cursor_x = g_cursor_x_target;
        g_cursor_y = g_cursor_y_target;
    }
    scroll_steps_v = g_scroll_steps;
    if (scroll_steps_v != 0)
    {
        scroll_ptr = &g_scroll_pos;
        scroll_step = ((s32)(g_scroll_target - (*scroll_ptr))) / ((s32)scroll_steps_v);
        g_scroll_steps -= 1;
        g_scroll_pos += scroll_step;
        return;
    }
    g_scroll_pos = g_scroll_target;
}

/**
 * @brief Emit the text cursor glyph as a SPRT + DR_TPAGE pair into @p ot.
 *
 * Builds a SPRT at @p prim from @c g_glyph_table[NAME_CURSOR_GLYPH_COUNT]
 * (UV, size, CLUT), then a texture-page DR_TPAGE packet right after it, and
 * chains both into @p ot.
 *
 * @param prim Write position; needs sizeof(SPRT) + sizeof(DR_TPAGE) bytes.
 * @param ot   OT slot to chain both primitives into.
 * @param x    Cursor sprite screen X.
 * @param y    Cursor sprite screen Y.
 * @return Pointer just past the written primitives.
 * @see decomp.me (100%) https://decomp.me/scratch/oXGkF
 */
static u_long* emit_cursor_glyph(u_long* prim, u_long* ot, s16 x, s16 y)
{
    u32 clut;
    SPRT* sprt = (SPRT*)prim;

    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    setXY0(sprt, x, y);

    setUV0(sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].u, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].v);
    setWH(sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].w, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].h);

    clut = g_glyph_table[NAME_CURSOR_GLYPH_COUNT].clut & GLYPH_CLUT_X_MASK;
    sprt->clut = clut | GLYPH_CLUT_PAGE_BITS;
    addPrim(ot, sprt);

    prim += PRIM_WORDS(SPRT);
    setDrawTPage(prim, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(ot, prim);

    return prim + PRIM_WORDS(DR_TPAGE);
}

/**
 * @brief Main per-frame render pass for the name-entry overlay.
 *
 * Runs each tick from @ref gname_tick (after @ref draw_name_cursor_row),
 * building this frame's primitives and chaining them into @p context's OT.
 * In order:
 *  1. Character grid glyphs (highlighting the selected @c g_cursor_tab).
 *  2. Append glyph + animation, then @ref emit_panel_tab_sprite.
 *  3. Text cursor SPRT at (@c g_cursor_x, @c g_cursor_y) + its DrawMode.
 *  4. Scroll indicator arrows, conditional on @c g_scroll_pos and the row.
 *  5. @ref emit_panel_label, @ref render_char_panel, @ref render_name_strip.
 *
 * @param context Render context; primitives are chained into its OT slots and
 *                @c prim_cursor is advanced.
 * @see decomp.me (100%) https://decomp.me/scratch/a0Oye
 */
static void gname_render(RenderContext* context)
{
    s32 tab;
    s32 scroll_pos;
    TabCursorEntry* grid_entry;
    void* prim;
    SPRT* cursor_sprt;
    DR_TPAGE* cursor_tpage;
    RenderContext* ctx;
    s32 cursor_x;
    s32 cursor_y;
    ctx = context;
    prim = context->prim_cursor;
    grid_entry = g_tab_cursor_entries;

    /* 1. Character grid: tabs 2..12, skip 9; highlight the selected tab. */
    for (tab = 2; tab < 13; tab++, grid_entry++)
    {
        if (tab != 9)
        {
            prim =
                emit_glyph_sprt(prim, &ctx->ot[GNAME_OT_CHAR_GRID], grid_entry->glyph, grid_entry->x, (s32)grid_entry->y - 8, 1, (tab - 2) == g_cursor_tab, 0);
        }
    }

    /* 2. Static glyph + append animation, then panel-tab sprite. */
    prim = emit_panel_tab_sprite(emit_draw_mode_prim(draw_char_append_anim(emit_glyph_sprt(emit_draw_mode_prim(prim, &ctx->ot[GNAME_OT_CHAR_GRID]),
                                                                                           &ctx->ot[GNAME_OT_CHAR_APPEND], (u8)3, 0xE8, 4, 0, 0, 0),
                                                                           ctx),
                                                     &ctx->ot[GNAME_OT_CHAR_APPEND]),
                                 &ctx->ot[GNAME_OT_FRONT]);

    /* 3. Text cursor SPRT at (g_cursor_x, g_cursor_y) + additive DrawTPage. */
    cursor_x = g_cursor_x;
    cursor_y = g_cursor_y;
    cursor_sprt = (SPRT*)prim;
    SET_BGR0_PACKED(cursor_sprt, GPU_TINT_NEUTRAL);
    setSprt(cursor_sprt);
    setXY0(cursor_sprt, cursor_x, cursor_y);
    setUV0(cursor_sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].u, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].v);
    setWH(cursor_sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].w, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].h);
    setClut(cursor_sprt, (g_glyph_table[NAME_CURSOR_GLYPH_COUNT].clut & GLYPH_CLUT_X_MASK) << 4, 498);
    addPrim(&ctx->ot[GNAME_OT_TEXT_CURSOR], cursor_sprt);
    cursor_tpage = (DR_TPAGE*)(cursor_sprt + 1);
    setDrawTPage(cursor_tpage, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(&ctx->ot[GNAME_OT_TEXT_CURSOR], cursor_tpage);
    prim = (DR_TPAGE*)cursor_tpage + 1;

    /* 4. Scroll indicators: top arrow whenever scrolled, bottom arrow unless
     * the current scroll page already shows the last character row. */
    if (g_scroll_pos != 0)
    {
        prim = emit_glyph_sprt(prim, &ctx->ot[GNAME_OT_FRONT], g_tab_cursor_pos[0].glyph, g_tab_cursor_pos[0].x, g_tab_cursor_pos[0].y, 0, 0, 0);
    }

    if (g_char_last_row >= 5)
    {
        /* scroll_pos / 16, with the round-toward-zero bias for negatives. */
        scroll_pos = g_scroll_pos;
        if (scroll_pos < 0)
        {
            scroll_pos += 15;
        }

        if ((scroll_pos >> 4) != (g_char_last_row - 4))
        {
            prim = emit_glyph_sprt(prim, &ctx->ot[GNAME_OT_FRONT], g_tab_cursor_pos[1].glyph, g_tab_cursor_pos[1].x, g_tab_cursor_pos[1].y, 0, 0, 0);
        }
    }

    /* 5. Panel label, character panel, and name strip sub-passes. */
    context->prim_cursor = emit_panel_label(emit_draw_mode_prim(prim, &ctx->ot[GNAME_OT_FRONT]), &ctx->ot[GNAME_OT_PANEL_LABEL]);
    render_char_panel(context, g_char_panel);
    render_name_strip(context, g_active_name, g_strip_width);
}

/**
 * @brief Emit the panel-tab indicator sprite for the current character-set mode.
 *
 * Resolves a @ref PANEL_RECORD by index and draws it via @ref func_800A88A0
 * at fixed screen position (0xB0, 0xC8). The index per mode:
 *  - mode 0-7 (kana/alpha): sprite_idx of the @ref g_tab_cursor_pos tab
 *    (mode + 2)
 *  - mode 0x10, panel 3-4:  panel + 10 (records 13-14)
 *  - mode 0x10, other:      12
 *
 * @param prim     Primitive write cursor (linked-list head).
 * @param ot_entry Pointer into the render context OT for chaining.
 * @return Updated primitive write cursor after appending the sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/RnoNS
 */
static void* emit_panel_tab_sprite(void* prim, u_long* ot_entry)
{
    s32 mode = g_char_set_mode;

    if (g_char_set_mode < 8)
    {
        prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(g_tab_cursor_pos[mode + 2].sprite_idx), 1, 0xB0, 0xC8, 2);
    }
    else if (g_char_set_mode == GNAME_MODE_GRID)
    {
        s32 panel = g_char_panel;

        if ((u32)(panel - 3) < 2U)
        {
            prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(panel + 10), 1, 0xB0, 0xC8, 2);
        }
        else
        {
            prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(12), 1, 0xB0, 0xC8, 2);
        }
    }
    return prim;
}

/**
 * @brief Emit the category-label sprite for the current character panel.
 *
 * Panels 0-3 use a @ref PANEL_RECORD label; panel >= 4 (kanji) uses
 * @ref g_kanji_cat_name directly. Drawn via @ref func_800A88A0 at fixed screen
 * position (0x23, 0x47).
 *
 * @param prim     Primitive write cursor (linked-list head).
 * @param ot_entry Pointer into the render context OT for chaining.
 * @return Updated primitive write cursor after appending the label sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/jK7bc
 */
static void* emit_panel_label(void* prim, u_long* ot_entry)
{
    s32 panel = g_char_panel;

    if (panel < 4)
    {
        prim = func_800A88A0(prim, ot_entry, PANEL_RECORD(panel), 1, 0x23, 0x47, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot_entry, g_kanji_cat_name, 1, 0x23, 0x47, 2);
    }

    return prim;
}

/**
 * @brief Append three GPU primitives to the render context's OT and reserve a
 *        right-edge VRAM strip for upload on the back page.
 *
 * Builds, in order, into @ref GNAME_OT_NAME_STRIP:
 *   1. A template packet copied from the inactive frame's reserve slot.
 *   2. A textured name sprite plus its Draw-Mode packet.
 *   3. A VRAM upload RECT (strip_width x 32) right-aligned on the current back
 *      page (Y = 0x18 or 0x100 by @c frame_parity).
 * Then advances @c ctx->prim_cursor past the last packet.
 *
 * @param ctx         Render context (OT, prim_cursor, frame_parity).
 * @param name_buf    Active name buffer; source data for the sprite.
 * @param strip_width Strip width in pixels; also sets its X as 0xF0 - strip_width.
 *
 * @see https://decomp.me/scratch/LxujJ (100%)
 */
static void render_name_strip(RenderContext* ctx, s32 name_buf, s32 strip_width)
{
    s32* ot_head;   /* &ctx->ot[GNAME_OT_NAME_STRIP] - passed as the OT head pointer */
    s32* prim;      /* current primitive being emitted */
    s32* next_prim; /* heap cursor after the sprite/draw-mode pair */
    s32 vram_y;     /* VRAM Y of the back page (0x18 or 0x100) */
    s32 vram_x;     /* VRAM X of the right-aligned strip */
    u32* pkt;
    u32 vram_load_pkt[0x19];

    ot_head = (s32*)&ctx->ot[GNAME_OT_NAME_STRIP];
    prim = ctx->prim_cursor;
    next_prim = prim;

    /* 1. Copy template packet from the *other* frame's reserve slot, then
     * splice it into the OT. */
    SetDrawEnv(prim, (void*)((s32)g_render_buf_base + ((ctx->frame_parity ^ 1) * DRAW_BUF_STRIDE) + DRAW_BUF_DRAWENV_OFF));

    addPrim(&ctx->ot[GNAME_OT_NAME_STRIP], prim);

    /* 2. Emit textured sprite (tag 0x64) wrapped by a Draw-Mode (0xE1) packet.
     * Returns the heap cursor just past both packets. */
    next_prim = emit_draw_mode_prim(emit_glyph_sprt(func_800A88A0(prim + 0x10, ot_head, name_buf, 1, 0x10, 8, 0), ot_head, 2, 0, 0, 0, 0, 0), ot_head);

    /* 3. Build a back-page VRAM upload RECT (W = strip_width, H = 32) at the
     * right edge of whichever page is currently the back buffer. */
    pkt = vram_load_pkt + 2;
    vram_x = 0xF0 - strip_width;
    vram_y = 0x18;
    if (ctx->frame_parity != 0)
    {
        vram_y = 0x100;
    }

    SetDefDrawEnv(pkt, vram_x, vram_y, strip_width, 0x20);
    SetDrawEnv(next_prim, pkt);

    addPrim(&ctx->ot[GNAME_OT_NAME_STRIP], next_prim);
    /* Advance heap cursor 0x40 bytes past the load packet. */
    next_prim += 0x10;
    ctx->prim_cursor = next_prim;
}

/**
 * @brief Render all visible character glyphs for the active panel into the OT.
 *
 * Splices a template packet into @c OT[0x0A], then walks every glyph entry in
 * the current panel (or kanji category when @c g_char_panel == 4), emitting a
 * sprite for each one whose scroll-adjusted Y position falls within the visible
 * grid window. After the loop, records the final grid position in
 * @c g_char_last_row / @c g_char_last_col, then appends a VRAM upload RECT
 * covering the full @c NAME_GRID_VIS_HEIGHT x @c 0xA0 grid area.
 *
 * Panel data sources:
 *  - @c g_char_panel == 4 (kanji picker): glyph entries come from
 *    @c g_kanji_panel_off, bounded by
 *    @c g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]] .. [..+1].
 *  - Otherwise: glyph entries come from @c g_panel_tbl_off, bounded by
 *    @c g_panel_char_offsets[panel_idx][0] .. [1].
 *
 * Visibility culling: a glyph at row @c r with @c g_scroll_pos is visible when
 * @c (u32)((r*16 - g_scroll_pos) + 11) < 91, i.e. its screen Y is in [-11, 79].
 * Glyphs are emitted at @c x = col*NAME_GRID_CELL_SIZE,
 * @c y = row*NAME_GRID_CELL_SIZE - g_scroll_pos.
 *
 * The final VRAM rect is at (@c NAME_GRID_VRAM_X, @c NAME_GRID_Y_TOP) on the
 * back buffer page (@c NAME_GRID_Y_TOP for @c frame_parity==0, @c 0x150 for
 * @c frame_parity==1), size @c 0xA0 x @c NAME_GRID_VIS_HEIGHT.
 *
 * @param ctx_ptr   Render context cast to void* for codegen; OT head at
 *                  @ref GNAME_OT_CHAR_PANEL, prim heap at @c prim_cursor,
 *                  parity at @c frame_parity.
 * @param panel_idx Active character panel index (0-3 normal, 4 kanji).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/ckF2S
 */
static void render_char_panel(RenderContext* ctx, s32 panel_idx)
{
    u8* new_var4;
    u32* ot_ptr;
    u8 _unused[8];
    u8 grid_load_pkt[0x60];
    u8* new_var;
    void* prim;
    u32* write_cur;
    u8* glyph_base;
    u16* entry_ptr;
    s32 col;
    s32 entry_idx;
    unsigned long long new_var3;
    s32 new_var5;
    s32 row;
    s32 screen_y;
    unsigned new_var2;
    s32 entry_end;
    /* (u8*)ctx + 0x28 == &ctx->ot[GNAME_OT_CHAR_PANEL]; raw offset is required to match. */
    ot_ptr = (u32*)(((u8*)ctx) + 0x28);
    prim = ctx->prim_cursor;
    SetDrawEnv(prim, (DRAWENV*)(((s32)g_render_buf_base + ((ctx->frame_parity ^ 1) * DRAW_BUF_STRIDE)) + DRAW_BUF_DRAWENV_OFF));
    ((P_TAG*)prim)->addr = (u_long)((u_long)((P_TAG*)((u32*)(((u8*)ctx) + 0x28)))->addr), ((P_TAG*)((u32*)(((u8*)ctx) + 0x28)))->addr = (u_long)prim;
    write_cur = prim + (sizeof(DR_ENV));
    if (g_char_panel == 4)
    {
        glyph_base = (u8*)((((u32)(&g_kanji_panel_off)) - 8) + g_kanji_panel_off);
        entry_idx = g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat]];
        entry_end = g_kanji_entry_offsets[g_kanji_cat_entries[g_kanji_cat] + 1];
        row = 0;
    }
    else
    {
        entry_end = g_panel_char_offsets[panel_idx];
        entry_idx = entry_end;
        entry_end = g_panel_char_offsets[panel_idx + 1];
        glyph_base = (u8*)((u16*)((((u8*)(&g_panel_tbl_off)) - 4) + g_panel_tbl_off));
        do
        {

        } while (0);
        row = 0;
    }
    col = row;
    while (1)
    {
        screen_y = (row * 16) - g_scroll_pos;
        new_var5 = entry_end;
        if (((u32)(screen_y + 0x0B)) <= (0x5B - 1))
        {
            write_cur = func_800A88A0(write_cur, ot_ptr, (void*)(glyph_base + ((u16*)glyph_base)[entry_idx]), 1, col * 16, screen_y, 0);
        }
        entry_idx++;
        if (new_var5 == entry_idx)
        {
            break;
        }
        col++;
        if (col == 10)
        {
            col = 0;
            row++;
        }
    }

    {
        u8* pkt = grid_load_pkt;
        u32 grid_vram_y;
        g_char_last_row = row;
        g_char_last_col = col;
        prim = write_cur;
        grid_vram_y = 104;
        if (ctx->frame_parity != 0)
        {
            grid_vram_y = 0x150;
        }
        SetDefDrawEnv(pkt, 0x60, grid_vram_y, 0xA0, 0x50);
        SetDrawEnv(prim, pkt);
        (((P_TAG*)prim)->addr = (u_long)((u_long)((P_TAG*)ot_ptr)->addr)), ((P_TAG*)ot_ptr)->addr = (u_long)prim;
        prim += sizeof(DR_ENV);
        ctx->prim_cursor = prim;
    }
}

/**
 * @brief Emit a Draw-Mode (GP0 0xE1) primitive and link it to the OT.
 *
 * Writes an 8-byte packet at @p prim:
 *  - byte 3 = 1 (one-word payload).
 *  - bytes 4..7 = `0xE1000005` (GP0 Draw Mode: texpage default, abr=1,
 *    dither off, drawing-to-display-area enabled).
 * Then splices the packet into the 24-bit OT whose head is at @p ot_head
 * using the standard `(top_byte | next_addr & 0xFFFFFF)` chain idiom and
 * returns the heap cursor advanced by 8 bytes.
 *
 * @param prim    Destination @ref DR_TPAGE packet (8 bytes on the primitive heap).
 * @param ot_head Pointer to the 24-bit OT head entry (@ref u_long).
 * @return Heap cursor advanced past the packet (`prim + 8`).
 *
 * @see https://decomp.me/scratch/EyVeo (100%)
 */
static void* emit_draw_mode_prim(DR_TPAGE* prim, u_long* ot_head)
{
    unsigned char* bytes = (unsigned char*)prim;
    u32* words = (u32*)prim;
    u32 temp1, temp2;

    setDrawTPage(bytes, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(ot_head, prim);

    return (void*)(bytes + 8);
}

/**
 * @brief Emit 1 or 2 glyph SPRT primitives and chain them onto an OT tag.
 *
 * Always writes a primary white-tinted (RGB=0x80) SPRT for glyph @p glyph_id
 * at @c (x - shadow_dist + primary_adj, y - shadow_dist + primary_adj).
 * When @p shadow_dist != 0, writes a second SPRT at @c (x + tmp, y + tmp)
 * with @c tmp = (shadow_dist - primary_adj) * 2. The second sprite's tint
 * and code byte depend on @p highlight:
 *   - @p highlight != 0: opaque blue (RGB=(0,0,0xA0), code 0x64).
 *   - @p highlight == 0: semi-transparent black (RGB=0, code 0x66) - drop shadow.
 *
 * Both sprites pull u/v/w/h/clut from @c g_glyph_table[glyph_id] and are
 * appended to the linked list at @p ot_tag via the standard addPrim sequence.
 *
 * @param prim_buf  Pointer to the next free byte in the primitive buffer.
 * @param ot_tag    Pointer to the OT head tag (addPrim "ot" arg).
 * @param glyph_id  Glyph index into @c g_glyph_table.
 * @param x         Base X screen coordinate.
 * @param y         Base Y screen coordinate.
 * @param shadow_dist Shadow offset distance in pixels. When 0, only the
 *                    primary SPRT is emitted.
 * @param primary_adj Added to the primary sprite's position offset. When
 *                    equal to @p shadow_dist the primary renders at (x,y)
 *                    with no shift (used for the selected/highlighted state).
 * @param highlight   Secondary-sprite mode: 0 = semi-transparent black drop
 *                    shadow; non-zero = opaque blue overlay.
 * @return Pointer to the byte after the last emitted primitive.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Au2h5
 */
static void* emit_glyph_sprt(void* prim_buf, u_long* ot_tag, s32 glyph_id, s32 x, s32 y, s32 shadow_dist, s32 primary_adj, s32 highlight)
{
    u8* ptr = (u8*)prim_buf;
    SPRT* sprt = (SPRT*)ptr;

    /* (offset) + (base) form so gcc emits `addu v1,v1,v0` (vs the reverse
       order from `&g_glyph_table[glyph_id]`). Also keeps glyph_id live for the
       second SPRT's re-derivation below. */
    u8* entry = (u8*)((glyph_id << 3) + (u32)g_glyph_table);
    u32 clut_word;
    s32 tmp2;

    /* Primary glyph SPRT - white tint, fully opaque. */
    *(u32*)&sprt->r0 = 0x808080; /* r=g=b=0x80, code byte = 0 */
    setSprt(sprt);
    setXY0(sprt, (s16)(x - shadow_dist + primary_adj), (s16)(y - shadow_dist + primary_adj));
    setUV0(sprt, entry[0], entry[1]);
    setWH(sprt, entry[2], entry[3]);
    setClut(sprt, *(u32*)(entry + 4) << 4, 498);
    addPrim(ot_tag, sprt);
    ptr += sizeof(SPRT);

    if (shadow_dist != 0)
    {
        /* Secondary SPRT - drop shadow (highlight==0) or blue overlay (highlight!=0). */
        u8* new_var2;
        u8* entry2;
        u32 clut_word2;

        *(u32*)&((SPRT*)ptr)->r0 = (highlight != 0) ? 0xA00000 : 0;

        setSprt((SPRT*)ptr);

        if (highlight == 0)
        {
            setSemiTrans((SPRT*)ptr, 1);
        }

        tmp2 = (shadow_dist - primary_adj) * 2;

        new_var2 = (u8*)g_glyph_table;

        entry2 = (u8*)((glyph_id << 3) + (u32)new_var2);

        setXY0((SPRT*)ptr, (s16)(x + tmp2), (s16)(y + tmp2));
        setUV0((SPRT*)ptr, entry2[0], entry2[1]);
        setWH((SPRT*)ptr, (s16)entry2[2], (s16)entry2[3]);
        setClut((SPRT*)ptr, *(u32*)(entry2 + 4) << 4, 498);
        addPrim(ot_tag, ptr);

        ptr += sizeof(SPRT);
    }

    return (void*)ptr;
}

/**
 * @brief Build the name-entry cursor's per-frame sprite packet:
 *        a TexWindow bracket around @ref NAME_CURSOR_GLYPH_COUNT textured
 *        glyph sprites, terminated by a DrawMode.
 *
 * Walks @c g_name_cursor_glyphs (a @ref GlyphSeqEntry array) one entry per glyph
 * cell, looks each glyph up in @c g_glyph_table, and emits a white
 * (RGB=0x80) free-size textured SPRT primitive (code 0x64). The chain is
 * wrapped with @c setTexWindow at both ends (rect @c {0,0,0xFF,0xFF} -
 * a no-op full-page window) and closed with a @c GNAME_GLYPH_TPAGE DrawMode.
 * The buffer cursor at @c obj->prim_cursor is advanced past the final primitive.
 *
 * @param ctx Render context (@ref RenderContext). Reads/writes:
 *             - @ref GNAME_OT_NAME_CURSOR (offset 0x3C) - addPrim OT entry.
 *             - @c prim_cursor at offset 0x4040 - primitive scratch-pool cursor.
 *
 * @note Called by @ref gname_tick via the implicit-@c $a0 convention
 *       (the call site passes no args; the function reads whatever the
 *       caller left in @c $a0).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Q6WL2
 */
static void draw_name_cursor_row(RenderContext* ctx)
{
    RECT tw_rect;

    s32 i;

    u8* ptr_t1;
    u8* list_ptr;
    DR_TWIN* twin;
    SPRT* sprt;
    u8* drawmode;
    u_long* ptr;
    GlyphSeqEntry* seq;

    /* Two aliases of the same context pointer: gcc allocates them to t6/t2
       and uses t6 for the very first addPrim/buf access and t2 for every
       subsequent addPrim. This split is required for the asm match. */
    RenderContext* obj = ctx;
    RenderContext* obj2;
    u8* glyph_table_base;
    obj2 = obj;

    ptr_t1 = (u8*)obj->prim_cursor;

    /* First TexWindow init: source order is h, w, y, x. */
    tw_rect.h = 0xFF;
    tw_rect.w = 0xFF;
    tw_rect.y = 0;
    tw_rect.x = 0;

    /* Opening texture window. The first addPrim runs *before* seq/i/glyph_table_base
       are assigned so gcc materializes the 0x00FFFFFF mask at the very top
       of the prologue (the mask is the first non-arg constant used). */
    twin = (DR_TWIN*)ptr_t1;
    setTexWindow(twin, &tw_rect);
    addPrim(&obj->ot[GNAME_OT_NAME_CURSOR], twin);

    seq = g_name_cursor_glyphs;
    i = 0;
    glyph_table_base = (u8*)g_glyph_table;

    ptr_t1 += sizeof(DR_TWIN);

    /* Glyph sprites. list_ptr is a separate variable aliased to ptr_t1 so
       gcc keeps both pointers live across the loop (target uses t1 + a2 in
       parallel). */
    list_ptr = ptr_t1;
    do
    {
        u32 idx = seq->id;
        u32 xy;
        u8* glyph;

        sprt = (SPRT*)list_ptr;
        /* RGB only (high byte lands in `code`, immediately overwritten). */
        *(u32*)&sprt->r0 = 0x808080;
        setlen(sprt, 4);
        setcode(sprt, 0x64);

        xy = seq->xy;
        /* (offset) + (base) order forces gcc to emit `addu v1,v1,s0` (vs.
           the reverse order `s0,v1` you'd get from `&glyph_table[idx]`). */
        glyph = (u8*)((idx << 3) + (u32)glyph_table_base);
        *(u32*)&sprt->x0 = xy;

        sprt->u0 = glyph[0];
        sprt->v0 = glyph[1];
        sprt->w = glyph[2];
        {
            /* `i++` between the load and store of `h` so gcc schedules the
               counter increment into the slot the target asm uses. */
            u8 hh = glyph[3];
            i++;
            sprt->h = hh;
        }
        {
            /* Read clut as a full word (gcc would otherwise optimize this
               to `lhu` since only the low 16 bits affect the result). The
               `seq++` advance sits between read and store to match the
               target's instruction scheduling. */
            u32 clut_word = *(u32*)(glyph + 4);
            seq++;
            sprt->clut = (u16)((clut_word & GLYPH_CLUT_X_MASK) | GLYPH_CLUT_PAGE_BITS);
        }

        addPrim(&obj2->ot[GNAME_OT_NAME_CURSOR], sprt);
        list_ptr += sizeof(SPRT);
    } while (i < NAME_CURSOR_GLYPH_COUNT);
    ptr_t1 = list_ptr;

    /* Closing texture window. Field-assignment order differs from the
       opening call (w,h,x,y vs. h,w,y,x) - the original C wrote them in
       this exact order and gcc preserves it. */
    tw_rect.w = 0xFF;
    tw_rect.h = 0xFF;
    tw_rect.x = 0;
    tw_rect.y = 0;
    twin = (DR_TWIN*)ptr_t1;
    setTexWindow(twin, &tw_rect);
    addPrim(&obj2->ot[GNAME_OT_NAME_CURSOR], twin);
    ptr_t1 += sizeof(DR_TWIN);

    /* DrawMode terminator: tpage 5, dfe=0, dtd=0. Writes only tag + 1 word.
       The buffer cursor is computed as `drawmode + 8` (not by mutating
       ptr_t1 first), which yields `addiu v0,t1,8; sw v0,0x4040(...)`. */
    drawmode = ptr_t1;
    setDrawTPage(drawmode, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(&obj2->ot[GNAME_OT_NAME_CURSOR], drawmode);

    ctx->prim_cursor = (u32*)(drawmode + 8);
}

/**
 * @brief Number of bytes in a name buffer, excluding the null terminator.
 *
 * Walks the variable-width encoding: each byte in [0x19, 0x20) consumes
 * two buffer bytes (DBCS lead + trail), every other byte consumes one.
 *
 * @param name Null-terminated name buffer.
 * @return Byte length excluding the terminator.
 * @see https://decomp.me/scratch/2QgjW (100%)
 */
static s32 name_byte_length(u8* name)
{
    s32 byte_len;
    u8 c;
    u8* p;

    p = name;
    c = *p;
    byte_len = 0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7U) /* DBCS lead byte: 2-byte glyph */
            {
                p += 2;
                byte_len += 2;
            }
            else
            {
                p += 1;
                byte_len += 1;
            }
            c = *p;
        } while (c != 0);
    }
    return byte_len;
}

/**
 * @brief Number of logical glyphs in a name buffer.
 *
 * Like @ref name_byte_length but counts each DBCS pair as one glyph.
 *
 * @param name Null-terminated name buffer.
 * @return Glyph (character) count.
 * @see https://decomp.me/scratch/c8fPe (100%)
 */
static s32 name_char_count(const u8* name)
{
    s32 char_count = 0;

    while (*name)
    {
        name += IS_DBSC_LEAD_BYTE(*name) ? 2 : 1;
        char_count++;
    }

    return char_count;
}

/**
 * @brief Append @p src to the end of @p dst (in-place concatenation).
 *
 * Computes the byte lengths of both buffers (respecting the DBCS-style
 * encoding) and copies @p src's payload after @p dst's existing payload,
 * writing a fresh null terminator. Caller is responsible for ensuring
 * @p dst has room for both.
 *
 * @param dst Null-terminated name buffer; appended to in-place.
 * @param src Null-terminated source name to append.
 * @see https://decomp.me/scratch/1lsbD (100%)
 */
static void name_append(u8* dst, const u8* src)
{
    const u8* p;
    s32 dst_len;
    s32 src_len;
    s32 offset;
    s32 i;

    p = dst;
    dst_len = 0;

    while (*p)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            dst_len += 2;
        }
        else
        {
            p += 1;
            dst_len += 1;
        }
    }

    p = src;
    src_len = 0;
    offset = dst_len;

    while (*p)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            src_len += 2;
        }
        else
        {
            p += 1;
            src_len += 1;
        }
    }

    for (i = 0; i < src_len; i++)
    {
        dst[offset + i] = src[i];
    }

    dst[offset + i] = 0;
}

/**
 * @brief Remove the last glyph from @p name and return it.
 *
 * Walks the buffer keeping a one-glyph-behind pointer; on exit @c prev_pos
 * points at the last glyph and @c scan_pos at the null. The returned
 * @c s32 packs the glyph as `lead | (trail << 8)` for a DBCS pair, or just
 * the byte value for a 1-byte glyph. The buffer is truncated by writing
 * 0 at @c prev_pos. Empty names return 0 unchanged.
 *
 * @param name Null-terminated name buffer (truncated in-place).
 * @return Removed glyph packed as `lead | (trail << 8)`, or 0 if empty.
 * @see https://decomp.me/scratch/agZ8y (100%)
 */
static s32 name_pop_last_char(u8* name)
{
    u8* prev_pos;
    u8* scan_pos;
    s32 last_char;

    prev_pos = name;
    scan_pos = prev_pos;

    while (*scan_pos)
    {
        prev_pos = scan_pos;
        if (IS_DBSC_LEAD_BYTE(*scan_pos))
        {
            scan_pos += 2;
        }
        else
        {
            scan_pos++;
        }
    }

    last_char = MAKE_DBCS_GLYPH(prev_pos[0], prev_pos[1]);

    if (prev_pos != scan_pos)
    {
        *prev_pos = 0;
    }

    return last_char;
}

/**
 * @brief Copy @p src into @p dst, including the null terminator.
 *
 * Computes the byte length of @p src walking the DBCS-style encoding, then
 * copies that many bytes and writes a terminator. Caller must ensure
 * @p dst has room.
 *
 * @param dst Destination name buffer.
 * @param src Null-terminated source name.
 * @see https://decomp.me/scratch/UeYRe (100%)
 */
static void name_copy(u8* dst, const u8* src)
{
    const u8* p;
    s32 i;
    s32 src_len;

    p = src;
    src_len = 0;

    while (*p)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            src_len += 2;
        }
        else
        {
            p += 1;
            src_len += 1;
        }
    }

    for (i = 0; i < src_len; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}

/**
 * @brief Recompute the active name's rendered pixel width and strip target.
 *
 * Measures every glyph of @c g_active_name via @c func_800644FC (which
 * fills one @ref GlyphMeasure per glyph and returns the glyph count), sums
 * the per-glyph widths into @c g_name_pixel_width, then sets
 * @c g_strip_width_target to that width plus a fixed 0x18 margin. Called
 * whenever the name buffer changes (append, delete, reset).
 *
 * @see https://decomp.me/scratch/y0CgJ (100%)
 */
static void recalc_name_width(void)
{
    GlyphMeasure glyphs[16];
    GlyphMeasure* cursor;
    GlyphMeasure* entry;
    s16 width;
    s32 glyph_count;
    s32 i;

    glyph_count = func_800644FC(glyphs, g_active_name, 0);
    i = 0;
    width = entry->width;
    g_name_pixel_width = 0;

    if (i < glyph_count)
    {
        cursor = glyphs;
        while (i < glyph_count)
        {
            entry = cursor;
            width = entry->width;
            g_name_pixel_width += width;
            cursor++;
            i++;
        }
    }

    g_strip_width_target = g_name_pixel_width + 0x18;
}

/**
 * @brief Insert a glyph at the front of @p buffer (in-place).
 *
 * @p new_char packs the glyph as `lead | (trail << 8)`; the lead byte
 * decides whether it is a 1- or 2-byte glyph. The existing buffer
 * contents (including the null terminator) are shifted right by that
 * amount and the new glyph is written at offset 0. Caller must ensure
 * @p buffer has room.
 *
 * No-op if the lead byte is 0.
 *
 * @param buffer   Null-terminated name buffer.
 * @param new_char Glyph to prepend, packed `lead | (trail << 8)`.
 * @see https://decomp.me/scratch/VOLcD (100%)
 */
static void name_prepend_char(u8* buffer, u16 new_char)
{
    u8* ptr;
    u32 len;
    u32 header_size;
    u32 move_count;
    u32 i;
    u16 h = new_char; /* copy to match register usage */

    if ((h & 0xFF) == 0)
    {
        return;
    }

    if (IS_DBSC_LEAD_BYTE(h & 0xFF))
    {
        header_size = 2;
    }
    else
    {
        header_size = 1;
    }

    ptr = buffer;
    len = 0;

    while (*ptr != 0)
    {

        if (IS_DBSC_LEAD_BYTE(*ptr))
        {
            ptr += 2;
            len += 2;
        }
        else
        {
            ptr += 1;
            len += 1;
        }
    }

    move_count = len + 1;
    for (i = move_count; i > 0; i--)
    {
        buffer[(header_size + i) - 1] = buffer[i - 1];
    }

    buffer[0] = (u8)(h & 0xFF);
    if (header_size == 2)
    {
        buffer[1] = (u8)(h >> 8);
    }
}

/**
 * @brief Remove the first glyph from @p name and return it.
 *
 * Reads one glyph at the head of the buffer (1 or 2 bytes per the
 * DBCS-style encoding), measures the rest of the buffer's byte length,
 * shifts the remaining bytes (plus null terminator) left by the glyph
 * size, and returns the removed glyph packed as `lead | (trail << 8)`.
 *
 * @param name Null-terminated name buffer (mutated in-place).
 * @return Removed glyph packed in low 16 bits, or 0 if @p name was empty.
 * @see https://decomp.me/scratch/ArXXq (100%)
 */
static s32 name_pop_first_char(u8* name)
{
    u8 first;
    u32 width;
    u16 first_char;
    u8* p;
    s32 tail_len;
    s32 move_count;
    s32 i;
    u32 mask_u16;

    first = name[0];

    if (first == 0)
    {
        return 0;
    }

    if (IS_DBSC_LEAD_BYTE(first))
    {
        first_char = MAKE_DBCS_GLYPH(name[0], name[1]);
        width = 2;
    }
    else
    {
        first_char = name[0];
        width = 1;
    }

    /* Measure tail (everything after the first glyph) in bytes. */
    tail_len = 0;
    p = name + width;

    while (*p != 0)
    {
        if (IS_DBSC_LEAD_BYTE(*p))
        {
            p += 2;
            tail_len += 2;
        }
        else
        {
            p += 1;
            tail_len += 1;
        }
    }

    move_count = tail_len + 1; /* +1 to also shift the null terminator */
    mask_u16 = 0xFFFFU;
    for (i = 0; i < move_count; i++)
    {
        name[i] = name[i + width];
    }

    return (s32)(first_char & mask_u16);
}

/**
 * @brief Draw the current frame of the character-append animation and
 *        advance its frame timer.
 *
 * When the player commits a glyph into the name being entered, the input
 * handler seeds @c g_append_anim_frame to 0 and @c g_append_anim_timer to 2
 * (see the @ref name_append call sites). This routine then runs once per
 * render tick from @ref gname_render:
 *
 *  1. Draw: emit up to @ref APPEND_ANIM_SLOT_COUNT textured-glyph SPRTs for
 *     the current frame. The frame index @c g_append_anim_frame selects a
 *     record in @c g_char_append_anim (logically an @ref AppendAnimFrame).
 *     Each glyph slot gives an (x, y, glyph) triple; a slot whose glyph id
 *     is 0 is skipped. X is biased by 0xE8, Y by 4.
 *  2. Advance: decrement @c g_append_anim_timer; when it reaches 0, step to
 *     the next frame. Frame @ref APPEND_ANIM_FRAME_COUNT wraps back to 0 and
 *     stops the animation (timer left at 0); otherwise the new frame's
 *     duration is loaded from byte 3 of its record (@c slots[0].pad).
 *
 * @param prim Primitive-buffer cursor (next free byte).
 * @param ctx  Render context; @ref GNAME_OT_CHAR_APPEND_ANIM (offset 0x30) is
 *             the OT head tag for this layer.
 * @return Primitive-buffer cursor advanced past the emitted SPRTs.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/3TQG6
 */
static void* draw_char_append_anim(void* prim, RenderContext* ctx)
{
    u8 frame = g_append_anim_frame;
    void* result = prim;
    u8* table;
    RenderContext* ot_base = ctx;
    s32 i;
    /* px points at a slot's x byte; py = px + 1 reads y at [0] and glyph at
       [1]. The two incrementing pointers are required to match. */
    u8* px = &g_char_append_anim[frame * APPEND_ANIM_FRAME_STRIDE];
    u8* py = px + 1;
    short glyph;

    for (i = 0; i < APPEND_ANIM_SLOT_COUNT; i++, py += 4, px += 4)
    {
        s32 glyph_byte = py[1];
        glyph = glyph_byte;
        if (glyph != 0)
        {
            /* (u8*)&ot->ot + 0x30 == &ot->ot[GNAME_OT_CHAR_APPEND_ANIM]; raw offset is required to match. */
            result = emit_glyph_sprt(result, (u8*)&ot_base->ot + 0x30, (u8)glyph, px[0] + 0xE8, py[0] + 4, 0, 0, 0);
        }
    }

    if (g_append_anim_timer == 0)
    {
        return result;
    }

    g_append_anim_timer--;

    if (g_append_anim_timer == 0)
    {
        g_append_anim_frame++;

        if (g_append_anim_frame == APPEND_ANIM_FRAME_COUNT)
        {
            g_append_anim_frame = 0;
            g_append_anim_timer = 0;
            return result;
        }

        table = g_char_append_anim;
        g_append_anim_timer = table[(g_append_anim_frame * APPEND_ANIM_FRAME_STRIDE) + 3];
    }

    return result;
}

/**
 * @brief Test whether a name buffer is empty or contains only blanks.
 *
 * Walks @p name byte-by-byte (note: not glyph-by-glyph). The buffer is
 * blank if every byte is either ASCII space (@ref CHAR_SPACE) or the
 * wide-space sentinel (@ref CHAR_WIDE_SPACE). An empty (immediate-null)
 * buffer also counts as blank.
 *
 * @param name Null-terminated name buffer.
 * @return 1 if blank, 0 otherwise.
 * @see https://decomp.me/scratch/rdbBA (100%)
 */
static s32 name_is_blank(u8* name)
{
    while (*name)
    {
        if (*name != CHAR_SPACE && *name != CHAR_WIDE_SPACE)
        {
            return FALSE;
        }

        name++;
    }

    return TRUE;
}