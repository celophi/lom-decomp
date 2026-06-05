#ifndef _GNAME_H
#define _GNAME_H

#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "main.h"
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

/** CLUT-page bit pattern OR'd over the low 6 bits of @c GlyphInfo::clut
 *  before writing it into a sprite primitive (see @ref draw_name_cursor_row,
 *  @ref func_80142274). */
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

extern void* func_80142274(void* arg0, s32* arg1, u8 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7);

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
extern u8 g_glyph_table[]; /* GlyphInfo[]; declared as u8[] for byte-level accesses elsewhere */
extern s32 D_80142E14;
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

/* --- ROM data tables (addresses known, names TODO) --- */
extern u8* D_80142F04;
extern u8* D_80142F00;
extern u32 D_80142E0C[];
extern u8* D_80142EFC;
extern u32 D_80142E40[];
extern u8* D_80142EF8;
extern u32 D_80142C98[];
extern u32 D_80142CAC[];
extern s32 D_80142CA4;
extern u8 D_80142EF4[];

extern void func_800A3938(int, int);
extern void func_8014139C(void);
extern s32 name_char_count(u8*);
extern s32 name_is_blank(u8*);
extern s32 func_80140AB8(s32, s32);
extern void name_copy(u8*, u8*);

#endif