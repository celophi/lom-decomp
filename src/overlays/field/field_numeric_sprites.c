/** @file field_numeric_sprites.c
 * @brief Draw decimal numbers from 8x8 digit sprites.
 */

#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"

/** @brief Width and height of one digit glyph, in pixels. */
#define FIELD_DIGIT_SIZE 8

/** @brief Texture row (v) of the digit glyphs 0-9. */
#define FIELD_DIGIT_ROW_V 32

/** @brief Texture row (v) of the symbol glyphs 10-19. */
#define FIELD_SYMBOL_ROW_V 40

/** @brief Number of glyphs in one texture row. */
#define FIELD_GLYPHS_PER_ROW 10

/** @brief Packed u0/v0 halfword of a glyph at texel @p u in texture row @p v. */
#define FIELD_GLYPH_UV(u, v) ((u) + ((v) << 8))

/** @brief Texture page holding the digit glyphs. */
#define FIELD_DIGIT_TPAGE getTPage(0, 0, 448, 0)

/** @brief Default (style 0) digit palette. */
#define FIELD_DIGIT_CLUT_NORMAL getClut(48, 492)

/** @brief Style 1 digit palette. */
#define FIELD_DIGIT_CLUT_STYLE1 getClut(128, 491)

/** @brief Style 2 digit palette. */
#define FIELD_DIGIT_CLUT_STYLE2 getClut(144, 491)

/** @brief Flag bits selecting the digit palette (0, 1 or 2). */
#define FIELD_DIGIT_STYLE_MASK 0x7F

/** @brief Flag bit that adds a black outline around the digits. */
#define FIELD_DIGIT_OUTLINE 0x80

/** @brief Outline copies: one pixel right, left, down and up. */
enum
{
    FIELD_OUTLINE_RIGHT = 0,
    FIELD_OUTLINE_LEFT = 1,
    FIELD_OUTLINE_DOWN = 2,
    FIELD_OUTLINE_UP = 3,
    FIELD_OUTLINE_DIRECTION_COUNT = 4
};

static SPRT* field_draw_digit(SPRT* sprite, u_long* ordering_table, s32 digit, u16* position, s32 flags);
SPRT* func_800AD658(u_long* ordering_table, SPRT* sprite_cursor, s32 sprite_count);

/**
 * @brief Set the palette of a digit sprite from its style flags.
 * @param sprite Sprite to update.
 * @param flags Digit style in the low seven bits; unknown styles use the default palette.
 */
static inline void field_set_digit_clut(SPRT* sprite, s32 flags)
{
    switch (flags & FIELD_DIGIT_STYLE_MASK)
    {
    case 1:
        sprite->clut = FIELD_DIGIT_CLUT_STYLE1;
        break;
    case 0:
        sprite->clut = FIELD_DIGIT_CLUT_NORMAL;
        break;
    case 2:
        sprite->clut = FIELD_DIGIT_CLUT_STYLE2;
        break;
    default:
        sprite->clut = FIELD_DIGIT_CLUT_NORMAL;
        break;
    }
}

/**
 * @brief Draw a right-aligned decimal value, then set the digit texture page.
 * @param ordering_table Ordering-table entry receiving the emitted primitives.
 * @param packet_cursor First free primitive-buffer address.
 * @param value Decimal value to draw.
 * @param digit_count Number of digit positions available for the value.
 * @param position Packed x/y screen position; the x coordinate advances as digits are emitted.
 * @param flags Digit style and FIELD_DIGIT_OUTLINE, passed to each digit.
 * @return First free primitive-buffer address after the emitted primitives.
 */
void* func_800AD208(u_long* ordering_table, void* packet_cursor, s32 value, s32 digit_count, u16* position, s32 flags)
{
    s32 base_x;
    s32 digit;
    s32 divisor;
    s32 digit_index;

    divisor = 1;
    for (digit_index = 1; digit_index < digit_count; digit_index++)
    {
        divisor *= 10;
    }
    for (digit_index = 0; digit_index < digit_count; digit_index++)
    {
        if (value >= divisor)
        {
            break;
        }
        divisor /= 10;
    }
    if (digit_index != digit_count)
    {
        *position += digit_index * FIELD_DIGIT_SIZE;
        for (; digit_index < digit_count; digit_index++)
        {
            digit = value / divisor;
            packet_cursor = field_draw_digit(packet_cursor, ordering_table, digit, position, flags);
            value -= digit * divisor;
            *position += FIELD_DIGIT_SIZE;
            divisor /= 10;
        }
    }
    else
    {
        /* A zero value draws a single 0 in the last position. */
        base_x = *position - FIELD_DIGIT_SIZE;
        *position = base_x + digit_count * FIELD_DIGIT_SIZE;
        packet_cursor = field_draw_digit(packet_cursor, ordering_table, value, position, flags);
        *position += FIELD_DIGIT_SIZE;
    }
    /* Linked last, so the GPU draws it before the digits. */
    setDrawTPage((DR_TPAGE*)packet_cursor, 0, 0, FIELD_DIGIT_TPAGE);
    addPrim(ordering_table, (DR_TPAGE*)packet_cursor);
    return (DR_TPAGE*)packet_cursor + 1;
}

