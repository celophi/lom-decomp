#include "gname.h"
#include "gname_types.h"

#include "cdrom.h"
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
#define GNAME_OT_CHAR_GRID 0x0B        /* action/panel selection glyphs */
#define GNAME_OT_GLYPH_APPEND_ANIM 0x0C /* glyph-append animation */
#define GNAME_OT_GLYPH_APPEND 0x0D      /* static append glyph + draw-mode */
#define GNAME_OT_NAME_STRIP 0x0E       /* name strip (entered-name display) */
#define GNAME_OT_LAYOUT_BACKGROUND 0x0F /* fixed background/layout sprite batch */
#define GNAME_OT_ENTRY_COUNT (GNAME_OT_LAYOUT_BACKGROUND + 1)
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
 *    (0x19..0x1F); their higher-level grouping is not established here.
 *  - 0x00 terminates the string.
 *
 * The `name_*` helpers in gname.c walk the buffer respecting this encoding:
 * `name_byte_length` returns raw bytes, `name_glyph_count` returns logical
 * glyphs, `name_pop_first_glyph` / `name_pop_last_glyph` strip and return one
 * glyph (packing a 2-byte glyph as `lead | (trail << 8)`), and
 * `name_prepend_glyph` inserts one glyph at the front.
 *
 * `name_is_blank` is a special case: it walks byte-by-byte (not
 * glyph-by-glyph) and treats both ASCII space (0x20) and byte 0x80 as blank.
 */
#define NAME_BYTE_SPACE 0x20     /**< ASCII space; blank glyph in name buffers. */
#define NAME_BYTE_ALT_BLANK 0x80 /**< Alternate byte value treated as blank. */

/* True if byte is a custom 2-byte DBCS-style lead byte */
#define IS_DBCS_LEAD_BYTE(c) ((c) >= 0x19 && (c) <= 0x1F)

/* Pack two bytes into a single 16-bit DBCS-style glyph */
#define MAKE_DBCS_GLYPH(lo, hi) (u16)(((u16)(hi) << 8) | (u16)(lo))
#define LOW_BYTE(value) ((value) & 0xFFU)
#define HIGH_BYTE(value) ((value) >> 8)
#define NAME_GLYPH_SIZE_SINGLE 1
#define NAME_GLYPH_SIZE_DOUBLE 2
#define NAME_GLYPH_VALUE_MASK 0xFFFFU

/**
 * Button mask for confirming the current name-entry selection.
 * Combines PAD_BTN_CROSS with PAD_BTN_L3 as an auxiliary confirm input.
 */
#define GNAME_BTN_CONFIRM (PAD_BTN_CROSS | PAD_BTN_L3)

/** L2: undo -- pop the last glyph from the active name back to the clipboard. */
#define GNAME_BTN_UNDO PAD_BTN_L2
/** R2: redo -- pop the first glyph from the clipboard and append to the active name. */
#define GNAME_BTN_REDO PAD_BTN_R2
/** Circle: delete the last glyph, or cancel when an empty name is allowed. */
#define GNAME_BTN_CANCEL PAD_BTN_CIRCLE
/** L1: scroll/cycle to the previous kanji category (decrement by 10). */
#define GNAME_BTN_KANJI_PREV PAD_BTN_L1
/** R1: scroll/cycle to the next kanji category (increment by 10). */
#define GNAME_BTN_KANJI_NEXT PAD_BTN_R1
/** Combined mask: either kanji-category navigation button (L1 or R1). */
#define GNAME_BTN_KANJI_NAV (GNAME_BTN_KANJI_PREV | GNAME_BTN_KANJI_NEXT)

/**
 * Full input mask passed to handle_navigation_input each frame: all four
 * D-pad directions plus the confirm pair.
 */
#define GNAME_BTN_NAV_MASK (PAD_BTN_UP | PAD_BTN_RIGHT | PAD_BTN_DOWN | PAD_BTN_LEFT | GNAME_BTN_CONFIRM)

