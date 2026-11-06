#ifndef GLYPH_FORMAT_H
#define GLYPH_FORMAT_H

#include "common.h"

/**
 * @file glyph_format.h
 * @brief Kanji ROM glyph cache and raster layout, shared by common/glyph_cache.h and CHECKPS's own glyph cache.
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

/** @brief Each Kanji ROM row contains two bytes of one-bit pixels, read MSB first. */
#define GLYPH_FONT_ROW_BYTES 2
#define GLYPH_FONT_SOURCE_MSB 0x80

/** @brief Each source byte expands to four pairs of 4-bit pixels. */
#define GLYPH_FONT_PIXEL_PAIRS 4

/** @brief Shared glyph colors occupy every second entry of the CLUT. */
#define GLYPH_PALETTE_COLOR_STRIDE 2

/** @brief Multiplier that places a pixel color in a raster byte's high nibble. */
#define GLYPH_HIGH_NIBBLE_SCALE 16

/** @brief Number of 4-bit pixels packed into one VRAM word. */
#define GLYPH_VRAM_WORD_PIXELS 4

/** @brief Krom2RawAdd returns this signed address when the glyph is unavailable. */
#define GLYPH_INVALID_KROM_ADDRESS (-1)

/** @brief VRAM x of the glyph cache (4-bit texels, so a glyph is GLYPH_SIZE / 4 halfwords wide). */
#define GLYPH_VRAM_X 320

/** @brief Pen x at which cached text wraps to the next line. */
#define GLYPH_TEXT_WRAP_X 640

/** @brief First character code of game text; lower codes are control codes (they end the text drawn by draw_cached_text). */
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
#define GLYPH_CHART_ROWS_PER_PAGE 16
#define GLYPH_CHART_PAGE_BYTES (GLYPH_CHART_ROWS_PER_PAGE * GLYPH_CHART_ROW_BYTES)

/** @brief Lead bytes of the chart's two-byte pages: GLYPH_CHART_PAGE_COUNT pages from GLYPH_CHART_FIRST_LEAD. */
#define GLYPH_CHART_FIRST_LEAD 0x19
#define GLYPH_CHART_PAGE_COUNT 7

/** @brief Each chart entry stores the two bytes of a Shift-JIS character. */
#define GLYPH_CHART_GLYPH_BYTES 2

/** @brief The second source byte stores the chart row above the column nibble. */
#define GLYPH_CHART_ROW_SHIFT 4
#define GLYPH_CHART_COLUMN_MASK (GLYPH_CHART_COLUMNS - 1)

#endif
