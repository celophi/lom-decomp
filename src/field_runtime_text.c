#include "field_runtime.h"

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

/**
 * @brief Emit one 8x8 field-font glyph sprite and link it into the ordering table.
 *
 * Spaces do not allocate a primitive, but every character advances the global
 * text cursor by eight pixels. The unusual temporaries and empty conditionals
 * are retained for the current partial assembly match.
 * @param character Character code whose atlas cell is selected.
 * @param ot_depth Ordering-table depth used to link the glyph primitive.
 * @param clut_offset Offset added to the base font CLUT identifier.
 * @see decomp.me (70%) https://decomp.me/scratch/1IyXY
 * @note This may have been built by a different compiler or optimization level.
 */
void field_draw_glyph(s32 character, s32 ot_depth, s32 clut_offset)
{
    int packed_uv_clut;
    u32 character_code;
    FieldGlyphPrimitive* primitive;
    u32 previous_ot_tag;
    u8* render_half_bytes;
    int ot_byte_offset;
    u8* primitive_bytes;
    u32 packed_clut;
    FieldOrderingTableEntry* ot_entry;
    u32 packed_uv;
    int tag_length_mask;
    int tag_length_mask_value;
    int primitive_tag_length;
    u32 working_value;
    int masked_character;
    u32 primitive_address;
    masked_character = character & 0xFF;
    character_code = masked_character;
    ot_byte_offset = ot_depth * 4;
    if (character_code != 0x20)
    {

        working_value = character_code;
        g_field_primitive_cursor->color_code = 0x66808080;
        packed_uv = (u16)g_text_clut_base;
        if (!g_text_cursor_x)
        {
        }
        packed_clut = (packed_uv + clut_offset) << 16;
        packed_uv = ((working_value & 0xF) * 8) + 0x80;
        if (!g_text_cursor_y)
        {
        }
        packed_uv = packed_clut | packed_uv;
        primitive = g_field_primitive_cursor; /* Load-bearing explicit cursor copy. */
        primitive_bytes = (u8*)primitive;
        working_value = (((character_code - 0x20) & ((short)0xF0)) >> 1) + 0xE0;
        packed_uv_clut = packed_uv | (working_value << 8);
        g_field_primitive_cursor->texcoord_clut = packed_uv_clut;
        g_field_primitive_cursor->position = (g_text_cursor_y << 16) | g_text_cursor_x;
        tag_length_mask = (tag_length_mask_value = 0xFF000000);
        render_half_bytes = (u8*)g_field_current_render_half;
        g_field_primitive_cursor->size = 0x80008;
        primitive_tag_length = 0x04000000;
        ot_entry = (FieldOrderingTableEntry*)(render_half_bytes + ot_byte_offset);
        previous_ot_tag = ot_entry->tag;
        g_field_primitive_cursor->tag = primitive_tag_length | (ot_entry->tag & 0x00FFFFFF);
        primitive_address = (u32)primitive;
        g_field_primitive_cursor = (FieldGlyphPrimitive*)(primitive_bytes + 0x14);
        working_value = previous_ot_tag & tag_length_mask;
        ot_entry->tag = working_value | (primitive_address & 0x00FFFFFF); /* Preserve the OT tag length byte. */
    }
    g_text_cursor_x += 8;
}

/**
 * @brief Draw the low byte of a value as two hexadecimal glyphs.
 * @param value Value whose low two nibbles are rendered.
 * @param x Starting X coordinate of the text cursor.
 * @param y Starting Y coordinate of the text cursor.
 * @param ot_depth Ordering-table depth used for both glyphs.
 * @param clut_offset Offset added to the base font CLUT identifier.
 * @see decomp.me (63%) https://decomp.me/scratch/7XlDl
 * @note This may have been built by a different compiler or optimization level.
 *
 * Local objdiff results (working/func_800166C8/): 40.8% at -O2, 67.2% at -O1
 * with this source. The remaining diffs look like non-gcc codegen: trapping
 * add (not addu) for the table indexing, ori+and for plain immediate masks,
 * the 5th arg loaded at 0x10(sp) BEFORE the sp decrement, and s0..s3,ra save
 * order. The glyph helper also contains a handwritten trapping addi. Suspected
 * LSI-style toolchain, as in the PSX BIOS/kernel.
 *
 * The target asm passes ot_depth to both field_draw_glyph calls
 * (addu s1,a1 / addu a1,s1 around the calls), so the second call below
 * passes ot_depth; an earlier revision passed the high-nibble mask instead.
 */
void field_draw_hex_byte_masked(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    u8 unused_stack_table[17]; /* Unused, but required for the -0x40 frame size. */
    short style_args_zero;
    signed char* digit_ptr;
    s32 clut_offset_copy;
    int ot_depth_copy;
    signed char* digit_table = g_hex_digit_table;
    signed char* digit_table_copy;
    short style_args_zero_copy;
    int working_mask;
    unsigned short low_nibble_mask;
    int high_nibble_mask;
    int depth_is_zero;
    s32 working_value;
    s32 codegen_temp0;
    unsigned int digit_index;
    s32 codegen_temp2;
    s32 codegen_temp3;
    clut_offset_copy = clut_offset;
    g_text_cursor_x = x;
    high_nibble_mask = ot_depth;
    working_mask = ot_depth;
    working_value = !high_nibble_mask;
    working_value = working_value && (!high_nibble_mask);
    low_nibble_mask = 0xf;
    depth_is_zero = working_value != 0;
    high_nibble_mask = 0xF0;
    ot_depth_copy = working_mask;
    style_args_zero = (!clut_offset) && (!ot_depth_copy);
    codegen_temp0 = (0, clut_offset);
    digit_index = working_mask;
    codegen_temp2 = clut_offset;
    codegen_temp3 = ot_depth;
    if (depth_is_zero)
    {
    }
    if ((!g_hex_digit_table) && (!g_hex_digit_table))
    {
    }
    g_text_cursor_y = y;
    style_args_zero_copy = style_args_zero;
    ot_depth_copy = (working_mask = high_nibble_mask);
    if (style_args_zero_copy)
    {
    }
    working_value = value;
    digit_index = (high_nibble_mask & ((int)working_value)) >> 3;
    field_draw_glyph(digit_table[digit_index >> 1], ot_depth, clut_offset_copy);
    digit_table_copy = digit_table;
    field_draw_glyph(*(digit_ptr = &digit_table_copy[low_nibble_mask & working_value]), ot_depth, clut_offset_copy);
}
