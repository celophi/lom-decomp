#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 count);

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
