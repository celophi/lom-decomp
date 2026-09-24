#include "field_runtime.h"
#include "sdk/strings.h"
#include "sdk/memory.h"

#define DIGIT_TO_ASCII(d) ((d) + '0')
#define HEX_DIGIT_TABLE_SIZE 17
#define FIELD_GLYPH_ADVANCE 8

extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;
extern u8 g_hex_digit_table[HEX_DIGIT_TABLE_SIZE];

void field_draw_glyph(s32 character, s32 ot_depth, s32 clut_offset);

/**
 * @brief Draw a string at a fixed screen position using the field text engine.
 * @param str      Null-terminated ASCII string to draw.
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut_offset Font CLUT/color variant, added to g_text_clut_base.
 * @see decomp.me (100%) https://decomp.me/scratch/mLcZm
 */
void field_draw_string(u8* str, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    s32 length;
    s32 i;

    g_text_cursor_x = x;
    g_text_cursor_y = y;
    length = strlen((const char*)str);
    for (i = 0; i < length; i++)
    {
        field_draw_glyph(str[i], ot_depth, clut_offset);
    }
}

/**
 * @brief Draw a value as a fixed 2-digit unsigned number (no leading-zero
 *        suppression) using the field text engine.
 * @param value    Number to draw (0-99 expected).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut_offset Font CLUT/color variant, added to g_text_clut_base.
 * @see decomp.me (100%) https://decomp.me/scratch/ENN60
 */
void field_draw_uint2(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    s32 digit;
    s32 tens_value;

    g_text_cursor_x = x;
    g_text_cursor_y = y;
    digit = value / 10;
    tens_value = digit * 10;
    field_draw_glyph(DIGIT_TO_ASCII(digit), ot_depth, clut_offset);
    digit = value - tens_value;
    field_draw_glyph(DIGIT_TO_ASCII(digit), ot_depth, clut_offset);
}

/**
 * @brief Draw a value as a right-aligned 3-digit unsigned number, blanking
 *        (advancing the cursor without drawing) any leading zero digits.
 *        The ones digit is always drawn.
 * @param value    Number to draw (0-999 expected).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut_offset Font CLUT/color variant, added to g_text_clut_base.
 * @see decomp.me (100%) https://decomp.me/scratch/RGs7q
 */
void field_draw_uint3(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    s32 blanking;
    s32 digit;
    s32 digit_value;

    blanking = 1;
    digit = value / 100;
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    digit_value = digit * 100;
    digit = DIGIT_TO_ASCII(digit);
    if (digit == DIGIT_TO_ASCII(0))
    {
        g_text_cursor_x = x + FIELD_GLYPH_ADVANCE;
    }
    else
    {
        field_draw_glyph(digit, ot_depth, clut_offset);
        blanking = 0;
    }

    value -= digit_value;
    digit = value / 10;
    digit_value = digit * 10;
    digit = DIGIT_TO_ASCII(digit);
    if (blanking == 0)
    {
        field_draw_glyph(digit, ot_depth, clut_offset);
    }
    else if (digit == DIGIT_TO_ASCII(0))
    {
        g_text_cursor_x += FIELD_GLYPH_ADVANCE;
    }
    else
    {
        field_draw_glyph(digit, ot_depth, clut_offset);
    }

    field_draw_glyph(DIGIT_TO_ASCII(value - digit_value), ot_depth, clut_offset);
}

/**
 * @brief Draw a value clamped to a byte as 2 hex digits.
 * @param value    Number to draw (clamped to 0-0xFF).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut_offset Font CLUT/color variant, added to g_text_clut_base.
 * @see decomp.me (100%) https://decomp.me/scratch/S8Wds
 */
void field_draw_hex_byte_clamped(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    u8 digit_table[HEX_DIGIT_TABLE_SIZE];
    s32 clamped;
    u32 high_nibble;
    u32 high_value;

    clamped = value;
    memcpy(digit_table, g_hex_digit_table, sizeof(digit_table));
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    if ((clamped & 0xFFFFU) > 0xFF)
    {
        clamped = 0xFF;
    }
    high_nibble = (clamped & 0xFFFFU) >> 4;
    high_value = high_nibble << 4;
    field_draw_glyph(digit_table[high_nibble], ot_depth, clut_offset);
    field_draw_glyph(digit_table[(u16)(clamped - high_value)], ot_depth, clut_offset);
}

/**
 * @brief Draw a value masked to 16 bits as 4 hex digits, most significant first.
 * @param value    Number to draw (masked to 0-0xFFFF).
 * @param x        Starting X of the text cursor (g_text_cursor_x).
 * @param y        Starting Y of the text cursor (g_text_cursor_y).
 * @param ot_depth Ordering-table depth/priority each glyph is linked into.
 * @param clut_offset Font CLUT/color variant, added to g_text_clut_base.
 * @see decomp.me (100%) https://decomp.me/scratch/4kW8K
 */
void field_draw_hex_word(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    u8 digit_table[HEX_DIGIT_TABLE_SIZE];
    u32 word;

    memcpy(digit_table, g_hex_digit_table, sizeof(digit_table));
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    word = value & 0xFFFF;
    field_draw_glyph(digit_table[(word >> 12) & 0xF], ot_depth, clut_offset);
    field_draw_glyph(digit_table[(word >> 8) & 0xF], ot_depth, clut_offset);
    field_draw_glyph(digit_table[(word >> 4) & 0xF], ot_depth, clut_offset);
    field_draw_glyph(digit_table[word & 0xF], ot_depth, clut_offset);
}
