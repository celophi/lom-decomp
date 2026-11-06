#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#include "common.h"
#include <libgpu.h>
#include <libgte.h>

#include "common/encoded_text.h"
#include "common/glyph_format.h"
#include "common/sjis.h"
#include "overlays/field/field_text.h"

/**
 * @file glyph_cache.h
 * @brief Kanji ROM text drawing through a VRAM glyph cache, compiled into ADDHERO, CARDA, CLOAD and NIKI.
 *
 * Text is converted to Shift-JIS, and each character's 16x15 bitmap is read
 * from the Kanji ROM (Krom2RawAdd), rasterized to 4 bits per pixel, uploaded
 * to a free cache slot in VRAM and drawn as a 16x16 sprite. Slots not drawn
 * during a frame are freed at its end.
 */

/** @brief Cached character code and flags recording use in the current frame. */
typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} GlyphCacheEntry;

/** @brief Cached-glyph sprite packet and its trailing padding word. */
typedef struct
{
    SPRT_16 packet;
    u32 padding;
} GlyphSprite;

extern GlyphCacheEntry g_glyph_cache[GLYPH_CACHE_SLOTS];
extern u8 g_glyph_raster_buffer[GLYPH_RASTER_BUFFER_BYTES];
extern u8* g_glyph_raster_cursor;
extern s32 g_glyph_cursor_x;
extern s32 g_glyph_cursor_y;
extern s32 g_glyph_line_start_x;
extern s32 g_glyph_upload_x;
extern s32 g_glyph_upload_y;

/** @brief Character chart rows for the one-byte codes, starting at GLYPH_TEXT_FIRST_PRINTABLE. */
extern u8 g_glyph_single_byte_chart[];

/**
 * @brief Base the two-byte character pages are reached from: page @c lead starts at
 *        this address + lead * GLYPH_CHART_PAGE_BYTES, inside the character chart.
 * @note Not a table of its own: it lies ENCODED_TEXT_DOUBLE_BYTE_LEAD_FIRST pages
 *       before the two-byte chart, so the address falls inside other data or code.
 */
extern u8 g_glyph_chart_page_base[];

/**
 * @brief Page base used by expand_text_glyph_codes. An overlay whose page base
 *        falls inside its own code defines this before including the header.
 */
#ifndef GLYPH_CHART_PAGE_BASE
#define GLYPH_CHART_PAGE_BASE g_glyph_chart_page_base
#endif

/** @brief Full-width digit characters "0"-"9" and "0"-"F". */
extern u16 g_glyph_decimal_digits[];
extern u16 g_glyph_hex_digits[];

void* draw_signed_decimal(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void draw_hex_byte(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 alignment);
void* draw_cached_text(void* prim, u_long* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void* render_cached_glyph(void* primitive, u_long* ot_tag, u16 character_code, s32 palette);
void* emit_glyph_sprite(GlyphSprite* sprite, u_long* ot, s32 cache_slot, s32 palette);
void begin_glyph_cache_frame(void);
void evict_unused_glyphs(void);
void reset_glyph_cache(void);
void expand_text_glyph_codes(u8* destination, u8* source);

#endif
