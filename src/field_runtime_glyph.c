#include "field_runtime.h"

/*
 * The two glyph helpers below only match at -O1, unlike the rest of
 * field_runtime_text.c (GCC 2.6.0 -O2). They are split out here so the build
 * can compile just this object at -O1 via a target-specific flag override; the
 * compiler (GCC 2.6.0) and maspsx version (ASPSX 2.34) are otherwise identical.
 * GCC 2.6.0 and 2.7.2 emit byte-identical code for these two at -O1.
 */

/**
 * @brief One glyph sprite primitive as written into the field render list.
 * @note Layout mirrors field_runtime_text.c's FieldGlyphPrimitive; duplicated
 *       here because this is a separate translation unit.
 */
typedef struct FieldGlyphPrimitive
{
    u32 tag;
    u32 color_code;
    u32 position;
    u32 texcoord_clut;
    u32 size;
} FieldGlyphPrimitive;

/**
 * @brief One ordering-table entry (16-byte payload followed by the link tag).
 */
typedef struct FieldOrderingTableEntry
{
    u8 pad[16];
    u32 tag;
} FieldOrderingTableEntry;

extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;
extern s32 g_text_clut_base;
extern u8 g_hex_digit_table[17];
/* Declared volatile in this unit: required to reproduce the glyph codegen. */
extern FieldGlyphPrimitive* volatile g_field_primitive_cursor;
extern FieldOrderingTableEntry* g_field_current_render_half;

void field_draw_glyph(u8 character, s32 ot_depth, s32 clut_offset);

/**
 * @brief Emit one font glyph sprite and link it into the ordering table.
 *
 * Space (0x20) draws nothing and only advances the cursor. The font atlas cell
 * is selected from the character code: the low nibble picks the U column and
 * the high nibble (offset from 0x20) picks the V row. The primitive is written
 * through g_field_primitive_cursor, then chained into g_field_current_render_half
 * at the requested ordering-table depth, preserving the tag's length byte.
 *
 * @param character   Character code whose atlas cell is selected.
 * @param ot_depth    Ordering-table depth used to link the glyph primitive.
 * @param clut_offset Offset added to the base font CLUT identifier.
 * @see decomp.me (70%) https://decomp.me/scratch/1IyXY
 * @note WIP 95.08% under GCC 2.6.0 -O1 (maspsx 2.34); residual is
 *       register allocation plus two addi/addiu rows.
 * @note gcc 2.7.2 (not CDK) produces equivalent asm
 */
void field_draw_glyph(u8 character, s32 ot_depth, s32 clut_offset)
{
    s32 masked_char;
    s32 ot_byte_offset;
    FieldGlyphPrimitive* primitive;
    FieldGlyphPrimitive* primitive2;
    u32 mask_lo24;
    s32 cursor_y;
    u32 clut;
    s32 cursor_x;
    u32 packed_pos;
    u32 u_lo;
    u32 uv_word;
    u32 row;
    u32 uv_dead; /* Declared but unused; retained to match register/stack layout. */
    u32 mask_hi8;
    FieldOrderingTableEntry* ot_entry;
    u32 dummy;
    u32 packed_len;
    FieldGlyphPrimitive* next;

    masked_char = character & 0xff;
    ot_byte_offset = ot_depth * 4;
    if (masked_char != 0x20)
    {
        primitive = g_field_primitive_cursor;
        primitive->color_code = 0x66808080;
        mask_lo24 = 0x00ffffff;
        cursor_y = g_text_cursor_y;
        packed_len = g_text_clut_base;
        clut = (u16)packed_len;
        cursor_x = g_text_cursor_x;
        packed_pos = (cursor_y << 16) | cursor_x;
        clut += clut_offset;
        clut <<= 16;
        u_lo = (masked_char & 0xf) << 3;
        uv_word = clut | (u_lo + 0x80);
        row = ((u32)((masked_char - 0x20) & 0xf0)) >> 1;
        row += 0xe0;
        row <<= 8;
        row |= uv_word;
        primitive->texcoord_clut = row;
        primitive->position = packed_pos;
        mask_hi8 = 0xff000000;
        primitive2 = g_field_primitive_cursor;
        primitive2->size = 0x80008;
        clut = (u32)g_field_current_render_half;
        dummy = *((volatile u32*)primitive2);
        ot_byte_offset += clut;
        ot_entry = (FieldOrderingTableEntry*)ot_byte_offset;
        clut = ot_entry->tag; /* Reuse of clut for the OT tag is required to match. */
        packed_len = 0x04000000;
        primitive2->tag = (clut & mask_lo24) | packed_len;
        next = (FieldGlyphPrimitive*)(((u8*)primitive2) + 0x14);
        cursor_x = ((u32)primitive2) & mask_lo24;
        g_field_primitive_cursor = next;
        clut &= mask_hi8;
        ot_entry->tag = clut | ((u32)cursor_x);
    }
    g_text_cursor_x += 8;
}

/**
 * @brief Draw the low byte of a value as two hexadecimal glyphs.
 * @param value       Value whose low two nibbles are rendered.
 * @param x           Starting X coordinate of the text cursor.
 * @param y           Starting Y coordinate of the text cursor.
 * @param ot_depth    Ordering-table depth used for both glyphs.
 * @param clut_offset Offset added to the base font CLUT identifier.
 * @see decomp.me (63%) https://decomp.me/scratch/7XlDl
 * @note WIP under GCC 2.6.0 -O1 (maspsx 2.34). The unusual temporaries
 *       and empty conditionals are retained for the current partial match.
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
