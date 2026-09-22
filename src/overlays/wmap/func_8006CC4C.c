/* Partial WMAP decompilation: 99.607840% (gcc280_g0). */
#include "common.h"

/** @brief Animation fields within a world-map actor. */
typedef struct
{
    u8 unknown_00[0xE];
    s16 sequence;
    s16 previous_sequence;
    u8 unknown_12[2];
    u8 *cursor;
    u8 *sequence_start;
    u8 *frame_data;
    s16 remaining;
} WmapAnimation;

/** @brief Resource slot containing the animation data block. */
typedef struct
{
    s32 unknown_00;
    u8 *data;
} WmapAnimationResource;

/**
 * @brief Advance the selected animation, wrapping when its end marker is reached.
 * @param actor Actor animation state and frame pointers.
 * @param resource Animation block containing sequence and frame offsets.
 * @return New frame index, or -1 when the current frame is still active.
 */
s32 func_8006CC4C(WmapAnimation *actor, WmapAnimationResource *resource)
{
    u8 *data;
    s16 *offsets;
    u8 *cursor;
    s32 result;
    s32 frame;
    s32 offset;

    result = -1;
    offsets = (s16 *)resource->data;
    data = (u8 *)offsets;
    if (actor->previous_sequence != actor->sequence)
    {
        actor->previous_sequence = (u16)actor->sequence;
        offset = offsets[actor->sequence];
        actor->remaining = 1;
        cursor = (u8 *)offsets + offset;
        actor->sequence_start = cursor;
        actor->cursor = cursor;
    }
    if (actor->remaining != 255)
    {
        actor->remaining = (u16)actor->remaining - 1;
    }
    if (actor->remaining == 0)
    {
        cursor = actor->cursor;
        frame = cursor[0];
        actor->remaining = cursor[1];
        if (frame == 255)
        {
            cursor = actor->sequence_start;
            actor->cursor = cursor;
            frame = cursor[0];
            actor->remaining = cursor[1];
        }
        actor->cursor += 4;
        actor->frame_data = data + ((s16 *)(frame * 2 + data))[32];
        result = frame;
    }
    return result;
}
