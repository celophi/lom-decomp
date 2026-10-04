/* CLOAD's copy of the shared glyph-cache text drawing (see include/common/glyph_cache.h). */
#include "internal/cload_internal.h"

/**
 * @brief g_glyph_chart_page_base as CLOAD reaches it.
 */
#if defined(VERSION_JP)
#define CLOAD_GLYPH_CHART_PAGE_BASE ((u8*)cload_emit_icon_highlight_strip + 0x70)
#else
#define CLOAD_GLYPH_CHART_PAGE_BASE ((u8*)cload_load_icon_resources + 0x2C)
#endif

#include "../../common/glyph_cache/draw_signed_decimal.inc.c"
#include "../../common/glyph_cache/draw_hex_byte.inc.c"
#include "../../common/glyph_cache/draw_cached_text.inc.c"
#include "../../common/glyph_cache/render_cached_glyph.inc.c"
#include "../../common/glyph_cache/emit_glyph_sprite.inc.c"
#include "../../common/glyph_cache/begin_glyph_cache_frame.inc.c"
#include "../../common/glyph_cache/evict_unused_glyphs.inc.c"
#include "../../common/glyph_cache/reset_glyph_cache.inc.c"

/**
 * @brief Translate a string into Shift-JIS through the character chart, two
 *        output bytes per character, and null-terminate the result.
 * @param out Destination buffer.
 * @param in Null-terminated source string.
 * @note Same code as the shared expand_text_glyph_codes, with the page base
 *       spelled as CLOAD_GLYPH_CHART_PAGE_BASE.
 */
void expand_text_glyph_codes(u8* out, u8* in)
{
    s32 index;
    s16 lead;

    while (1)
    {
        u8 c = *in;

        if (c == 0)
        {
            break;
        }
        if (ENCODED_TEXT_IS_DOUBLE_BYTE_LEAD(c))
        {
            u32 column;
            s32 row;
            u8* first_byte;
            u8* second_byte;

            column = in[1];
            row = column >> 4;
            column &= 0xF;
            first_byte = CLOAD_GLYPH_CHART_PAGE_BASE + column * 2;
            first_byte += row * GLYPH_CHART_ROW_BYTES;
            lead = *in;
            first_byte += lead * GLYPH_CHART_PAGE_BYTES;
            *out = *first_byte;
            out++;
            column = in[1];
            row = column >> 4;
            column &= 0xF;
            second_byte = CLOAD_GLYPH_CHART_PAGE_BASE + 1 + column * 2;
            second_byte += row * GLYPH_CHART_ROW_BYTES;
            lead = *in;
            second_byte += lead * GLYPH_CHART_PAGE_BYTES;
            *out = *second_byte;
            out++;
            in += 2;
        }
        else if (c > GLYPH_TEXT_FIRST_PRINTABLE)
        {
            lead = *in;
            index = lead - GLYPH_TEXT_FIRST_PRINTABLE;
            *out = g_glyph_single_byte_chart[(index / GLYPH_CHART_COLUMNS) * GLYPH_CHART_ROW_BYTES + (index & (GLYPH_CHART_COLUMNS - 1)) * 2];
            out++;
            lead = *in;
            index = lead - GLYPH_TEXT_FIRST_PRINTABLE;
            *out = g_glyph_single_byte_chart[(index / GLYPH_CHART_COLUMNS) * GLYPH_CHART_ROW_BYTES + (index & (GLYPH_CHART_COLUMNS - 1)) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = g_glyph_single_byte_chart[0];
            out++;
            *out = g_glyph_single_byte_chart[1];
            out++;
            in += 1;
        }
    }
    *out = 0;
}
