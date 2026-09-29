/*
 * Shared glyph-cache function; see include/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "glyph_cache.h"

/**
 * @brief Render a multibyte string through the glyph cache: measure it, apply
 *        left/center/right alignment, then emit one cached glyph per character
 *        and terminate the primitive list.
 * @param prim      Current primitive cursor.
 * @param ot        Ordering table the glyphs are linked into.
 * @param text      Null-terminated multibyte string to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment FIELD_TEXT_ALIGN_LEFT, FIELD_TEXT_ALIGN_RIGHT or FIELD_TEXT_ALIGN_CENTER; every glyph is GLYPH_SIZE wide.
 * @return The updated primitive cursor past the terminator.
 */
void* draw_cached_text(void* prim, u_long* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8* cursor;
    s32 count;
    u16 code;
    u8* scan;
    DR_TPAGE* draw_mode;

    cursor = text;
    count = 0;
    if (*cursor >= GLYPH_TEXT_FIRST_PRINTABLE)
    {
        for (scan = cursor; *scan >= GLYPH_TEXT_FIRST_PRINTABLE; scan++)
        {
            if (*scan >= SJIS_LEAD_MIN)
            {
                scan++;
            }
            count++;
        }
    }

    switch (alignment)
    {
    case FIELD_TEXT_ALIGN_RIGHT:
        x -= count * GLYPH_SIZE;
        break;
    case FIELD_TEXT_ALIGN_CENTER:
        x -= count * (GLYPH_SIZE / 2);
        break;
    case FIELD_TEXT_ALIGN_LEFT:
    default:
        break;
    }
    g_glyph_line_start_x = x;
    g_glyph_cursor_x = x;
    g_glyph_cursor_y = y;

    while (1)
    {
        if (*cursor == ' ')
        {
            cursor++;
            g_glyph_cursor_x += GLYPH_SIZE;
            continue;
        }
        if (*cursor >= SJIS_LEAD_MIN)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if (*cursor < GLYPH_TEXT_FIRST_PRINTABLE)
            {
                break;
            }
            if (*cursor >= '0' && *cursor < SJIS_LEAD_MIN)
            {
                code = *cursor + GLYPH_SJIS_ALNUM_OFFSET;
                cursor++;
            }
            else
            {
                code = *cursor + GLYPH_SJIS_SYMBOL_OFFSET;
                cursor++;
            }
        }
        prim = render_cached_glyph(prim, ot, code, palette);
    }

    draw_mode = prim;
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 0, GLYPH_VRAM_X, 0));
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}
