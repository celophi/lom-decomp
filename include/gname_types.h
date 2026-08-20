/**
 * @file gname_types.h
 * @brief Record types for the GNAME .data tables.
 *
 * These types give gname.c documented views over the packed data extracted from
 * the original overlay. Their layouts must continue to agree with the binary
 * records consumed by the matched code.
 */
#ifndef GNAME_TYPES_H
#define GNAME_TYPES_H

#include "common.h"

/** Number of glyph slots per @ref GlyphAppendAnimFrame; used as the array
 *  dimension of the struct's @c slots[] member below. */
#define GLYPH_APPEND_ANIM_SLOT_COUNT 3

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
    u32 clut; /* 0x4 - CLUT X-column index; low 6 bits are encoded in the sprite CLUT id */
} GlyphInfo;

/**
 * @brief One entry of the fixed layout-sprite table at
 *        @c g_layout_sprite_sequence.
 *
 * 20 of these are walked by @ref render_layout_sprite_batch each frame to emit
 * the backmost fixed layout sprites.
 */
typedef struct
{
    u32 id; /* 0x0 - index into g_glyph_table (selects which sprite tile to draw) */
    u32 xy; /* 0x4 - packed s16 x,y screen position (low half = x, high = y) */
} GlyphSeqEntry;

/**
 * @brief One entry in the tab-cursor and scroll-indicator position table.
 *
 * @c g_tab_cursor_pos[0..1] are the scroll-up and scroll-down indicator
 * glyphs. @c g_tab_cursor_entries[0..10] are the cursor target positions for
 * the 11 action/panel entries. The tables are contiguous, so matched accesses
 * anchored at @c g_tab_cursor_pos[2 + n] alias @c g_tab_cursor_entries[n].
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
 * @brief One glyph slot inside an @ref GlyphAppendAnimFrame.
 *
 * @ref render_glyph_append_anim emits a textured-glyph SPRT for every slot
 * whose @c glyph id is non-zero, at screen position (@c x + 0xE8,
 * @c y + 4).
 */
typedef struct
{
    u8 x;     /* 0x0 - X position (biased by 0xE8 when drawn) */
    u8 y;     /* 0x1 - Y position (biased by 4 when drawn) */
    u8 glyph; /* 0x2 - glyph id (index into g_glyph_table); 0 = empty slot */
    u8 pad;   /* 0x3 - slot 0: frame duration in render ticks; otherwise unused */
} GlyphAppendAnimSlot;

/**
 * @brief One frame of the glyph-append animation played by
 *        @ref render_glyph_append_anim.
 *
 * @c g_glyph_append_anim_frames holds @ref GLYPH_APPEND_ANIM_FRAME_COUNT of these. Frame 0
 * is the resting frame and is still rendered while the timer is zero. Starting
 * the timer advances through frames 0..6, then returns to frame 0 and stops.
 * Each active frame lasts @c slots[0].pad render ticks.
 */
typedef struct
{
    GlyphAppendAnimSlot slots[GLYPH_APPEND_ANIM_SLOT_COUNT];
} GlyphAppendAnimFrame; /* 0xC bytes */

#endif /* GNAME_TYPES_H */
