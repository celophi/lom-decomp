#include "field_runtime.h"
#include "gpu_packet.h"

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
extern FieldGlyphPrimitive* g_field_primitive_cursor;
extern FieldOrderingTableEntry* g_field_current_render_half;

/**
 * @brief Build the glyph texcoord+CLUT word and store it into the primitive.
 *
 * Both glyph UV offsets are emitted as trapping signed @c addi instructions
 * (@c addi u,0x80 for the column and @c addi v,0xe0 for the row), which the
 * game's disassembly flags as handwritten - GCC never emits a trapping
 * @c addi for a plain @c +constant, so the original devs hand-wrote these two
 * instructions. Reproducing that is a sanctioned exception to the no-inline-asm
 * rule for this function. At the very least, @c addi is indeed handwritten or 
 * part of a macro and this is verified by the decomp community.
 *
 * @note The extra operands on the two asm statements (the @c "$6" clobber on
 *       the column addi and the @c "=&r"(clut_word) output plus the two
 *       @c "m"(*(p)) memory operands on the row addi) do not correspond to any
 *       handwritten instruction; they only steer GCC 2.6.0's register
 *       allocator onto the exact coloring the target uses. Dropping them keeps
 *       the two @c addi but regresses the match to ~97% (pure register
 *       permutation). A clobber-free source shape that colors naturally has not
 *       been found.
 * @param p          Glyph primitive being written (used as the store target).
 * @param ch         Masked character code selecting the atlas cell.
 * @param clut_word  Shifted CLUT word ORed into the low half of the result.
 * @param u_work     Scratch that receives the column offset (@c (ch&0xf)<<3).
 * @param packed_work Scratch that receives the final texcoord+CLUT word.
 */
#define FIELD_SET_GLYPH_UV(p, ch, clut_word, u_work, packed_work) ({ \
    u32 _v; \
    u32 _uv; \
    s32 _field_v; \
    (u_work) = ((ch) & 0xf) << 3; \
    __asm__ ("addi %0, %1, %2" : "=r"(u_work) : "r"(u_work), "i"(0x80) : "$6", "memory"); \
    (_uv) = (clut_word) | (u32)(u_work); \
    (_v) = ((u32)(((ch) - 0x20) & 0xf0)) >> 1; \
    _field_v = (_v); \
    __asm__ ("addi %0, %2, %3" \
             : "=r"(_field_v), "=&r"(clut_word) \
             : "r"(_field_v), "i"(0xe0), "m"(*(p)), "m"(*(p)) \
             : "memory"); \
    (_v) = _field_v; \
    (_v) <<= 8; \
    (packed_work) = (_uv) | (_v); \
    SET_SPRT_UV_CLUT_WORD((p), (packed_work)); \
})

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
 * @see decomp.me (100%) https://decomp.me/scratch/1IyXY OR https://decomp.me/scratch/BhIpy
 * @note Matches 100% in-tree under GCC 2.6.0 -O1 (maspsx 2.34); the linked
 *       scratch predates the register-allocation fix and still reads 97.06%.
 *       The two handwritten UV @c addi offsets and the allocator-steering
 *       operands that pin the coloring both live in @ref FIELD_SET_GLYPH_UV.
 * @note gcc 2.7.2 (not CDK) produces equivalent asm.
 */
void field_draw_glyph(u8 character, s32 ot_depth, s32 clut_offset)
{
    s32 masked_char;
    s32 ot_byte_offset;
    FieldGlyphPrimitive* primitive;
    volatile FieldGlyphPrimitive* primitive2;
    u32 mask_lo24;
    s32 cursor_y;
    u32 clut;
    u32 packed_pos;
    u32 u_lo;
    u32 uv_dead; /* Reused as scratch for the UV word and the render-half base. */
    u32 dummy;
    u32 packed_len;
    FieldGlyphPrimitive* next;

    masked_char = character & 0xff;
    ot_byte_offset = ot_depth << 2;
    if (masked_char != 0x20)
    {
        primitive = g_field_primitive_cursor;
        SET_BGR0_PACKED(primitive, 0x66000000u | GPU_TINT_NEUTRAL);
        mask_lo24 = 0x00ffffff;
        cursor_y = g_text_cursor_y;
        packed_len = g_text_clut_base;
        clut = (u16)packed_len;
        ot_depth = g_text_cursor_x;
        packed_pos = (cursor_y << 16) | ot_depth;
        clut += clut_offset;
        clut <<= 16;
        FIELD_SET_GLYPH_UV(primitive, masked_char, clut, ot_depth, uv_dead);
        SET_SPRT_XY0_WORD(primitive, packed_pos);
        clut_offset = (s32)0xff000000;
        primitive2 = g_field_primitive_cursor;
        SET_SPRT_WH_PACKED(primitive2, 8, 8);
        uv_dead = (u32)g_field_current_render_half;
        next = (FieldGlyphPrimitive*)primitive2->tag;
        ot_byte_offset += uv_dead;
        packed_len = *(u32*)(ot_byte_offset + 0x10);
        dummy = 0x04000000;
        ot_depth = (s32)(packed_len & mask_lo24);
        dummy |= (u32)ot_depth;
        primitive2->tag = dummy;
        next = (FieldGlyphPrimitive*)(primitive2 + 1);
        u_lo = ((u32)primitive2) & mask_lo24;
        g_field_primitive_cursor = next;
        packed_len &= (u32)clut_offset;
        *(u32*)(ot_byte_offset + 0x10) = packed_len | u_lo;
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
