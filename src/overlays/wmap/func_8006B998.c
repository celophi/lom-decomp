/* Partial WMAP decompilation: 89.228070% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    u8 pad_00[14];
    s16 sequence;
    s16 previous_sequence;
    u8 pad_12[2];
    u8 *cursor;
    u8 *sequence_start;
    u8 *frame_data;
    s16 remaining;
    u8 pad_22[10];
} WmapActor;

typedef struct
{
    s16 state, angle;
    s32 velocity, radius;
    s16 lifetime, z;
    s32 angular_velocity;
} WmapMotion;

typedef struct
{
    s32 field_00;
    u8 *data;
} WmapResource;
extern WmapActor D_800D9268[];
extern WmapResource D_80139988[];
extern void func_80066F9C(void *, s32, s32, s32, s32);

typedef struct
{
    SVECTOR position;
    SVECTOR velocity;
} WmapMovingPoint;

/** @brief Advance and project animated particles, bouncing at the supplied bounds. */
void func_8006B998(s32 first, s32 end, WmapMovingPoint *points, s32 frame, s32 depth)
{
    s32 screen_position;
    s32 i;
    WmapMovingPoint *point;
    WmapMovingPoint *bounds;
    WmapActor *actor;
    u8 *data;
    s16 *offsets;
    u8 *cursor;
    s32 offset;
    s32 animation_frame;

    point = &points[first];
    bounds = &points[end];
    for (i = first; i < end; i++, point++)
    {
        actor = &D_800D9268[i];
        gte_ldv0(&point->position);
        gte_rtps();
        offsets = (s16 *)D_80139988[i].data;
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
            actor->remaining--;
        }
        if (actor->remaining == 0)
        {
            cursor = actor->cursor;
            animation_frame = cursor[0];
            actor->remaining = cursor[1];
            if (animation_frame == 255)
            {
                cursor = actor->sequence_start;
                actor->cursor = cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
            }
            actor->cursor += 4;
            actor->frame_data = data + ((s16 *)(animation_frame * 2 + data))[32];
        }
        gte_stsxy(&screen_position);
        func_80066F9C(actor, screen_position, frame, depth, 0);
        point->position.vx += point->velocity.vx;
        point->position.vy += point->velocity.vy;
        point->position.vz += point->velocity.vz;
        if (point->position.vx < bounds->position.vx || bounds->velocity.vx < point->position.vx)
        {
            point->velocity.vx = -point->velocity.vx;
        }
        if (point->position.vy < bounds->position.vy || bounds->velocity.vy < point->position.vy)
        {
            point->velocity.vy = -point->velocity.vy;
        }
        if (point->position.vz < bounds->position.vz || bounds->velocity.vz < point->position.vz)
        {
            point->velocity.vz = -point->velocity.vz;
        }
    }
}
