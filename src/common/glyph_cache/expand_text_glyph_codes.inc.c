/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Translate a source string into internal glyph codes via the single-
 *        and double-byte character tables, writing two output bytes per input
 *        character and null-terminating the result.
 * @param out Destination glyph-code buffer.
 * @param in  Null-terminated source string.
 * @note Lead bytes 0x19-0x1F start a two-byte code whose second byte's nibbles
 *       pick the row and column of one 16-row page of the double-byte table;
 *       bytes above GLYPH_TEXT_FIRST_PRINTABLE index the single-byte table by their offset from it; any other
 *       byte becomes the table's first (blank) glyph.
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
            first_byte = g_glyph_chart_page_base + column * 2;
            first_byte += row * GLYPH_CHART_ROW_BYTES;
            lead = *in;
            first_byte += lead * GLYPH_CHART_PAGE_BYTES;
            *out = *first_byte;
            out++;
            column = in[1];
            row = column >> 4;
            column &= 0xF;
            second_byte = g_glyph_chart_page_base + 1 + column * 2;
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
