#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 count);

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
