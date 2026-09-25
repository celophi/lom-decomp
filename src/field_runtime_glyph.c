/**
 * @file
 * @brief Field font helpers preserved in their original assembly form.
 * 
 * For these two functions, the original source is not known. 
 * After asking smart people in the community, the best guess is that it could have been
 * a custom .LIB compiled with something other than GCC, or it could have been handwritten assembly.
 * Also, it appears right before SDK code, and these don't seem to be referenced in any running game code,
 * so maybe they existed for the purpose of debugging.
 *
 * Supporting Evidence:
 * The glyph helper uses two trapping addi instructions.
 * The hex helper uses two trapping add instructions for digit-table addresses.
 * 
 * These differ from the non-trapping additions expected from the GCC versions tested for this code. 
 * The hex helper also loads its fifth argument before allocating its frame and 
 * leaves an unused saved-register slot at sp+0x38.
 * Its 0x40-byte frame resembles the neighboring clamped hex helper, 
 * which uses that slot for s4 and keeps a local digit-table copy absent here.
 *
 * C variants of the hex helper were tested with IDO 5.3 (-O1/-O2), IDO 7.1,
 * and PlayStation MWCC 2.44.14 (CodeWarrior Release 4, -O0 through -O4).
 * 
 * None reproduced its original stack layout. 
 * This does not rule out either compiler family with other versions, flags, or source forms.
 *
 * These are just clues, and aren't proof of any origin, 
 * but since none of the compilers tested produce anything like the original, 
 * it is marked as handwritten assembly for now.
 * 
 * The assembly preserves the original instructions and delay slots, 
 * with a disabled C reference below to describe the behavior.
 */
#include "field_runtime.h"

/**
 * @brief Draw one 8x8 glyph, or advance past a space, at the text cursor.
 * @param character Character code selecting the font atlas cell.
 * @param ot_depth Ordering-table depth used to link the sprite.
 * @param clut_offset Offset added to the base font CLUT identifier.
 * @note Always advances the X cursor by eight pixels.
 * @see decomp.me (prior C/inline-asm version, 100% in-tree) https://decomp.me/scratch/1IyXY
 * @see decomp.me (prior C/inline-asm version, 100% in-tree) https://decomp.me/scratch/BhIpy
 */
void field_draw_glyph(u8 character, s32 ot_depth, s32 clut_offset);

