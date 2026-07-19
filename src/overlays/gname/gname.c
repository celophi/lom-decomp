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
#define GNAME_OT_FRONT 0x00            /* fade overlay, scroll indicators, panel-tab sprite, label draw-mode */
#define GNAME_OT_TEXT_CURSOR 0x08      /* text cursor glyph + DrawTPage */
#define GNAME_OT_PANEL_LABEL 0x09      /* category-label sprite */
#define GNAME_OT_CHAR_PANEL 0x0A       /* scrolling character-panel grid */
#define GNAME_OT_CHAR_GRID 0x0B        /* character grid glyphs */
#define GNAME_OT_CHAR_APPEND_ANIM 0x0C /* character-append animation glyphs */
#define GNAME_OT_CHAR_APPEND 0x0D      /* static append glyph + draw-mode */
#define GNAME_OT_NAME_STRIP 0x0E       /* name strip (entered-name display) */
#define GNAME_OT_NAME_CURSOR 0x0F      /* name-entry cursor row */
#define GNAME_OT_ENTRY_COUNT (GNAME_OT_NAME_CURSOR + 1)
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
#define CHAR_SPACE 0x20      /**< ASCII space; blank glyph in name buffers. */
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
/** Circle: delete the last character, or cancel when an empty name is allowed. */
#define GNAME_BTN_CANCEL PAD_BTN_CIRCLE
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
#define GNAME_BTN_NAV_MASK (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT | GNAME_BTN_CONFIRM)

/*
 * Character-set navigation mode values stored in g_char_set_mode and
 * passed to / returned from handle_char_set_input.
 *
 *   0-3  : action tab bar (OK, Delete, Random, Default)
 *   4-7  : character-panel selector tabs (panel N at mode 4+N)
 *   0x10 : in-grid character cursor
 */
#define GNAME_MODE_ACTION_OK 0      /* action bar: commit the name */
#define GNAME_MODE_ACTION_DELETE 1  /* action bar: delete last character */
#define GNAME_MODE_ACTION_RANDOM 2  /* action bar: fill with random name */
#define GNAME_MODE_ACTION_DEFAULT 3 /* action bar: reset to default name */
#define GNAME_MODE_PANEL_BASE 4     /* first char-panel tab; panel N is at 4+N */
#define GNAME_MODE_PANEL_NAV_LAST (GNAME_MODE_PANEL_BASE + 2)
#define GNAME_MODE_PANEL_LAST (GNAME_MODE_PANEL_BASE + 3)
#define GNAME_MODE_GRID 0x10        /* in-grid character cursor mode */

#define GNAME_REDISPATCH_PENDING 0xFF
#define GNAME_CURSOR_POS_TABLE_OFFSET 2
#define GNAME_TAB_CURSOR_X_BIAS 8
#define GNAME_CURSOR_LERP_STEPS 5
#define GNAME_GRID_LERP_STEPS 4

/* Sentinel for g_cursor_tab meaning "no tab/grid cell selected". */
#define GNAME_TAB_NONE 0xFF

/* g_overlay_result values: how the overlay finished, read by the caller. */
#define GNAME_RESULT_CANCEL 2  /* cancelled with an empty name (when allowed) */
#define GNAME_RESULT_CONFIRM 5 /* name committed; advance to the next overlay stage */

/* Frame count seeded into g_strip_width_steps to start a name-strip width lerp. */
#define NAME_STRIP_LERP_STEPS 5

/* Frame count seeded into g_append_anim_timer to start the append animation. */
#define APPEND_ANIM_TIMER_START 2

/* g_name_source_mode values: selects which name is pasted on Random/Default action. */
#define GNAME_SRC_CUSTOM 1       /* use g_custom_name_buf */
#define GNAME_SRC_HISTORY 3      /* pick from g_history_names_off via g_history_name_idx */
#define GNAME_SRC_RAND_PRIMARY 4 /* random entry from g_random_names_off primary index table */
#define GNAME_SRC_RAND_ALT 5     /* random entry from g_random_names_off alternate offset table */

/* Run-loop display and history-copy constants. */
#define GNAME_FADE_IN_FRAMES 20
#define GNAME_STARTUP_DELAY_FRAMES 40
#define GNAME_HISTORY_LAYOUT_MASK 0x7F
#define GNAME_HISTORY_LAYOUT_LARGE 4
#define GNAME_HISTORY_COPY_SIZE 0x15
#define GNAME_LARGE_HISTORY_STRIDE sizeof(((PadContext*)0)->large_history_names[0])
#define GNAME_SMALL_HISTORY_STRIDE sizeof(((PadContext*)0)->small_history_names[0])
#define GNAME_LARGE_HISTORY_OFFSET 0x2B0C
#define GNAME_SMALL_HISTORY_OFFSET 0x2EF4
#define GNAME_USES_LARGE_HISTORY(ctx) (((ctx)->unkAA8 & GNAME_HISTORY_LAYOUT_MASK) == GNAME_HISTORY_LAYOUT_LARGE)

/* Maximum number of logical characters allowed in a name (distinct from
 * NAME_GRID_CHARS_PER_ROW, which is the grid display width). */
#define NAME_MAX_CHARS 10

/* Sound effect IDs passed as the first argument to play_menu_sfx. */
#define GNAME_SFX_ERROR 0x78   /* error: name is full, blank, or action is invalid */
#define GNAME_SFX_MOVE 0x7D    /* cursor movement / navigation */
#define GNAME_SFX_CONFIRM 0x7E /* confirm / OK action */
#define GNAME_SFX_CANCEL 0x7F  /* cancel / back action */
#define GNAME_SFX_VOLUME 0x80  /* default volume argument for play_menu_sfx */

/* Character selection grid layout constants. */
#define NAME_GRID_CHARS_PER_ROW 10 /**< Characters per row in the grid. */
#define NAME_GRID_LAST_COL (NAME_GRID_CHARS_PER_ROW - 1)
#define NAME_GRID_CELL_SIZE 16     /**< Pixel width and height of each grid cell (0x10). */
#define NAME_GRID_X_BASE 84        /**< Pixel X of the leftmost grid column (0x54). */
#define NAME_GRID_Y_TOP 104        /**< Pixel Y of the top of the visible grid area (0x68). */
#define NAME_GRID_Y_BOTTOM 168     /**< Pixel Y of the bottom clamp (0xA8). */
#define NAME_GRID_Y_EXIT_BOUND (NAME_GRID_Y_BOTTOM + 1)
#define NAME_GRID_SCROLL_STEP 64   /**< Scroll delta per step: 4 rows * 16 px/row (0x40). */
#define NAME_GRID_CELL_SHIFT 4     /**< Shift equivalent of division by the 16-pixel cell size. */
#define NAME_GRID_DIV_BIAS (NAME_GRID_CELL_SIZE - 1)
#define NAME_GRID_VISIBLE_ROWS (NAME_GRID_VIS_HEIGHT / NAME_GRID_CELL_SIZE)
#define NAME_GRID_VRAM_X 0x60      /**< VRAM X of the grid area upload rect (96 px). */
#define NAME_GRID_VRAM_W 0xA0      /**< Width of the grid area upload rect (160 px). */
#define NAME_GRID_VIS_HEIGHT 0x50  /**< Visible grid height in pixels: 5 rows * 16 (80 px). */
#define NAME_GRID_VRAM_PAGE0_Y NAME_GRID_Y_TOP
#define NAME_GRID_VRAM_PAGE1_Y 0x150
#define NAME_GRID_OVERSCAN 0x0B    /**< Rows partly above the window are still drawn down to y = -11. */
#define CHAR_PANEL_STANDARD_COUNT 3 /**< Number of ordinary character panels. */
#define CHAR_PANEL_KANJI_CATEGORY 3 /**< g_char_panel value selecting the kanji category list. */
#define CHAR_PANEL_KANJI 4          /**< g_char_panel value selecting the kanji picker. */
#define KANJI_CATEGORY_EMPTY 0xFF
#define CHAR_PANEL_GLYPH_COLOR 1
#define CHAR_PANEL_GLYPH_MODE 0

