/*
 * Shared glyph-cache function; see include/common/glyph_cache.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/glyph_cache.h"

/**
 * @brief Convert game-encoded text to a null-terminated Shift-JIS string.
 * @param destination Buffer with room for two bytes per character and a terminator.
 * @param source Null-terminated string in the game's text encoding.
 * @note Codes 0x19-0x1F consume two source bytes, even when the second is zero.
 *       Other nonzero codes at or below the space code become blank glyphs.
 */
void expand_text_glyph_codes(u8* destination, u8* source)
{
    s32 glyph_index;
    s16 character_code;

    while (1)
    {
        u8 source_byte = *source;

        if (source_byte == 0)
        {
            break;
        }
        if (ENCODED_TEXT_IS_DOUBLE_BYTE_LEAD(source_byte))
        {
            u32 column;
            s32 row;
            u8* first_glyph_byte;
            u8* second_glyph_byte;

            /* The lead byte selects a page; the next byte selects its row and column. */
            column = source[1];
            row = column >> GLYPH_CHART_ROW_SHIFT;
            column &= GLYPH_CHART_COLUMN_MASK;
            first_glyph_byte = g_glyph_chart_page_base + column * GLYPH_CHART_GLYPH_BYTES;
            first_glyph_byte += row * GLYPH_CHART_ROW_BYTES;
            character_code = *source;
            first_glyph_byte += character_code * GLYPH_CHART_PAGE_BYTES;
            *destination = *first_glyph_byte;
            destination++;
            column = source[1];
            row = column >> GLYPH_CHART_ROW_SHIFT;
            column &= GLYPH_CHART_COLUMN_MASK;
            second_glyph_byte = g_glyph_chart_page_base + 1 + column * GLYPH_CHART_GLYPH_BYTES;
            second_glyph_byte += row * GLYPH_CHART_ROW_BYTES;
            character_code = *source;
            second_glyph_byte += character_code * GLYPH_CHART_PAGE_BYTES;
            *destination = *second_glyph_byte;
            destination++;
            source += 2;
        }
        else if (source_byte > GLYPH_TEXT_FIRST_PRINTABLE)
        {
            /* Single-byte codes index the chart starting at the space glyph. */
            character_code = *source;
            glyph_index = character_code - GLYPH_TEXT_FIRST_PRINTABLE;
            *destination = g_glyph_single_byte_chart[
                (glyph_index / GLYPH_CHART_COLUMNS) * GLYPH_CHART_ROW_BYTES +
                (glyph_index & GLYPH_CHART_COLUMN_MASK) * GLYPH_CHART_GLYPH_BYTES];
            destination++;
            character_code = *source;
            glyph_index = character_code - GLYPH_TEXT_FIRST_PRINTABLE;
            *destination = g_glyph_single_byte_chart[
                (glyph_index / GLYPH_CHART_COLUMNS) * GLYPH_CHART_ROW_BYTES +
                (glyph_index & GLYPH_CHART_COLUMN_MASK) * GLYPH_CHART_GLYPH_BYTES + 1];
            destination++;
            source++;
        }
        else
        {
            /* Spaces and other control bytes use the chart's blank glyph. */
            *destination = g_glyph_single_byte_chart[0];
            destination++;
            *destination = g_glyph_single_byte_chart[1];
            destination++;
            source++;
        }
    }
    *destination = 0;
}