/*
 * Navigation mode values stored in g_navigation_mode and passed to / returned
 * from handle_navigation_input.
 *
 *   0-3  : action tab bar (OK, Delete, Random, Default)
 *   4-6  : visible character-panel selector tabs (panel N at mode 4+N)
 *   7    : hidden kanji-category tab retained by the state machine
 *   0x10 : in-grid character cursor
 */
#define GNAME_MODE_ACTION_OK 0      /* action bar: commit the name */
#define GNAME_MODE_ACTION_DELETE 1  /* action bar: delete last glyph */
#define GNAME_MODE_ACTION_RANDOM 2  /* action bar: fill with random name */
#define GNAME_MODE_ACTION_DEFAULT 3 /* action bar: reset to default name */
#define GNAME_MODE_PANEL_BASE 4     /* first char-panel tab; panel N is at 4+N */
#define GNAME_MODE_PANEL_NAV_LAST (GNAME_MODE_PANEL_BASE + 2)
#define GNAME_MODE_PANEL_LAST (GNAME_MODE_PANEL_BASE + 3) /* hidden kanji-category tab */
#define GNAME_MODE_GRID 0x10        /* in-grid character cursor mode */

#define GNAME_REDISPATCH_PENDING 0xFF
#define GNAME_CURSOR_POS_TABLE_OFFSET 2
#define GNAME_TAB_CURSOR_X_BIAS 8
#define GNAME_CURSOR_LERP_STEPS 5
#define GNAME_GRID_LERP_STEPS 4

/* Sentinel for g_activated_entry meaning no entry was activated this frame. */
#define GNAME_ENTRY_NONE 0xFF

/* g_overlay_result values: how the overlay finished, read by the caller. */
#define GNAME_RESULT_CANCEL 2  /* cancelled with an empty name (when allowed) */
#define GNAME_RESULT_CONFIRM 5 /* name committed; advance to the next overlay stage */

/* Frame count seeded into g_strip_width_steps to start a name-strip width lerp. */
#define NAME_STRIP_LERP_STEPS 5

/* Frame count seeded into g_glyph_append_anim_timer to start the append animation. */
#define GLYPH_APPEND_ANIM_TIMER_START 2

/* g_name_source_mode values: selects the source used by the Random action. */
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

/* Maximum number of logical glyphs allowed in a name. */
#define NAME_MAX_GLYPHS 10

/* Sound effect IDs passed as the first argument to play_menu_sfx. */
#define GNAME_SFX_ERROR 0x78   /* error: name is full, blank, or action is invalid */
#define GNAME_SFX_MOVE 0x7D    /* cursor movement / navigation */
#define GNAME_SFX_CONFIRM 0x7E /* confirm / OK action */
#define GNAME_SFX_CANCEL 0x7F  /* cancel / back action */
#define GNAME_SFX_VOLUME 0x80  /* default volume argument for play_menu_sfx */

/* Character selection grid layout constants. */
#define NAME_GRID_COLUMNS 10 /**< Glyph cells per row in the grid. */
#define NAME_GRID_LAST_COL (NAME_GRID_COLUMNS - 1)
#define NAME_GRID_CELL_SIZE 16     /**< Pixel width and height of each grid cell (0x10). */
#define NAME_GRID_X_BASE 84        /**< Pixel X of the leftmost grid column (0x54). */
#define NAME_GRID_Y_TOP 104        /**< Pixel Y of the top of the visible grid area (0x68). */
#define NAME_GRID_Y_BOTTOM 168     /**< Pixel Y of the bottom clamp (0xA8). */
#define NAME_GRID_Y_EXIT_BOUND (NAME_GRID_Y_BOTTOM + 1)
#define NAME_GRID_SCROLL_STEP 64   /**< Scroll delta per step: 4 rows * 16 px/row (0x40). */
#define NAME_GRID_CELL_SHIFT 4     /**< Shift equivalent of division by the 16-pixel cell size. */
#define NAME_GRID_DIV_BIAS (NAME_GRID_CELL_SIZE - 1)
#define NAME_GRID_VISIBLE_ROWS (NAME_GRID_VIS_HEIGHT / NAME_GRID_CELL_SIZE)
#define NAME_GRID_BACKING_X 0x60   /**< VRAM X of the grid backing region (96 px). */
#define NAME_GRID_BACKING_W 0xA0   /**< Width of the grid backing region (160 px). */
#define NAME_GRID_VIS_HEIGHT 0x50  /**< Visible grid height in pixels: 5 rows * 16 (80 px). */
#define NAME_GRID_BACKING_PAGE0_Y NAME_GRID_Y_TOP
#define NAME_GRID_BACKING_PAGE1_Y 0x150
#define NAME_GRID_OVERSCAN 0x0B    /**< Glyphs partly above the window are still drawn down to y = -11. */
#define CHAR_PANEL_STANDARD_COUNT 3 /**< Number of ordinary character panels. */
#define CHAR_PANEL_KANJI_CATEGORY 3 /**< Dormant panel value for the kanji category list. */
#define CHAR_PANEL_KANJI 4          /**< Dormant panel value for the kanji character picker. */
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
#define KANJI_CATEGORY_NEXT_EDGE 9

