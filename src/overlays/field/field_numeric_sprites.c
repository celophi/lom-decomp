#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"
#include "sdk/memory.h"

/* Forward declarations for intra-TU forward calls (real definition signatures). */
void *func_800AD42C(SPRT *sprite, s32 *ordering_table, s32 texture_index, s32 *packed_position, s32 flags);
SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 sprite_count);

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
void *func_800AD208(s32 *ordering_table, u8 *packet_cursor, s32 value, s32 digit_count, u16 *position, s32 flags)
{
    s32 base_x;
    s32 digit;
    s32 divisor;
    s32 digit_index;

    divisor = 1;
    digit_index = divisor;
    if (digit_index < digit_count)
    {
        do
        {
            divisor *= 10;
            digit_index += 1;
        } while (digit_index < digit_count);
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
        *position += digit_index * 8;
        if (digit_index < digit_count)
        {
            do
            {
                digit = value / divisor;
                packet_cursor = func_800AD42C(packet_cursor, ordering_table, digit, position, flags);
                value -= digit * divisor;
                digit_index += 1;
                *position += 8;
                divisor /= 10;
            } while (digit_index < digit_count);
        }
    }
    else
    {
        base_x = *position - 8;
        *position = base_x + digit_count * 8;
        packet_cursor = func_800AD42C(packet_cursor, ordering_table, value, position, flags);
        *position += 8;
    }
    setlen((DR_TPAGE *)packet_cursor, 1);
    ((DR_TPAGE *)packet_cursor)->code[0] = 0xE1000007;
    addPrim(ordering_table, (DR_TPAGE *)packet_cursor);
    return (DR_TPAGE *)packet_cursor + 1;
}

/**
 * @brief Build and link a textured sprite primitive.
 * @param sprite Sprite primitive to populate.
 * @param ordering_table Ordering-table tag to link the sprite into.
 * @param texture_index Index used to select the packed texture coordinates.
 * @param packed_position Packed x/y position copied into the sprite.
 * @param flags Low bits select the clut; bit 7 requests the follow-up sprite pass.
 * @return Pointer just past the generated primitive data.
 */
void *func_800AD42C(SPRT *sprite, s32 *ordering_table, s32 texture_index, s32 *packed_position, s32 flags)
{
    s32 position;
    s32 masked;
    s16 clut;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    position = *packed_position;
    SET_SPRT_UV0_PACKED(sprite, texture_index * 8 + 0x2000);
    SET_SPRT_WH_WORD(sprite, 0x80008);
    masked = flags & 0x7F;
    SET_SPRT_XY0_WORD(sprite, position);
    switch (masked)
    {
    case 1:
        clut = 0x7AC8;
        break;
    case 0:
        clut = 0x7B03;
        break;
    case 2:
        clut = 0x7AC9;
        break;
    default:
        sprite->clut = 0x7B03;
        goto after_switch;
    }
    sprite->clut = clut;
after_switch:
    addPrim(ordering_table, sprite);
    sprite++;
    if (flags & 0x80)
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
 * @param flags Low bits select the clut; bit 7 requests the shadow pass.
 * @return Pointer just past the generated primitive data.
 */
void *func_800AD524(SPRT *sprite, s32 *ordering_table, s32 value, s32 *packed_position, s32 flags)
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
    SET_SPRT_WH_WORD(sprite, 0x80008);
    palette_selector = flags & 0x7F;
    switch (palette_selector)
    {
    case 1:
        sprite_value = 0x7AC8;
        break;
    case 0:
        sprite_value = 0x7B03;
        break;
    case 2:
        sprite_value = 0x7AC9;
        break;
    default:
        sprite->clut = 0x7B03;
        goto after_switch;
    }
    sprite->clut = sprite_value;
after_switch:
    addPrim(ordering_table, sprite);
    sprite++;
    if (flags & 0x80)
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
SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 sprite_count)
{
    SPRT *source = sprite_cursor;
    s32 direction = 0;
    s32 count;
    SPRT *sprite;

    do
    {
        bcopy((u8 *)(source - sprite_count), (u8 *)sprite_cursor, sprite_count * sizeof(SPRT));
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
