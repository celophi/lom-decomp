/** @file field_numeric_sprites.c
 * @brief Draw decimal numbers from 8x8 digit sprites.
 */

#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"

/** @brief Width and height of one digit glyph, in pixels. */
#define FIELD_DIGIT_SIZE 8

/** @brief Default (style 0) digit palette. */
#define FIELD_DIGIT_CLUT_NORMAL getClut(48, 492)

/** @brief Style 1 digit palette. */
#define FIELD_DIGIT_CLUT_STYLE1 getClut(128, 491)

/** @brief Style 2 digit palette. */
#define FIELD_DIGIT_CLUT_STYLE2 getClut(144, 491)

/** @brief Flag bit that adds a black outline around the digits. */
#define FIELD_DIGIT_OUTLINE 0x80

/** @brief Draw-mode command restoring texture page 7 after the digits. */
#define FIELD_DIGIT_DRAW_MODE 0xE1000007

void* func_800AD42C(SPRT* sprite, s32* ordering_table, s32 texture_index, s32* packed_position, s32 flags);
SPRT* func_800AD658(s32* ordering_table, SPRT* sprite_cursor, s32 sprite_count);

/**
 * @brief Draw a right-aligned decimal value and append its draw-mode primitive.
 * @param ordering_table Ordering-table entry receiving the emitted primitives.
 * @param packet_cursor First free primitive-buffer address.
 * @param value Decimal value to draw.
 * @param digit_count Number of digit positions available for the value.
 * @param position Packed x/y position; the x coordinate advances as digits are emitted.
 * @param flags Digit rendering flags passed to the sprite builder.
 * @return First free primitive-buffer address after the emitted primitives.
 */
void* func_800AD208(s32* ordering_table, u8* packet_cursor, s32 value, s32 digit_count, u16* position, s32 flags)
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
            packet_cursor = func_800AD42C(packet_cursor, ordering_table, digit, position, flags);
            value -= digit * divisor;
            *position += FIELD_DIGIT_SIZE;
            divisor /= 10;
        }
    }
    else
    {
        base_x = *position - FIELD_DIGIT_SIZE;
        *position = base_x + digit_count * FIELD_DIGIT_SIZE;
        packet_cursor = func_800AD42C(packet_cursor, ordering_table, value, position, flags);
        *position += FIELD_DIGIT_SIZE;
    }
    setlen((DR_TPAGE*)packet_cursor, 1);
    ((DR_TPAGE*)packet_cursor)->code[0] = FIELD_DIGIT_DRAW_MODE;
    addPrim(ordering_table, (DR_TPAGE*)packet_cursor);
    return (DR_TPAGE*)packet_cursor + 1;
}

/**
 * @brief Build and link a textured sprite primitive.
 * @param sprite Sprite primitive to populate.
 * @param ordering_table Ordering-table tag to link the sprite into.
 * @param texture_index Index used to select the packed texture coordinates.
 * @param packed_position Packed x/y position copied into the sprite.
 * @param flags Low seven bits select the palette; FIELD_DIGIT_OUTLINE adds the outline sprites.
 * @return Pointer just past the generated primitive data.
 */
void* func_800AD42C(SPRT* sprite, s32* ordering_table, s32 texture_index, s32* packed_position, s32 flags)
{
    s32 position;
    s32 masked;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    position = *packed_position;
    SET_SPRT_UV0_PACKED(sprite, texture_index * 8 + 0x2000);
    SET_SPRT_WH_PACKED(sprite, FIELD_DIGIT_SIZE, FIELD_DIGIT_SIZE);
    masked = flags & 0x7F;
    SET_SPRT_XY0_WORD(sprite, position);
    switch (masked)
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
    addPrim(ordering_table, sprite);
    sprite++;
    if (flags & FIELD_DIGIT_OUTLINE)
    {
        sprite = func_800AD658(ordering_table, sprite, 1);
    }
    return sprite;
}

/**
 * @brief Emit a decimal digit sprite and optionally append its shadow sprites.
 * @param sprite Sprite primitive to populate.
 * @param ordering_table Ordering-table tag to link the sprite into.
 * @param value Value whose decimal units digit selects the glyph.
 * @param packed_position Packed x/y position copied into the sprite.
 * @param flags Low seven bits select the palette; FIELD_DIGIT_OUTLINE adds the outline sprites.
 * @return Pointer just past the generated primitive data.
 */
void* func_800AD524(SPRT* sprite, s32* ordering_table, s32 value, s32* packed_position, s32 flags)
{
    s32 palette_selector;
    s16 sprite_value;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    SET_SPRT_XY0_WORD(sprite, *packed_position);
    sprite_value = (value % 10) * 8;
    if (value >= 10)
    {
        SET_SPRT_UV0_PACKED(sprite, sprite_value + 0x2800);
    }
    else
    {
        SET_SPRT_UV0_PACKED(sprite, sprite_value + 0x2000);
    }
    SET_SPRT_WH_PACKED(sprite, FIELD_DIGIT_SIZE, FIELD_DIGIT_SIZE);
    palette_selector = flags & 0x7F;
    switch (palette_selector)
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
    addPrim(ordering_table, sprite);
    sprite++;
    if (flags & FIELD_DIGIT_OUTLINE)
    {
        sprite = func_800AD658(ordering_table, sprite, 1);
    }
    return sprite;
}

/**
 * @brief Add four black one-pixel-offset copies of a sprite group.
 * @param ordering_table Ordering-table entry receiving the copied sprites.
 * @param sprite_cursor First free sprite after the template group.
 * @param sprite_count Number of sprites in the template group.
 * @return First free sprite after the four copied groups.
 */
SPRT* func_800AD658(s32* ordering_table, SPRT* sprite_cursor, s32 sprite_count)
{
    SPRT* source = sprite_cursor;
    s32 direction = 0;
    s32 count;
    SPRT* sprite;

    do
    {
        bcopy((u8*)(source - sprite_count), (u8*)sprite_cursor, sprite_count * sizeof(SPRT));
        count = 0;
        if (sprite_count > 0)
        {
            sprite = sprite_cursor;
            do
            {
                sprite->b0 = 0;
                sprite->g0 = 0;
                sprite->r0 = 0;
                switch (direction)
                {
                case 0:
                    sprite->x0++;
                    break;
                case 1:
                    sprite->x0--;
                    break;
                case 2:
                    sprite->y0++;
                    break;
                default:
                    sprite->y0--;
                    break;
                }
                sprite++;
                count++;
                addPrim(ordering_table, sprite_cursor);
                sprite_cursor++;
            } while (count < sprite_count);
        }
        direction++;
    } while (direction < 4);

    return sprite_cursor;
}
