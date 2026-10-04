/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Render a byte as two hex-digit glyphs via the cached-text renderer.
 * @param prim      Current primitive cursor.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Byte value to render.
 * @param x         X position.
 * @param y         Y baseline.
 * @param alignment Text alignment mode.
 */
void draw_hex_byte(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 alignment)
{
    u16 buf[3];
    u16* high_glyph;

    high_glyph = &g_glyph_hex_digits[value / 16];
    buf[0] = *high_glyph;
    buf[1] = g_glyph_hex_digits[value % 16];
    buf[2] = 0;
    draw_cached_text(prim, ot, (u8*)buf, x, y, 0, alignment);
}
