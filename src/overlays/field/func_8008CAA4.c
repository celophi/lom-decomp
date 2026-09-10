#include "common.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
/** @brief One signed X/Z point in a sampled route. */
typedef struct
{
    s16 x, z;
} FieldRoutePoint;
/** @brief Fixed-point position and route-state index in a 0x54-byte actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x2E];
    u8 index;
    u8 pad_3b[0x19];
} FieldRouteActor;
/** @brief Route count and first waypoint in a 0x23C-byte actor state. */
typedef struct
{
    u8 pad0[0x1A4];
    u16 count;
    u8 pad_1a6[6];
    s32 x, z;
    u8 pad_1b4[0x88];
} FieldRouteState;
s32 SquareRoot0(s32);
s32 func_8001CDAC(s32 *, s32 *);
void func_8008A0B0(FieldRouteActor *, s32, s32);
s32 func_8008D104(FieldRouteActor *, FieldRouteActor *);
extern FieldRouteState D_80105AE0[];
extern s8 D_8010CFE0[];

/**
 * @brief Sample an actor route into signed points and parallel heading entries.
 * @param points Output X/Z points.
 * @param remaining Requested point count, including the actor's initial position.
 * @param actor Actor whose generated route is sampled.
 * @param target Destination actor used for routing and heading calculation.
 * @param heading_offset Starting index in the shared heading array.
 * @param unused_limit Unused sixth argument retained to agree with the caller.
 * @note Coordinates are fixed point with eight fractional bits; interpolated
 *       normalized directions use twelve fractional bits.
 */
