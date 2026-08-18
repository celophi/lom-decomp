#include "field_runtime.h"
#include "psyq/strings.h"
#include "psyq/memory.h"

#define DIGIT_TO_ASCII(d) ((d) + 0x30)

typedef struct FieldGlyphPrimitive
{
    u32 tag;
    u32 color_code;
    u32 position;
    u32 texcoord_clut;
    u32 size;
} FieldGlyphPrimitive;

typedef struct FieldOrderingTableEntry
{
    u8 pad[0x10];
    u32 tag;
} FieldOrderingTableEntry;

extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;
extern u8 g_hex_digit_table[17];
extern FieldGlyphPrimitive *g_field_primitive_cursor;
extern FieldOrderingTableEntry *g_field_current_render_half;
extern s32 g_text_clut_base;

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
    u8* cursor;
    u8* end;
    s32 length;
    s32 depth;

    g_text_cursor_x = x;
    g_text_cursor_y = y;
    depth = ot_depth;
    length = strlen((const char*)str);
    if (length > 0)
    {
        cursor = str;
        end = (u8*)(length + (s32)cursor);
        do
        {
            u8 character = *cursor++;
            field_draw_glyph(character, depth, clut_offset);
        } while ((s32)cursor < (s32)end);
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
    int tens_base;
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    digit = value / 10;
    tens_base = digit * 10;
    field_draw_glyph(DIGIT_TO_ASCII(digit), ot_depth, clut_offset);
    digit = value - tens_base;
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
    s32 remaining = value;
    s32 still_blanking = 1;
    s32 unused_codegen_temp;
    s32 quotient;
    s32 digit_base;
    s32 digit;
    s32 sign_copy = remaining >> 31;
    digit = remaining / 100;
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    quotient = digit;
    digit_base = quotient * 100;
    digit = DIGIT_TO_ASCII(quotient);
    if (digit == DIGIT_TO_ASCII(0))
    {
        g_text_cursor_x = x + 8;
    }
    else
    {
        field_draw_glyph(digit, ot_depth, clut_offset);
        still_blanking = 0;
    }

    remaining -= digit_base;
    quotient = remaining >> 31;
    sign_copy = quotient;
    quotient = (digit = remaining / 10);
    digit_base = quotient * 10;
    digit = DIGIT_TO_ASCII(quotient);

    if (still_blanking == 0)
    {
        field_draw_glyph(digit, ot_depth, clut_offset);
    }
    else if (digit == DIGIT_TO_ASCII(0))
    {
        g_text_cursor_x += 8;
    }
    else
    {
        field_draw_glyph(digit, ot_depth, clut_offset);
    }

    field_draw_glyph(DIGIT_TO_ASCII(remaining - digit_base), ot_depth, clut_offset);
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
    u8 digit_table[17];
    s32 clamped_value;
    u32 masked_value;
    u32 high_nibble_base;
    u32 high_nibble;
    clamped_value = value;
    memcpy(digit_table, g_hex_digit_table, 17);
    g_text_cursor_x = x;
    g_text_cursor_y = y;
    if (((u32)(clamped_value & 0xFFFF)) >= 0x100)
    {
        clamped_value = 0xFF;
    }
    masked_value = (u32)(clamped_value & 0xFFFF);
    high_nibble = masked_value >> 4;
    high_nibble_base = high_nibble << 4;
    field_draw_glyph(digit_table[high_nibble], ot_depth, clut_offset);
    field_draw_glyph(digit_table[(unsigned short)((u32)((clamped_value - ((s32)high_nibble_base)) & 0xFFFF))], ot_depth, clut_offset);
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
    /* local copy (stack area sp+0x10 to sp+0x20) */
    u8 digit_table[17];
    u32 masked_value;

    /* Copy the unaligned data using memcpy (compiles to efficient byte loop) */
    memcpy(digit_table, g_hex_digit_table, 17);

    /* match assembly order */
    g_text_cursor_x = x;
    masked_value = (u32)(value & 0xFFFF);
    g_text_cursor_y = y;

    /* Four calls using the four nibbles of the 16-bit value */
    field_draw_glyph(digit_table[(masked_value >> 12) & 0xF], ot_depth, clut_offset);
    field_draw_glyph(digit_table[(masked_value >> 8) & 0xF], ot_depth, clut_offset);
    field_draw_glyph(digit_table[(masked_value >> 4) & 0xF], ot_depth, clut_offset);
    field_draw_glyph(digit_table[masked_value & 0xF], ot_depth, clut_offset);
}