#define RANDOM_NAME_COUNT 128
#define HISTORY_NAME_INDEX_LIMIT 0x81
#define HISTORY_SUFFIX_INDEX_BASE 130
#define NAME_CLIPBOARD_MAX_CHARS 11

#define KANJI_CATEGORY_STEP 10
#define KANJI_CATEGORY_COUNT 50
#define KANJI_CATEGORY_WRAP_OFFSET 41
#define KANJI_CATEGORY_NEXT_EDGE 9

/**
 * @brief True when a glyph drawn at screen Y @p y falls inside the scrolling
 *        grid window, i.e. y is in [-NAME_GRID_OVERSCAN, NAME_GRID_VIS_HEIGHT-1].
 *
 * The single unsigned compare (rather than two signed ones) is what the
 * original emits, so the biased form is required to match.
 */
#define NAME_GRID_ROW_VISIBLE(y) (((u32)((y) + NAME_GRID_OVERSCAN)) <= (NAME_GRID_VIS_HEIGHT + NAME_GRID_OVERSCAN - 1))

/* FadeState channel sentinels. Channels run 0 (fully dark) up to
 * FADE_CHAN_NEUTRAL (identity, no tint). Values >= FADE_CHAN_ADDITIVE
 * select additive blend; values below select subtractive. */
#define FADE_CHAN_NEUTRAL 0x100
#define FADE_CHAN_ADDITIVE 0x101

/* tpage arguments for the blend-mode DR_TPAGE emitted by render_fade_overlay.
 * The tile is flat-colored so only the abr bits matter; x=320 is the
 * right-half VRAM column used as the tpage base. */
#define FADE_TPAGE_ADD 0x25 /* getTPage(0, 1, 320, 0) - abr=1: Back + Front */
#define FADE_TPAGE_SUB 0x45 /* getTPage(0, 2, 320, 0) - abr=2: Back - Front */

/* tpage for the overlay's 4-bit glyph/font texture (cursor, text, DrawMode
 * packets). getTPage(0, 0, 320, 0): 4-bit CLUT, abr=0, VRAM page at x=320. */
#define GNAME_GLYPH_TPAGE 5

/** Number of glyph cells in the name-entry cursor row drawn by
 *  @ref draw_name_cursor_row. */
#define NAME_CURSOR_GLYPH_COUNT 20
#define GNAME_FULL_TEX_WINDOW_SIZE 0xFF

/* Entries rendered from g_tab_cursor_entries by gname_render. */
#define GNAME_RENDER_TAB_FIRST 2
#define GNAME_RENDER_TAB_END_EXCLUSIVE 13
#define GNAME_RENDER_TAB_HIDDEN 9
#define GNAME_RENDER_TAB_Y_BIAS 8
#define GNAME_SCROLL_UP_ENTRY 0
#define GNAME_SCROLL_DOWN_ENTRY 1

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

/* Entered-name strip placement and back-page upload dimensions. */
#define NAME_STRIP_TEXT_COLOR 1
#define NAME_STRIP_TEXT_X 0x10
#define NAME_STRIP_TEXT_Y 8
#define NAME_STRIP_TEXT_MODE 0
#define NAME_STRIP_DECOR_GLYPH 2
#define NAME_STRIP_VRAM_RIGHT 0xF0
#define NAME_STRIP_VRAM_PAGE0_Y 0x18
#define NAME_STRIP_VRAM_PAGE1_Y 0x100
#define NAME_STRIP_VRAM_HEIGHT 0x20

/** Mask for the CLUT X-column index stored in @c GlyphInfo::clut.
 *  Bits [5:0] hold CLUT_X/16; upper bits carry unrelated data and must be
 *  discarded before writing the CLUT id into a sprite primitive. */
#define GLYPH_CLUT_X_MASK 0x3F

/** CLUT-page bit pattern OR'd over the low 6 bits of @c GlyphInfo::clut
 *  before writing it into a sprite primitive (see @ref draw_name_cursor_row,
 *  @ref emit_glyph_sprt). Encodes the fixed VRAM Y row (498) shared by all
 *  name-entry palettes; bits [5:0] are zero and supplied by @c GLYPH_CLUT_X_MASK. */
#define GLYPH_CLUT_PAGE_BITS 0x7C80
#define GLYPH_HIGHLIGHT_TINT GPU_COLOR_WORD(0, 0, 0xA0)

/** Number of frames in @c g_char_append_anim; reaching this index wraps the
 *  animation back to the idle frame and stops it. */
#define APPEND_ANIM_FRAME_COUNT 7

#define APPEND_ANIM_X_BIAS 0xE8
#define APPEND_ANIM_Y_BIAS 4

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

/** Same blob, reached via the kanji header field at blob + 8. Kept separate
 *  from @ref PANEL_DATA_BLOB so the lui/addiu pair is shared with the field
 *  load, exactly as in @ref PANEL_DATA_BLOB. */
#define KANJI_DATA_BLOB (((u32)(&g_kanji_panel_off)) - 8)

/** The kanji picker's self-relative glyph table (blob + the kanji offset). */
#define KANJI_GLYPH_TBL ((u8*)(KANJI_DATA_BLOB + g_kanji_panel_off))

/** Entry i of a self-relative u16 offset table at @p tbl, as a byte pointer.
 *  Generalizes @ref PANEL_RECORD over a table chosen at runtime. */
#define TBL_ENTRY(tbl, i) ((u8*)(tbl) + ((u16*)(tbl))[(i)])

/*
 * Name and glyph records are selected through self-relative u16 offset
 * tables. These macros deliberately preserve the original symbol anchors and
 * addition order; simplifying them to the generic blob helpers changes the
 * generated address setup under GCC 2.7.2.
 */
#define RANDOM_NAME_TABLE_BASE ((g_random_names_off - 0x10) + (*((u32*)g_random_names_off)))
#define RANDOM_NAME(index) (RANDOM_NAME_TABLE_BASE + (*((u16*)(RANDOM_NAME_TABLE_BASE + ((index) * 2)))))

#define HISTORY_NAME_TABLE_BASE ((g_random_names_off - 0x10) + (*((u32*)g_history_names_off)))
#define HISTORY_NAME(index) (HISTORY_NAME_TABLE_BASE + (*((u16*)(HISTORY_NAME_TABLE_BASE + ((index) * 2)))))
#define HISTORY_SUFFIX_TABLE_BASE ((g_history_names_off - 0x10) + (*((u32*)g_history_names_off)))
#define HISTORY_SUFFIX(index) (HISTORY_NAME_TABLE_BASE + (*((u16*)(HISTORY_SUFFIX_TABLE_BASE + ((index) * 2)))))

#define PANEL_CHAR_TABLE_BASE ((g_random_names_off - 0x10) + g_panel_tbl_off)
#define PANEL_CHAR(panel, cursor) \
    (PANEL_CHAR_TABLE_BASE + \
     (*((u16*)((PANEL_CHAR_TABLE_BASE + (g_panel_char_offsets[(panel)] * 2)) + ((cursor) * 2)))))
#define KANJI_CATEGORY_NAME(category) \
    (PANEL_CHAR_TABLE_BASE + \
     (*((u16*)((PANEL_CHAR_TABLE_BASE + (g_kanji_cat_names_offset * 2)) + ((category) * 2)))))

#define KANJI_CHAR_TABLE_BASE ((g_random_names_off - 0x10) + ((u32)g_kanji_panel_off))
#define KANJI_CHAR(category, cursor) \
    (KANJI_CHAR_TABLE_BASE + \
     (*((u16*)((KANJI_CHAR_TABLE_BASE + (g_kanji_entry_offsets[g_kanji_cat_entries[(category)]] * 2)) + ((cursor) * 2)))))

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

/** Packet view used while emitting the fade TILE followed by its draw mode. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} FadePrimitive;

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define NEXT_FADE_PACKET(packet, type) ((FadePrimitive*)((u8*)(packet) + sizeof(type)))

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
typedef struct
{
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

/** Stack scratch used to build a DRAWENV before packing it into a DR_ENV. */
typedef struct
{
    u32 reserved[2]; /* DRAWENV scratch begins at byte offset 8. */
    DRAWENV draw_env;
} DrawEnvScratch;