/**
 * @brief True when a glyph drawn at panel-local Y @p y falls inside the scrolling
 *        grid window, i.e. y is in [-NAME_GRID_OVERSCAN, NAME_GRID_VIS_HEIGHT-1].
 *
 * The single unsigned compare (rather than two signed ones) is what the
 * target uses, so the biased form is required to match.
 */
#define NAME_GRID_ROW_VISIBLE(y) (((u32)((y) + NAME_GRID_OVERSCAN)) <= (NAME_GRID_VIS_HEIGHT + NAME_GRID_OVERSCAN - 1))

/* FadeState channel sentinels. Channels run 0 (fully dark) up to
 * FADE_CHAN_NEUTRAL (identity, no tint). The red channel selects additive
 * blending at FADE_CHAN_ADDITIVE and above; otherwise blending is subtractive. */
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

/** Number of entries in the fixed background/layout sprite sequence. */
#define GNAME_LAYOUT_SPRITE_COUNT 20
/** Glyph-table entry used for the editable-name text cursor. */
#define GNAME_TEXT_CURSOR_GLYPH_ID 20
/** Width/height that encodes a zero texture-window mask through Psy-Q's _get_tw. */
#define GNAME_FULL_TEX_WINDOW_SIZE 0xFF

/* Selection entries rendered from g_tab_cursor_entries by gname_render. */
#define GNAME_SELECTION_ENTRY_FIRST 2
#define GNAME_SELECTION_ENTRY_END_EXCLUSIVE 13
#define GNAME_SELECTION_ENTRY_HIDDEN 9
#define GNAME_SELECTION_ENTRY_Y_BIAS 8
#define GNAME_SELECTION_SHADOW_OFFSET 1
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

/** Mask for the CLUT X-column index stored in @c GlyphInfo::clut.
 *  Bits [5:0] hold CLUT_X/16, the portion encoded in a sprite's CLUT id. */
#define GLYPH_CLUT_X_MASK 0x3F
#define GLYPH_CLUT_X_SHIFT 4

/** CLUT-page bit pattern OR'd over the low 6 bits of @c GlyphInfo::clut
 *  before writing it into a sprite primitive (see @ref render_layout_sprite_batch,
 *  @ref emit_glyph_sprt). Encodes the fixed VRAM Y row (498) shared by all
 *  name-entry palettes; bits [5:0] are zero and supplied by @c GLYPH_CLUT_X_MASK. */
#define GLYPH_CLUT_PAGE_BITS 0x7C80
#define GLYPH_SECONDARY_OFFSET_SCALE 2
#define GLYPH_SECONDARY_BLACK_TINT GPU_COLOR_WORD(0, 0, 0)
#define GLYPH_SECONDARY_BLUE_TINT GPU_COLOR_WORD(0, 0, 0xA0)

/** Number of frames in @c g_glyph_append_anim_frames; reaching this index wraps the
 *  animation back to the resting frame and stops advancement. */
#define GLYPH_APPEND_ANIM_FRAME_COUNT 7

#define GLYPH_APPEND_ANIM_X_BIAS 0xE8
#define GLYPH_APPEND_ANIM_Y_BIAS 4