void func_8008CAA4(FieldRoutePoint *points, s32 remaining, FieldRouteActor *actor, FieldRouteActor *target,
                   s32 heading_offset, s32 unused_limit)
{
    s32 work[12];
    s32 actor_index;
    FieldRouteState *states;
    s32 heading;
    s16 *z_cursor;
    s16 *cursor;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s32 waypoint_offset;
    s32 spacing;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 point_index;
    s32 total_distance;
    s32 path_offset;
    s32 path_index;
    s32 sample_path_index;
    s32 var_v0;
    s32 var_v0_10;
    s32 var_v0_11;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 temp_a1;

    func_8008A0B0(actor, target->index, 0);
    work[0] = actor->x;
    total_distance = 0;
    work[2] = actor->z;
    path_index = 0;
    actor_index = actor->index;
    if (D_80105AE0[actor_index].count != 0)
    {
        do
        {
            waypoint_offset = path_index * 8;
            var_v0 = ((FieldRouteState *)(waypoint_offset + (actor_index * 0x23C) + (u8 *)D_80105AE0))->x - work[0];
            if (var_v0 < 0)
            {
                var_v0 += 0xFF;
            }
            work[4] = var_v0 >> 8;
            var_v0_2 = ((FieldRouteState *)(waypoint_offset + (actor->index * 0x23C) + (u8 *)D_80105AE0))->z - work[2];
            if (var_v0_2 < 0)
            {
                var_v0_2 += 0xFF;
            }
            work[6] = var_v0_2 >> 8;
            work[5] = 0;
            gte_ldlvl(&work[4]);
            gte_sqr0();
            gte_stlvnl(&work[8]);
            temp_v0 = SquareRoot0(work[8] + work[10]);
            work[0] = ((FieldRouteState *)(waypoint_offset + (actor->index * 0x23C) + (u8 *)D_80105AE0))->x;
            work[2] = ((FieldRouteState *)(waypoint_offset + (actor->index * 0x23C) + (u8 *)D_80105AE0))->z;
            path_index += 1;
            total_distance += temp_v0;
            actor_index = actor->index;
        } while (path_index < (s32)D_80105AE0[actor_index].count);
    }
    temp_a1 = func_8008D104(actor, target);
    var_v1 = actor->x;
    if (var_v1 < 0)
    {
        var_v1 += 0xFF;
    }
    points->x = (s16)(var_v1 >> 8);
    var_v0_3 = actor->z;
    if (var_v0_3 < 0)
    {
        var_v0_3 += 0xFF;
    }
    remaining -= 1;
    points->z = (s16)(var_v0_3 >> 8);
    cursor = (s16 *)((u8 *)points + 4);
    *(heading_offset + D_8010CFE0) = temp_a1;
    point_index = 1;
    if ((total_distance / remaining) < 3)
    {
    loop_13:
        *(point_index + heading_offset + D_8010CFE0) = temp_a1;
        var_v0_4 = actor->x;
        if (var_v0_4 < 0)
        {
            var_v0_4 += 0xFF;
        }
        cursor[0] = (s16)(var_v0_4 >> 8);
        var_v0_5 = actor->z;
        if (var_v0_5 < 0)
        {
            var_v0_5 += 0xFF;
        }
        cursor[1] = (s16)(var_v0_5 >> 8);
        cursor += 2;
        remaining -= 1;
        point_index += 1;
        if (remaining != 0)
        {
            if ((total_distance / remaining) >= 3)
            {
                goto block_19;
            }
            goto loop_13;
        }
    }
    else
    {
    block_19:
        work[0] = actor->x;
        work[2] = actor->z;
        spacing = total_distance / remaining;
        sample_path_index = 0;
        states = D_80105AE0;
        if (states[actor->index].count != 0)
        {
            path_offset = 0 * 8;
        loop_21:
            z_cursor = cursor + 1;
        loop_22:
            var_v0_6 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->x - work[0];
            if (var_v0_6 < 0)
            {
                var_v0_6 += 0xFF;
            }
            work[4] = var_v0_6 >> 8;
            var_v0_7 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->z - work[2];
            if (var_v0_7 < 0)
            {
                var_v0_7 += 0xFF;
            }
            work[6] = var_v0_7 >> 8;
            work[5] = 0;
            if (spacing >= SquareRoot0(func_8001CDAC(&work[4], &work[8])))
            {
                var_v0_8 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->x;
                if (var_v0_8 < 0)
                {
                    var_v0_8 += 0xFF;
                }
                *cursor = (s16)(var_v0_8 >> 8);
                var_v0_9 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->z;
                if (var_v0_9 < 0)
                {
                    var_v0_9 += 0xFF;
                }
                *z_cursor = (s16)(var_v0_9 >> 8);
                work[0] = *cursor << 8;
                cursor += 2;
                work[2] = *z_cursor << 8;
                heading = func_8008D104(actor, target);
                remaining -= 1;
                temp_v1 = point_index + heading_offset;
                point_index += 1;
                *(temp_v1 + D_8010CFE0) = heading;
            }
            else
            {
                var_v1_2 = work[0];
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 0xFF;
                }
                var_v0_10 = work[8] * spacing;
                if (var_v0_10 < 0)
                {
                    var_v0_10 += 0xFFF;
                }
                temp_v0_3 = (var_v1_2 >> 8) + (var_v0_10 >> 0xC);
                *cursor = temp_v0_3;
                var_v1_3 = work[2];
                work[0] = (s32)(temp_v0_3 << 0x10) >> 8;
                if (var_v1_3 < 0)
                {
                    var_v1_3 += 0xFF;
                }
                var_v0_11 = work[10] * spacing;
                if (var_v0_11 < 0)
                {
                    var_v0_11 += 0xFFF;
                }
                temp_v0_4 = (var_v1_3 >> 8) + (var_v0_11 >> 0xC);
                *z_cursor = temp_v0_4;
                z_cursor += 2;
                cursor += 2;
                work[2] = (s32)(temp_v0_4 << 0x10) >> 8;
                heading = func_8008D104(actor, target);
                remaining -= 1;
                temp_v1_2 = point_index + heading_offset;
                point_index += 1;
                *(temp_v1_2 + D_8010CFE0) = heading;
                if (remaining != 0)
                {
                    goto loop_22;
                }
            }
            sample_path_index += 1;
            if (remaining != 0)
            {
                path_offset = sample_path_index * 8;
                if (sample_path_index < (s32)states[actor->index].count)
                {
                    goto loop_21;
                }
            }
        }
    }
}