/** DRAWENV scratch with one trailing word to preserve render_char_panel's frame. */
typedef union
{
    DRAWENV draw_env;
    u8 bytes[sizeof(DRAWENV) + sizeof(u32)];
} GridDrawEnvScratch;

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
extern AppendAnimFrame g_char_append_anim[];
extern Tim g_name_entry_tim;    /* glyph TIM blob; Tim covers the fixed header + CLUT, pixel block follows */
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
static void gname_render(RenderContext* render_ctx);
static void* emit_panel_tab_sprite(void* prim_cursor, u_long* ot_entry);
static void* emit_panel_label(void* prim_cursor, u_long* ot_entry);
static void render_name_strip(RenderContext* ctx, u8* name_buf, s32 strip_width);
static void render_char_panel(RenderContext* ctx, s32 panel_idx);
static void* emit_draw_mode_prim(DR_TPAGE* prim, u_long* ot_head);
static void* emit_glyph_sprt(void* prim_cursor, u_long* ot_entry, s32 glyph_id, s32 x, s32 y, s32 shadow_dist, s32 primary_adj, s32 highlight);
static void draw_name_cursor_row(RenderContext* ctx);
static s32 name_byte_length(const u8* name);
static s32 name_char_count(const u8* name);
static void name_append(u8* dst, const u8* src);
static s32 name_pop_last_char(u8* name);
static void name_copy(u8* dst, const u8* src);
static void recalc_name_width(void);
static void name_prepend_char(u8* buffer, u16 new_char);
static s32 name_pop_first_char(u8* name);
static void* draw_char_append_anim(void* prim_cursor, RenderContext* ctx);
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
 * @brief Run the name-entry UI until the user confirms or cancels.
 *
 * On exit, when entering a history name (@c source_mode == 3) targeting the pad
 * context's history buffer, the entered name is copied back into the
 * appropriate large or compact per-slot history table.
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
    RenderContext* render_bufs;

    /* Install the caller's buffers and seed the persistent name-entry state. */
    g_render_buf_base = buf_base;
    bcopy(initial_name, g_initial_name, sizeof(g_initial_name));
    bcopy(custom_name, g_custom_name_buf, sizeof(g_custom_name_buf));
    g_allow_empty_cancel = allow_empty_cancel;
    g_active_name = active_name;
    g_name_source_mode = source_mode;
    g_history_name_idx = history_idx;

    /* Configure the two vertically stacked VRAM display/draw buffers. */
    g_render_buf_base[0].clear_rect.x = 0;
    g_render_buf_base[0].clear_rect.y = 8;
    g_render_buf_base[0].clear_rect.w = SCREEN_WIDTH;
    g_render_buf_base[0].clear_rect.h = VRAM_DRAW_HEIGHT;
    g_render_buf_base[1].clear_rect.x = 0;
    g_render_buf_base[1].clear_rect.y = SCREEN_HEIGHT;
    g_render_buf_base[1].clear_rect.w = SCREEN_WIDTH;
    g_render_buf_base[1].clear_rect.h = VRAM_DRAW_HEIGHT;

    VSync(0);
    DrawSync(0);
    SetDefDispEnv(&g_render_buf_base[0].disp_env, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDispEnv(&g_render_buf_base[1].disp_env, 0, VRAM_BACK_DISP_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&g_render_buf_base[0].draw_env, 0, SCREEN_HEIGHT, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);
    SetDefDrawEnv(&g_render_buf_base[1].draw_env, 0, VRAM_BACK_DRAW_Y, SCREEN_WIDTH, VRAM_DRAW_HEIGHT);

    render_bufs = g_render_buf_base;
    render_bufs[1].draw_env.dtd = 0;
    render_bufs[0].draw_env.dtd = 0;
    g_overlay_result = 0;
    g_render_buf_base[0].frame_parity = 0;
    g_render_buf_base[1].frame_parity = 1;

    reset_fade_state();
    set_fade_target(FADE_CHAN_NEUTRAL, FADE_CHAN_NEUTRAL, FADE_CHAN_NEUTRAL, GNAME_FADE_IN_FRAMES);
    gname_init();

    /* Clear both ordering tables before enabling display output. */
    next_buf = g_render_buf_base;
    ClearOTagR(next_buf->ot, GNAME_OT_ENTRY_COUNT);
    ClearOTagR(g_render_buf_base[1].ot, GNAME_OT_ENTRY_COUNT);
    VSync(0);
    PutDispEnv(&next_buf->disp_env);
    func_800157DC();
    SetDispMask(1);
    func_800AA02C();

    /* Render and submit frames until the overlay reports a final result. */
    while (1)
    {
        draw_buf = next_buf;
        ClearOTagR(draw_buf->ot, GNAME_OT_ENTRY_COUNT);
        draw_buf->prim_cursor = &draw_buf->ot[GNAME_OT_ENTRY_COUNT];
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

        /* Select the other half of the double buffer for the next frame. */
        other_buf = g_render_buf_base;

        if (draw_buf == g_render_buf_base)
        {
            other_buf = &g_render_buf_base[1];
        }

        next_buf = other_buf;
        PutDispEnv(&other_buf->disp_env);
        PutDrawEnv(&next_buf->draw_env);
        DrawOTag(&draw_buf->ot[GNAME_OT_NAME_CURSOR]);
        draw_buf = other_buf;
        func_800157DC();
        cdrom_process_state();
    }

    DrawSync(0);
    VSync(0);
    func_800AA02C();

    /* Persist edits only when this run targeted the pad context's history name. */
    if ((source_mode == GNAME_SRC_HISTORY) && (active_name == g_pad_ctx->gname_name))
    {
        i = 0;
        if (GNAME_USES_LARGE_HISTORY(g_pad_ctx))
        {
            while (i < GNAME_HISTORY_COPY_SIZE)
            {
                s32 history_offset = g_pad_ctx->large_history_index * GNAME_LARGE_HISTORY_STRIDE;

                /* Keeping the member offset on the store preserves GCC's original register allocation. */
                u8* history_byte = (u8*)g_pad_ctx + history_offset + i;
                history_byte[GNAME_LARGE_HISTORY_OFFSET] = active_name[i];
                i += 1;
            }
        }
        else
        {
            while (i < GNAME_HISTORY_COPY_SIZE)
            {
                s32 history_offset = g_pad_ctx->small_history_index * GNAME_SMALL_HISTORY_STRIDE;

                /* See the large-history loop above: direct array indexing rotates v0/v1/a0. */
                u8* history_byte = (u8*)g_pad_ctx + history_offset + i;
                history_byte[GNAME_SMALL_HISTORY_OFFSET] = active_name[i];
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
 * @brief Update the fade and emit its full-screen overlay packets.
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
    FadePrimitive* packet = ctx->prim_cursor;
    RenderContext* ot_ctx = ctx;
    s32 step_r;
    s32 step_g;
    s32 step_b;
    s32 blend_tpage;

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
    if ((g_fade_current.r == FADE_CHAN_NEUTRAL) &&
        (g_fade_current.g == FADE_CHAN_NEUTRAL) &&
        (g_fade_current.b == FADE_CHAN_NEUTRAL))
    {
        ctx->prim_cursor = packet;
        return;
    }

    /* Write RGB into the flat-quad color bytes. */
    if (g_fade_current.r >= FADE_CHAN_ADDITIVE)
    {
        /* Additive bias: subtract 1 so FADE_CHAN_ADDITIVE maps to 0x00. */
        packet->tile.r0 = (u8)g_fade_current.r - 1;
        packet->tile.g0 = (u8)g_fade_current.g - 1;
        packet->tile.b0 = (u8)g_fade_current.b - 1;
    }
    else
    {
        /* Subtractive bias: bitwise NOT so 0xFF->0x00, 0x00->0xFF.
         * FADE_CHAN_NEUTRAL (casts to 0 as u8) is clamped to 0 explicitly. */
        if (g_fade_current.r == FADE_CHAN_NEUTRAL)
        {
            packet->tile.r0 = 0;
        }
        else
        {
            packet->tile.r0 = ~g_fade_current.r;
        }

        if (g_fade_current.g == FADE_CHAN_NEUTRAL)
        {
            packet->tile.g0 = 0;
        }
        else
        {
            packet->tile.g0 = ~g_fade_current.g;
        }

        if (g_fade_current.b == FADE_CHAN_NEUTRAL)
        {
            packet->tile.b0 = 0;
        }
        else
        {
            packet->tile.b0 = ~g_fade_current.b;
        }
    }

    setTile(&packet->tile);
    setSemiTrans(&packet->tile, 1);
    SET_YX0(&packet->tile, 0, 0);
    setWH(&packet->tile, SCREEN_WIDTH, SCREEN_HEIGHT);
    addPrim(&ot_ctx->ot[GNAME_OT_FRONT], &packet->tile);
    packet = NEXT_FADE_PACKET(packet, TILE);

    /* Choose blend mode by direction of tint. */
    blend_tpage = g_fade_current.r < FADE_CHAN_ADDITIVE ? FADE_TPAGE_SUB : FADE_TPAGE_ADD;

    setDrawTPage(&packet->draw_mode, 0, 0, blend_tpage);
    addPrim(&ot_ctx->ot[GNAME_OT_FRONT], &packet->draw_mode);
    packet = NEXT_FADE_PACKET(packet, DR_TPAGE);

    ctx->prim_cursor = packet;
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
    volatile int stack_pad[2];

    load_name_entry_tim();
    func_800AA02C();
    g_startup_delay = GNAME_STARTUP_DELAY_FRAMES;
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
    s32 repeat_dispatch = GNAME_REDISPATCH_PENDING;
    /*
     * Holds the constant 1, assigned only inside the case 0-3 confirm branch
     * but reused as an operand in later branches. Required to match the
     * original register allocation; do not fold back to literal 1.
     */
    int one;

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
                g_cursor_tab = mode;
                one = 1;
                switch (mode)
                {
                case GNAME_MODE_ACTION_OK:
                    if ((name_char_count(g_active_name) != 0) && (!name_is_blank(g_active_name)))
                    {
                        play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                        g_overlay_result = GNAME_RESULT_CONFIRM;
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                    repeat_dispatch = 0;
                    continue;

                case GNAME_MODE_ACTION_DELETE:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    name_pop_last_char(g_active_name);
                    recalc_name_width();
                    /* empty statement required to match */
                    do
                    {
                    } while (0);
                    repeat_dispatch = 0;
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
                    else if (g_name_source_mode == one)
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
                    repeat_dispatch = 0;
                    g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                    continue;

                case GNAME_MODE_ACTION_DEFAULT:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    g_name_clipboard[0] = 0;
                    name_copy(g_active_name, g_initial_name);
                    break;

                default:
                    repeat_dispatch = 0;
                    continue;
                }

                recalc_name_width();
                repeat_dispatch = 0;
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
                        mode = (mode == GNAME_MODE_ACTION_OK) ? GNAME_MODE_ACTION_DEFAULT : (mode - one);
                    }
                    else if (buttons & PAD_BTN_RIGHT)
                    {
                        mode = (mode < GNAME_MODE_ACTION_DEFAULT) ? (mode + 1) : GNAME_MODE_ACTION_OK;
                    }
                }
                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].x - GNAME_TAB_CURSOR_X_BIAS;
                g_cursor_y_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].y;
                g_cursor_lerp_steps = GNAME_CURSOR_LERP_STEPS;
                repeat_dispatch = 0;
            }
            break;

        /* Left-column character-panel selector tabs. */
        case GNAME_MODE_PANEL_BASE:
        case GNAME_MODE_PANEL_BASE + 1:
        case GNAME_MODE_PANEL_NAV_LAST:
        case GNAME_MODE_PANEL_LAST:
            /* Confirm changes panels only when the selected tab is not already active. */
            if (((buttons & GNAME_BTN_CONFIRM) && ((g_cursor_tab = mode, g_char_panel != (mode - GNAME_MODE_PANEL_BASE)))) != 0)
            {
                g_char_panel = g_cursor_tab - GNAME_MODE_PANEL_BASE;
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
                        mode = (mode == GNAME_MODE_PANEL_BASE) ? GNAME_MODE_PANEL_NAV_LAST : (mode - one);
                    }
                    else if (buttons & PAD_BTN_DOWN)
                    {
                        mode = (mode < GNAME_MODE_PANEL_NAV_LAST) ? (mode + one) : GNAME_MODE_PANEL_BASE;
                    }
                }
                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].x - GNAME_TAB_CURSOR_X_BIAS;
                g_cursor_y_target = g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].y;
                g_cursor_lerp_steps = GNAME_CURSOR_LERP_STEPS;
                repeat_dispatch = 0;
            }
            break;

        default:
            /* The only remaining valid mode is the character grid. */
            if (((buttons & GNAME_BTN_CONFIRM) && (((g_char_last_row * NAME_GRID_CHARS_PER_ROW) + g_char_last_col) >= g_char_cursor)) != 0U)
            {
                if (g_char_panel < CHAR_PANEL_STANDARD_COUNT)
                {
                    if (name_char_count(g_active_name) < NAME_MAX_CHARS)
                    {
                        u8* selected_char;
                        g_append_anim_timer = APPEND_ANIM_TIMER_START;
                        selected_char = PANEL_CHAR(g_char_panel, g_char_cursor);
                        g_append_anim_frame = 0;
                        name_append(g_active_name, selected_char);
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
                        repeat_dispatch = 0;
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
                    g_kanji_cat_name = KANJI_CATEGORY_NAME(g_kanji_cat);
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                }
                else if (g_char_panel == CHAR_PANEL_KANJI)
                {
                    if (name_char_count(g_active_name) < NAME_MAX_CHARS)
                    {
                        u8* selected_char;
                        g_append_anim_timer = APPEND_ANIM_TIMER_START;
                        selected_char = KANJI_CHAR(g_kanji_cat, g_char_cursor);
                        g_append_anim_frame = 0;
                        name_append(g_active_name, selected_char);
                        recalc_name_width();
                        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
                        play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                    }
                    else
                    {
                        play_menu_sfx(GNAME_SFX_ERROR, GNAME_SFX_VOLUME);
                    }
                }
                repeat_dispatch = 0;
            }
            else
            {
                s32 scroll_off;

                if (buttons != 0)
                {
                    if ((buttons & PAD_BTN_UP) && ((g_char_cursor / NAME_GRID_CHARS_PER_ROW) == 0))
                    {
                        mode = GNAME_MODE_ACTION_OK;
                        buttons = 0;
                        continue;
                    }
                    if ((buttons & PAD_BTN_LEFT) && ((g_char_cursor % NAME_GRID_CHARS_PER_ROW) == 0))
                    {
                        mode = GNAME_MODE_PANEL_BASE;
                        buttons = 0;
                        continue;
                    }
                    if ((buttons & PAD_BTN_UP) && ((g_char_cursor / NAME_GRID_CHARS_PER_ROW) != 0))
                    {
                        g_char_cursor -= NAME_GRID_CHARS_PER_ROW;
                    }
                    else if ((buttons & PAD_BTN_DOWN) && ((g_char_cursor / NAME_GRID_CHARS_PER_ROW) != g_char_last_row))
                    {
                        g_char_cursor += NAME_GRID_CHARS_PER_ROW;
                    }
                    else if ((buttons & PAD_BTN_LEFT) && ((g_char_cursor % NAME_GRID_CHARS_PER_ROW) != 0))
                    {
                        g_char_cursor -= 1;
                    }
                    else
                    {

                        if ((buttons & PAD_BTN_RIGHT) && ((g_char_cursor % NAME_GRID_CHARS_PER_ROW) != NAME_GRID_LAST_COL))
                        {
                            g_char_cursor += one;
                        }
                        else
                        {
                            repeat_dispatch = 0;
                            continue;
                        }
                    }
                }

                play_menu_sfx(GNAME_SFX_MOVE, GNAME_SFX_VOLUME);
                g_cursor_x_target = ((g_char_cursor % NAME_GRID_CHARS_PER_ROW) * NAME_GRID_CELL_SIZE) + NAME_GRID_X_BASE;
                g_cursor_y_target = ((g_char_cursor / NAME_GRID_CHARS_PER_ROW) * NAME_GRID_CELL_SIZE) + NAME_GRID_Y_TOP - g_scroll_pos;

                if (g_cursor_y_target < NAME_GRID_Y_TOP)
                {
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_scroll_target = (g_char_cursor / NAME_GRID_CHARS_PER_ROW) * NAME_GRID_CELL_SIZE;
                    g_scroll_steps = GNAME_GRID_LERP_STEPS;
                }

                if (g_cursor_y_target >= NAME_GRID_Y_EXIT_BOUND)
                {
                    g_cursor_y_target = NAME_GRID_Y_BOTTOM;
                    g_scroll_target = ((g_char_cursor / NAME_GRID_CHARS_PER_ROW) * NAME_GRID_CELL_SIZE) - NAME_GRID_SCROLL_STEP;
                    g_scroll_steps = GNAME_GRID_LERP_STEPS;
                }

                g_cursor_lerp_steps = GNAME_GRID_LERP_STEPS;
                repeat_dispatch = 0;
            }
            break;
        }
    }

    return mode;
}

