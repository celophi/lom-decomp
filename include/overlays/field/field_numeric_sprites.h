#ifndef FIELD_NUMERIC_SPRITES_H
#define FIELD_NUMERIC_SPRITES_H

#include "common.h"

#include "common/vector.h"

struct SPRT;

/** @brief Flag bits selecting the digit palette (0, 1 or 2). */
#define FIELD_DIGIT_STYLE_MASK 0x7F

/** @brief Flag bit that adds a black outline around the digits. */
#define FIELD_DIGIT_OUTLINE 0x80

/** @brief field_draw_sprite_glyph glyph of the separator between two counts, as in "3/12". */
#define FIELD_DIGIT_GLYPH_SEPARATOR 11

/**
 * @brief Draw a right-aligned decimal value, then set the digit texture page.
 * @param ordering_table Ordering-table entry receiving the emitted primitives.
 * @param packet_cursor First free primitive-buffer address.
 * @param value Decimal value to draw.
 * @param digit_count Number of digit positions available for the value.
 * @param position Screen position; the x coordinate advances as digits are emitted.
 * @param flags Digit style and FIELD_DIGIT_OUTLINE, passed to each digit.
 * @return First free primitive-buffer address after the emitted primitives.
 */
void* field_draw_sprite_number(u_long* ordering_table, void* packet_cursor, s32 value, s32 digit_count, Vec2s* position, s32 flags);

/**
 * @brief Build and link the sprite of one digit or symbol glyph.
 * @param sprite Sprite primitive to populate.
 * @param ordering_table Ordering-table entry to link the sprite into.
 * @param glyph Glyph 0-19: 0-9 are the digits, 10-19 the symbols in the next texture row.
 * @param position Screen position of the glyph.
 * @param flags Digit style in the low seven bits; FIELD_DIGIT_OUTLINE adds the outline sprites.
 * @return First free primitive-buffer address after the emitted primitives.
 */
void* field_draw_sprite_glyph(struct SPRT* sprite, u_long* ordering_table, s32 glyph, Vec2s* position, s32 flags);

/**
 * @brief Add a black outline: four copies of a sprite group, each one pixel off.
 * @param ordering_table Ordering-table entry receiving the copied sprites.
 * @param sprite_cursor First free sprite, right after the template group.
 * @param sprite_count Number of sprites in the template group.
 * @return First free sprite after the four copied groups.
 */
struct SPRT* field_add_sprite_outline(u_long* ordering_table, struct SPRT* sprite_cursor, s32 sprite_count);

#endif