/* The panel data blob is described by the PanelDataHeader struct below. Its
 * header fields and record-offset table
 * (g_random_names_off, g_history_names_off, g_kanji_panel_offset,
 * g_panel_record_offsets, g_panel_tbl_off, g_panel_data_base) plus
 * g_kanji_cat_entries and g_kanji_entry_offsets are declared below in this file.
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
 * matched code shares a single @c lui between loading the field's value
 * and forming the base address, leaving the @c -4 as a separate @c addiu
 * in the binary.
 */
#define PANEL_DATA_BLOB (((u8*)(&g_panel_tbl_off)) - 4)

/** The blob's u16 record-offset table. Must stay a macro: the target
 *  re-derives the table at every use (no CSE), which a named local would
 *  destroy. */
#define PANEL_REC_TBL ((u16*)(PANEL_DATA_BLOB + g_panel_tbl_off))

/** Pointer to record i: the table is self-relative, entries are byte
 *  offsets from the table itself (same idiom as FF8's string tables). */
#define PANEL_RECORD(i) ((u8*)PANEL_REC_TBL + PANEL_REC_TBL[(i)])

/** Same blob, reached via the kanji header field at blob + 8. Kept separate
 *  from @ref PANEL_DATA_BLOB so the lui/addiu pair is shared with the field
 *  load, exactly as in @ref PANEL_DATA_BLOB. */
#define KANJI_DATA_BLOB (((u32)(&g_kanji_panel_offset)) - 8)

/** The dormant kanji path's self-relative glyph table (blob + the kanji offset). */
#define KANJI_GLYPH_TBL ((u8*)(KANJI_DATA_BLOB + g_kanji_panel_offset))

/** Entry i of a self-relative u16 offset table at @p tbl, as a byte pointer.
 *  Generalizes @ref PANEL_RECORD over a table chosen at runtime. */
#define TBL_ENTRY(tbl, i) ((u8*)(tbl) + ((u16*)(tbl))[(i)])

/*
 * Name and glyph records are selected through self-relative u16 offset
 * tables. These macros deliberately preserve the matched symbol anchors and
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
#define PANEL_GLYPH(panel, cursor) \
    (PANEL_CHAR_TABLE_BASE + \
     (*((u16*)((PANEL_CHAR_TABLE_BASE + (g_panel_char_offsets[(panel)] * 2)) + ((cursor) * 2)))))
#define KANJI_CATEGORY_NAME(category) \
    (PANEL_CHAR_TABLE_BASE + \
     (*((u16*)((PANEL_CHAR_TABLE_BASE + (g_kanji_cat_names_offset * 2)) + ((category) * 2)))))

#define KANJI_GLYPH_TABLE_BASE ((g_random_names_off - 0x10) + ((u32)g_kanji_panel_offset))
#define KANJI_GLYPH(category, cursor) \
    (KANJI_GLYPH_TABLE_BASE + \
     (*((u16*)((KANJI_GLYPH_TABLE_BASE + (g_kanji_entry_offsets[g_kanji_cat_entries[(category)]] * 2)) + ((cursor) * 2)))))

/**
 * @brief RGB lerp state.
 *
 * Used as a pair: `g_fade_target` is the *target* (final color + remaining
 * step count), `g_fade_current` is the *current* interpolated value (its
 * `steps` field is unused). Each tick @ref render_fade_overlay advances the
 * current toward the target by `(target - current) / steps` and decrements
 * `steps`. Channels are 0..FADE_CHAN_NEUTRAL (identity = no tint), with values
 * above neutral representing additive intensity. The red channel selects the
 * packet's additive or subtractive blend mode.
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
    u32 kanji_offset; /* 0x08 - g_kanji_panel_offset: offset of the kanji panel glyph data (0x2A0) */
    u32 history_off; /* 0x0C - g_history_names_off: offset of the history name list (0x3754) */
    u32 random_off;  /* 0x10 - g_random_names_off: offset of the random name pool (0x3C9C) */
    /* g_panel_record_offsets (u16[]) follows at +0x14; each entry is a byte
     * offset from the table itself to one record (panel glyph lists, category
     * labels, tab sprites, kanji category names). */
} PanelDataHeader;

