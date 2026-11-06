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

/** @brief Scale-table entry that draws a sprite at full size. */
#define WMAP_SPRITE_SCALE_FULL 15
/** @brief Sprite shade that draws the texture unmodified; higher values fade it back down. */
#define WMAP_SPRITE_SHADE_NEUTRAL 0x80

/** @brief An animation resource starts with this many sequence offsets, followed by the frame offsets. */
#define WMAP_ANIMATION_SEQUENCES 32
/** @brief Animation duration that never counts down. */
#define WMAP_ANIMATION_HOLD 255
/** @brief End-of-sequence marker in a sequence's frame list. */
#define WMAP_ANIMATION_LOOP 255

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

/** @brief Signed coordinate pair. */
typedef struct
{
    s16 x;
    s16 y;
} WmapPoint;

void wmap_draw_actor_sprite(WmapSpriteActor* actor, s32 screen_position, s32 texture_index, s32 ot_index, s32 variant);

/** @brief Number of sprite actors, animation slots and motion slots. */
#define WMAP_ACTOR_COUNT 256

/** @brief Sprite actor pool; actor i animates from g_wmap_actor_animations[i]. */
extern WmapSpriteActor g_wmap_sprite_actors[WMAP_ACTOR_COUNT];

/** @brief Animation slot of each sprite actor, parallel to g_wmap_sprite_actors. */
extern WmapAnimationSlot g_wmap_actor_animations[WMAP_ACTOR_COUNT];

/**
 * @brief The run of sprite actors an effect uses, starting at actor @p first; index it with [i].
 * @param first Index of the block's first actor in g_wmap_sprite_actors.
 */
#define WMAP_ACTOR_BLOCK(first) (&g_wmap_sprite_actors[first])

/** @brief Sprite actor and animation slot of the travel vehicle. */
extern WmapSpriteActor g_wmap_vehicle_actor;
extern WmapAnimationSlot g_wmap_vehicle_animation;

/**
 * @brief Advance a sprite actor's animation by one frame and point it at the current frame's data.
 * @param actor Sprite actor to animate.
 * @param resource Animation resource: WMAP_ANIMATION_SEQUENCES sequence offsets, then frame offsets.
 * @note wmap_step_actor_animation does the same and also returns the new frame.
 */
static inline void wmap_advance_actor_animation(WmapSpriteActor* actor, u8* resource)
{
    s16* offsets;
    u8* data;
    u8* cursor;
    s32 frame;

    offsets = (s16*)resource;
    data = (u8*)offsets;

    if (actor->previous_sequence != actor->sequence)
    {
        s32 offset;
        u8* sequence_cursor;

        actor->previous_sequence = actor->sequence;
        offset = offsets[actor->sequence];
        actor->remaining = 1;
        sequence_cursor = (u8*)offsets + offset;
        actor->sequence_start = sequence_cursor;
        actor->cursor = sequence_cursor;
    }

    if (actor->remaining != WMAP_ANIMATION_HOLD)
    {
        actor->remaining--;
    }

    if (actor->remaining == 0)
    {
        cursor = actor->cursor;
        frame = cursor[0];
        actor->remaining = cursor[1];

        if (frame == WMAP_ANIMATION_LOOP)
        {
            cursor = actor->sequence_start;
            actor->cursor = cursor;
            frame = cursor[0];
            actor->remaining = cursor[1];
        }

        actor->cursor += 4;
        actor->frame_data = data + offsets[frame + WMAP_ANIMATION_SEQUENCES];
    }
}

#endif
