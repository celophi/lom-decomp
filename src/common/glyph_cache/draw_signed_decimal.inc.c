/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Render a signed decimal value as cached-glyph text, suppressing
 *        leading zeros and prefixing a minus glyph when negative.
 * @param prim      Current primitive cursor.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Signed value to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment Text alignment mode passed to draw_cached_text.
 * @return The updated primitive cursor.
 */
void* draw_signed_decimal(void* prim, u_long* ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    buf[1] = g_glyph_decimal_digits[magnitude / 10000];
    buf[2] = g_glyph_decimal_digits[(magnitude % 10000) / 1000];
    buf[3] = g_glyph_decimal_digits[(magnitude % 1000) / 100];
    buf[4] = g_glyph_decimal_digits[(magnitude % 100) / 10];
    buf[5] = g_glyph_decimal_digits[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == GLYPH_PAIR_ZERO)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = GLYPH_PAIR_MINUS;
    }
    prim = draw_cached_text(prim, ot, (u8*)&buf[first_digit], x, y, palette, alignment);
    return prim;
}
