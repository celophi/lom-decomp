/**
 * @file gname_types.h
 * @brief Record types for the GNAME .data tables built by gname_data.c.
 *
 * These typedefs mirror the ones defined inline in gname.c (the code that reads
 * these tables). They live here so gname_data.c can declare the tables with the
 * same layout gname.c expects. The definitions must stay byte-compatible with
 * gname.c; the gname_data.o byte-check (compiled bytes vs the original ROM)
 * catches any drift.
 *
 * @note TODO: fold gname.c's inline copies into this header once that file can
 *       include it without disturbing its matched codegen.
 */
#ifndef GNAME_TYPES_H
#define GNAME_TYPES_H

#include "common.h"

/** Glyph slots per @ref GlyphAppendAnimFrame. */
#define GLYPH_APPEND_ANIM_SLOT_COUNT 3

/**
 * @brief Glyph metrics entry: how to draw one glyph from VRAM (g_glyph_table).
 */
typedef struct
{
    u8 u;     /* 0x0 - texture U in VRAM */
    u8 v;     /* 0x1 - texture V in VRAM */
    u8 w;     /* 0x2 - sprite width */
    u8 h;     /* 0x3 - sprite height */
    u32 clut; /* 0x4 - CLUT X-column index */
} GlyphInfo;

/**
 * @brief One entry of the fixed layout-sprite table (g_layout_sprite_sequence).
 */
typedef struct
{
    u32 id; /* 0x0 - index into g_glyph_table */
    u32 xy; /* 0x4 - packed s16 x,y screen position (low half = x, high = y) */
} GlyphSeqEntry;

/**
 * @brief One entry in the tab-cursor / scroll-indicator position table
 *        (g_tab_cursor_pos, g_tab_cursor_entries).
 *
 * @note The x/sprite_idx bitfields pack into the low 16 bits of the first word;
 *       y and glyph occupy bytes 2 and 3, giving a 4-byte element.
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
 */
typedef struct
{
    u8 x;     /* 0x0 - X position (biased by 0xE8 when drawn) */
    u8 y;     /* 0x1 - Y position (biased by 4 when drawn) */
    u8 glyph; /* 0x2 - glyph id (index into g_glyph_table); 0 = empty slot */
    u8 pad;   /* 0x3 - slot 0: frame duration in render ticks; otherwise unused */
} GlyphAppendAnimSlot;

/**
 * @brief One frame of the glyph-append animation (g_glyph_append_anim_frames).
 */
typedef struct
{
    GlyphAppendAnimSlot slots[GLYPH_APPEND_ANIM_SLOT_COUNT];
} GlyphAppendAnimFrame; /* 0xC bytes */

#endif /* GNAME_TYPES_H */
