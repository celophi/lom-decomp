#ifndef GLYPH_CACHE_H
#define GLYPH_CACHE_H

#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sjis.h"
#include "encoded_text.h"
#include "field_text.h"

/**
 * @file glyph_cache.h
 * @brief Kanji ROM text drawing through a VRAM glyph cache, compiled into ADDHERO, CARDA, CLOAD and NIKI.
 *
 * Text is converted to Shift-JIS, and each character's 16x15 bitmap is read
 * from the Kanji ROM (Krom2RawAdd), rasterized to 4 bits per pixel, uploaded
 * to a free cache slot in VRAM and drawn as a 16x16 sprite. Slots not drawn
 * during a frame are freed at its end.
 *
 * Each overlay includes these functions from src/common/glyph_cache/<function>.inc.c
 * at the point where they sit in its binary and maps the globals below to its
 * own data, so every overlay still links its own copy.
 */

/** @brief Cache layout: 256 slots of 16x15 4-bit glyphs, 16 per VRAM row. */
#define GLYPH_CACHE_SLOTS 256
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0

/** @brief Bytes of one rasterized glyph, and of the raster buffer a frame's new glyphs are built in. */
#define GLYPH_RASTER_BYTES 0x80
#define GLYPH_RASTER_BUFFER_BYTES 0x8000

/** @brief GlyphCacheEntry.raw flag: the slot was drawn this frame. */
#define GLYPH_CACHE_USED 0x10000

/** @brief GlyphCacheEntry.raw bits that hold the cached character code. */
#define GLYPH_CACHE_CODE_MASK 0xFFFF

/** @brief Glyph cell: GLYPH_SIZE square on screen, GLYPH_ROWS rows of Kanji ROM bitmap. */
#define GLYPH_SIZE 16
#define GLYPH_ROWS 15

/** @brief VRAM x of the glyph cache (4-bit texels, so a glyph is GLYPH_SIZE / 4 halfwords wide). */
#define GLYPH_VRAM_X 320

/** @brief CLUT the cached glyphs are drawn with: FIELD's text CLUT. */
#define GLYPH_CLUT_X 304
#define GLYPH_CLUT_Y 511

/** @brief Pen x at which cached text wraps to the next line. */
#define GLYPH_TEXT_WRAP_X 640

/** @brief Codes below this end the text drawn by draw_cached_text. */
#define GLYPH_TEXT_FIRST_PRINTABLE 0x20

/** @brief Added to an ASCII byte to get its Shift-JIS character: letters and digits, and other symbols. */
#define GLYPH_SJIS_ALNUM_OFFSET 0x821F
#define GLYPH_SJIS_SYMBOL_OFFSET 0x851F

/** @brief Full-width "0" and "-" as stored in the little-endian u16 digit tables. */
#define GLYPH_PAIR_ZERO 0x4F82
#define GLYPH_PAIR_MINUS 0x5B81

/** @brief Character chart: 33-byte rows of 16 two-byte Shift-JIS characters plus a newline, 16 rows per page. */
#define GLYPH_CHART_ROW_BYTES 33
#define GLYPH_CHART_COLUMNS 16
#define GLYPH_CHART_PAGE_BYTES (16 * GLYPH_CHART_ROW_BYTES)

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
extern u8_ptr g_glyph_raster_cursor;
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
 * @note Not a table of its own; the address itself falls inside other data.
 */
extern u8 g_glyph_chart_page_base[];

/** @brief Full-width digit characters "0"-"9" and "0"-"F". */
extern u16 g_glyph_decimal_digits[];
extern u16 g_glyph_hex_digits[];

void* draw_signed_decimal(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void draw_hex_byte(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 alignment);
void* draw_cached_text(void* prim, u_long* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void* render_cached_glyph(void* prim, u_long* ot, u16 code, s32 palette);
void* emit_glyph_sprite(GlyphSprite* sprite, u_long* ot, s32 cache_slot, s32 palette);
void begin_glyph_cache_frame(void);
void evict_unused_glyphs(void);
void reset_glyph_cache(void);
void expand_text_glyph_codes(u8* out, u8* in);

#endif