/**
 * @brief Build and link the sprite of one digit glyph.
 * @param sprite Sprite primitive to populate.
 * @param ordering_table Ordering-table entry to link the sprite into.
 * @param digit Digit 0-9 to draw.
 * @param position Packed x/y screen position, copied into the sprite as one word.
 * @param flags Digit style in the low seven bits; FIELD_DIGIT_OUTLINE adds the outline sprites.
 * @return First free sprite after the emitted primitives.
 */
static SPRT* field_draw_digit(SPRT* sprite, u_long* ordering_table, s32 digit, u16* position, s32 flags)
{
    u32 xy;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    xy = *(u32*)position;
    SET_SPRT_UV0_PACKED(sprite, FIELD_GLYPH_UV(digit * FIELD_DIGIT_SIZE, FIELD_DIGIT_ROW_V));
    SET_SPRT_WH_PACKED(sprite, FIELD_DIGIT_SIZE, FIELD_DIGIT_SIZE);
    SET_SPRT_XY0_WORD(sprite, xy);
    field_set_digit_clut(sprite, flags);
    addPrim(ordering_table, sprite);
    sprite++;
    if (flags & FIELD_DIGIT_OUTLINE)
    {
        sprite = func_800AD658(ordering_table, sprite, 1);
    }
    return sprite;
}

/**
 * @brief Build and link the sprite of one digit or symbol glyph.
 * @param sprite Sprite primitive to populate.
 * @param ordering_table Ordering-table entry to link the sprite into.
 * @param glyph Glyph 0-19: 0-9 are the digits, 10-19 the symbols in the next texture row.
 * @param position Packed x/y screen position, copied into the sprite as one word.
 * @param flags Digit style in the low seven bits; FIELD_DIGIT_OUTLINE adds the outline sprites.
 * @return First free sprite after the emitted primitives.
 */
SPRT* func_800AD524(SPRT* sprite, u_long* ordering_table, s32 glyph, u16* position, s32 flags)
{
    s16 u;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    SET_SPRT_XY0_WORD(sprite, *(u32*)position);
    u = (glyph % FIELD_GLYPHS_PER_ROW) * FIELD_DIGIT_SIZE;
    if (glyph >= FIELD_GLYPHS_PER_ROW)
    {
        SET_SPRT_UV0_PACKED(sprite, FIELD_GLYPH_UV(u, FIELD_SYMBOL_ROW_V));
    }
    else
    {
        SET_SPRT_UV0_PACKED(sprite, FIELD_GLYPH_UV(u, FIELD_DIGIT_ROW_V));
    }
    SET_SPRT_WH_PACKED(sprite, FIELD_DIGIT_SIZE, FIELD_DIGIT_SIZE);
    field_set_digit_clut(sprite, flags);
    addPrim(ordering_table, sprite);
    sprite++;
    if (flags & FIELD_DIGIT_OUTLINE)
    {
        sprite = func_800AD658(ordering_table, sprite, 1);
    }
    return sprite;
}

/**
 * @brief Add a black outline: four copies of a sprite group, each one pixel off.
 * @param ordering_table Ordering-table entry receiving the copied sprites.
 * @param sprite_cursor First free sprite, right after the template group.
 * @param sprite_count Number of sprites in the template group.
 * @return First free sprite after the four copied groups.
 */
SPRT* func_800AD658(u_long* ordering_table, SPRT* sprite_cursor, s32 sprite_count)
{
    SPRT* template_end;
    s32 direction;
    s32 index;

    template_end = sprite_cursor;
    for (direction = 0; direction < FIELD_OUTLINE_DIRECTION_COUNT; direction++)
    {
        bcopy((u8*)(template_end - sprite_count), (u8*)sprite_cursor, sprite_count * sizeof(SPRT));
        for (index = 0; index < sprite_count; index++)
        {
            sprite_cursor->b0 = 0;
            sprite_cursor->g0 = 0;
            sprite_cursor->r0 = 0;
            switch (direction)
            {
            case FIELD_OUTLINE_RIGHT:
                sprite_cursor->x0++;
                break;
            case FIELD_OUTLINE_LEFT:
                sprite_cursor->x0--;
                break;
            case FIELD_OUTLINE_DOWN:
                sprite_cursor->y0++;
                break;
            default:
                sprite_cursor->y0--;
                break;
            }
            addPrim(ordering_table, sprite_cursor);
            sprite_cursor++;
        }
    }
    return sprite_cursor;
}