__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel field_draw_glyph\n"

    /* A space only advances the cursor; each ordering-table entry is a word. */
    "    andi    $t1, $a0, 0xFF\n"
    "    ori     $v0, $zero, 0x20\n"
    "    beq     $t1, $v0, .Lfield_glyph_advance\n"
    "     sll    $t2, $a1, 2\n"

    /* Start a textured sprite with neutral RGB modulation. */
    "    lui     $v1, %hi(g_field_primitive_cursor)\n"
    "    lw      $v1, %lo(g_field_primitive_cursor)($v1)\n"
    "    lui     $v0, 0x6680\n"
    "    ori     $v0, $v0, 0x8080\n"
    "    sw      $v0, 0x04($v1)\n"
    "    lui     $a3, 0x00FF\n"
    "    ori     $a3, $a3, 0xFFFF\n"

    /* Pack the cursor position and select the font palette. */
    "    lui     $a0, %hi(g_text_cursor_y)\n"
    "    lw      $a0, %lo(g_text_cursor_y)($a0)\n"
    "    lui     $t0, %hi(g_text_clut_base)\n"
    "    lhu     $t0, %lo(g_text_clut_base)($t0)\n"
    "    lui     $a1, %hi(g_text_cursor_x)\n"
    "    lw      $a1, %lo(g_text_cursor_x)($a1)\n"
    "    sll     $a0, $a0, 16\n"
    "    or      $a0, $a0, $a1\n"
    "    addu    $t0, $t0, $a2\n"
    "    sll     $t0, $t0, 16\n"

    /* Each atlas cell is 8x8; retain the original trapping UV additions. */
    "    andi    $a1, $t1, 0x0F\n"
    "    sll     $a1, $a1, 3\n"
    "    addi    $a1, $a1, 0x80\n"
    "    or      $a1, $t0, $a1\n"
    "    addiu   $v0, $t1, -0x20\n"
    "    andi    $v0, $v0, 0xF0\n"
    "    srl     $v0, $v0, 1\n"
    "    addi    $v0, $v0, 0xE0\n"
    "    sll     $v0, $v0, 8\n"
    "    or      $v0, $a1, $v0\n"
    "    sw      $v0, 0x0C($v1)\n"
    "    sw      $a0, 0x08($v1)\n"

    /* Finish the sprite and locate the OT link at render_half + 0x10 + depth*4. */
    "    lui     $a2, 0xFF00\n"
    "    lui     $a0, %hi(g_field_primitive_cursor)\n"
    "    lw      $a0, %lo(g_field_primitive_cursor)($a0)\n"
    "    lui     $v0, 0x0008\n"
    "    ori     $v0, $v0, 0x0008\n"
    "    sw      $v0, 0x10($a0)\n"
    "    lui     $v0, %hi(g_field_current_render_half)\n"
    "    lw      $v0, %lo(g_field_current_render_half)($v0)\n"
    "    lw      $v1, 0x00($a0)\n"
    "    addu    $t2, $t2, $v0\n"
    "    lw      $v0, 0x10($t2)\n"

    /* Prepend a four-word GPU packet, preserving the OT tag's upper byte. */
    "    lui     $v1, 0x0400\n"
    "    and     $a1, $v0, $a3\n"
    "    or      $v1, $a1, $v1\n"
    "    sw      $v1, 0x00($a0)\n"
    "    addiu   $v1, $a0, 0x14\n"
    "    and     $a0, $a0, $a3\n"
    "    lui     $at, %hi(g_field_primitive_cursor)\n"
    "    sw      $v1, %lo(g_field_primitive_cursor)($at)\n"
    "    and     $v0, $v0, $a2\n"
    "    or      $v0, $v0, $a0\n"
    "    sw      $v0, 0x10($t2)\n"

    /* Spaces and visible glyphs both consume one character cell. */
    ".Lfield_glyph_advance:\n"
    "    lui     $v0, %hi(g_text_cursor_x)\n"
    "    lw      $v0, %lo(g_text_cursor_x)($v0)\n"
    "    nop\n"
    "    addiu   $v0, $v0, 8\n"
    "    lui     $at, %hi(g_text_cursor_x)\n"
    "    sw      $v0, %lo(g_text_cursor_x)($at)\n"
    "    jr      $ra\n"
    "     nop\n"
    "endlabel field_draw_glyph\n"
    ".set reorder\n"
    ".set at\n"
);

/**
 * @brief Draw the low byte as two hexadecimal glyphs, including a leading zero.
 * @param value Value whose low two nibbles are rendered.
 * @param x Starting X coordinate of the text cursor.
 * @param y Starting Y coordinate of the text cursor.
 * @param ot_depth Ordering-table depth used for both glyphs.
 * @param clut_offset Offset added to the base font CLUT identifier.
 * @note Leaves the cursor at (x + 16, y).
 * @see decomp.me (prior C version, 67.69% in-tree) https://decomp.me/scratch/7XlDl
 */
void field_draw_hex_byte_masked(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset);

__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel field_draw_hex_byte_masked\n"

    /* Fetch the stack argument before allocating the original 0x40-byte frame. */
    "    lw      $v0, 0x10($sp)\n"
    "    addiu   $sp, $sp, -0x40\n"
    "    sw      $s0, 0x28($sp)\n"
    "    sw      $s1, 0x2C($sp)\n"
    "    sw      $s2, 0x30($sp)\n"
    "    sw      $s3, 0x34($sp)\n"
    "    sw      $ra, 0x3C($sp)\n"

    /* Set the cursor; s0/s1/s2/s3 retain value/depth/CLUT offset/digit table. */
    "    lui     $s3, %hi(g_hex_digit_table)\n"
    "    addiu   $s3, $s3, %lo(g_hex_digit_table)\n"
    "    lui     $at, %hi(g_text_cursor_x)\n"
    "    sw      $a1, %lo(g_text_cursor_x)($at)\n"
    "    lui     $at, %hi(g_text_cursor_y)\n"
    "    sw      $a2, %lo(g_text_cursor_y)($at)\n"
    "    addu    $a1, $a3, $zero\n"
    "    addu    $a2, $v0, $zero\n"
    "    addu    $s2, $v0, $zero\n"
    "    addu    $s0, $a0, $zero\n"

    /* Draw the high nibble; preserve depth in the call's delay slot. */
    "    ori     $v0, $zero, 0xF0\n"
    "    and     $v0, $v0, $s0\n"
    "    srl     $v0, $v0, 4\n"
    "    add     $v0, $v0, $s3\n"
    "    lb      $a0, 0x00($v0)\n"
    "    jal     field_draw_glyph\n"
    "     addu   $s1, $a1, $zero\n"

    /* Draw the low nibble with the same depth and palette offset. */
    "    ori     $v0, $zero, 0x0F\n"
    "    and     $v0, $v0, $s0\n"
    "    addu    $a1, $s1, $zero\n"
    "    add     $v0, $v0, $s3\n"
    "    lb      $a0, 0x00($v0)\n"
    "    jal     field_draw_glyph\n"
    "     addu   $a2, $s2, $zero\n"

    "    lw      $ra, 0x3C($sp)\n"
    "    lw      $s3, 0x34($sp)\n"
    "    lw      $s2, 0x30($sp)\n"
    "    lw      $s1, 0x2C($sp)\n"
    "    lw      $s0, 0x28($sp)\n"
    "    addiu   $sp, $sp, 0x40\n"
    "    jr      $ra\n"
    "     nop\n"
    "endlabel field_draw_hex_byte_masked\n"
    ".set reorder\n"
    ".set at\n"
);

