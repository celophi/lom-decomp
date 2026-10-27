#include "common/glyph_cache.h"

/**
 * @brief Fully reset the glyph cache: zero all cache entries and clear the
 *        entire glyph raster buffer.
 */
void reset_glyph_cache(void)
{
    s32 i;

    for (i = GLYPH_CACHE_SLOTS - 1; i >= 0; i--)
    {
        g_glyph_cache[i].raw = 0;
    }

    for (i = 0; i < GLYPH_RASTER_BUFFER_BYTES; i++)
    {
        g_glyph_raster_buffer[i] = 0;
    }
}