/* --- Typed views into tables stored in the gname_data databin. --- */
extern GlyphAppendAnimFrame g_glyph_append_anim_frames[];
extern Tim g_name_entry_tim;    /* glyph TIM blob; Tim covers the fixed header + CLUT, pixel block follows */
extern GlyphSeqEntry g_layout_sprite_sequence[];

/* --- Overlay .bss scratch globals -------------------------------------------
 *
 * Uninitialized run-state RAM owned by this translation unit (gname.o(.bss),
 * 0x8014F7B0..0x8014F8D8). Defined here in ascending address order so the
 * compiler lays them out matching the original; do not reorder. The single-byte
 * fields (g_glyph_append_anim_frame, g_glyph_append_anim_timer) are each followed by an
 * explicit 3-byte pad field: this compiler does not implicitly align the
 * following s32 global to a 4-byte boundary, so the gap must be spelled out
 * or the subsequent globals land 3 bytes early.
 */

/** 48-byte name buffer holding the custom preset name (used when g_name_source_mode == 1). */
u8 g_custom_name_buf[48];
/** Which source the Random action uses: 1 = custom, 3 = history-derived,
 *  and 4/5 = the first or second half of the random-name table. */
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
/** Active character panel index: 0-2 = standard panels; 3-4 are dormant
 *  kanji category/character paths not reachable through the shipped tab data. */
s32 g_char_panel;
/** Pointer to the current kanji category's display data (set when g_char_panel == 4). */
void* g_kanji_cat_name;
/** 48-byte clipboard buffer; deleted chars are prepended here and can be re-pasted. */
u8 g_name_clipboard[48];
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
/** Explicit alignment pad; see the block comment above. */
u8 pad_8014F8B1[3];
/** Current horizontal scroll position of the character grid in pixels. */
s32 g_scroll_pos;
/** Render ticks until the next append-animation frame. */
u8 g_glyph_append_anim_timer;
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
 *  unknown. Kept so the overlay's linked .bss extent matches the original. */
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
 * @ref gname_run is the overlay entry point. @ref gname_init and
 * @ref gname_tick also have external linkage and are declared in gname.h;
 * the remaining helpers are private to this translation unit.
 */
static void reset_fade_state(void);
static void render_fade_overlay(RenderContext* ctx);
static void set_fade_target(s32 r, s32 g, s32 b, s32 steps);
static void load_name_entry_tim(void);
static void load_tim_to_vram(TimDstCoords* dst_coords);
static void gname_update_state(void);
static void reset_run_state(void);
static s32 handle_navigation_input(s32 mode, s32 buttons);
static void gname_process_input(void);
static u_long* emit_cursor_glyph(u_long* prim, u_long* ot, s16 x, s16 y);
static void gname_render(RenderContext* render_ctx);
static void* emit_panel_tab_sprite(void* packet_cursor, u_long* ot_entry);
static void* emit_panel_label(void* packet_cursor, u_long* ot_entry);
static void render_name_strip(RenderContext* render_ctx, u8* name, s32 strip_width);
static void render_char_panel(RenderContext* render_ctx, s32 panel_index);
static void* emit_draw_mode_prim(DR_TPAGE* packet, u_long* ot_entry);
static void* emit_glyph_sprt(void* packet_start, u_long* ot_entry, s32 glyph_id, s32 base_x, s32 base_y, s32 shadow_offset, s32 activation_adjust, s32 use_blue_overlay);
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

/** Overlay header identifier stored immediately before @ref gname_run. */
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

/** Record-index boundaries for the non-kanji character panels.
 *  Index 3 aliases @ref g_kanji_cat_names_offset; index 4 is the following
 *  boundary word immediately before @ref g_kanji_cat_entries. */
extern u32 g_panel_char_offsets[];

/** First record-offset entry index of the kanji category name records. */
extern s32 g_kanji_cat_names_offset;

/** Kanji category entry index table: [cat] -> sub-index into
 *  g_kanji_entry_offsets, or 0xFF when empty. The shipped table contains ten
 *  entries and all are 0xFF, leaving the kanji path disabled. */