#if 0
/* Behavioral C reference for PS1 RAM; not compiled or intended to match. */
#include "gpu_packet.h"

#define FIELD_GLYPH_SIZE 8
#define FIELD_TEXT_OT_OFFSET 4
#define GPU_LINK_ADDRESS_MASK 0x00FFFFFFu
#define GPU_LINK_LENGTH_MASK 0xFF000000u

/** @brief Four-word sprite payload preceded by its GPU linked-list tag. */
typedef struct FieldGlyphPrimitive
{
    u32 tag;
    u32 color_code;
    u32 position;
    u32 texcoord_clut;
    u32 size;
} FieldGlyphPrimitive;

extern s32 g_text_cursor_x;
extern s32 g_text_cursor_y;
extern s32 g_text_clut_base;
extern u8 g_hex_digit_table[17];
extern FieldGlyphPrimitive* g_field_primitive_cursor;
extern FieldRenderHalf* g_field_current_render_half;

/**
 * @brief C reference for glyph emission and cursor advancement.
 * @param character Character code selecting the font atlas cell.
 * @param ot_depth Ordering-table depth used to link the sprite.
 * @param clut_offset Offset added to the base font CLUT identifier.
 */
void field_draw_glyph(u8 character, s32 ot_depth, s32 clut_offset)
{
    FieldGlyphPrimitive* primitive;
    u_long* ot_entry;
    u32 clut;
    u32 u;
    u32 v;

    if (character != ' ')
    {
        primitive = g_field_primitive_cursor;
        clut = (u32)(u16)g_text_clut_base + (u32)clut_offset;
        u = 0x80 + (character & 0x0F) * FIELD_GLYPH_SIZE;
        v = 0xE0 + (((u32)character - 0x20) & 0xF0) / 2;

        primitive->color_code = 0x66000000u | GPU_TINT_NEUTRAL;
        primitive->position = ((u32)g_text_cursor_y << 16) | (u32)g_text_cursor_x;
        /* Keep the full V result: the original packs it without a byte mask. */
        primitive->texcoord_clut = (clut << 16) | (v << 8) | u;
        primitive->size = (FIELD_GLYPH_SIZE << 16) | FIELD_GLYPH_SIZE;

        ot_entry = &g_field_current_render_half->ordering_table[ot_depth + FIELD_TEXT_OT_OFFSET];
        primitive->tag = 0x04000000u | (*ot_entry & GPU_LINK_ADDRESS_MASK);
        *ot_entry = (*ot_entry & GPU_LINK_LENGTH_MASK) | ((u32)primitive & GPU_LINK_ADDRESS_MASK);
        g_field_primitive_cursor = primitive + 1;
    }
    g_text_cursor_x = (s32)((u32)g_text_cursor_x + FIELD_GLYPH_SIZE);
}

/**
 * @brief C reference for drawing the low byte as two hexadecimal digits.
 * @param value Value whose low two nibbles are rendered.
 * @param x Starting X coordinate of the text cursor.
 * @param y Starting Y coordinate of the text cursor.
 * @param ot_depth Ordering-table depth used for both glyphs.
 * @param clut_offset Offset added to the base font CLUT identifier.
 */
void field_draw_hex_byte_masked(s32 value, s32 x, s32 y, s32 ot_depth, s32 clut_offset)
{
    g_text_cursor_x = x;
    g_text_cursor_y = y;

    field_draw_glyph(g_hex_digit_table[(value & 0xF0) >> 4], ot_depth, clut_offset);
    field_draw_glyph(g_hex_digit_table[value & 0x0F], ot_depth, clut_offset);
}
#endif
