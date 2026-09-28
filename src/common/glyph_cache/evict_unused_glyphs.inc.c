/*
 * Shared glyph-cache function; see include/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "glyph_cache.h"

/**
 * @brief Evict cache entries not touched this frame by zeroing any slot whose
 *        "used" flag (GLYPH_CACHE_USED) is clear.
 */
void evict_unused_glyphs(void)
{
    s32 i;

    for (i = 0; i < GLYPH_CACHE_SLOTS; i++)
    {
        if (!(g_glyph_cache[i].raw & GLYPH_CACHE_USED))
        {
            g_glyph_cache[i].raw = 0;
        }
    }
}
