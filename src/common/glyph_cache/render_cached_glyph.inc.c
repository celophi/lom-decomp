/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Draw a cached glyph, rasterizing and uploading it on a cache miss.
 * @param primitive Primitive-buffer cursor.
 * @param ot_tag Ordering-table tag to append to.
 * @param character_code Shift-JIS character code.
 * @param palette Selects color index (palette + 1) * GLYPH_PALETTE_COLOR_STRIDE.
 * @return Updated cursor, or the original cursor if the glyph is missing or the cache is full.
 * @note Cached glyphs retain the color index used when first rasterized.
 */
void* render_cached_glyph(void* primitive, u_long* ot_tag, u16 character_code, s32 palette)
{
    const u8* font_data;
    u8* raster_cursor;
    s32 index;
    s32 row;
    s32 row_byte_index;
    s32 color_index;
    s32 high_nibble_color;
    u16 source_mask;
    RECT upload_rect;

    for (index = 0; index < GLYPH_CACHE_SLOTS; index++)
    {
        if (character_code == g_glyph_cache[index].data.code)
        {
            return emit_glyph_sprite(primitive, ot_tag, index, palette);
        }
    }

    /* Psy-Q exposes the KROM pointer as a signed integer address. */
    font_data = (const u8*)Krom2RawAdd(character_code);
    if (font_data == (const u8*)GLYPH_INVALID_KROM_ADDRESS)
    {
        return primitive;
    }

    raster_cursor = g_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * GLYPH_PALETTE_COLOR_STRIDE;
    high_nibble_color = color_index * GLYPH_HIGH_NIBBLE_SCALE;
    /* Expand ROM bits to 4-bit colors, packing two pixels into each output byte. */
    for (; row < GLYPH_ROWS; row++)
    {
        for (row_byte_index = 0; row_byte_index < GLYPH_FONT_ROW_BYTES; row_byte_index++)
        {
            source_mask = GLYPH_FONT_SOURCE_MSB;
            for (index = 0; index < GLYPH_FONT_PIXEL_PAIRS; index++)
            {
                *raster_cursor = (*font_data & source_mask) ? color_index : 0;
                source_mask >>= 1;
                *raster_cursor += (*font_data & source_mask) ? high_nibble_color : 0;
                source_mask >>= 1;
                raster_cursor++;
            }
            font_data++;
        }
    }

    /* A free slot has neither a character code nor a per-frame usage mark. */
    for (index = 0; index < GLYPH_CACHE_SLOTS; index++)
    {
        if (g_glyph_cache[index].raw == 0)
        {
            break;
        }
    }

    if (index == GLYPH_CACHE_SLOTS)
    {
        return primitive;
    }
    g_glyph_cache[index].raw = character_code;
    primitive = emit_glyph_sprite(primitive, ot_tag, index, palette);

    g_glyph_upload_x = (index % GLYPH_CACHE_COLUMNS) * (GLYPH_SIZE / GLYPH_VRAM_WORD_PIXELS);
    g_glyph_upload_y = index & GLYPH_CACHE_ROW_MASK;

    setWH(&upload_rect, GLYPH_SIZE / GLYPH_VRAM_WORD_PIXELS, GLYPH_ROWS);
    upload_rect.x = g_glyph_upload_x + GLYPH_VRAM_X;
    upload_rect.y = g_glyph_upload_y;

    LoadImage(&upload_rect, (u_long*)g_glyph_raster_cursor);
    DrawSync(0);

    g_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return primitive;
}
