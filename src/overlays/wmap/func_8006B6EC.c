/* Partial WMAP decompilation: 88.614040% (gcc280_g0). */
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
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Project moving particles, advance their animation, and draw by depth. */
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth)
{
    SVECTOR position;
    s32 screen_position;
    s32 i;
    WmapMotion *motion;
    WmapActor *actor;
    u8 *data;
    u8 *cursor;
    s16 *offsets;
    s32 animation_frame;
    s32 offset;

    for (i = first; i < end; i++)
    {
        actor = &D_800D9268[i];
        motion = &D_801AFBD0[i];
        if (motion->state != 0)
        {
            position.vx = ((motion->radius >> 6) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->radius >> 6) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->z;
            gte_ldv0(&position);
            gte_rtps();
            motion->z += z_step;
            motion->radius += motion->velocity;
            motion->angle = (motion->angle + motion->angular_velocity) & 4095;
            gte_stsxy(&screen_position);
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
            if ((u32)((u16)motion->angle - 1025) < 2047U)
            {
                func_80066F9C(actor, screen_position, frame, depth + 2, 0);
            }
            else
            {
                func_80066F9C(actor, screen_position, frame, depth - 2, 0);
            }
            motion->lifetime--;
            if (motion->lifetime == 0)
            {
                motion->state = 0;
            }
        }
    }
}