/**
 * @brief Handle one frame of name-entry input.
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
 * @c CHAR_PANEL_KANJI), L1/R1 cycle @c g_kanji_cat by one category page with
 * wrap and reset the page. Finally, advances the cursor and scroll lerps.
 *
 * @note The kanji-nav block re-tests @c GNAME_BTN_KANJI_NAV in a nested @c if
 *       that is always true (the outer @c if already guaranteed it). This
 *       redundant check is a codegen artifact and must be preserved.
 * @see decomp.me (100%) https://decomp.me/scratch/pCzH6
 */
static void gname_process_input(void)
{
    s32 next_category;
    s32 category_before_step;
    u8 clipboard_glyph[3];
    s32 sfx_id;
    s32 previous_category;
    u8* clipboard_ptr;
    s32 nav_input;
    s32 previous_page_category;
    s32 moved_char;
    s32 scroll_step;
    s32* scroll_pos_ptr;
    s32 category_table_base_index;
    u16 clipboard_char_u16;
    u8* panel_data_base;
    u32 category_index;
    u32 category_entry;
    u32 table_byte_offset;
    u16* category_name_entry;
    u16 category_name_offset;
    s32 category_table_base_index_copy;
    int volume_or_input_mask;
    void** category_name_dst;
    s32 remaining_scroll_steps;

    g_cursor_tab = GNAME_TAB_NONE;
    nav_input = g_pad_input & GNAME_BTN_NAV_MASK;

    if (nav_input != 0)
    {
        g_char_set_mode = handle_char_set_input(g_char_set_mode, nav_input);
    }
    /* Undo: move the last name glyph to the front of the clipboard. */
    else if (g_pad_input & GNAME_BTN_UNDO)
    {
        moved_char = name_pop_last_char(g_active_name);
        while (name_char_count(g_name_clipboard) >= NAME_CLIPBOARD_MAX_CHARS)
        {
            name_pop_last_char(g_name_clipboard);
        }

        name_prepend_char(g_name_clipboard, moved_char);
        recalc_name_width();
        g_strip_width_steps = NAME_STRIP_LERP_STEPS;
        sfx_id = GNAME_SFX_MOVE;
        volume_or_input_mask = GNAME_SFX_VOLUME;
        play_menu_sfx(sfx_id, volume_or_input_mask);
    }
    /* Redo: move the first clipboard glyph back into the active name. */
    else if (g_pad_input & GNAME_BTN_REDO)
    {
        if (name_char_count(g_active_name) < NAME_MAX_CHARS)
        {
            clipboard_ptr = g_name_clipboard;
            moved_char = name_pop_first_char(clipboard_ptr);
            clipboard_char_u16 = moved_char;
            if (clipboard_char_u16 != 0)
            {
                clipboard_glyph[0] = moved_char;
                clipboard_glyph[1] = clipboard_char_u16 >> 8;
                clipboard_glyph[2] = 0;
                name_append(g_active_name, clipboard_glyph);
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
    else if (g_pad_input & GNAME_BTN_CANCEL)
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
    /* Cycle kanji categories, skipping entries that have no category data. */
    if (((g_char_set_mode == GNAME_MODE_GRID) && (g_char_panel == CHAR_PANEL_KANJI)) && (g_pad_input & GNAME_BTN_KANJI_NAV))
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
                    if (previous_page_category == -1)
                    {
                        g_kanji_cat = 0;
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
                if (g_kanji_cat_entries[category_entry] != KANJI_CATEGORY_EMPTY)
                {
                    category_name_dst = &g_kanji_cat_name;
                    g_scroll_target = 0;
                    g_scroll_pos = 0;
                    category_table_base_index = g_panel_char_offsets[CHAR_PANEL_KANJI_CATEGORY];
                    volume_or_input_mask = ~GNAME_BTN_KANJI_NAV;
                    g_scroll_steps = 0;
                    g_char_cursor = 0;
                    g_cursor_x_target = NAME_GRID_X_BASE;
                    g_cursor_y_target = NAME_GRID_Y_TOP;
                    g_cursor_lerp_steps = GNAME_GRID_LERP_STEPS;
                    category_table_base_index_copy = category_table_base_index;
                    category_index = g_kanji_cat;
                    table_byte_offset = (category_index * sizeof(u16)) +
                                        ((category_table_base_index_copy * sizeof(u16)) + g_panel_tbl_off);
                    panel_data_base = g_panel_data_base;
                    /* Panel offsets are serialized u16 entries inside the raw byte blob. */
                    category_name_entry = (u16*)(panel_data_base + table_byte_offset);
                    category_name_offset = *category_name_entry;
                    g_pad_input &= volume_or_input_mask;
                    /* Integer address arithmetic preserves the target's final addu operand order. */
                    *category_name_dst = (void*)(g_panel_tbl_off + (category_name_offset + ((unsigned long)panel_data_base)));
                }
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
        scroll_pos_ptr = &g_scroll_pos;
        scroll_step = (g_scroll_target - *scroll_pos_ptr) / remaining_scroll_steps;
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
 * @param render_ctx Render context; primitives are chained into its OT slots
 *                   and @c prim_cursor is advanced.
 * @see decomp.me (100%) https://decomp.me/scratch/a0Oye
 */
static void gname_render(RenderContext* render_ctx)
{
    s32 tab_index;
    s32 scroll_offset;
    const TabCursorEntry* grid_entry;
    void* prim;
    SPRT* cursor_sprt;
    DR_TPAGE* cursor_tpage;
    RenderContext* ot_ctx;
    s32 cursor_x;
    s32 cursor_y;
    ot_ctx = render_ctx;
    prim = render_ctx->prim_cursor;
    grid_entry = g_tab_cursor_entries;

    /* 1. Character grid: emit the visible tab entries and highlight the selection. */
    for (tab_index = GNAME_RENDER_TAB_FIRST; tab_index < GNAME_RENDER_TAB_END_EXCLUSIVE; tab_index++, grid_entry++)
    {
        if (tab_index != GNAME_RENDER_TAB_HIDDEN)
        {
            prim =
                emit_glyph_sprt(prim, &ot_ctx->ot[GNAME_OT_CHAR_GRID], grid_entry->glyph, grid_entry->x, grid_entry->y - GNAME_RENDER_TAB_Y_BIAS, 1,
                                (tab_index - GNAME_RENDER_TAB_FIRST) == g_cursor_tab, 0);
        }
    }

    /* 2. Static glyph + append animation, then panel-tab sprite. */
    prim = emit_draw_mode_prim(prim, &ot_ctx->ot[GNAME_OT_CHAR_GRID]);
    prim = emit_glyph_sprt(prim, &ot_ctx->ot[GNAME_OT_CHAR_APPEND], GNAME_APPEND_GLYPH, GNAME_APPEND_X, GNAME_APPEND_Y, 0, 0, 0);
    prim = draw_char_append_anim(prim, ot_ctx);
    prim = emit_draw_mode_prim(prim, &ot_ctx->ot[GNAME_OT_CHAR_APPEND]);
    prim = emit_panel_tab_sprite(prim, &ot_ctx->ot[GNAME_OT_FRONT]);

    /* 3. Text cursor SPRT at (g_cursor_x, g_cursor_y) plus its glyph DrawTPage. */
    cursor_x = g_cursor_x;
    cursor_y = g_cursor_y;
    cursor_sprt = (SPRT*)prim;
    SET_BGR0_PACKED(cursor_sprt, GPU_TINT_NEUTRAL);
    setSprt(cursor_sprt);
    setXY0(cursor_sprt, cursor_x, cursor_y);
    setUV0(cursor_sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].u, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].v);
    setWH(cursor_sprt, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].w, g_glyph_table[NAME_CURSOR_GLYPH_COUNT].h);
    setClut(cursor_sprt, (g_glyph_table[NAME_CURSOR_GLYPH_COUNT].clut & GLYPH_CLUT_X_MASK) << 4, VRAM_CLUT_Y);
    addPrim(&ot_ctx->ot[GNAME_OT_TEXT_CURSOR], cursor_sprt);
    cursor_tpage = (DR_TPAGE*)(cursor_sprt + 1);
    setDrawTPage(cursor_tpage, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(&ot_ctx->ot[GNAME_OT_TEXT_CURSOR], cursor_tpage);
    prim = cursor_tpage + 1;

    /* 4. Scroll indicators: top arrow whenever scrolled, bottom arrow unless
     * the current scroll page already shows the last character row. */
    if (g_scroll_pos != 0)
    {
        prim = emit_glyph_sprt(prim, &ot_ctx->ot[GNAME_OT_FRONT], g_tab_cursor_pos[GNAME_SCROLL_UP_ENTRY].glyph,
                               g_tab_cursor_pos[GNAME_SCROLL_UP_ENTRY].x, g_tab_cursor_pos[GNAME_SCROLL_UP_ENTRY].y, 0, 0, 0);
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
            prim = emit_glyph_sprt(prim, &ot_ctx->ot[GNAME_OT_FRONT], g_tab_cursor_pos[GNAME_SCROLL_DOWN_ENTRY].glyph,
                                   g_tab_cursor_pos[GNAME_SCROLL_DOWN_ENTRY].x, g_tab_cursor_pos[GNAME_SCROLL_DOWN_ENTRY].y, 0, 0, 0);
        }
    }

    /* 5. Panel label, character panel, and name strip sub-passes. */
    prim = emit_draw_mode_prim(prim, &ot_ctx->ot[GNAME_OT_FRONT]);
    render_ctx->prim_cursor = emit_panel_label(prim, &ot_ctx->ot[GNAME_OT_PANEL_LABEL]);
    render_char_panel(render_ctx, g_char_panel);
    render_name_strip(render_ctx, g_active_name, g_strip_width);
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
 * @param prim_cursor Primitive write cursor (linked-list head).
 * @param ot_entry Pointer into the render context OT for chaining.
 * @return Updated primitive write cursor after appending the sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/RnoNS
 */
static void* emit_panel_tab_sprite(void* prim_cursor, u_long* ot_entry)
{
    s32 mode = g_char_set_mode;

    if (mode <= GNAME_MODE_PANEL_LAST)
    {
        /* Action and panel modes select the record stored in their tab entry. */
        prim_cursor = func_800A88A0(prim_cursor, ot_entry, PANEL_RECORD(g_tab_cursor_pos[mode + GNAME_CURSOR_POS_TABLE_OFFSET].sprite_idx),
                                   GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
    }
    else if (mode == GNAME_MODE_GRID)
    {
        s32 panel = g_char_panel;

        /* Unsigned range check accepts only the category and kanji panels. */
        if ((u32)(panel - CHAR_PANEL_KANJI_CATEGORY) < (CHAR_PANEL_KANJI - CHAR_PANEL_KANJI_CATEGORY + 1))
        {
            prim_cursor = func_800A88A0(prim_cursor, ot_entry, PANEL_RECORD(panel + GNAME_PANEL_TAB_KANJI_RECORD_OFFSET),
                                       GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
        }
        else
        {
            prim_cursor = func_800A88A0(prim_cursor, ot_entry, PANEL_RECORD(GNAME_PANEL_TAB_DEFAULT_RECORD), GNAME_PANEL_SPRITE_COLOR,
                                       GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
        }
    }
    return prim_cursor;
}

/**
 * @brief Emit the category-label sprite for the current character panel.
 *
 * Panels 0-3 use a @ref PANEL_RECORD label; panel >= 4 (kanji) uses
 * @ref g_kanji_cat_name directly. Drawn via @ref func_800A88A0 at fixed screen
 * position (0x23, 0x47).
 *
 * @param prim_cursor Primitive write cursor (linked-list head).
 * @param ot_entry Pointer into the render context OT for chaining.
 * @return Updated primitive write cursor after appending the label sprite.
 * @see decomp.me (100%) https://decomp.me/scratch/jK7bc
 */
static void* emit_panel_label(void* prim_cursor, u_long* ot_entry)
{
    s32 panel = g_char_panel;

    if (panel < CHAR_PANEL_KANJI)
    {
        prim_cursor = func_800A88A0(prim_cursor, ot_entry, PANEL_RECORD(panel), GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_LABEL_X,
                                   GNAME_PANEL_LABEL_Y, GNAME_PANEL_SPRITE_MODE);
    }
    else
    {
        prim_cursor = func_800A88A0(prim_cursor, ot_entry, g_kanji_cat_name, GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_LABEL_X,
                                   GNAME_PANEL_LABEL_Y, GNAME_PANEL_SPRITE_MODE);
    }

    return prim_cursor;
}

/**
 * @brief Render the entered-name strip and queue its VRAM upload.
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
static void render_name_strip(RenderContext* ctx, u8* name_buf, s32 strip_width)
{
    u_long* ot_head;
    DR_ENV* template_packet;
    DR_ENV* packet_cursor;
    s32 vram_y;     /* VRAM Y of the back page (0x18 or 0x100) */
    s32 vram_x;     /* VRAM X of the right-aligned strip */
    DRAWENV* upload_env;
    DrawEnvScratch vram_load_scratch;

    ot_head = &ctx->ot[GNAME_OT_NAME_STRIP];
    template_packet = ctx->prim_cursor;
    /* The initial alias is required to preserve the target's register allocation. */
    packet_cursor = template_packet;

    /* 1. Copy template packet from the *other* frame's reserve slot, then
     * splice it into the OT. */
    SetDrawEnv(template_packet, &g_render_buf_base[ctx->frame_parity ^ 1].draw_env);

    addPrim(&ctx->ot[GNAME_OT_NAME_STRIP], template_packet);

    /* 2. Emit textured sprite (tag 0x64) wrapped by a Draw-Mode (0xE1) packet.
     * Returns the heap cursor just past both packets. */
    packet_cursor = func_800A88A0(template_packet + 1, ot_head, name_buf, NAME_STRIP_TEXT_COLOR, NAME_STRIP_TEXT_X, NAME_STRIP_TEXT_Y,
                                  NAME_STRIP_TEXT_MODE);
    packet_cursor = emit_glyph_sprt(packet_cursor, ot_head, NAME_STRIP_DECOR_GLYPH, 0, 0, 0, 0, 0);
    packet_cursor = emit_draw_mode_prim(packet_cursor, ot_head);

    /* 3. Build a back-page VRAM upload RECT (W = strip_width, H = 32) at the
     * right edge of whichever page is currently the back buffer. */
    upload_env = &vram_load_scratch.draw_env;
    vram_x = NAME_STRIP_VRAM_RIGHT - strip_width;
    vram_y = NAME_STRIP_VRAM_PAGE0_Y;
    if (ctx->frame_parity != 0)
    {
        vram_y = NAME_STRIP_VRAM_PAGE1_Y;
    }

    SetDefDrawEnv(upload_env, vram_x, vram_y, strip_width, NAME_STRIP_VRAM_HEIGHT);
    SetDrawEnv(packet_cursor, upload_env);

    addPrim(&ctx->ot[GNAME_OT_NAME_STRIP], packet_cursor);
    packet_cursor += 1;
    ctx->prim_cursor = packet_cursor;
}

/**
 * @brief Render the visible character panel and queue its VRAM upload.
 *
 * @param ctx       Render context (OT, primitive heap, frame parity).
 * @param panel_idx Active character panel index (0-3 normal; 4 selects the
 *                  kanji picker, whose entries come from the kanji tables
 *                  instead and ignore this index).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/ckF2S
 */
static void render_char_panel(RenderContext* ctx, s32 panel_idx)
{
    u_long* ot_entry;
    u8 frame_pad[8]; /* Without this pad the stack frame is 8 bytes too small. */
    GridDrawEnvScratch grid_load_scratch;
    DR_ENV* packet_cursor;
    void* glyph_cursor; /* Opaque cursor advanced over variable-sized glyph packets. */
    u8* glyph_base;
    s32 column;
    s32 glyph_index;
    s32 glyph_limit;
    s32 row;
    s32 screen_y;
    s32 glyph_end;
    u32 kanji_category_entry;
    DRAWENV* upload_env;
    s32 upload_y;
    ot_entry = &ctx->ot[GNAME_OT_CHAR_PANEL];
    packet_cursor = ctx->prim_cursor;
    SetDrawEnv(packet_cursor, &g_render_buf_base[ctx->frame_parity ^ 1].draw_env);
    addPrim(ot_entry, packet_cursor);
    glyph_cursor = packet_cursor + 1;
    /* Select the glyph range from either the kanji or standard panel tables. */
    if (g_char_panel == CHAR_PANEL_KANJI)
    {
        glyph_base = KANJI_GLYPH_TBL;
        kanji_category_entry = g_kanji_cat_entries[g_kanji_cat];
        glyph_index = g_kanji_entry_offsets[kanji_category_entry];
        glyph_end = g_kanji_entry_offsets[kanji_category_entry + 1];
        row = 0;
    }
    else
    {
        glyph_index = g_panel_char_offsets[panel_idx];
        glyph_end = g_panel_char_offsets[panel_idx + 1];
        glyph_base = (u8*)PANEL_REC_TBL;
        /* Empty statement: required to match (it splits the basic block here). */
        do
        {
        } while (0);
        row = 0;
    }
    /* Copying row (always 0 here) rather than assigning 0 is required to match. */
    column = row;
    /* Walk the range row-major, emitting only glyphs inside the visible window. */
    while (1)
    {
        screen_y = (row * NAME_GRID_CELL_SIZE) - g_scroll_pos;
        /* Reloading the limit into its own local each pass is required to match. */
        glyph_limit = glyph_end;
        if (NAME_GRID_ROW_VISIBLE(screen_y))
        {
            glyph_cursor = func_800A88A0(glyph_cursor, ot_entry, TBL_ENTRY(glyph_base, glyph_index), CHAR_PANEL_GLYPH_COLOR,
                                        column * NAME_GRID_CELL_SIZE, screen_y, CHAR_PANEL_GLYPH_MODE);
        }
        glyph_index++;
        if (glyph_limit == glyph_index)
        {
            break;
        }
        column++;
        if (column == NAME_GRID_CHARS_PER_ROW)
        {
            column = 0;
            row++;
        }
    }

    /* Queue the completed grid area for upload to this frame's VRAM page. */
    upload_env = &grid_load_scratch.draw_env;
    g_char_last_row = row;
    g_char_last_col = column;
    packet_cursor = glyph_cursor;
    upload_y = NAME_GRID_VRAM_PAGE0_Y;
    if (ctx->frame_parity != 0)
    {
        upload_y = NAME_GRID_VRAM_PAGE1_Y;
    }
    SetDefDrawEnv(upload_env, NAME_GRID_VRAM_X, upload_y, NAME_GRID_VRAM_W, NAME_GRID_VIS_HEIGHT);
    SetDrawEnv(packet_cursor, upload_env);
    addPrim(ot_entry, packet_cursor);
    packet_cursor += 1;
    ctx->prim_cursor = packet_cursor;
}

/**
 * @brief Emit a Draw-Mode (GP0 0xE1) primitive and link it to the OT.
 *
 * Writes an 8-byte packet at @p prim:
 *  - byte 3 = 1 (one-word payload).
 *  - bytes 4..7 = `0xE1000005` (GP0 Draw Mode: glyph texture page,
 *    abr=0, dither off, drawing-to-display-area disabled).
 * Then splices the packet into the 24-bit OT whose head is at @p ot_head
 * using the standard `(top_byte | next_addr & 0xFFFFFF)` chain idiom and
 * returns the heap cursor advanced by 8 bytes.
 *
 * @param prim    Destination @ref DR_TPAGE packet (8 bytes on the primitive heap).
 * @param ot_head Pointer to the 24-bit OT head entry (@ref u_long).
 * @return Heap cursor advanced past the packet (`prim + 1`, eight bytes).
 *
 * @see https://decomp.me/scratch/EyVeo (100%)
 */
static void* emit_draw_mode_prim(DR_TPAGE* prim, u_long* ot_head)
{
    setDrawTPage(prim, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(ot_head, prim);

    return prim + 1;
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
 * @param prim_cursor Pointer to the next free byte in the primitive buffer.
 * @param ot_entry    Pointer to the OT head tag (addPrim "ot" arg).
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
static void* emit_glyph_sprt(void* prim_cursor, u_long* ot_entry, s32 glyph_id, s32 x, s32 y, s32 shadow_dist, s32 primary_adj, s32 highlight)
{
    u8* ptr = prim_cursor;
    SPRT* primary_sprt = prim_cursor;

    /* (offset) + (base) form so gcc emits `addu v1,v1,v0` (vs the reverse
       order from `&g_glyph_table[glyph_id]`). Also keeps glyph_id live for the
       second SPRT's re-derivation below. */
    GlyphInfo* glyph_info = (GlyphInfo*)((glyph_id << 3) + (u32)g_glyph_table);
    s32 secondary_offset;

    /* Primary glyph SPRT - white tint, fully opaque. */
    SET_BGR0_PACKED(primary_sprt, GPU_TINT_NEUTRAL);
    setSprt(primary_sprt);
    setXY0(primary_sprt, x - shadow_dist + primary_adj, y - shadow_dist + primary_adj);
    setUV0(primary_sprt, glyph_info->u, glyph_info->v);
    setWH(primary_sprt, glyph_info->w, glyph_info->h);
    setClut(primary_sprt, glyph_info->clut << 4, VRAM_CLUT_Y);
    addPrim(ot_entry, primary_sprt);
    ptr += sizeof(SPRT);

    if (shadow_dist != 0)
    {
        /* Secondary SPRT - drop shadow (highlight==0) or blue overlay (highlight!=0). */
        GlyphInfo* glyph_table_base;
        GlyphInfo* secondary_glyph_info;

        /* A persistent SPRT alias here changes register allocation; cast at each write. */
        SET_BGR0_PACKED((SPRT*)ptr, (highlight != 0) ? GLYPH_HIGHLIGHT_TINT : 0);

        setSprt((SPRT*)ptr);

        if (highlight == 0)
        {
            setSemiTrans((SPRT*)ptr, 1);
        }

        secondary_offset = (shadow_dist - primary_adj) * 2;

        /* Force a fresh table-base materialization for the secondary lookup. */
        glyph_table_base = g_glyph_table;
        secondary_glyph_info = (GlyphInfo*)((glyph_id << 3) + (u32)glyph_table_base);

        setXY0((SPRT*)ptr, x + secondary_offset, y + secondary_offset);
        setUV0((SPRT*)ptr, secondary_glyph_info->u, secondary_glyph_info->v);
        setWH((SPRT*)ptr, secondary_glyph_info->w, secondary_glyph_info->h);
        setClut((SPRT*)ptr, secondary_glyph_info->clut << 4, VRAM_CLUT_Y);
        addPrim(ot_entry, ptr);

        ptr += sizeof(SPRT);
    }

    return ptr;
}

/**
 * @brief Render the name-cursor row and its texture state.
 *
 * Walks @c g_name_cursor_glyphs (a @ref GlyphSeqEntry array) one entry per glyph
 * cell, looks each glyph up in @c g_glyph_table, and emits a white
 * (RGB=0x80) free-size textured SPRT primitive (code 0x64). The chain is
 * wrapped with @c setTexWindow at both ends (rect @c {0,0,0xFF,0xFF} -
 * a no-op full-page window) and closed with a @c GNAME_GLYPH_TPAGE DrawMode.
 * The render context's primitive cursor is advanced past the final packet.
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

    s32 glyph_count;

    u8* packet_cursor;
    u8* glyph_cursor;
    DR_TWIN* tex_window;
    SPRT* glyph_sprt;
    DR_TPAGE* draw_mode;
    const GlyphSeqEntry* sequence;

    /* Two aliases of the same context pointer: gcc allocates them to t6/t2
       and uses t6 for the very first addPrim/buf access and t2 for every
       subsequent addPrim. This split is required for the asm match. */
    RenderContext* first_ctx = ctx;
    RenderContext* ot_ctx;
    GlyphInfo* glyph_table_base;
    ot_ctx = first_ctx;

    packet_cursor = first_ctx->prim_cursor;

    /* First TexWindow init: source order is h, w, y, x. */
    tw_rect.h = GNAME_FULL_TEX_WINDOW_SIZE;
    tw_rect.w = GNAME_FULL_TEX_WINDOW_SIZE;
    tw_rect.y = 0;
    tw_rect.x = 0;

    /* The first addPrim precedes the sequence/count/table assignments so gcc
       materializes the OT address mask at the top of the prologue. */
    tex_window = (DR_TWIN*)packet_cursor;
    setTexWindow(tex_window, &tw_rect);
    addPrim(&first_ctx->ot[GNAME_OT_NAME_CURSOR], tex_window);

    sequence = g_name_cursor_glyphs;
    glyph_count = 0;
    glyph_table_base = g_glyph_table;

    packet_cursor += sizeof(DR_TWIN);

    /* glyph_cursor is a separate alias of packet_cursor so
       gcc keeps both pointers live across the loop (target uses t1 + a2 in
       parallel). */
    glyph_cursor = packet_cursor;
    do
    {
        u32 glyph_id = sequence->id;
        u32 packed_xy;
        GlyphInfo* glyph;

        glyph_sprt = (SPRT*)glyph_cursor;
        /* RGB only (high byte lands in `code`, immediately overwritten). */
        SET_BGR0_PACKED(glyph_sprt, GPU_TINT_NEUTRAL);
        setSprt(glyph_sprt);

        packed_xy = sequence->xy;
        /* Offset + base order preserves the target's addu operand order. */
        glyph = (GlyphInfo*)((glyph_id << 3) + (u32)glyph_table_base);
        /* Coordinates are pre-packed and must be written with one word store. */
        *(u32*)&glyph_sprt->x0 = packed_xy;

        glyph_sprt->u0 = glyph->u;
        glyph_sprt->v0 = glyph->v;
        glyph_sprt->w = glyph->w;
        {
            /* Increment between the load and store of h to preserve scheduling. */
            u8 glyph_height = glyph->h;
            glyph_count++;
            glyph_sprt->h = glyph_height;
        }
        {
            /* Read clut as a full word (gcc would otherwise optimize this
               to `lhu` since only the low 16 bits affect the result). The
               sequence advance sits between read and store to match the
               target's instruction scheduling. */
            u32 clut_word = glyph->clut;
            sequence++;
            glyph_sprt->clut = (clut_word & GLYPH_CLUT_X_MASK) | GLYPH_CLUT_PAGE_BITS;
        }

        addPrim(&ot_ctx->ot[GNAME_OT_NAME_CURSOR], glyph_sprt);
        glyph_cursor += sizeof(SPRT);
    } while (glyph_count < NAME_CURSOR_GLYPH_COUNT);
    packet_cursor = glyph_cursor;

    /* Closing texture window. Field-assignment order differs from the
       opening call (w,h,x,y vs. h,w,y,x) - the original C wrote them in
       this exact order and gcc preserves it. */
    tw_rect.w = GNAME_FULL_TEX_WINDOW_SIZE;
    tw_rect.h = GNAME_FULL_TEX_WINDOW_SIZE;
    tw_rect.x = 0;
    tw_rect.y = 0;
    tex_window = (DR_TWIN*)packet_cursor;
    setTexWindow(tex_window, &tw_rect);
    addPrim(&ot_ctx->ot[GNAME_OT_NAME_CURSOR], tex_window);
    packet_cursor += sizeof(DR_TWIN);

    /* DrawMode terminator: tpage 5, dfe=0, dtd=0. Writes only tag + 1 word.
       Advance from the typed packet so the final cursor is exactly 8 bytes later. */
    draw_mode = (DR_TPAGE*)packet_cursor;
    setDrawTPage(draw_mode, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(&ot_ctx->ot[GNAME_OT_NAME_CURSOR], draw_mode);

    ctx->prim_cursor = draw_mode + 1;
}

/**
 * @brief Get the encoded byte length of a name.
 *
 * @param name Null-terminated name buffer.
 * @return Byte length excluding the terminator.
 * @see https://decomp.me/scratch/2QgjW (100%)
 */
static s32 name_byte_length(const u8* name)
{
    s32 byte_len;
    u8 c;
    const u8* cursor;

    cursor = name;
    c = *cursor;
    byte_len = 0;
    if (c != 0)
    {
        do
        {
            if (IS_DBSC_LEAD_BYTE(c))
            {
                cursor += 2;
                byte_len += 2;
            }
            else
            {
                cursor += 1;
                byte_len += 1;
            }
            c = *cursor;
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
 * @brief Render and advance the character-append animation.
 *
 * Emits every populated glyph slot in the current frame. When the frame timer
 * expires, advances to the next frame or returns to the idle frame.
 *
 * @param prim_cursor Primitive-buffer cursor (next free byte).
 * @param ctx  Render context; @ref GNAME_OT_CHAR_APPEND_ANIM (offset 0x30) is
 *             the OT head tag for this layer.
 * @return Primitive-buffer cursor advanced past the emitted SPRTs.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/3TQG6
 */
static void* draw_char_append_anim(void* prim_cursor, RenderContext* ctx)
{
    u8 frame = g_append_anim_frame;
    void* packet_cursor = prim_cursor;
    const AppendAnimFrame* anim_frames;
    RenderContext* ot_ctx = ctx;
    s32 slot_index;
    /* Separate X and Y cursors preserve the target's parallel pointer walk. */
    const AppendAnimSlot* slot = g_char_append_anim[frame].slots;
    const u8* slot_y = &slot->y;
    s16 glyph_id;

    for (slot_index = 0; slot_index < APPEND_ANIM_SLOT_COUNT; slot_index++, slot_y += sizeof(AppendAnimSlot), slot++)
    {
        s32 raw_glyph_id = slot_y[1];
        /* This s32 -> s16 -> u8 path preserves a separate argument-register move. */
        glyph_id = raw_glyph_id;
        if (glyph_id != 0)
        {
            packet_cursor = emit_glyph_sprt(packet_cursor, &ot_ctx->ot[GNAME_OT_CHAR_APPEND_ANIM], (u8)glyph_id, slot->x + APPEND_ANIM_X_BIAS,
                                            slot_y[0] + APPEND_ANIM_Y_BIAS, 0, 0, 0);
        }
    }

    if (g_append_anim_timer == 0)
    {
        return packet_cursor;
    }

    g_append_anim_timer--;

    if (g_append_anim_timer == 0)
    {
        g_append_anim_frame++;

        if (g_append_anim_frame == APPEND_ANIM_FRAME_COUNT)
        {
            g_append_anim_frame = 0;
            g_append_anim_timer = 0;
            return packet_cursor;
        }

        anim_frames = g_char_append_anim;
        g_append_anim_timer = anim_frames[g_append_anim_frame].slots[0].pad;
    }

    return packet_cursor;
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
