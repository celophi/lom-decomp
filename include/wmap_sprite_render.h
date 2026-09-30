#ifndef WMAP_SPRITE_RENDER_H
#define WMAP_SPRITE_RENDER_H

#include "common.h"

/** @brief Sprite actor: animation state (see wmap_step_actor_animation) and shading. */
typedef struct
{
    s16 unknown_00;
    /** @brief Resource the actor draws from; -1 for none. */
    s16 resource_index;
    u8 unknown_04[2];
    s8 scale_index;
    u8 unknown_07[7];
    s16 sequence;
    s16 previous_sequence;
    u8 unknown_12[2];
    u8* cursor;
    u8* sequence_start;
    /** @brief Current animation frame: a part count followed by WmapSpritePart records. */
    u8* frame_data;
    s16 remaining;
    s16 target_shade;
    s16 shade;
    s16 shade_step;
    u8 unknown_28[4];
} WmapSpriteActor;

/** @brief Animation resource slot of a sprite actor. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapAnimationSlot;

/** @brief GTE screen coordinate, read as one packed word or as two halves. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x;
        s16 y;
    } point;
} WmapScreenPosition;

void wmap_draw_actor_sprite(WmapSpriteActor* actor, s32 screen_position, s32 texture_index, s32 ot_index, s32 variant);

#endif
