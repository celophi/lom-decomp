/*
 * Shared glyph-cache function; see include/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "glyph_cache.h"

/**
 * @brief Write a 16x16 sprite for a cached glyph at the current text cursor,
 *        mark the slot used, and advance the cursor (wrapping to the next line).
 * @param sprite     Destination sprite primitive.
 * @param ot         Ordering table the sprite is linked into.
 * @param cache_slot Glyph cache slot whose VRAM tile to sample.
 * @param palette    Unused here; the CLUT is fixed.
 * @return The primitive cursor advanced past the emitted sprite.
 */
void* emit_glyph_sprite(GlyphSprite* sprite, u_long* ot, s32 cache_slot, s32 palette)
{
    g_glyph_cache[cache_slot].raw |= GLYPH_CACHE_USED;

    setSprt16(&sprite->packet);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    setXY0(&sprite->packet, g_glyph_cursor_x, g_glyph_cursor_y);
    setUV0(&sprite->packet, (cache_slot % GLYPH_CACHE_COLUMNS) * GLYPH_SIZE, cache_slot & GLYPH_CACHE_ROW_MASK);
    sprite->packet.clut = getClut(GLYPH_CLUT_X, GLYPH_CLUT_Y);
    addPrim(ot, &sprite->packet);
    sprite++;

    g_glyph_cursor_x += GLYPH_SIZE;
    if (g_glyph_cursor_x + GLYPH_SIZE >= GLYPH_TEXT_WRAP_X)
    {
        g_glyph_cursor_x = g_glyph_line_start_x;
        g_glyph_cursor_y += GLYPH_SIZE;
    }

    return sprite;
}