extern u32 g_kanji_cat_entries[];

/** Glyph metrics table indexed by character id (see @ref GlyphInfo). */
extern GlyphInfo g_glyph_table[];

/** Scroll-up [0] and scroll-down [1] indicator entries. */
extern TabCursorEntry g_tab_cursor_pos[];
/** Cursor targets and glyphs for the 11 action/panel selection entries. */
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
 * The header fields are u32 byte offsets. Some declarations deliberately use
 * pointer or byte-array types to preserve the matched address arithmetic.
 */

/** Blob + 0x00: stored value 4; purpose unknown. */
extern u8 g_panel_data_base[];
/** Blob + 0x04: offset (0x14) of the u16 record-offset table. */
extern u32 g_panel_tbl_off;
/** Blob + 0x08: offset (0x2A0) of the kanji panel glyph data. */
extern u8* g_kanji_panel_offset;
/** Blob + 0x0C: offset (0x3754) of the history name list. */
extern u8 g_history_names_off[];
/** Blob + 0x10: offset (0x3C9C) of the random name pool. */
extern u8 g_random_names_off[];

/** Blob + 0x14: u16 record-offset table (138 entries). Each entry is a byte
 *  offset from the table itself to one record. */
extern u16 g_panel_record_offsets[];

/* --- Cross-module helpers invoked by the GNAME run loop (defined in other
 *     overlays / the main executable). ------------------------------------- */
void set_controller_vsync_interval(unsigned long interval);
void update_controllers(void);
void func_80063194(void);
void func_8006441C(void);
void field_update_audio_timer(void);
void func_800A9E78(void);
void func_800AA02C(void);

/* func_800644FC and func_800A88A0 intentionally have no visible prototypes
 * here. Their call sites therefore use this compiler's implicit-int rules,
 * which are part of the matched declaration context. */

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

    /* Configure the two overlapping VRAM display/draw pages. */
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
    update_controllers();
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

        field_update_audio_timer();
        DrawSync(0);
        set_controller_vsync_interval(2U);
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
        DrawOTag(&draw_buf->ot[GNAME_OT_LAYOUT_BACKGROUND]);
        draw_buf = other_buf;
        update_controllers();
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

                /* Keeping the member offset on the store preserves the target register allocation. */
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
 * additive (@c FADE_TPAGE_ADD, brighten) when the red channel is >=
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

    /* The red channel selects the representation and blend mode for all channels. */
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
 * @param r     Target red; values above 0x100 select additive blending.
 * @param g     Target green intensity, encoded in the mode selected by @p r.
 * @param b     Target blue intensity, encoded in the mode selected by @p r.
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
 * @brief Initialize name-entry resources and per-run state.
 *
 * @see https://decomp.me/scratch/pnzC1 (100%)
 */
