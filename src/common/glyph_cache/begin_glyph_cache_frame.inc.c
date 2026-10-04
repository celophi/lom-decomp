/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Start a new glyph cache frame: rewind the raster cursor and clear each
 *        cache entry's per-frame "used" flag (the high half-word).
 */
void begin_glyph_cache_frame(void)
{
    s32 i;

    g_glyph_raster_cursor = g_glyph_raster_buffer;
    for (i = 0; i < GLYPH_CACHE_SLOTS; i++)
    {
        g_glyph_cache[i].raw &= GLYPH_CACHE_CODE_MASK;
    }
}
