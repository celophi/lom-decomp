#ifndef _DECOMP8_H
#define _DECOMP8_H

#include "common.h"
#include "psyq/strings.h"
#include "psyq/memory.h"

/** @brief Convert a 0-9 digit value to its ASCII character code. */
#define DIGIT_TO_ASCII(d) ((d) + 0x30)

/** @brief Packed five-word SPRT primitive emitted for one field-font glyph. */
typedef struct FieldGlyphPrimitive
{
    u32 tag;
    u32 color_code;
    u32 position;
    u32 texcoord_clut;
    u32 size;
} FieldGlyphPrimitive;

/** @brief Ordering-table entry viewed through the field render-half base. */
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

void field_draw_string(u8* str, s32 x, s32 y, s32 ot_depth, s32 clut_offset);
void field_draw_uint2(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset);
void field_draw_uint3(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset);
void field_draw_hex_byte_clamped(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset);
void field_draw_hex_word(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset);
extern void field_draw_glyph(s32 character, s32 ot_depth, s32 clut_offset);
void field_draw_hex_byte_masked(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset);
void card_resource_noop_hook(void);

#endif