void gname_init(void)
{
    /* Unused storage preserves the target stack frame. */
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

    /* The escaped RECT keeps these otherwise-unused trailing stores observable. */
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
    render_layout_sprite_batch(ctx);
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
        if ((name_glyph_count(g_active_name) != 0) && (name_is_blank(g_active_name) == 0))
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
 * Zeros the counters and indices, seeds the cursor from its initial targets,
 * computes the initial @c g_navigation_mode, and copies @c g_initial_name into
 * the active name buffer.
 *
 * @see https://decomp.me/scratch/FboaU (100%)
 */
static void reset_run_state(void)
{
    g_activated_entry = GNAME_ENTRY_NONE;
    g_navigation_mode = handle_navigation_input(0, 0);
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
    g_glyph_append_anim_frame = 0;
    g_glyph_append_anim_timer = GLYPH_APPEND_ANIM_TIMER_START;
    g_char_panel = 0;
}

/**
 * @brief Process one frame of name-entry UI input and return the new char-set mode.
 *
 * State machine for the character-selection screen. @p mode names the focused
 * UI region:
 *   - GNAME_MODE_ACTION_OK..DEFAULT (0-3): action tab bar
 *   - GNAME_MODE_PANEL_BASE..+2 (4-6): visible character-panel selector tabs
 *   - GNAME_MODE_PANEL_LAST (7): hidden kanji-category tab
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
static s32 handle_navigation_input(s32 mode, s32 buttons)
{
    /* 0xFF: re-run the switch with the updated mode; 0: done */
    s32 repeat_dispatch = GNAME_REDISPATCH_PENDING;
    /*
     * Holds the constant 1, assigned only inside the case 0-3 confirm branch
     * but reused as an operand in later branches. Required to match the
     * target register allocation; do not fold back to literal 1.
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
                g_activated_entry = mode;
                one = 1;
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
                    repeat_dispatch = 0;
                    continue;

                case GNAME_MODE_ACTION_DELETE:
                    play_menu_sfx(GNAME_SFX_CONFIRM, GNAME_SFX_VOLUME);
                    name_pop_last_glyph(g_active_name);
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
            /* Other mode values are handled as the active character grid. */
            if (((buttons & GNAME_BTN_CONFIRM) && (((g_char_last_row * NAME_GRID_COLUMNS) + g_char_last_col) >= g_char_cursor)) != 0U)
            {
                if (g_char_panel < CHAR_PANEL_STANDARD_COUNT)
                {
                    if (name_glyph_count(g_active_name) < NAME_MAX_GLYPHS)
                    {
                        u8* selected_glyph;
                        g_glyph_append_anim_timer = GLYPH_APPEND_ANIM_TIMER_START;
                        selected_glyph = PANEL_GLYPH(g_char_panel, g_char_cursor);
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
                    if (name_glyph_count(g_active_name) < NAME_MAX_GLYPHS)
                    {
                        u8* selected_glyph;
                        g_glyph_append_anim_timer = GLYPH_APPEND_ANIM_TIMER_START;
                        selected_glyph = KANJI_GLYPH(g_kanji_cat, g_char_cursor);
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
                repeat_dispatch = 0;
            }
            else
            {
                s32 scroll_off;

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
                        g_char_cursor -= 1;
                    }
                    else
                    {

                        if ((buttons & PAD_BTN_RIGHT) && ((g_char_cursor % NAME_GRID_COLUMNS) != NAME_GRID_LAST_COL))
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
 *    @ref handle_navigation_input.
 *  - Undo (L2): move the last glyph from @c g_active_name back onto the
 *    clipboard @c g_name_clipboard.
 *  - Redo (R2): move the first clipboard character back into @c g_active_name.
 *  - Cancel (Circle): pop the last glyph, or finish with
 *    @c GNAME_RESULT_CANCEL when empty and @c g_allow_empty_cancel is set.
 *
 * In the dormant kanji grid path (@c g_navigation_mode == GNAME_MODE_GRID,
 * @c g_char_panel == @c CHAR_PANEL_KANJI), L1/R1 cycle @c g_kanji_cat by one
 * category page with wrap and reset the page. Finally, advances the cursor
 * and scroll lerps.
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
    s32 moved_glyph;
    s32 scroll_step;
    s32* scroll_pos_ptr;
    s32 category_table_base_index;
    u16 clipboard_glyph_u16;
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

    g_activated_entry = GNAME_ENTRY_NONE;
    nav_input = g_pad_input & GNAME_BTN_NAV_MASK;

    if (nav_input != 0)
    {
        g_navigation_mode = handle_navigation_input(g_navigation_mode, nav_input);
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
        sfx_id = GNAME_SFX_MOVE;
        volume_or_input_mask = GNAME_SFX_VOLUME;
        play_menu_sfx(sfx_id, volume_or_input_mask);
    }
    /* Redo: move the first clipboard glyph back into the active name. */
    else if (g_pad_input & GNAME_BTN_REDO)
    {
        if (name_glyph_count(g_active_name) < NAME_MAX_GLYPHS)
        {
            clipboard_ptr = g_name_clipboard;
            moved_glyph = name_pop_first_glyph(clipboard_ptr);
            clipboard_glyph_u16 = moved_glyph;
            if (clipboard_glyph_u16 != 0)
            {
                clipboard_glyph[0] = moved_glyph;
                clipboard_glyph[1] = clipboard_glyph_u16 >> 8;
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
            if (name_glyph_count(g_active_name) == 0)
            {
                g_overlay_result = GNAME_RESULT_CANCEL;
                play_menu_sfx(GNAME_SFX_CANCEL, GNAME_SFX_VOLUME);
                return;
            }
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
 * Builds a SPRT at @p prim from @c g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID]
 * (UV, size, CLUT), then a texture-page DR_TPAGE packet right after it, and
 * chains both into @p ot.
 *
 * @param prim Write position; needs sizeof(SPRT) + sizeof(DR_TPAGE) bytes.
 * @param ot   OT slot to chain both primitives into.
 * @param x    Cursor sprite screen X.
 * @param y    Cursor sprite screen Y.
 * @return Pointer just past the written primitives.
 * @note No current call sites; @ref gname_render emits the same packet pair inline.
 * @see decomp.me (100%) https://decomp.me/scratch/oXGkF
 */
static u_long* emit_cursor_glyph(u_long* prim, u_long* ot, s16 x, s16 y)
{
    u32 clut;
    SPRT* sprt = (SPRT*)prim;

    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    setXY0(sprt, x, y);

    setUV0(sprt, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].u, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].v);
    setWH(sprt, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].w, g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].h);

    clut = g_glyph_table[GNAME_TEXT_CURSOR_GLYPH_ID].clut & GLYPH_CLUT_X_MASK;
    sprt->clut = clut | GLYPH_CLUT_PAGE_BITS;
    addPrim(ot, sprt);

    prim += PRIM_WORDS(SPRT);
    setDrawTPage(prim, 0, 0, GNAME_GLYPH_TPAGE);
    addPrim(ot, prim);

    return prim + PRIM_WORDS(DR_TPAGE);
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
                                     PANEL_RECORD(g_tab_cursor_pos[navigation_mode + GNAME_CURSOR_POS_TABLE_OFFSET].sprite_idx),
                                     GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
    }
    else if (navigation_mode == GNAME_MODE_GRID)
    {
        s32 panel_index = g_char_panel;

        /* Select specialized tabs only for the category and kanji panels. */
        if ((u32)(panel_index - CHAR_PANEL_KANJI_CATEGORY) < (CHAR_PANEL_KANJI - CHAR_PANEL_KANJI_CATEGORY + 1))
        {
            packet_cursor = func_800A88A0(packet_cursor, ot_entry, PANEL_RECORD(panel_index + GNAME_PANEL_TAB_KANJI_RECORD_OFFSET),
                                         GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_TAB_X, GNAME_PANEL_TAB_Y, GNAME_PANEL_SPRITE_MODE);
        }
        else
        {
            packet_cursor = func_800A88A0(packet_cursor, ot_entry, PANEL_RECORD(GNAME_PANEL_TAB_DEFAULT_RECORD), GNAME_PANEL_SPRITE_COLOR,
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
        packet_cursor = func_800A88A0(packet_cursor, ot_entry, PANEL_RECORD(panel_index), GNAME_PANEL_SPRITE_COLOR, GNAME_PANEL_LABEL_X,
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
    u8* glyph_table;
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
        glyph_table = KANJI_GLYPH_TBL;
        category_entry = g_kanji_cat_entries[g_kanji_cat];
        glyph_index = g_kanji_entry_offsets[category_entry];
        glyph_end = g_kanji_entry_offsets[category_entry + 1];
        grid_row = 0;
    }
    else
    {
        glyph_index = g_panel_char_offsets[panel_index];
        glyph_end = g_panel_char_offsets[panel_index + 1];
        glyph_table = (u8*)PANEL_REC_TBL;
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
            glyph_packet_cursor = func_800A88A0(glyph_packet_cursor, ot_entry, TBL_ENTRY(glyph_table, glyph_index), CHAR_PANEL_GLYPH_COLOR,
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
static void* emit_glyph_sprt(void* packet_start, u_long* ot_entry, s32 glyph_id, s32 base_x, s32 base_y, s32 shadow_offset, s32 activation_adjust, s32 use_blue_overlay)
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
