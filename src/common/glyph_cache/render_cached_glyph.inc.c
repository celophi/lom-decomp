/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Emit one glyph sprite, rasterizing and uploading the glyph to the VRAM
 *        cache first when it is not already cached.
 * @param prim    Current primitive cursor.
 * @param ot      Ordering table the sprite is linked into.
 * @param code    Shift-JIS glyph code to render.
 * @param palette Glyph palette index; sets the 4-bit pixel value (palette + 1) * 2.
 * @return The updated primitive cursor, unchanged when the glyph is missing or
 *         the cache is full.
 */
void* render_cached_glyph(void* prim, u_long* ot, u16 code, s32 palette)
{
    u8* font_data;
    u8* raster;
    s32 i;
    s32 row;
    s32 source_byte;
    s32 color_index;
    s32 high_nibble_color;
    u16 mask;
    RECT rect;

    for (i = 0; i < GLYPH_CACHE_SLOTS; i++)
    {
        if (code == g_glyph_cache[i].data.code)
        {
            return emit_glyph_sprite(prim, ot, i, palette);
        }
    }

    font_data = (u8*)Krom2RawAdd(code);
    if (font_data == (u8*)-1)
    {
        return prim;
    }

    raster = g_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < GLYPH_ROWS; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;
            for (i = 0; i < 4; i++)
            {
                *raster = (*font_data & mask) ? color_index : 0;
                mask >>= 1;
                *raster += (*font_data & mask) ? high_nibble_color : 0;
                mask >>= 1;
                raster++;
            }
            font_data++;
        }
    }

    for (i = 0; i < GLYPH_CACHE_SLOTS; i++)
    {
        if (g_glyph_cache[i].raw == 0)
        {
            break;
        }
    }

    if (i == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_glyph_cache[i].raw = code;
    prim = emit_glyph_sprite(prim, ot, i, palette);

    g_glyph_upload_x = (i % GLYPH_CACHE_COLUMNS) * (GLYPH_SIZE / 4);
    g_glyph_upload_y = i & GLYPH_CACHE_ROW_MASK;

    setWH(&rect, GLYPH_SIZE / 4, GLYPH_ROWS);
    rect.x = g_glyph_upload_x + GLYPH_VRAM_X;
    rect.y = g_glyph_upload_y;

    LoadImage(&rect, (u_long*)g_glyph_raster_cursor);
    DrawSync(0);

    g_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return prim;
}
