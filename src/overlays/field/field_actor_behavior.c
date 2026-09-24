/** @file field_actor_behavior.c
 * @brief Party follower routes, controller movement, actor commands, and action
 *        animation dispatch.
 *
 * One translation unit: g_field_follower_animation_map (used only by the route
 * code) follows the behavior tables g_field_actor_turn_animations ..
 * g_field_action_sound_ids in .data, so route and behavior data share one object.
 */

/* ---- Party follower routes and position history -------------------------- */
#include "field_actor_routes.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_POSITION_HISTORY_LENGTH 48
#define FIELD_FIXED_POINT_SHIFT 8
#define FIELD_FIXED_POINT_ROUND_BIAS 0xFF

#define FIELD_ROUTE_MAP_SIZE ((FieldRouteMapSize*)0x801ED400)
#define FIELD_ROUTE_COLLISION_WORK ((FieldRouteMover*)0x1F800000)
#define FIELD_ROUTE_DIRECTION_SHIFT 12
#define FIELD_ROUTE_DIRECTION_ROUND_BIAS 0xFFF
#define FIELD_ROUTE_MIN_SPACING 3
#define FIELD_ROUTE_HALF_HISTORY 24
#define FIELD_ROUTE_ACTOR_COUNT 13
#define FIELD_ROUTE_ACTOR_ABSENT 0xFF
#define FIELD_ROUTE_CONTROL_MODE_MASK 0x1FF
#define FIELD_ROUTE_MOVING_FLAG 0x200
#define FIELD_ROUTE_MOVEMENT_MASK 0x600
#define FIELD_ROUTE_RESOURCE_ANIMATION_FLAG 1
#define FIELD_ROUTE_RUN_DISTANCE_SQUARED 0x181
#define FIELD_ROUTE_FOLLOW_DISTANCE_SQUARED 3000
#define FIELD_ROUTE_ANIMATION_INDEX_MASK 0x7F
#define FIELD_ROUTE_ANIMATION_FLIP 0x80

/** @brief Sparse view of the actor fields used to refresh position history. */
typedef struct
{
    u8 pad_0[0x54];
    s32 primary_x;
    u8 pad_58[4];
    s32 primary_z;
    u8 pad_60[0x10];
    s32 primary_flags;
    u8 pad_74[5];
    u8 primary_actor;
    u8 pad_7a[0xA8 - 0x7A];
    s32 secondary_x;
    u8 pad_ac[4];
    s32 secondary_z;
    u8 pad_b4[0xCD - 0xB4];
    u8 secondary_actor;
} FieldRouteParty;

/** @brief Sparse position-history view used at the base and indexed offsets. */
typedef struct
{
    u8 pad_0[0x6C];
    FieldRoutePoint points[FIELD_POSITION_HISTORY_LENGTH];
    u8 pad_12c[0x3AA - 0x12C];
    u8 primary_index;
    u8 pad_3ab[0x5E6 - 0x3AB];
    u8 secondary_index;
} FieldRouteHistoryView;

/** @brief Fixed-point X/Z waypoint produced by the path finder. */
typedef struct
{
    s32 x, z;
} FieldRouteWaypoint;

/** @brief Route count and fixed-point waypoints in a 0x23C-byte actor state. */
typedef struct
{
    u8 pad_0[0x1A4];
    u16 waypoint_count;
    u8 pad_1a6[6];
    FieldRouteWaypoint waypoints[18];
} FieldRouteState;

/** @brief Strided history view anchored at the first sample in an actor state. */
typedef struct
{
    FieldRoutePoint entries[FIELD_POSITION_HISTORY_LENGTH];
    u8 pad_c0[0x102 - FIELD_POSITION_HISTORY_LENGTH * sizeof(FieldRoutePoint)];
    u8 history_index;
    u8 pad_103[0x23C - 0x103];
} FieldRouteHistory;

/** @brief Partial 0x23C-byte slot containing 48 recorded positions and collision state. */
typedef struct
{
    u8 pad_0[0x6c];
    FieldRoutePoint points[FIELD_POSITION_HISTORY_LENGTH];
    u8 pad_12c[0x16e - 0x12c];
    u8 history_index;
    u8 pad_16f[7];
    s16 ground_height;
    u8 pad_178[0x24];
    s32 contact_index, surface;
    u8 pad_1a4[0x98];
} FieldRouteSlot;

/** @brief Twenty-byte resource descriptor exposing the behavior flags. */
typedef struct
{
    u8 pad_0[0x10];
    s32 flags;
} FieldRouteResource;

/** @brief Partial 0x48-byte appearance descriptor with the collision-size selector. */
typedef struct
{
    u8 pad_0[0x2e];
    u8 collision_scale;
    u8 pad_2f[0x19];
} FieldRouteAppearance;

/** @brief Map dimensions at the fixed field geometry address. */
typedef struct
{
    s16 width;
    u16 height;
} FieldRouteMapSize;

/** @brief Scratchpad collision mover with overlapping radius and flag storage. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, ground, surface, state;
    s16 radius, height;
    union
    {
        s32 word;
        struct
        {
            s16 radius_z;
            u16 flag0 : 1;
            u16 flag1 : 1;
            u16 rest : 14;
        } parts;
    } flags;
} FieldRouteMover;

extern FieldRouteHistory g_field_route_position_histories[];
extern u8 g_field_route_animation_history[];

s32 func_8001CDAC(s32*, s32*);
void func_8008A0B0(FieldRouteActor*, s32, s32);
extern long ratan2(long y, long x);

/**
 * @brief Route entry point with an empty implementation.
 */
void func_8008C728(void)
{
}

/**
 * @brief Fill the leader history with its current position and animation.
 *
 * The fixed-point coordinates are divided by 256 with truncation toward zero
 * before being stored as signed halfwords.
 */
void field_reset_leader_position_history(void)
{
    extern FieldRouteActor g_field_actors;

    FieldRoutePoint* dest;
    s32 index;
    s32 value;

    dest = g_field_route_position_histories[0].entries;
    index = 0;
    do
    {
        value = g_field_actors.x;
        if (value < 0)
        {
            value += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        dest->x = value >> FIELD_FIXED_POINT_SHIFT;

        value = g_field_actors.z;
        if (value < 0)
        {
            value += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        dest->z = value >> FIELD_FIXED_POINT_SHIFT;
        g_field_route_animation_history[index] = g_field_actors.animation;
        index++;
        dest++;
    } while (index < FIELD_POSITION_HISTORY_LENGTH);
}

/**
 * @brief Refresh actor position histories when their current samples differ.
 * @note Coordinate adjustment before shifting preserves truncation toward zero.
 * @note The primary path requires an active actor and a nonzero low flag field.
 */
void field_refresh_party_routes(void)
{
    extern FieldRouteParty g_field_actors;
    extern FieldRouteActor g_field_primary_party_actor[];
    extern FieldRouteActor g_field_secondary_party_actor[];
    extern FieldRouteHistoryView g_field_object_states;

    s32 primary_x;
    s32 secondary_x;
    s32 primary_only_x;
    s32 secondary_only_x;
    s32 primary_z;
    s32 secondary_z;
    s32 primary_only_z;
    s32 secondary_only_z;
    FieldRouteHistoryView* primary_history;
    FieldRouteHistoryView* secondary_history;
    FieldRouteHistoryView* primary_only_history;
    FieldRouteHistoryView* secondary_only_history;
    FieldRouteHistoryView* history_base;

    if ((g_field_actors.primary_actor != FIELD_ROUTE_ACTOR_ABSENT) && (g_field_actors.primary_flags & FIELD_ROUTE_CONTROL_MODE_MASK))
    {
        if (g_field_actors.secondary_actor != FIELD_ROUTE_ACTOR_ABSENT)
        {
            primary_x = g_field_actors.primary_x;
            if (primary_x < 0)
            {
                primary_x += FIELD_FIXED_POINT_ROUND_BIAS;
            }
            primary_history = (FieldRouteHistoryView*)((u8*)&g_field_object_states + g_field_object_states.primary_index * sizeof(FieldRoutePoint));
            if ((primary_x >> FIELD_FIXED_POINT_SHIFT) == primary_history->points[0].x)
            {
                primary_z = g_field_actors.primary_z;
                if (primary_z < 0)
                {
                    primary_z += FIELD_FIXED_POINT_ROUND_BIAS;
                }
                if ((primary_z >> FIELD_FIXED_POINT_SHIFT) == primary_history->points[0].z)
                {
                    secondary_x = g_field_actors.secondary_x;
                    if (secondary_x < 0)
                    {
                        secondary_x += FIELD_FIXED_POINT_ROUND_BIAS;
                    }
                    secondary_history = (FieldRouteHistoryView*)((u8*)&g_field_object_states + g_field_object_states.secondary_index * sizeof(FieldRoutePoint));
                    if ((secondary_x >> FIELD_FIXED_POINT_SHIFT) == secondary_history->points[0].x)
                    {
                        secondary_z = g_field_actors.secondary_z;
                        if (secondary_z < 0)
                        {
                            secondary_z += FIELD_FIXED_POINT_ROUND_BIAS;
                        }
                        if ((secondary_z >> FIELD_FIXED_POINT_SHIFT) == secondary_history->points[0].z)
                        {
                            return;
                        }
                    }
                }
            }
            field_sample_actor_route(g_field_route_position_histories[0].entries, FIELD_ROUTE_HALF_HISTORY, g_field_primary_party_actor,
                                     g_field_primary_party_actor, 0, FIELD_ROUTE_HALF_HISTORY);
            field_sample_actor_route(g_field_route_position_histories[0].entries + FIELD_ROUTE_HALF_HISTORY, FIELD_ROUTE_HALF_HISTORY - 1,
                                     g_field_primary_party_actor, g_field_primary_party_actor - 1, FIELD_ROUTE_HALF_HISTORY, FIELD_ROUTE_HALF_HISTORY);
            /* The history symbol is an interior alias of the first actor state. */
            history_base = (FieldRouteHistoryView*)((u8*)g_field_route_position_histories - (s32) & ((FieldRouteHistoryView*)0)->points);
            history_base->primary_index = FIELD_ROUTE_HALF_HISTORY;
            field_sample_actor_route(g_field_route_position_histories[0].entries, FIELD_ROUTE_HALF_HISTORY, g_field_primary_party_actor + 1,
                                     g_field_primary_party_actor, 0, FIELD_ROUTE_HALF_HISTORY);
            history_base->secondary_index = 0;
        }
        else
        {
            primary_only_x = g_field_actors.primary_x;
            if (primary_only_x < 0)
            {
                primary_only_x += FIELD_FIXED_POINT_ROUND_BIAS;
            }
            primary_only_history = (FieldRouteHistoryView*)((u8*)&g_field_object_states + g_field_object_states.primary_index * sizeof(FieldRoutePoint));
            if ((primary_only_x >> FIELD_FIXED_POINT_SHIFT) == primary_only_history->points[0].x)
            {
                primary_only_z = g_field_actors.primary_z;
                if (primary_only_z < 0)
                {
                    primary_only_z += FIELD_FIXED_POINT_ROUND_BIAS;
                }
                if ((primary_only_z >> FIELD_FIXED_POINT_SHIFT) == primary_only_history->points[0].z)
                {
                    return;
                }
            }
            field_sample_actor_route(g_field_route_position_histories[0].entries, FIELD_ROUTE_HALF_HISTORY, g_field_primary_party_actor,
                                     g_field_primary_party_actor, 0, FIELD_ROUTE_HALF_HISTORY);
            field_sample_actor_route(g_field_route_position_histories[0].entries + FIELD_ROUTE_HALF_HISTORY, FIELD_ROUTE_HALF_HISTORY - 1,
                                     g_field_primary_party_actor, g_field_primary_party_actor - 1, FIELD_ROUTE_HALF_HISTORY, FIELD_ROUTE_HALF_HISTORY);
            g_field_route_position_histories[1].history_index = FIELD_ROUTE_HALF_HISTORY;
        }
    }
    else if (g_field_actors.secondary_actor != FIELD_ROUTE_ACTOR_ABSENT)
    {
        secondary_only_x = g_field_actors.secondary_x;
        if (secondary_only_x < 0)
        {
            secondary_only_x += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        secondary_only_history = (FieldRouteHistoryView*)((u8*)&g_field_object_states + g_field_object_states.secondary_index * sizeof(FieldRoutePoint));
        if ((secondary_only_x >> FIELD_FIXED_POINT_SHIFT) == secondary_only_history->points[0].x)
        {
            secondary_only_z = g_field_actors.secondary_z;
            if (secondary_only_z < 0)
            {
                secondary_only_z += FIELD_FIXED_POINT_ROUND_BIAS;
            }
            if ((secondary_only_z >> FIELD_FIXED_POINT_SHIFT) == secondary_only_history->points[0].z)
            {
                return;
            }
        }
        field_sample_actor_route(g_field_route_position_histories[0].entries, FIELD_ROUTE_HALF_HISTORY, g_field_secondary_party_actor,
                                 g_field_secondary_party_actor, 0, FIELD_ROUTE_HALF_HISTORY);
        field_sample_actor_route(g_field_route_position_histories[0].entries + FIELD_ROUTE_HALF_HISTORY, FIELD_ROUTE_HALF_HISTORY - 1,
                                 g_field_secondary_party_actor, g_field_secondary_party_actor - 2, FIELD_ROUTE_HALF_HISTORY, FIELD_ROUTE_HALF_HISTORY);
        g_field_route_position_histories[2].history_index = FIELD_ROUTE_HALF_HISTORY;
    }
}

/**
 * @brief Sample an actor route into signed points and parallel heading entries.
 * @param points Output X/Z points.
 * @param remaining Requested point count, including the initial position; at least two.
 * @param actor Actor whose generated route is sampled.
 * @param target Destination actor used for routing and heading calculation.
 * @param heading_offset Starting index in the shared heading array.
 * @param unused_limit Unused sixth argument retained to agree with the caller.
 * @note Coordinates are fixed point with eight fractional bits; interpolated
 *       normalized directions use twelve fractional bits.
 * @see decomp.me (99.754906%) with the pinned assembler.
 * @see decomp.me (100%) with the experimental maspsx operand fix, now removed.
 * @note The pinned assembler inserts one extra nop; see
 *       docs/decompilation/maspsx-division-register-hazard.md.
 */
void field_sample_actor_route(FieldRoutePoint* points, s32 remaining, FieldRouteActor* actor, FieldRouteActor* target, s32 heading_offset, s32 unused_limit)
{
    extern FieldRouteState g_field_object_states[];

    struct
    {
        VECTOR position;
        VECTOR delta;
        VECTOR direction;
    } work;
    s32 actor_index;
    FieldRouteState* states;
    FieldRouteState* waypoint_state;
    s32 heading;
    s16* z_cursor;
    s16 sample_x;
    s16 sample_z;
    s32 spacing;
    s32 segment_length;
    s32 waypoint_heading_index;
    s32 sample_heading_index;
    s32 point_index;
    s32 total_distance;
    s32 waypoint_offset;
    s32 path_index;
    s32 segment_dx;
    s32 step_x;
    s32 step_z;
    s32 segment_dz;
    s32 initial_z;
    s32 padding_x;
    s32 padding_z;
    s32 waypoint_dx;
    s32 waypoint_dz;
    s32 waypoint_x;
    s32 waypoint_z;
    s32 initial_x;
    s32 sample_origin_x;
    s32 sample_origin_z;
    s32 initial_heading;

    /* Measure the generated path before choosing the sample spacing. */
    func_8008A0B0(actor, target->object_index, 0);
    work.position.vx = actor->x;
    total_distance = 0;
    work.position.vz = actor->z;
    path_index = 0;
    actor_index = actor->object_index;
    if (g_field_object_states[actor_index].waypoint_count != 0)
    {
        do
        {
            segment_dx = g_field_object_states[actor_index].waypoints[path_index].x - work.position.vx;
            if (segment_dx < 0)
            {
                segment_dx += FIELD_FIXED_POINT_ROUND_BIAS;
            }
            work.delta.vx = segment_dx >> FIELD_FIXED_POINT_SHIFT;
            segment_dz = g_field_object_states[actor->object_index].waypoints[path_index].z - work.position.vz;
            if (segment_dz < 0)
            {
                segment_dz += FIELD_FIXED_POINT_ROUND_BIAS;
            }
            work.delta.vz = segment_dz >> FIELD_FIXED_POINT_SHIFT;
            work.delta.vy = 0;
            gte_ldlvl(&work.delta);
            gte_sqr0();
            gte_stlvnl(&work.direction);
            segment_length = SquareRoot0(work.direction.vx + work.direction.vz);
            work.position.vx = g_field_object_states[actor->object_index].waypoints[path_index].x;
            work.position.vz = g_field_object_states[actor->object_index].waypoints[path_index].z;
            path_index += 1;
            total_distance += segment_length;
            actor_index = actor->object_index;
        } while (path_index < (s32)g_field_object_states[actor_index].waypoint_count);
    }
    initial_heading = field_get_route_heading_animation(actor, target);
    initial_x = actor->x;
    if (initial_x < 0)
    {
        initial_x += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    points->x = (s16)(initial_x >> FIELD_FIXED_POINT_SHIFT);
    initial_z = actor->z;
    if (initial_z < 0)
    {
        initial_z += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    remaining -= 1;
    points->z = (s16)(initial_z >> FIELD_FIXED_POINT_SHIFT);
    points++;
    g_field_route_animation_history[heading_offset] = initial_heading;
    point_index = 1;
    /* Short paths begin with repeated origin samples to keep the remaining steps useful. */
    while ((total_distance / remaining) < FIELD_ROUTE_MIN_SPACING)
    {
        g_field_route_animation_history[point_index + heading_offset] = initial_heading;
        padding_x = actor->x;
        if (padding_x < 0)
        {
            padding_x += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        points->x = (s16)(padding_x >> FIELD_FIXED_POINT_SHIFT);
        padding_z = actor->z;
        if (padding_z < 0)
        {
            padding_z += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        points->z = (s16)(padding_z >> FIELD_FIXED_POINT_SHIFT);
        points++;
        remaining -= 1;
        point_index += 1;
        if (remaining == 0)
        {
            return;
        }
    }
    work.position.vx = actor->x;
    work.position.vz = actor->z;
    spacing = total_distance / remaining;
    path_index = 0;
    if (g_field_object_states[actor->object_index].waypoint_count != 0)
    {
        states = g_field_object_states;
        do
        {
            waypoint_offset = path_index * sizeof(FieldRouteWaypoint);
            z_cursor = &points->z;
            do
            {
                waypoint_state =
                    (FieldRouteState*)((waypoint_offset + ((actor->object_index * (s32)(sizeof(FieldRouteState) / sizeof(s32))) << 2)) + (s32)states);
                waypoint_dx = waypoint_state->waypoints[0].x - work.position.vx;
                if (waypoint_dx < 0)
                {
                    waypoint_dx += FIELD_FIXED_POINT_ROUND_BIAS;
                }
                work.delta.vx = waypoint_dx >> FIELD_FIXED_POINT_SHIFT;
                waypoint_state =
                    (FieldRouteState*)((waypoint_offset + ((actor->object_index * (s32)(sizeof(FieldRouteState) / sizeof(s32))) << 2)) + (s32)states);
                waypoint_dz = waypoint_state->waypoints[0].z - work.position.vz;
                if (waypoint_dz < 0)
                {
                    waypoint_dz += FIELD_FIXED_POINT_ROUND_BIAS;
                }
                work.delta.vz = waypoint_dz >> FIELD_FIXED_POINT_SHIFT;
                work.delta.vy = 0;
                if (spacing >= SquareRoot0(func_8001CDAC((s32*)&work.delta, (s32*)&work.direction)))
                {
                    waypoint_state =
                        (FieldRouteState*)((waypoint_offset + ((actor->object_index * (s32)(sizeof(FieldRouteState) / sizeof(s32))) << 2)) + (s32)states);
                    waypoint_x = waypoint_state->waypoints[0].x;
                    if (waypoint_x < 0)
                    {
                        waypoint_x += FIELD_FIXED_POINT_ROUND_BIAS;
                    }
                    points->x = (s16)(waypoint_x >> FIELD_FIXED_POINT_SHIFT);
                    waypoint_state =
                        (FieldRouteState*)((waypoint_offset + ((actor->object_index * (s32)(sizeof(FieldRouteState) / sizeof(s32))) << 2)) + (s32)states);
                    waypoint_z = waypoint_state->waypoints[0].z;
                    if (waypoint_z < 0)
                    {
                        waypoint_z += FIELD_FIXED_POINT_ROUND_BIAS;
                    }
                    *z_cursor = (s16)(waypoint_z >> FIELD_FIXED_POINT_SHIFT);
                    work.position.vx = points->x << FIELD_FIXED_POINT_SHIFT;
                    points++;
                    work.position.vz = *z_cursor << FIELD_FIXED_POINT_SHIFT;
                    heading = field_get_route_heading_animation(actor, target);
                    remaining -= 1;
                    waypoint_heading_index = point_index + heading_offset;
                    point_index += 1;
                    g_field_route_animation_history[waypoint_heading_index] = heading;
                }
                else
                {
                    sample_origin_x = work.position.vx;
                    if (sample_origin_x < 0)
                    {
                        sample_origin_x += FIELD_FIXED_POINT_ROUND_BIAS;
                    }
                    sample_origin_x >>= FIELD_FIXED_POINT_SHIFT;
                    step_x = work.direction.vx * spacing;
                    if (step_x < 0)
                    {
                        step_x += FIELD_ROUTE_DIRECTION_ROUND_BIAS;
                    }
                    sample_x = sample_origin_x + (step_x >> FIELD_ROUTE_DIRECTION_SHIFT);
                    points->x = sample_x;
                    sample_origin_z = work.position.vz;
                    work.position.vx = (s32)(sample_x << 0x10) >> FIELD_FIXED_POINT_SHIFT;
                    if (sample_origin_z < 0)
                    {
                        sample_origin_z += FIELD_FIXED_POINT_ROUND_BIAS;
                    }
                    sample_origin_z >>= FIELD_FIXED_POINT_SHIFT;
                    step_z = work.direction.vz * spacing;
                    if (step_z < 0)
                    {
                        step_z += FIELD_ROUTE_DIRECTION_ROUND_BIAS;
                    }
                    sample_z = sample_origin_z + (step_z >> FIELD_ROUTE_DIRECTION_SHIFT);
                    *z_cursor = sample_z;
                    z_cursor += 2;
                    points++;
                    work.position.vz = (s32)(sample_z << 0x10) >> FIELD_FIXED_POINT_SHIFT;
                    heading = field_get_route_heading_animation(actor, target);
                    remaining -= 1;
                    sample_heading_index = point_index + heading_offset;
                    point_index += 1;
                    g_field_route_animation_history[sample_heading_index] = heading;
                    continue;
                }
                break;
            } while (remaining != 0);
            path_index += 1;
            if (remaining == 0)
            {
                break;
            }
        } while (path_index < (s32)states[actor->object_index].waypoint_count);
    }
}

/**
 * @brief Maps the heading from @p source to @p destination onto a direction-table entry.
 *
 * Computes the ratan2 angle between the two points, quantizes it to a 256-step
 * heading wrapped into [0, 0x100), and returns g_field_direction_animation_modes[heading >> 5] plus 10.
 *
 * @param destination Destination point whose heading is measured.
 * @param source Source point.
 * @return Direction-table entry for the quantized heading, offset by 10.
 */
s32 field_get_route_heading_animation(FieldRouteActor* destination, FieldRouteActor* source)
{
    extern s32 g_field_direction_animation_modes[];

    s32 angle;
    s32 heading;

    angle = ratan2(destination->z - source->z, source->x - destination->x) >> 4;
    heading = angle + 0x10;
    if (heading < 0)
    {
        heading = angle + 0x110;
    }
    if (heading >= 0x100)
    {
        heading -= 0x100;
    }
    return g_field_direction_animation_modes[heading >> 5] + 10;
}

/**
 * @brief Update an actor's position history when its rounded position changes.
 * @param record FieldRouteActor position and status record to sample.
 */
void field_record_actor_position(FieldRouteActor* record)
{
    extern FieldRouteSlot g_field_object_states[];

    s32 x;
    s32 z;
    s32 index;
    s32 value;
    FieldRoutePoint* history;
    u8* status_history;

    x = record->x;
    if (x < 0)
    {
        x += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    if ((x >> FIELD_FIXED_POINT_SHIFT) == g_field_object_states[record->object_index].points[FIELD_POSITION_HISTORY_LENGTH - 1].x)
    {
        z = record->z;
        if (z < 0)
        {
            z += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        if ((z >> FIELD_FIXED_POINT_SHIFT) == g_field_object_states[record->object_index].points[FIELD_POSITION_HISTORY_LENGTH - 1].z)
        {
            return;
        }
    }

    index = 0;
    history = g_field_route_position_histories[record->object_index].entries;
    /* Shift packed X/Z pairs together. */
    do
    {
        index++;
        *(s32*)history = *(s32*)(history + 1);
        history++;
    } while (index < FIELD_POSITION_HISTORY_LENGTH - 1);

    value = record->x;
    if (value < 0)
    {
        value += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    history->x = (s16)(value >> FIELD_FIXED_POINT_SHIFT);

    value = record->z;
    if (value < 0)
    {
        value += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    history->z = (s16)(value >> FIELD_FIXED_POINT_SHIFT);

    if (record->object_index == 0)
    {
        status_history = g_field_route_animation_history;
        index = 0;
        do
        {
            index++;
            status_history[0] = status_history[1];
            status_history++;
        } while (index < FIELD_POSITION_HISTORY_LENGTH - 1);
        *status_history = record->animation;
    }
}

/**
 * @brief Follow the active leader's recorded path and update collision and animation.
 *
 * Selects the first eligible leader, adjusts the follower's history index, and
 * measures movement with the GTE. Collision resolution updates the follower's
 * height and cached surface; horizontal displacement selects its animation.
 *
 * @param actor Follower actor whose position and state are updated.
 * @param follower_index Follower order controlling the permitted path separation.
 * @see decomp.me (100%)
 */
void field_follow_leader_route(FieldRouteActor* actor, s32 follower_index)
{
    extern FieldRouteActor g_field_actors[];
    extern FieldRouteSlot g_field_object_states[];
    extern FieldRouteResource g_field_resource_entries[];
    extern FieldRouteAppearance g_field_object_parts[];
    extern u8 g_field_follower_animation_map[], g_field_route_animation_history[], D_8010AE84;
    void field_update_actor_command(FieldRouteActor*);
    void field_restart_actor_animation(FieldRouteActor*);
    s32 func_8005B6AC(FieldRouteMover*);

    VECTOR square;
    VECTOR delta;
    FieldRouteMapSize* dimensions = FIELD_ROUTE_MAP_SIZE;
    FieldRouteMover* mover = FIELD_ROUTE_COLLISION_WORK;
    FieldRouteActor* scan;
    s16 decay_period;
    s32 limit;
    s32 position_z;
    s32 animation_kind;
    s32 actor_x;
    s32 advance_dx;
    s32 retreat_dx;
    s32 position_x;
    s32 leader_index;
    s32 dz;
    s32 dx;
    s32 leader_dx;
    s32 leader_dz;
    s32 decay;
    u8 advance_index;
    s32 sample_index;
    u8 recorded_animation;
    u8 old_animation;
    u8 animation;
    FieldRouteSlot* sample_base;
    FieldRouteActor* leader;
    FieldRouteSlot* advance_slot;
    FieldRouteSlot* retreat_slot;
    s32 collision_height;
    u8 collision_flag;

    /* The leader search inspects only the low half of the actor flags. */
    for (leader_index = 0; leader_index < FIELD_ROUTE_ACTOR_COUNT; leader_index++)
    {
        scan = &g_field_actors[leader_index];
        if (scan->state != FIELD_ROUTE_ACTOR_ABSENT && !(scan->flags.half.low & FIELD_ROUTE_CONTROL_MODE_MASK))
        {
            break;
        }
    }
    if (leader_index == FIELD_ROUTE_ACTOR_COUNT)
    {
        leader_index = 0;
    }
    delta.vy = 0;
    delta.vz = 0;
    delta.vx = 0;
    decay_period = actor->motion_divisor;
    leader = &g_field_actors[leader_index];
    if (decay_period != 0)
    {
        decay = (s8)actor->motion_remainder / decay_period;
    }
    else
    {
        decay = 0;
    }
    if (decay != 0)
    {
        actor->motion_remainder = (s8)((u8)actor->motion_remainder - decay);
    }
    if (actor->action_id != 0)
    {
        field_update_actor_command(actor);
        return;
    }
    actor_x = actor->x;
    {
        FieldRouteSlot* base = g_field_object_states;
        FieldRouteSlot* slot = &base[actor->object_index];
        sample_base = (FieldRouteSlot*)((s32)g_field_object_states + (leader->object_index * 0x8F + slot->history_index) * 4);
    }
    if (sample_base->points[0].x != actor_x / 256 || sample_base->points[0].z != actor->z / 256)
    {
        limit = FIELD_ROUTE_HALF_HISTORY;
        if (follower_index != 0)
        {
            limit = 0;
        }
        {
            FieldRouteSlot* base = g_field_object_states;
            retreat_slot = &base[actor->object_index];
        }
        sample_index = retreat_slot->history_index;
        dx = 0;
        if (limit < (s32)sample_index)
        {
            dz = dx;
            retreat_slot->history_index = (u8)(sample_index - 1);
            delta.vx = 0;
            delta.vz = 0;
            goto update_collision;
        }
        else
        {
            retreat_dx = (g_field_object_states[leader->object_index].points[sample_index].x << FIELD_FIXED_POINT_SHIFT) - actor->x;
            delta.vx = retreat_dx;
            dx = retreat_dx;
            delta.vz =
                (g_field_object_states[leader->object_index].points[g_field_object_states[actor->object_index].history_index].z << FIELD_FIXED_POINT_SHIFT) -
                actor->z;
            dz = delta.vz;
            gte_ldlvl(&delta);
            gte_sqr12();
            gte_stlvnl(&square);
            if ((square.vx + square.vz) >= FIELD_ROUTE_RUN_DISTANCE_SQUARED)
            {
                if (g_field_resource_entries[actor->resource_index].flags & FIELD_ROUTE_RESOURCE_ANIMATION_FLAG)
                {
                    actor->movement_mode = 0;
                }
                else
                {
                    goto mark_moving;
                }
            }
            else
            {
                goto mark_walking;
            }
        }
    }
    else
    {
        leader_dx = leader->x - actor_x;
        if (leader_dx < 0)
        {
            leader_dx += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        delta.vx = leader_dx >> FIELD_FIXED_POINT_SHIFT;
        leader_dz = leader->z - actor->z;
        if (leader_dz < 0)
        {
            leader_dz += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        delta.vz = leader_dz >> FIELD_FIXED_POINT_SHIFT;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&square);
        dx = 0;
        if ((square.vx + square.vz) > ((follower_index + 1) * FIELD_ROUTE_FOLLOW_DISTANCE_SQUARED))
        {
            advance_slot = &g_field_object_states[actor->object_index];
            advance_index = advance_slot->history_index;
            if (advance_index < (FIELD_POSITION_HISTORY_LENGTH - 1U))
            {
                advance_slot->history_index = (u8)(advance_index + 1);
                advance_dx = (g_field_object_states[leader->object_index].points[g_field_object_states[actor->object_index].history_index].x
                              << FIELD_FIXED_POINT_SHIFT) -
                             actor->x;
                delta.vx = advance_dx;
                dx = advance_dx;
                delta.vz = (g_field_object_states[leader->object_index].points[g_field_object_states[actor->object_index].history_index].z
                            << FIELD_FIXED_POINT_SHIFT) -
                           actor->z;
                dz = delta.vz;
                gte_ldlvl(&delta);
                gte_sqr12();
                gte_stlvnl(&square);
                if (((square.vx + square.vz) >= FIELD_ROUTE_RUN_DISTANCE_SQUARED) &&
                    (!(g_field_resource_entries[actor->resource_index].flags & FIELD_ROUTE_RESOURCE_ANIMATION_FLAG)))
                {
                mark_moving:
                    actor->movement_mode = 1;
                }
                else
                {
                mark_walking:
                    actor->movement_mode = 0;
                }
                collision_height = actor->y;
                collision_flag = D_8010AE84;
                goto apply_collision_state;
            }
        }
        dz = dx;
        goto clear_delta;
    }
    collision_height = actor->y;
    collision_flag = D_8010AE84;
    goto apply_collision_state;
clear_delta:
    delta.vx = 0;
    delta.vz = 0;
update_collision:
    collision_height = actor->y;
    collision_flag = D_8010AE84;
apply_collision_state:
    mover->y = collision_height;
    if (collision_flag == 0)
    {
        position_x = actor->x;
        if ((position_x >= 0) && (position_x < (dimensions->width << FIELD_FIXED_POINT_SHIFT)) && ((position_z = actor->z) >= 0) &&
            (position_z < ((s32)(dimensions->height << 16) >> 7)))
        {
            mover->x = position_x;
            mover->y = actor->y;
            mover->z = actor->z;
            mover->dx = dx;
            mover->dy = 0;
            mover->dz = dz;
            if (g_field_object_parts[actor->object_index].collision_scale == 0x40)
            {
                mover->radius = 0xC;
                mover->flags.parts.radius_z = 8;
            }
            else
            {
                mover->radius = 9;
                mover->flags.parts.radius_z = 6;
            }
            mover->height = 0x10;
            /* Clear the two collision flags without disturbing the radius. */
            mover->flags.parts.flag1 = 0;
            mover->flags.parts.flag0 = 0;
            mover->surface = g_field_object_states[actor->object_index].contact_index;
            mover->state = g_field_object_states[actor->object_index].surface;
            func_8005B6AC(mover);
            g_field_object_states[actor->object_index].contact_index = (s32)mover->surface;
            g_field_object_states[actor->object_index].surface = (s32)mover->state;
            actor->y = (s32)mover->y;
            g_field_object_states[actor->object_index].ground_height = mover->ground / 256;
        }
        else
        {
            g_field_object_states[actor->object_index].contact_index = -1;
            g_field_object_states[actor->object_index].surface = 0;
            g_field_object_states[actor->object_index].ground_height = 0;
        }
    }
    if ((dx | dz) != 0)
    {
        if (g_field_resource_entries[actor->resource_index].flags & FIELD_ROUTE_RESOURCE_ANIMATION_FLAG)
        {
            recorded_animation = g_field_route_animation_history[g_field_object_states[actor->object_index].history_index];
            animation = g_field_follower_animation_map[recorded_animation & FIELD_ROUTE_ANIMATION_INDEX_MASK];
            animation |= recorded_animation & FIELD_ROUTE_ANIMATION_FLIP;
        }
        else
        {
            animation = g_field_route_animation_history[g_field_object_states[actor->object_index].history_index];
        }
        if (actor->animation != animation)
        {
            actor->animation = animation;
            actor->animation_frame = 0;
            actor->animation_active = 1;
            field_restart_actor_animation(actor);
        }
    }
    else
    {
        old_animation = actor->animation;
        animation_kind = old_animation & FIELD_ROUTE_ANIMATION_INDEX_MASK;
        if ((animation_kind < 5) || (g_field_resource_entries[actor->resource_index].flags & FIELD_ROUTE_RESOURCE_ANIMATION_FLAG))
        {
            if ((animation_kind != 0) && (g_field_resource_entries[actor->resource_index].flags & FIELD_ROUTE_RESOURCE_ANIMATION_FLAG))
            {
                actor->animation = (u8)(old_animation & FIELD_ROUTE_ANIMATION_FLIP);
                goto restart_animation;
            }
        }
        else
        {
            actor->animation = (u8)((animation_kind % 5) | (old_animation & FIELD_ROUTE_ANIMATION_FLIP));
        restart_animation:
            actor->animation_frame = 0;
            actor->animation_active = 1;
            field_restart_actor_animation(actor);
        }
    }
    if ((dx | dz) != 0)
    {
        actor->x = (s32)(actor->x + dx);
        actor->z = (s32)(actor->z + dz);
    }
    if ((delta.vx | delta.vz) != 0)
    {
        actor->flags.word = (actor->flags.word & ~FIELD_ROUTE_MOVEMENT_MASK) | FIELD_ROUTE_MOVING_FLAG;
    }
    else
    {
        actor->flags.word = actor->flags.word & ~FIELD_ROUTE_MOVEMENT_MASK;
    }
}
#include "field_actor_behavior.h"
#include "field_text.h"
#include "field_effect_render_state.h"
#include "common.h"
#include "field_types.h"
#include "controller_internal.h"
#include "sdk/libetc.h"

/* ---- Controller movement, actor commands, and action animation dispatch ---- */

#define FIELD_ACTOR_COMMAND_BASE 0x81
#define FIELD_ACTION_INSTRUMENT 0x400
#define FIELD_ACTION_TECHNIQUE 0x8000
#define FIELD_ACTION_TARGETED 0x800
#define FIELD_ANIMATION_FACING 0x80
#define FIELD_ANIMATION_INDEX_MASK 0x7F
#define FIELD_RESOURCE_ACTION_BYTES 0x190
#define FIELD_TECHNIQUE_GAUGE_FULL 0xFF

/** @brief Position, input, and animation state of a field object. */
typedef struct FieldBehaviorActor
{
    s32 x, y, z;
    u8 pad0c[0x16 - 0x0c];
    s16 speed_divisor;
    u8 pad18[3];
    u8 direction;
    u32 flags;
    u8 command_timer;
    u8 animation;
    u8 pad22[2];
    u8 active;
    u8 state;
    u8 pad26;
    u8 animation_frame;
    u8 pad28[2];
    s16 command;
    u8 pad2c[2];
    u16 animation_state;
    u16 variant;
    u8 pad32;
    u8 movement_flags;
    u8 pad34[2];
    s8 speed_accumulator;
    u8 pad37[3];
    u8 object_index;
    u8 resource_index;
    u8 pad3c;
    u8 retirement_delay;
    u8 pad3e[0x54 - 0x3e];
} FieldBehaviorActor;

/** @brief Alternate signed loads and stores for the shared behavior record. */
typedef struct
{
    s32 x, y, z;
    u8 pad0c[0x16 - 0x0c];
    s16 speed_divisor;
    u8 pad18[3];
    u8 direction;
    s32 flags;
    u8 command_timer;
    u8 animation;
    u8 pad22[2];
    s8 active;
    u8 state;
    u8 pad26;
    s8 animation_frame;
    u8 pad28[2];
    s16 command;
    u8 pad2c[2];
    s16 animation_state;
    u16 variant;
    u8 pad32;
    s8 movement_flags;
    u8 pad34[2];
    s8 speed_accumulator;
    u8 pad37[3];
    u8 object_index;
    u8 resource_index;
    u8 pad3c;
    u8 retirement_delay;
    u8 pad3e[0x54 - 0x3e];
} FieldBehaviorActorViews;

/** @brief Map bounds used by the collision query. */
typedef struct
{
    s16 width;
    u16 height;
} FieldMovementBounds;

/** @brief Secondary collision probe stored in scratchpad memory. */
typedef struct
{
    s32 x, y, z;
    s16 radius_x, radius_y, radius_z;
} FieldMovementProbe;

/** @brief Action, target, and route state accessed by actor commands. */
typedef struct
{
    u8 pad_000[0x3c];
    s32 action_parameter;
    s32 sequence_id;
    s32 sequence_position;
    s16 technique_gauge;
    u16 action_charge;
    u8 pad_04c[0x4];
    s32 target_x;
    u8 pad_054[0x4];
    s32 target_z;
    u8 pad_05c[0x113];
    u8 action_index;
    u8 pad_170[0x1];
    u8 command_timer;
    u16 sequence;
    union
    {
        u32 u32_value;
        s32 s32_value;
        u16 u16_value;
    } sequence_flags;
    union
    {
        s32 word;
        struct
        {
            u8 flags;
            s8 animation_slot;
            u8 unknown_0x17a;
            union
            {
                u8 u;
                s8 s;
            } target_count;
        } bytes;
    } action_flags;
    u8 pad_17c[0x4];
    u8 targets;
    u8 pad_181[0x23];
    u16 waypoint_count;
    u16 waypoint_index;
    u8 pad_1a8[0x4];
    s32 waypoint_x;
    s32 waypoint_z;
} FieldBehaviorState;

/** @brief Party combo and timed transition state. */
typedef struct
{
    u8 pad_000[0x3];
    u8 kind;
    u8 pad_004[0x253];
    union
    {
        u8 u8_value;
        s8 s8_value;
    } combo_timer;
    u8 pad_258[0x6];
    s16 transition_time;
    s16 transition_limit;
    s16 destination_x;
    s16 destination_y;
    s16 destination_z;
} FieldBehaviorPlayer;

/** @brief Resource action descriptor and adjacent combo command. */
typedef struct
{
    u16 command;
    union
    {
        u16 u16_value;
        u8 u8_value;
    } flags;
    u16 animation;
    u16 requirement;
    u16 second_command;
} FieldBehaviorAction;

/** @brief Owner and animation slot of a sequence request. */
typedef struct
{
    s32 state;
    u8 pad_004[0x8];
    s32 object_index;
    u8 pad_010[0x8];
    s32 animation_slot;
} FieldBehaviorBinding;

/** @brief Animation activity and action flags. */
typedef struct
{
    u8 pad_000[0x24];
    union
    {
        u8 u8_value;
        s8 s8_value;
    } active;
    u8 pad_025[0x1ff];
    s32 action_flags;
} FieldBehaviorActorSlot;

/** @brief Camera offsets used by route completion bounds. */
typedef struct
{
    u8 pad_000[0x4];
    s32 x;
    u8 pad_008[0x4];
    s32 z;
} FieldBehaviorView;

#define ACTOR_COMMAND_STATE(i) (((FieldCommandState*)&g_field_object_states)[(i)])
#define PLAYER_COMMAND_STATE(i) (((FieldCommandPlayer*)&g_field_player_records)[(i)])
#define ACTOR_COMMAND_RESOURCE(i) (((FieldCommandResource*)&g_field_resource_entries)[(i)])

/** @brief Partial 0x54-byte actor layout used by the field_prepare_actor_action action dispatcher. */
typedef struct FieldActionActor
{
    u8 pad0[0x21];
    u8 animation;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 animation_frame;
    u8 pad28[2];
    union
    {
        u16 word;
        u8 bytes[2];
    } state;
    u8 pad2c[2];
    u16 animation_state, variant;
    u8 pad32[8];
    u8 object_index, resource_index;
    u8 pad3c[0x54 - 0x3C];
} FieldActionActor;

/** @brief Partial field record containing direction and animation state (field_update_actor_movement_animation). */
typedef struct FieldMotionRecord
{
    u8 pad00[0x1B];
    u8 direction;
    u8 pad1C[5];
    u8 animation;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 animation_frame;
    u8 pad28[6];
    u16 idle_mode;
    u8 pad30[3];
    u8 movement_flags;
    u8 pad34[5];
    u8 stop_delay;
    u8 pad3A;
    u8 resource_index;
} FieldMotionRecord;

/** @brief Animation entry with state, frame counters, variant, and actor slot (field_apply_action_animation). */
typedef struct
{
    u8 pad0[0x1C];
    s32 flags;
    u8 pad20;
    u8 animation;
    u8 pad22[2];
    u8 active;
    u8 pad25[2];
    u8 animation_frame;
    u8 pad28[2];
    u16 command;
    u8 pad2C[2];
    u16 animation_state;
    union
    {
        u16 half;
        u8 byte[2];
    } variant;
    u8 pad32[8];
    u8 object_index;
} FieldActionAnimationActor;

/** @brief Actor fields affected by animation commands (field_apply_action_animation). */
typedef struct
{
    u8 pad0[12];
    s32 status_flags;
    u8 pad10[4];
    s32 unknown_0x14;
    u8 pad18[0x3C - 0x18];
    s32 action_parameter;
    u8 pad40[8];
    u16 technique_gauge;
    u8 pad4A[0x174 - 0x4A];
    s32 sequence_flags;
} FieldActionAnimationState;

/** @brief Animation command code and argument (field_apply_action_animation). */
typedef struct
{
    union
    {
        u16 half;
        u8 byte[2];
    } code;
    u8 pad2[4];
    u16 argument;
} FieldActionAnimationCommand;

/** @brief Actor record whose slot index selects a slot (field_actor_action_is_charging). */
typedef struct
{
    u8 pad0[0x3A];
    u8 object_index;
} FieldChargeActor;

/**
 * @brief Apply controller movement, resolve collisions, and update an actor's movement state.
 * @param actor Actor position and state record.
 * @param pad_index Controller bank index.
 * @return The actor's two-bit movement state, or zero when input processing is blocked.
 * @note The collision request occupies scratchpad 0x1F800000; the secondary probe uses 0x1F800040.
 * @see decomp.me (100%)
 */
s32 field_update_actor_input(FieldBehaviorActor* actor, s32 pad_index)
{
    /** @brief Resource flags selecting movement-state handling. */
    typedef struct
    {
        u8 pad00[0x10];
        u32 flags;
    } MovementResource;
    /** @brief Actor metadata containing the collision-scale selector at offset 0x2E. */
    typedef struct
    {
        u8 pad00[0x2E];
        u8 scale;
        u8 pad2F[0x19];
    } MovementScale;
    /** @brief Accessed fields of a 0x23C-byte actor slot; flags overlap the height halfword. */
    typedef struct
    {
        u8 pad00[0xC];
        s32 flags;
        u8 pad10[0x164];
        union
        {
            s32 flags;
            struct
            {
                s16 low;
                s16 height;
            } half;
        } state;
        u8 pad178[0x24];
        s32 polygon;
        s32 region;
        u8 pad1A4[0x98];
    } MovementSlot;
    /** @brief Scratchpad collision request with coordinates, displacement, and packed bounds. */
    typedef struct
    {
        s32 x, y, z, dx, dy, dz, height, polygon, region;
        s16 radius_x, radius_y;
        union
        {
            s32 flags;
            s16 radius_z;
        } tail;
    } MovementCollision;
    /** @brief Contiguous input, destination, and displacement vectors with projected screen
     * coordinates. */
    typedef struct
    {
        FieldVector input, motion, change;
        s16 screen_x, screen_y;
    } MovementWork;

    extern s32 g_field_view_offset_x, g_field_view_offset_y, g_field_view_offset_z, g_field_active_group, D_8010AE64, D_80122B20;
    extern u8 D_8010AE84;
    extern MovementScale g_field_object_parts[];
    extern MovementSlot g_field_object_states[];
    extern MovementResource g_field_resource_entries[];

    s32 func_8001CDAC(s32*, s32*);
    s32 func_8005B368(void*);
    s32 func_8005B6AC(void*);

    s16 func_80091914(void*, s32);
    void func_80091AC8(void*, s32);
    s32 func_80092988(void*, s32*);
    s32 field_find_actor_overlap(void*, void*, s32);
    void field_start_actor_contact_interaction(void*, s32);
    void func_800A2594(s32, s32);
    s32 func_800A6490(void);
    s32 func_800B0850(void);

    FieldBehaviorActor* movement_actor;
    MovementWork work;
    ControllerPortState* pad_base = CONTROLLER_STATE->ports;
    FieldMovementBounds* map = (FieldMovementBounds*)0x801ED400;
    FieldMovementProbe* probe = (FieldMovementProbe*)0x1F800040;
    MovementCollision* collision = (MovementCollision*)0x1F800000;
    s16 actor_state;
    s16 speed_divisor;
    s32 input_z;
    s32 input_x;
    s32 motion_x;
    s32 sentinel_or_reaction;
    s32 buttons_offset;
    s32 analog_offset;
    s32 collision_x;
    s32 motion_z;
    s32 actor_x;
    s32 actor_z;
    s32 collision_z;
    s32 record_input;
    s32 controller_state;
    s32 has_input;
    s32 speed_step;
    u16 buttons_word;
    u8 pad_status;
    ControllerSample* pad;
    MovementSlot* old_slot;
    MovementSlot* new_slot;

    buttons_offset = pad_index;
    if ((u8)pad_base[buttons_offset].published_sample.device_type >= 0xFEU)
    {
        D_8010AE64 = 0;
    }
    else
    {
        buttons_word = pad_base[buttons_offset].published_sample.held_buttons;
        D_8010AE64 = ((buttons_word << 8) & 0xFF00) | (buttons_word >> 8);
    }
    D_8010AE64 =
        ((u32)(D_8010AE64 & 0x40) >> 1) | ((D_8010AE64 & 0x20) * 2) | ((u32)(D_8010AE64 & 0x80) >> 3) | ((D_8010AE64 & 0x10) * 8) | (D_8010AE64 & ~0xF0);
    actor_state = (s16)actor->command;
    record_input = 0;
    if (actor_state != 0x86)
    {
        record_input = actor_state != 0x96;
    }
    func_800A2594(pad_index, record_input);
    if (D_80122B20 != 0)
    {
        D_8010AE64 = 0;
    }
    if (actor->command != 0)
    {
        field_update_actor_command(actor);
        return 0;
    }
    if (func_800B0850() != 0)
    {
        return 0;
    }
    work.input.vx = 0;
    work.input.vy = 0;
    work.input.vz = 0;
    actor->flags = (u32)(actor->flags & ~0x600);
    if ((D_80122B20 == 0) && (g_field_resource_entries[actor->resource_index].flags & 1))
    {
        actor->command = func_80091914(actor, pad_index);
    }
    if (func_800A6490() == 0)
    {
        controller_state = field_text_get_status(0);
        sentinel_or_reaction = -1;
        if (controller_state != sentinel_or_reaction)
        {
            return 0;
        }
        if (field_text_get_status(1) != sentinel_or_reaction)
        {
            return 0;
        }
    }
    input_z = 0;
    analog_offset = pad_index;
    pad = &pad_base[analog_offset].published_sample;
    pad_status = pad_base[analog_offset].published_sample.device_type;
    input_x = input_z;
    if (pad_status != 0 && D_80122B20 == 0)
    {
        if (pad_status >= 0xFEU)
        {
            work.input.vx = 0;
            work.input.vz = 0;
        }
        else
        {
            input_x = work.input.vx = pad->left_stick_x;
            input_z = work.input.vz = -pad->left_stick_y;
        }
    }
    has_input = input_x | input_z;
    if (has_input == 0)
    {
        if (D_8010AE64 & PADLright)
        {
            actor->flags = (u32)((actor->flags & ~0x600) | 0x200);
            input_x += 0x1000;
            work.input.vx += 0x1000;
        }
        if (D_8010AE64 & PADLleft)
        {
            actor->flags = (u32)((actor->flags & ~0x600) | 0x200);
            input_x -= 0x1000;
            work.input.vx -= 0x1000;
        }
        if (D_8010AE64 & PADLdown)
        {
            input_z -= 0x1000;
            work.input.vz -= 0x1000;
            actor->flags = (u32)((actor->flags & ~0x600) | 0x200);
        }
        if (D_8010AE64 & PADLup)
        {
            input_z += 0x1000;
            work.input.vz += 0x1000;
            actor->flags = (u32)((actor->flags & ~0x600) | 0x200);
        }
    }
    else
    {
        actor->flags = (u32)((actor->flags & ~0x600) | 0x200);
    }
    if (g_field_object_states[actor->object_index].flags & 8)
    {
        input_x = -input_x;
        input_z = -input_z;
        work.input.vx = -work.input.vx;
        work.input.vz = -work.input.vz;
    }
    func_8001CDAC(&work.input.vx, &work.motion.vx);
    movement_actor = actor;
    speed_divisor = movement_actor->speed_divisor;
    if (speed_divisor != 0)
    {
        speed_step = (s8)movement_actor->speed_accumulator / speed_divisor;
        movement_actor->speed_accumulator = (s8)((u8)movement_actor->speed_accumulator - speed_step);
    }
    else
    {
        speed_step = 0;
    }
    motion_x = (s32)(work.motion.vx * speed_step) >> 4;
    work.motion.vx = motion_x;
    motion_z = (s32)(work.motion.vz * speed_step) >> 4;
    work.motion.vz = motion_z;
    if (D_8010AE84 == 0)
    {
        if (func_80092988(movement_actor, &work.motion.vx) == 0)
        {
            collision->x = movement_actor->x;
            collision->y = movement_actor->y;
            collision->z = movement_actor->z;
            actor_x = movement_actor->x;
            if ((actor_x >= 0) && (actor_x < (map->width << 8)) && (actor_z = movement_actor->z, (actor_z >= 0)) &&
                (actor_z < ((s32)(map->height << 0x10) >> 7)))
            {
                collision->dy = 0;
                collision->radius_y = 0x10;
                probe->radius_y = 0x10;
                collision->dx = work.motion.vx;
                collision->dz = work.motion.vz;
                if (g_field_object_parts[movement_actor->object_index].scale == 0x40)
                {
                    collision->radius_x = 0xC;
                    probe->radius_x = 0xC;
                    collision->tail.radius_z = 8;
                    probe->radius_z = 8;
                }
                else
                {
                    collision->radius_x = 9;
                    probe->radius_x = 9;
                    collision->tail.radius_z = 6;
                    probe->radius_z = 6;
                }

                collision->tail.flags &= 0xFFFDFFFF;
                collision->tail.flags &= 0xFFFEFFFF;
                collision->polygon = g_field_object_states[movement_actor->object_index].polygon;
                collision->region = g_field_object_states[movement_actor->object_index].region;
                if ((func_8005B6AC(collision) & 3) == 3)
                {
                    movement_actor->flags = (u32)(movement_actor->flags & ~0x600);
                }
                g_field_object_states[movement_actor->object_index].polygon = (s32)collision->polygon;
                g_field_object_states[movement_actor->object_index].region = (s32)collision->region;
                movement_actor->y = (s32)collision->y;
                g_field_object_states[movement_actor->object_index].state.half.height = collision->height / 256;
            }
            else
            {
                g_field_object_states[movement_actor->object_index].polygon = -1;
                g_field_object_states[movement_actor->object_index].region = 0;
                g_field_object_states[movement_actor->object_index].state.half.height = 0;
            }
            collision_x = collision->x;
            collision_z = collision->z;
            work.motion.vx = collision_x;
            work.motion.vz = collision_z;
            work.change.vx = collision_x - movement_actor->x;
            work.change.vy = 0;
            work.change.vz = collision_z - movement_actor->z;
            if (func_80092988(movement_actor, &work.change.vx) != 0)
            {
                work.motion.vx = movement_actor->x;
                work.motion.vz = movement_actor->z;
                movement_actor->flags = (u32)(movement_actor->flags & ~0x600);
            }
            probe->x = (s32)collision->x;
            probe->y = (s32)movement_actor->y;
            probe->z = (s32)collision->z;
            if ((g_field_active_group != 0) && (func_8005B368(probe) != -1))
            {
                work.motion.vx = movement_actor->x;
                work.motion.vz = movement_actor->z;
                movement_actor->flags &= ~0x600;
            }
        }
        else
        {
            work.motion.vx = movement_actor->x;
            work.motion.vz = movement_actor->z;
            movement_actor->flags = (u32)(movement_actor->flags & ~0x600);
        }
    }
    else
    {
        work.motion.vx = motion_x + movement_actor->x;
        work.motion.vz = motion_z + movement_actor->z;
    }
    work.motion.vy = movement_actor->y;
    if (movement_actor->command == 0)
    {
        field_update_actor_movement_animation(movement_actor, input_x, input_z);
    }
    else
    {
        field_prepare_actor_action(movement_actor);
    }
    work.screen_x = g_field_view_offset_x / 256 + (s16)(work.motion.vx / 256 + 160);
    work.screen_y = g_field_view_offset_y / 256 + (s16)(work.motion.vy / 256 + 112) - work.motion.vz / 512 - g_field_view_offset_z / 512;
    if (movement_actor->command == 0)
    {
        if (((u32)g_field_object_states[movement_actor->object_index].state.flags >> 0xE) & 1)
        {
            movement_actor->x = work.motion.vx;
            movement_actor->z = work.motion.vz;
            if (field_find_actor_overlap(movement_actor, movement_actor, 0) == 0)
            {
                old_slot = &g_field_object_states[movement_actor->object_index];
                old_slot->state.flags = (s32)(old_slot->state.flags & ~0x4000);
            }
        }
        else
        {
            sentinel_or_reaction = field_find_actor_overlap(movement_actor, &work.motion.vx, 0) & 0x7FFF;
            if (sentinel_or_reaction < 3)
            {
                movement_actor->x = work.motion.vx;
                movement_actor->z = work.motion.vz;
            }
            else
            {
                if (field_find_actor_overlap(movement_actor, movement_actor, 0) != 0)
                {
                    new_slot = &g_field_object_states[movement_actor->object_index];
                    new_slot->state.flags = (s32)(new_slot->state.flags | 0x4000);
                }
                movement_actor->flags = (u32)(movement_actor->flags & ~0x600);
                field_start_actor_contact_interaction(movement_actor, sentinel_or_reaction);
            }
        }
    }
    func_80091AC8(movement_actor, pad_index);
    if (movement_actor->command != 0)
    {
        field_update_actor_command(movement_actor);
    }
    return ((u32)movement_actor->flags >> 9) & 3;
}

/**
 * @brief Validate and dispatch the pending resource action for an actor.
 *
 * The high command byte selects a resource descriptor. Failed availability or
 * slot checks clear the pending command; accepted actions update slot flags,
 * optionally start an animation, then apply the descriptor's action flags.
 *
 * @param actor Actor whose pending command and action state are updated.
 */
void field_prepare_actor_action(FieldActionActor* actor)
{
    /** @brief Partial 0x23C-byte actor-slot layout and overlapping status bytes. */
    typedef struct
    {
        u8 pad0[0x3C];
        s32 action_parameter;
        u8 pad40[8];
        u16 technique_gauge;
        u8 pad4a[2];
        s32 action_status;
        u8 pad50[0x16F - 0x50];
        u8 action_index;
        u8 pad170[4];
        s32 sequence_flags;
        union
        {
            s32 word;
            u8 bytes[4];
        } flags;
        u8 pad17c[0x23C - 0x17C];
    } Slot;
    /** @brief Eight-byte resource action descriptor. */
    typedef struct
    {
        u16 command, flags, animation, requirement;
    } Action;
    /** @brief Resource directory entry containing the availability flags. */
    typedef struct
    {
        u8 pad0[0x10];
        s32 flags;
    } Resource;
    /** @brief Party entry with the action-table bank selector at offset one. */
    typedef struct
    {
        u8 flags, weapon_type;
        u8 pad2[0x268 - 2];
    } Party;

    extern Slot g_field_object_states[];
    extern Action g_field_resource_actions[];
    extern Party g_field_player_records[];
    extern Resource g_field_resource_entries[];
    extern s32 D_8010AE58;

    s32 field_object_has_active_actor_tracks(s32);
    s32 field_count_free_actor_slots(s32);
    s32 func_8008404C(s32, s32);
    void func_800A3938(s32, s32);
    void field_restart_actor_animation(FieldActionActor*);
    s32 func_800839F8(s32, s32);
    s32 func_80083EEC(s32, s32, s32);
    void field_start_actor_animation(s32, s32, u8*);
    void func_8009D4D8(FieldActionActor*, s32);

    s32 animation_slot;
    s32 resource_offset;
    s32 party_offset;
    s32 action_index;
    u16 requirement;
    u16 animation;
    u8 actor_index;
    u8 mode;
    Slot* slot;
    Action* action;

    if (actor->state.bytes[0] == 0x85)
    {
        g_field_object_states[actor->object_index].flags.word = (s32)(g_field_object_states[actor->object_index].flags.word & ~0x1C);
        if (g_field_resource_entries[actor->resource_index].flags & 1)
        {
            g_field_object_states[actor->object_index].action_index = (s8)(actor->state.word >> 8);
            /* Each resource owns 50 eight-byte descriptors. */
            resource_offset = actor->resource_index * FIELD_RESOURCE_ACTION_BYTES;
            actor->state.word = actor->state.bytes[0];
            slot = &g_field_object_states[actor->object_index];
            mode = slot->action_index;
            {
                s32 descriptor_address = (s32)&g_field_resource_actions[mode];
                action = (Action*)(resource_offset + descriptor_address);
            }
            if (mode == 0xB)
            {
                slot->action_index = *(u8*)&action->command;
            }
            if (!(action->flags & FIELD_ACTION_INSTRUMENT))
            {
                if (action->command == 0 && action->animation == 0)
                {
                    func_800A3938(0x78, 0x80);
                    actor->state.word = 0;
                    return;
                }
            }
            if (!(action->flags & FIELD_ACTION_INSTRUMENT) ||
                ((field_object_has_active_actor_tracks(actor->object_index) == 0) && (field_count_free_actor_slots(actor->object_index) >= 3) &&
                 (actor->variant == 0)))
            {
                g_field_object_states[actor->object_index].flags.word = (s32)(g_field_object_states[actor->object_index].flags.word & ~2);
                if (((u16)action->command & FIELD_ACTION_TECHNIQUE) && !(action->flags & FIELD_ACTION_INSTRUMENT))
                {
                    if ((field_object_has_active_actor_tracks(actor->object_index) == 0) && (D_8010AE58 == 0) &&
                        (field_count_free_actor_slots(actor->object_index) >= 3))
                    {
                        actor_index = actor->object_index;
                        if (g_field_object_states[actor_index].technique_gauge != FIELD_TECHNIQUE_GAUGE_FULL)
                        {
                            func_800A3938(0x78, 0x80);
                        cancel_action:
                            actor->state.word = 0;
                            return;
                        }
                        action_index = action->command & 0x7fff;
                        party_offset = (g_field_player_records[actor_index].weapon_type * 0x18) + 0x88;
                        if (func_8008404C(actor_index, action_index + party_offset) != 0)
                        {
                            g_field_object_states[actor->object_index].sequence_flags =
                                (s32)(g_field_object_states[actor->object_index].sequence_flags | 0x8000);
                            goto start_action;
                        }
                        goto cancel_action;
                    }
                    goto cancel_action;
                }
                requirement = action->requirement;
                if (!(requirement & 0x8000) || (func_8008404C(actor->object_index, requirement & 0x3FF) != 0))
                {
                start_action:
                    if (action->flags & FIELD_ACTION_INSTRUMENT)
                    {
                        g_field_object_states[actor->object_index].action_status = (s32)(g_field_object_states[actor->object_index].action_status & ~1);
                        g_field_object_states[actor->object_index].flags.word = (s32)(g_field_object_states[actor->object_index].flags.word | 0x40);
                        actor->animation_state = 1;
                        actor->active = 1;
                        actor->animation_frame = 0;
                        actor->animation = (u8)((actor->animation & FIELD_ANIMATION_FACING) + 0x10);
                        g_field_object_states[actor->object_index].sequence_flags = (s32)(g_field_object_states[actor->object_index].sequence_flags & ~0x1800);
                        field_restart_actor_animation(actor);
                        if (action->animation != 0)
                        {
                            g_field_object_states[actor->object_index].action_parameter = (s32)action->animation;
                        }
                        g_field_object_states[actor->object_index].sequence_flags = (s32)(g_field_object_states[actor->object_index].sequence_flags & ~0x400);
                    }
                    else if (!((u16)action->command & FIELD_ACTION_TECHNIQUE))
                    {
                        animation = action->animation;
                        if ((animation != 0xFFFF) && (animation != 0))
                        {
                            animation_slot = func_800839F8(actor->object_index, 0);
                            if ((animation_slot != -1) && (func_80083EEC(actor->object_index, animation_slot, action->animation) != 0))
                            {
                                field_start_actor_animation(animation_slot, 0, 0);
                                g_field_object_states[actor->object_index].flags.bytes[1] = animation_slot;
                            }
                        }
                    }
                    func_8009D4D8(actor, *(u8*)&action->flags);
                }
                else
                {
                    goto cancel_action;
                }
            }
            else
            {
                goto cancel_action;
            }
        }
        else
        {
            goto cancel_action;
        }
    }
}

/**
 * @brief Select a movement animation from the requested displacement, or settle to idle.
 * @param record Field record whose direction and animation state are updated.
 * @param delta_x Horizontal displacement used to select a direction.
 * @param delta_z Depth displacement used to select a direction.
 * @note Resource flag 1 selects the alternate directional animation tables.
 */
void field_update_actor_movement_animation(FieldMotionRecord* record, s32 delta_x, s32 delta_z)
{
    /** @brief Resource metadata entry containing animation-layout flags. */
    typedef struct FieldMotionResource
    {
        u8 pad00[0x10];
        u32 flags;
    } FieldMotionResource;

    extern FieldMotionResource g_field_resource_entries[];
    extern s32 g_field_direction_animation_modes[];
    extern s32 g_field_actor_walk_animations[];
    extern s32 g_field_actor_diagonal_walk_animations[];
    extern s32 rand(void);
    extern void field_restart_actor_animation(FieldMotionRecord * record);

    s32* vertical_entry;
    s32* direction_entry;
    s32 sector_index;
    s32 direction_or_animation;
    s32 movement_direction;
    s32 old_animation;
    s32 direction_table_address;
    s32 sector_or_flags;
    s32 direction_offset;
    u8 resource_index;
    u8 current_animation;
    s32 previous_animation;
    s32 low_state;
    s32 idle_state;
    s32 state_mask;
    FieldMotionRecord* refresh_record;
    u8 idle_delay;
    u8 movement_delay;

    resource_index = record->resource_index;
    if (g_field_resource_entries[resource_index].flags & 1)
    {
        if ((delta_x | delta_z) != 0)
        {
            direction_or_animation = ratan2(-delta_z, delta_x);
            direction_or_animation >>= 4;
            direction_or_animation += 0x10;
            if (direction_or_animation < 0)
            {
                direction_or_animation += 0x100;
            }
            if (direction_or_animation >= 0x100)
            {
                direction_or_animation -= 0x100;
            }
            sector_or_flags = direction_or_animation >> 5;
            if ((sector_or_flags == 2) || (sector_or_flags == 6))
            {
                s32* vertical_base;

                state_mask = ~0x80;
                vertical_base = g_field_actor_walk_animations;
                vertical_entry = vertical_base + sector_or_flags;
                if ((record->animation & state_mask) != (*vertical_entry & state_mask))
                {
                    record->direction = (s8)(direction_or_animation & 0xE0);
                    direction_or_animation = (u8)*vertical_entry;
                    old_animation = record->animation;
                    record->animation_frame = 0;
                    record->active = 1;
                    direction_or_animation |= old_animation & FIELD_ANIMATION_FACING;
                    record->animation = direction_or_animation;
                    field_restart_actor_animation(record);
                }
            }
            else
            {
                sector_index = sector_or_flags;
                current_animation = record->animation;
                if (((current_animation != g_field_actor_walk_animations[sector_index]) && (delta_z == 0)) ||
                    ((current_animation != g_field_actor_diagonal_walk_animations[sector_index]) && (delta_z != 0)))
                {
                    record->direction = (s8)(direction_or_animation & 0xE0);
                    if (delta_z != 0)
                    {
                        record->animation = g_field_actor_diagonal_walk_animations[direction_or_animation >> 5];
                    }
                    else
                    {
                        record->animation = g_field_actor_walk_animations[direction_or_animation >> 5];
                    }
                    record->animation_frame = 0;
                    record->active = 1;
                    field_restart_actor_animation(record);
                }
            }
            record->stop_delay = 3U;
            record->active = 1;
            return;
        }
        idle_delay = record->stop_delay;
        if (idle_delay != 0)
        {
            record->stop_delay = (u8)(idle_delay - 1);
        }
        idle_state = record->animation;
        idle_state &= 0x7F;
        if (((idle_state >= 2) && (record->stop_delay == 0)) || ((idle_state < 2) && (record->idle_mode == 0)))
        {
            s32 random_value;

            delta_z = record->animation;
            delta_z &= 0x80;
            random_value = rand();
            refresh_record = record;
            delta_z += random_value >= 0x6001;
            refresh_record->animation = delta_z;
            refresh_record->animation_frame = 0;
            refresh_record->idle_mode = 1U;
            refresh_record->active = 1;
            field_restart_actor_animation(refresh_record);
        }
    }
    else
    {
        if ((delta_x | delta_z) != 0)
        {
            movement_direction = ratan2(-delta_z, delta_x);
            movement_direction >>= 4;
            movement_direction += 0x10;
            if (movement_direction < 0)
            {
                movement_direction += 0x100;
            }
            if (movement_direction >= 0x100)
            {
                movement_direction -= 0x100;
            }
            direction_offset = movement_direction >> 5;
            direction_table_address = (s32)g_field_direction_animation_modes;
            direction_offset *= 4;
            direction_entry = (s32*)(direction_table_address + direction_offset);
            if (record->animation != (*direction_entry + ((record->movement_flags & 1) * 5) + 5))
            {
                record->direction = (s8)(movement_direction & 0xE0);
                direction_or_animation = (u8)*direction_entry;
                sector_or_flags = record->movement_flags;
                record->animation_frame = 0;
                record->active = 1;
                direction_or_animation += (sector_or_flags & 1) * 5;
                direction_or_animation += 5;
                record->animation = (u8)direction_or_animation;
                field_restart_actor_animation(record);
            }
            record->stop_delay = 3U;
            record->active = 1;
            return;
        }
        movement_delay = record->stop_delay;
        if (movement_delay != 0)
        {
            record->stop_delay = (u8)(movement_delay - 1);
        }
        previous_animation = record->animation;
        low_state = previous_animation;
        low_state &= 0x7F;
        if ((low_state >= 5) && (record->stop_delay == 0))
        {
            refresh_record = record;
            refresh_record->active = 1;
            refresh_record->animation_frame = 0;
            refresh_record->animation = (u8)((low_state % 5) | (previous_animation & FIELD_ANIMATION_FACING));
            field_restart_actor_animation(refresh_record);
        }
    }
}

/**
 * @brief Field actor per-frame state machine dispatch (opcodes 0x81..).
 *
 * Reads the actor's current opcode at offset 0x2A (biased by 0x81) and dispatches
 * to the matching movement, animation, path-following, resource-load, or transition
 * handler. Covers the large secondary opcode block for a field actor's active state.
 *
 * @param arg0 Pointer to the field actor state record (0x54-byte layout).
 * @return Unspecified; callers use the updated actor state.
 * @see decomp.me (100%)
 */
s32 field_update_actor_command(FieldBehaviorActor* actor)
{
    /** @brief Target workspace, synthesized action, and GTE displacement vectors. */
    typedef struct FieldCommandWork
    {
        s32 targets[12];
        u16 command;
        u8 pad4A[2];
        u16 animation;
        s16 argument;
        VECTOR displacement;
        VECTOR squared;
    } FieldCommandWork;

    /** @brief Array view of action state and collected target identifiers. */
    typedef struct FieldCommandState
    {
        u8 pad000[0x48];
        s16 technique_gauge;
        u8 pad04A[0x16F - 0x4A];
        u8 action_index;
        u8 pad170[0x172 - 0x170];
        u16 sequence;
        u32 sequence_flags;
        union
        {
            s32 word;
            struct
            {
                u8 flags, animation_slot, unknown_0x17a, target_count;
            } b;
        } action_flags;
        u8 pad17C[4];
        u8 targets[0x23C - 0x180];
    } FieldCommandState;

    /** @brief Party weapon selection and combo input window. */
    typedef struct FieldCommandPlayer
    {
        u8 flags, weapon_type, companion_id, kind;
        u8 pad004[0x257 - 4];
        u8 combo_timer;
        u8 pad258[0x268 - 0x258];
    } FieldCommandPlayer;

    /** @brief Resource byte range, actor slot, and sequence selector. */
    typedef struct FieldCommandResource
    {
        u8* start;
        u8* end;
        u8 unknown_0x08;
        u8 slot_index;
        u8 padA[4];
        s16 sequence_id;
        u32 flags;
    } FieldCommandResource;

    extern u8 g_field_actor_turn_animations;
    extern u8 g_field_action_animation_parameters;
    extern u8 g_field_player_records;
    extern u8 g_field_actors;
    extern u8 g_field_actor_bindings;
    extern u8 g_field_object_states;
    extern u8 g_field_resource_actions;
    extern u8 D_8010A090;
    extern s32 D_8010AE54;
    extern u8 g_field_actor_slots;
    extern u8 g_field_resource_entries;

    void field_stop_actor_animations_for_object(void* record, s32 force);

    FieldCommandWork scratch;
    s16 command_index;
    s32* target_cursor;
    s32 combo_offset;
    s32 waypoint_x;
    s32 target_delta_x;
    s32 approach_delta_x;
    s32 sequence_dx;
    s32 jump_dx;
    s32 walk_dx;
    s32 run_dx;
    s32 screen_route_dx;
    s32 approach_dx;
    s32 timed_dx;
    s32 movement_dx;
    s32 target_count_or_slot;
    s32 animation_target_count;
    s32 vertical_step;
    s32 target_index;
    s32 combo_or_slot;
    s32 eligible_count;
    s32 route_distance_z;
    s32 screen_route_delta_x;
    s32 screen_route_delta_z;
    s32 target_distance_z;
    s32 run_distance_z;
    s32 approach_distance_z;
    s32 binding_offset_17;
    s32 binding_offset_18;
    s32 binding_offset_19;
    s32 binding_offset_20;
    s32 binding_offset_21;
    s32 binding_offset_2;
    s32 binding_offset_3;
    s32 binding_offset_4;
    s32 binding_offset_5;
    s32 route_delta_x;
    s32 route_delta_z;
    s32 heading;
    s32 route_distance_x;
    s32 target_distance_x;
    s32 run_distance_x;
    s32 approach_distance_x;
    u16* view_or_action;
    s32 next_waypoint;
    s32 next_run_waypoint;
    s32 next_screen_waypoint;
    u16 charge_animation;
    u32 sequence_flags;
    u8 action_object_index;
    u8 charge_object_index;
    s32 technique_object_index;
    s32 combo_object_index;
    u8 transition_object_index;
    u8 animation;
    u8 release_object_index;
    s32 combo_target_index;
    u8 turn_animation;
    u8 command_timer;
    void* binding_or_followed_actor;
    void* followed_actor_base;
    void* combo_player_base;
    void* bindings_base;
    void* release_bindings;
    void* target_bindings;
    void* actors_base;
    void* stop_actors;
    void* slot_base1;
    void* slot_base2;
    void* slot_base3;
    void* player_base;
    void* charge_state;
    void* combo_timer_base;
    void* pending_state_base;
    void* actions_base;
    void* action_state;
    void* state_or_base;
    void* combo_player;
    void* transition_player;
    void* completed_player;
    void* destination_player;
    void* waypoint_state;
    void* run_waypoint_state;
    void* screen_waypoint_state;
    void* finished_state;
    void* pending_state;
    void* instrument_state;
    void* released_state;
    void* cancelled_state;
    void* animation_slot;
    void* target_state;
    void* target_list_state;
    void* target_count_state;
    void* retry_state;
    void* instrument_start_state;
    void* combo_actions;
    void* checked_state;
    void* combo_player_state;
    void* near_waypoint_state;

    view_or_action = (u16*)0x801ED480;
    state_or_base = (actor->object_index * 0x23C) + &g_field_object_states;
    command_index = ((FieldBehaviorActorViews*)actor)->command - FIELD_ACTOR_COMMAND_BASE;
    switch (command_index)
    {
    case 0x27:
        if (actor->animation_state == 0)
        {
            ((FieldBehaviorActorViews*)actor)->command = 0U;
        }
        scratch.displacement.vx = 0;
        scratch.displacement.vy = 0;
        scratch.displacement.vz = 0;
        field_resolve_actor_movement(actor, &scratch.displacement.vx, 0);
        return;
    case 0x36:
        if (actor->animation_state == 0)
        {
            if ((actor->command_timer == 0) || (--actor->command_timer == 0))
            {
                ((FieldBehaviorActorViews*)actor)->command = 0U;
            }
        }
        return;
    case 0x32:

        ((FieldBehaviorActorViews*)actor)->command = 0U;
        return;
    case 0x3A:
        if ((actor->retirement_delay != 0) && (--actor->retirement_delay != 0))
        {
            return;
        }
        if (field_object_has_active_actor_tracks(actor->object_index) != 0)
        {
            return;
        }
        goto retire_actor;
    case 0x37:
        if ((actor->retirement_delay != 0) && (--actor->retirement_delay != 0))
        {
            goto follow_actor_position;
        }
        if (field_object_has_active_actor_tracks(actor->object_index) == 0)
        {
            goto stop_following_actor;
        }
    follow_actor_position:
        followed_actor_base = &g_field_actors;
        binding_or_followed_actor = (actor->command_timer * 0x54) + followed_actor_base;
        if (((FieldBehaviorActor*)binding_or_followed_actor)->state == 0xFF)
        {
        stop_following_actor:
            field_stop_actor_animations_for_object(actor, 0);
        retire_actor:
            actor->state = 0xFF;
            return;
        }
        actor->x = ((FieldBehaviorActor*)binding_or_followed_actor)->x;
        actor->y = ((FieldBehaviorActor*)(((actor->command_timer * 0x54) + followed_actor_base)))->y;
        actor->z = ((FieldBehaviorActor*)(((actor->command_timer * 0x54) + followed_actor_base)))->z;
        return;
    case 0x5:
        if (actor->object_index < 2U)
        {
            combo_player_base = &g_field_player_records;
            combo_player_state = (actor->object_index * 0x268) + combo_player_base;
            if ((((FieldBehaviorPlayer*)combo_player_state)->kind == 0) && (((FieldBehaviorPlayer*)combo_player_state)->combo_timer.u8_value != 0))
            {
                if ((func_80091728(actor->object_index, 0U, actor) != 0) && (func_80091728(actor->object_index, 1U, actor) != 0))
                {
                    actions_base = &g_field_resource_actions;
                    combo_actions = (actor->resource_index * FIELD_RESOURCE_ACTION_BYTES) + actions_base;
                    combo_or_slot = func_800AD7DC(((FieldBehaviorAction*)combo_actions)->command, ((FieldBehaviorAction*)combo_actions)->second_command);
                    combo_offset = combo_or_slot * 2;
                    if (combo_or_slot != 0xFF)
                    {
                        scratch.command = combo_or_slot;
                        scratch.animation = (u16) * (combo_offset + &g_field_action_animation_parameters);
                        scratch.argument = (s16)(&g_field_action_animation_parameters)[combo_offset + 1];
                        switch (combo_or_slot)
                        {
                        case 0x34:
                        case 0x3E:
                        case 0x45:
                        case 0x4E:
                        case 0x4F:
                        case 0x50:
                        case 0x51:
                            ACTOR_COMMAND_STATE(actor->object_index).action_index = (u8)combo_or_slot;
                            break;
                        }
                        if (scratch.animation != 0xFFFF)
                        {
                            if (scratch.animation != 0)
                            {
                                combo_or_slot = func_800839F8(actor->object_index, 0);
                                if (combo_or_slot != -1)
                                {
                                    if (func_80083EEC(actor->object_index, combo_or_slot, scratch.animation) != 0)
                                    {
                                        field_start_actor_animation(combo_or_slot, 0, 0);
                                        ACTOR_COMMAND_STATE(actor->object_index).action_flags.b.animation_slot = (u8)combo_or_slot;
                                    }
                                }
                            }
                        }
                        field_apply_action_animation(actor, state_or_base, &scratch.command);
                        PLAYER_COMMAND_STATE(actor->object_index).combo_timer = 0;
                        combo_or_slot = 0;
                        func_800A2DD8(actor->object_index);
                        func_8008BC5C(actor);
                        combo_target_index = actor->object_index;
                        if (ACTOR_COMMAND_STATE(combo_target_index).action_flags.b.target_count != 0)
                        {
                            do
                            {
                                combo_target_index = ACTOR_COMMAND_STATE(combo_target_index).targets[combo_or_slot];
                                ACTOR_COMMAND_STATE(combo_target_index).action_flags.word &= ~0x80;
                                combo_target_index = actor->object_index;
                                combo_or_slot += 1;
                            } while (combo_or_slot < ACTOR_COMMAND_STATE(combo_target_index).action_flags.b.target_count);
                        }
                        ACTOR_COMMAND_STATE(actor->object_index).action_flags.b.target_count = 0;
                        return;
                    }
                }
                combo_timer_base = &g_field_player_records;
                combo_player = (actor->object_index * 0x268) + combo_timer_base;
                ((FieldBehaviorPlayer*)combo_player)->combo_timer.u8_value = (u8)(((FieldBehaviorPlayer*)combo_player)->combo_timer.u8_value - 1);
                goto update_buffered_action;
            }
        }
        goto update_buffered_action;
    case 0x15:
        func_800925EC(actor, 0);
        return;
    case 0x17:
    case 0x1A:
    update_buffered_action:
        func_800925EC(actor, 1);
        return;
    case 0x6:
        func_80093EB4(actor);
        return;
    case 0x10:
        func_8009403C(actor, ((FieldBehaviorState*)state_or_base)->sequence);
        return;
    case 0x31:
        if (!(ACTOR_COMMAND_RESOURCE(actor->resource_index).flags & 1))
        {
            turn_animation = *(((volatile FieldBehaviorActor*)actor)->command_timer + &g_field_actor_turn_animations);
            actor->command_timer = (u8)(((volatile FieldBehaviorActor*)actor)->command_timer + 1);
            actor->animation = turn_animation;
            if (*(actor->command_timer + &g_field_actor_turn_animations) == 0xFF)
            {
                ((FieldBehaviorActorViews*)actor)->command = 0U;
                actor->command_timer = 0U;
            }
        }
        else
        {
            ((FieldBehaviorActorViews*)actor)->command = 0U;
            actor->command_timer = 0U;
            actor->animation = (u8)(actor->animation ^ 0x80);
        }
        ((FieldBehaviorActorViews*)actor)->animation_state = 1;
        ((FieldBehaviorActorViews*)actor)->animation_frame = 0;
        ((FieldBehaviorActorViews*)actor)->active = 1;
        field_restart_actor_animation(actor);
        return;
    case 0x19:
        func_80092550(actor);
        return;
    case 0x18:
        func_800924D8(actor);
        return;
    case 0x1:
        func_800923F0(actor);
        return;
    case 0xD:
    {
        u8 transition_index;
        void* timed_player;
        s16 transition_limit;
        void* transition_base;
        transition_base = &g_field_player_records;
        transition_index = actor->object_index;
        timed_player = (transition_index * 0x268) + transition_base;
        transition_limit = ((FieldBehaviorPlayer*)timed_player)->transition_limit;
        if (transition_limit != 0)
        {
            if (transition_index < 3U)
            {
                if (((((FieldBehaviorPlayer*)timed_player)->transition_time >= transition_limit) ||
                     (((FieldBehaviorPlayer*)timed_player)->transition_time = (s16)((u16)((FieldBehaviorPlayer*)timed_player)->transition_time + 1),
                      transition_player = (actor->object_index * 0x268) + &g_field_player_records,
                      ((((FieldBehaviorPlayer*)transition_player)->transition_time < ((FieldBehaviorPlayer*)transition_player)->transition_limit) == 0))))
                {
                    completed_player = (actor->object_index * 0x268) + &g_field_player_records;
                    ((FieldBehaviorPlayer*)completed_player)->transition_limit = 0;
                    ((FieldBehaviorPlayer*)completed_player)->transition_time = 0;
                    transition_object_index = actor->object_index;
                    destination_player = (transition_object_index * 0x268) + &g_field_player_records;
                    func_80089D44(transition_object_index, ((FieldBehaviorPlayer*)destination_player)->destination_x,
                                  ((FieldBehaviorPlayer*)destination_player)->destination_y, ((FieldBehaviorPlayer*)destination_player)->destination_z);
                    return;
                }
            }
        }
        break;
    }
    case 0xF:
        func_80095168(actor);
        return;
    case 0x11:
        binding_or_followed_actor = &g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            binding_offset_2 = actor->object_index * 0x1C;
        }
        else
        {
            binding_offset_2 = 0x38;
        }
        if (((FieldBehaviorBinding*)((binding_or_followed_actor + binding_offset_2)))->state != 0)
        {
            binding_or_followed_actor = &g_field_actor_bindings;
            if ((u8)actor->object_index < 2U)
            {
                binding_offset_3 = actor->object_index * 0x1C;
            }
            else
            {
                binding_offset_3 = 0x38;
            }
            if (((FieldBehaviorBinding*)((binding_or_followed_actor + binding_offset_3)))->object_index == actor->object_index)
            {
                func_80094EA4(actor);
                return;
            }
        }
        func_8008404C(actor->object_index, ACTOR_COMMAND_RESOURCE(actor->resource_index).sequence_id & 0x3FF);
        return;
    case 0x12:
        binding_or_followed_actor = &g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            binding_offset_4 = actor->object_index * 0x1C;
        }
        else
        {
            binding_offset_4 = 0x38;
        }
        if (((FieldBehaviorBinding*)((binding_or_followed_actor + binding_offset_4)))->state != 0)
        {
            binding_or_followed_actor = &g_field_actor_bindings;
            if ((u8)actor->object_index < 2U)
            {
                binding_offset_5 = actor->object_index * 0x1C;
            }
            else
            {
                binding_offset_5 = 0x38;
            }
            if (((FieldBehaviorBinding*)((binding_or_followed_actor + binding_offset_5)))->object_index == actor->object_index)
            {
                func_80094F40(actor);
                return;
            }
        }

        func_8008404C(actor->object_index, ACTOR_COMMAND_RESOURCE(actor->resource_index).sequence_id & 0x3FF);
        return;
    case 0x13:
        func_80094FDC(actor);
        return;
    case 0x2D:
        command_timer = actor->command_timer;
        if (command_timer != 0)
        {
            command_timer--;
            goto store_command_timer;
        }
        if (!(((FieldBehaviorState*)state_or_base)->action_flags.word & 1))
        {
            func_8008C104(actor);
            return;
        }
        break;
    case 0x0:
        vertical_step = 0;
        func_80094508(actor, 0, vertical_step, 0);
        return;
    case 0xC:
        sequence_dx = rcos(actor->direction * 0x10) >> 4;
        field_apply_sequence_displacement(actor, sequence_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        return;
    case 0xE:
        jump_dx = rcos(actor->direction * 0x10) >> 4;
        func_800949CC(actor, jump_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        return;
    case 0x2E:
        func_800946FC(actor);
        return;
    case 0x2F:
        route_delta_x = ((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_x - actor->x;
        if (route_delta_x < 0)
        {
            route_delta_x += 0xFF;
        }
        scratch.displacement.vx = route_delta_x >> 8;
        route_delta_z = ((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_z - actor->z;
        if (route_delta_z < 0)
        {
            route_delta_z += 0xFF;
        }
        scratch.displacement.vy = route_delta_z >> 8;
        gte_ldlvl(&scratch.displacement);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if ((scratch.squared.vx + scratch.squared.vy) < 0x32)
        {
            actor->x = (s32)((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_x;
            actor->z = (s32)((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_z;
            next_waypoint = ((FieldBehaviorState*)state_or_base)->waypoint_index + 1;
            if (((FieldBehaviorState*)state_or_base)->waypoint_count == next_waypoint)
            {
                ((FieldBehaviorActorViews*)actor)->command = 0U;
            }
            else
            {
                ((FieldBehaviorState*)state_or_base)->waypoint_index = next_waypoint;
            }
            return;
        }
        waypoint_state = state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3);
        heading = ratan2(-((FieldBehaviorState*)waypoint_state)->waypoint_z + actor->z, ((FieldBehaviorState*)waypoint_state)->waypoint_x - actor->x) >> 4;
        actor->direction = (u8)heading;
        movement_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094508(actor, movement_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        return;
    case 0x7:
        walk_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094508(actor, walk_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        return;
    case 0x30:
        near_waypoint_state = state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3);
        route_distance_z = ((FieldBehaviorState*)near_waypoint_state)->waypoint_z - actor->z;
        waypoint_x = ((FieldBehaviorState*)near_waypoint_state)->waypoint_x;
        route_distance_z = abs(route_distance_z);
        route_distance_x = waypoint_x - actor->x;
        route_distance_x = abs(route_distance_x);
        if ((route_distance_z + route_distance_x) < 0x2000)
        {
            actor->x = waypoint_x;
            actor->z = (s32)((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_z;
            next_run_waypoint = ((FieldBehaviorState*)state_or_base)->waypoint_index + 1;
            if (((FieldBehaviorState*)state_or_base)->waypoint_count != next_run_waypoint)
            {
                ((FieldBehaviorState*)state_or_base)->waypoint_index = next_run_waypoint;
                return;
            }
            goto finish_running_command;
        }
        ((FieldBehaviorActorViews*)actor)->movement_flags = 1;
        run_waypoint_state = state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3);
        actor->direction =
            (u8)(ratan2(-((FieldBehaviorState*)run_waypoint_state)->waypoint_z + actor->z, ((FieldBehaviorState*)run_waypoint_state)->waypoint_x - actor->x) >>
                 4);
        movement_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094508(actor, movement_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        if (actor->command == 0)
        {
            ((FieldBehaviorActorViews*)actor)->movement_flags = 0;
            return;
        }
        break;
    case 0x2C:
        run_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094508(actor, run_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        if (actor->command == 0)
        {
            ((FieldBehaviorActorViews*)actor)->movement_flags = 0;
            return;
        }
        break;
    case 0x34:
        screen_route_delta_x = ((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_x - actor->x;
        if (screen_route_delta_x < 0)
        {
            screen_route_delta_x += 0xFF;
        }
        scratch.displacement.vx = screen_route_delta_x >> 8;
        screen_route_delta_z = ((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_z - actor->z;
        if (screen_route_delta_z < 0)
        {
            screen_route_delta_z += 0xFF;
        }
        scratch.displacement.vy = screen_route_delta_z >> 8;
        gte_ldlvl(&scratch.displacement);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if ((scratch.squared.vx + scratch.squared.vy) < 0x32)
        {
            actor->x = (s32)((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_x;
            actor->z = (s32)((FieldBehaviorState*)((state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3))))->waypoint_z;
            next_screen_waypoint = ((FieldBehaviorState*)state_or_base)->waypoint_index + 1;
            if (((FieldBehaviorState*)state_or_base)->waypoint_count == next_screen_waypoint)
            {
                ((FieldBehaviorActorViews*)actor)->command = 0U;
            }
            else
            {
                ((FieldBehaviorState*)state_or_base)->waypoint_index = next_screen_waypoint;
            }
        }
        else
        {
            screen_waypoint_state = state_or_base + (((FieldBehaviorState*)state_or_base)->waypoint_index << 3);
            actor->direction = (u8)(ratan2(-((FieldBehaviorState*)screen_waypoint_state)->waypoint_z + actor->z,
                                           ((FieldBehaviorState*)screen_waypoint_state)->waypoint_x - actor->x) >>
                                    4);
            screen_route_dx = rcos(actor->direction * 0x10) >> 4;
            func_80094508(actor, screen_route_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        }
        {
            s32 negative_view_offset;
            s32 position;
            s32 view_offset;
            view_offset = ((FieldBehaviorView*)view_or_action)->x;
            position = actor->x;
            negative_view_offset = -view_offset;
            if ((negative_view_offset + 0xA00) < position)
            {
                if (position < (negative_view_offset + 0x13600))
                {
                    view_offset = ((FieldBehaviorView*)view_or_action)->z;
                    position = actor->z;
                    negative_view_offset = -view_offset;
                    if ((negative_view_offset + 0xA00) < position)
                    {
                        if (position < (negative_view_offset + 0x1B600))
                        {
                            ((FieldBehaviorActorViews*)actor)->command = 0U;
                            return;
                        }
                    }
                }
            }
        }
        break;
    case 0xA:
        target_distance_z = ((FieldBehaviorState*)state_or_base)->target_z - actor->z;
        target_distance_z = abs(target_distance_z);
        target_delta_x = ((FieldBehaviorState*)state_or_base)->target_x - actor->x;
        target_distance_x = abs(target_delta_x);
        if ((target_distance_z + target_distance_x) >= 0x1000)
        {
            heading = ratan2(actor->z - ((FieldBehaviorState*)state_or_base)->target_z, target_delta_x) >> 4;
            actor->direction = (u8)heading;
            movement_dx = rcos(actor->direction * 0x10) >> 4;
            func_80094508(actor, movement_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
            return;
        }
        ((FieldBehaviorActorViews*)actor)->command = 0U;
        return;
    case 0x2B:
        run_distance_z = ((FieldBehaviorState*)state_or_base)->target_z - actor->z;
        run_distance_z = abs(run_distance_z);
        run_distance_x = ((FieldBehaviorState*)state_or_base)->target_x - actor->x;
        run_distance_x = abs(run_distance_x);
        if ((run_distance_z + run_distance_x) < 0x1000)
        {
        finish_running_command:
            ((FieldBehaviorActorViews*)actor)->command = 0U;
            ((FieldBehaviorActorViews*)actor)->movement_flags = 0;
            return;
        }
        ((FieldBehaviorActorViews*)actor)->movement_flags = 1;
        {
            s32 target_z = ((FieldBehaviorState*)state_or_base)->target_z;
            s32 current_z = actor->z;
            s32 target_x = ((FieldBehaviorState*)state_or_base)->target_x;
            s32 current_x = actor->x;
            actor->direction = (u8)(ratan2(current_z - target_z, target_x - current_x) >> 4);
        }
        movement_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094508(actor, movement_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        if (actor->command == 0)
        {
            ((FieldBehaviorActorViews*)actor)->movement_flags = 0;
            return;
        }
        break;
    case 0xB:
    {
        s32 target_z;
        s32 current_z;
        s32 target_x;
        s32 current_x;
        target_z = ((FieldBehaviorState*)state_or_base)->target_z;
        current_z = actor->z;
        target_x = ((FieldBehaviorState*)state_or_base)->target_x;
        current_x = actor->x;
        heading = (ratan2(current_z - target_z, target_x - current_x) >> 4) - 0x80;
        actor->direction = (u8)heading;
        movement_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094508(actor, movement_dx, 0, (s32)-rsin(actor->direction * 0x10) >> 4);
        return;
    }
    case 0x8:
        func_80094508(actor, 0, -0x100, 0);
        return;
    case 0x9:
        func_80094508(actor, 0, 0x100, 0);
        return;
    case 0x1B:
        func_80094B5C(actor, 1);
        return;
    case 0x1C:
        func_80094B5C(actor, 0);
        return;
    case 0x1F:
        approach_distance_z = ((FieldBehaviorState*)state_or_base)->target_z - actor->z;
        approach_distance_z = abs(approach_distance_z);
        approach_delta_x = ((FieldBehaviorState*)state_or_base)->target_x - actor->x;
        approach_distance_x = abs(approach_delta_x);
        if ((approach_distance_z + approach_distance_x) >= 0x2000)
        {
            actor->direction = (u8)(ratan2(actor->z - ((FieldBehaviorState*)state_or_base)->target_z, approach_delta_x) >> 4);
            approach_dx = rcos(actor->direction * 0x10) >> 4;
            func_80094BC4(actor, approach_dx, (s32)-rsin(actor->direction * 0x10) >> 4);
            return;
        }
        ((FieldBehaviorActorViews*)actor)->command = 0U;
        return;
    case 0x26:
        timed_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094C00(actor, timed_dx, (s32)-rsin(actor->direction * 0x10) >> 4);
        if ((((FieldBehaviorState*)state_or_base)->command_timer == 0) || (--((FieldBehaviorState*)state_or_base)->command_timer == 0))
        {
            ((FieldBehaviorActorViews*)actor)->command = 0U;
        }
        return;
    case 0x35:
        movement_dx = rcos(actor->direction * 0x10) >> 4;
        func_80094690(actor, movement_dx, (s32)-rsin(actor->direction * 0x10) >> 4);
        if ((((FieldBehaviorState*)state_or_base)->command_timer == 0) || (--((FieldBehaviorState*)state_or_base)->command_timer == 0))
        {
            ((FieldBehaviorActorViews*)actor)->command = 0U;
        }
        return;
    case 0x3B:
        actors_base = &g_field_actor_slots;
        bindings_base = &g_field_actor_bindings;
        if ((u8)actor->object_index < 2U)
        {
            binding_offset_17 = actor->object_index * 0x1C;
        }
        else
        {
            binding_offset_17 = 0x38;
        }
        if (((FieldBehaviorActorSlot*)((actors_base - (-(((FieldBehaviorBinding*)((bindings_base + binding_offset_17)))->animation_slot * 0x244)))))
                ->active.u8_value == 0)
        {
            ((FieldBehaviorActorViews*)actor)->command = 0U;
            return;
        }
        break;
    case 0x3C:
        if (actor->animation_state == 0)
        {
            animation = actor->animation;
            if ((animation & FIELD_ANIMATION_INDEX_MASK) == 0xF)
            {
                slot_base1 = &g_field_object_states;
                actor->animation = (u8)((animation & FIELD_ANIMATION_FACING) + 0x31);
                actor->animation_state = 1U;
                ((FieldBehaviorActorViews*)actor)->animation_frame = 0;
                ((FieldBehaviorActorViews*)actor)->active = 1;
                finished_state = (actor->object_index * 0x23C) + slot_base1;
                ((FieldBehaviorState*)finished_state)->sequence_flags.s32_value =
                    (s32)(((FieldBehaviorState*)finished_state)->sequence_flags.s32_value & ~0x1800);
                field_restart_actor_animation(actor);
                return;
            }
            ((FieldBehaviorActorViews*)actor)->command = 0U;
            return;
        }
        break;
    case 0x23:
        if (actor->animation_state == 0)
        {
            ((FieldBehaviorActorViews*)actor)->command = 0U;
            actor->animation_state = 1U;
            ((FieldBehaviorActorViews*)actor)->active = 1;
            actor->animation = (u8)(actor->animation & FIELD_ANIMATION_FACING);
            field_restart_actor_animation(actor);
            return;
        }
        break;
    case 0x14:
        command_timer = actor->command_timer;
        if (command_timer != 0)
        {
            command_timer--;
            goto store_command_timer;
        }
        field_restart_sequence_animation(actor);
        ((FieldBehaviorActorViews*)actor)->command = 0U;
        func_800A2DD8(actor->object_index);
        return;
    store_command_timer:
        actor->command_timer = command_timer;
        return;
    case 0x4:
    {
        void* action_states;
        action_states = &g_field_object_states;
        ((FieldBehaviorState*)(((actor->object_index * 0x23C) + action_states)))->action_flags.bytes.target_count.s = 0;
        if ((u8)actor->object_index < 3U)
        {
            player_base = &g_field_player_records;
            ((FieldBehaviorPlayer*)(((actor->object_index * 0x268) + player_base)))->combo_timer.s8_value = 0;
        }
        if (((FieldBehaviorActorViews*)actor)->flags & 0x1FF)
        {
            s32 action_row = actor->resource_index * FIELD_RESOURCE_ACTION_BYTES;
            view_or_action = (void*)(action_row - -(s32)((((FieldBehaviorState*)(((actor->object_index * 0x23C) + action_states)))->action_index * 8) +
                                                         &g_field_resource_actions));
        }
        else
        {
            action_state = (actor->object_index * 0x23C) + action_states;
            if ((u8)((FieldBehaviorState*)action_state)->action_index < 0xCU)
            {
                {
                    s32 action_row = actor->resource_index * FIELD_RESOURCE_ACTION_BYTES;
                    view_or_action = (void*)(action_row - -(s32)((((FieldBehaviorState*)action_state)->action_index * 8) + &g_field_resource_actions));
                }
            }
            else
            {
                view_or_action = (actor->resource_index * FIELD_RESOURCE_ACTION_BYTES) + &D_8010A090;
            }
        }
    }
        if (((FieldBehaviorAction*)view_or_action)->flags.u8_value != 0xFF)
        {
            s32 action_pending;
            target_count_or_slot = func_8009D1E4(actor->object_index, view_or_action, (((FieldBehaviorAction*)view_or_action)->flags.u16_value >> 8) & 3,
                                                 ((FieldBehaviorState*)state_or_base)->sequence_flags.u16_value & 0x3FF, scratch.targets);
            action_pending = 0;
            if (D_8010AE54 == 0)
            {
                if (!(((FieldBehaviorActorViews*)actor)->flags & 0x1FF))
                {
                    void* check_base = &g_field_object_states;
                    action_object_index = actor->object_index;
                    checked_state = (action_object_index * 0x23C) + check_base;
                    if ((u8)((FieldBehaviorState*)checked_state)->action_index < 0xCU)
                    {
                        action_pending = func_80091728(action_object_index, ((FieldBehaviorState*)checked_state)->action_index, actor);
                    }
                }
                else
                {
                    action_pending = field_actor_action_is_charging(actor);
                }
            }
            if (action_pending != 0)
            {
                target_count_or_slot = (s32)&g_field_object_states;
                pending_state = (actor->object_index * 0x23C) + (void*)target_count_or_slot;
                ((FieldBehaviorState*)pending_state)->action_flags.word = (s32)(((FieldBehaviorState*)pending_state)->action_flags.word | 0x40);
                func_8009D9E0(actor, ((FieldBehaviorAction*)view_or_action)->flags.u8_value);
                if ((((FieldBehaviorAction*)view_or_action)->flags.u16_value & 0x400) && (actor->animation_state == 0))
                {
                    actor->animation_state = 1U;
                    ((FieldBehaviorActorViews*)actor)->active = 1;
                    ((FieldBehaviorActorViews*)actor)->animation_frame = 0;
                    actor->animation = (u8)((actor->animation & FIELD_ANIMATION_FACING) + 0xE);
                    instrument_state = (actor->object_index * 0x23C) + (void*)target_count_or_slot;
                    ((FieldBehaviorState*)instrument_state)->sequence_flags.s32_value =
                        (s32)(((FieldBehaviorState*)instrument_state)->sequence_flags.s32_value & ~0x1800);
                    field_restart_actor_animation(actor);
                }
                state_or_base = &g_field_object_states;
                charge_state = (actor->object_index * 0x23C) + state_or_base;
                sequence_flags = ((FieldBehaviorState*)charge_state)->sequence_flags.u32_value;
                if (!((sequence_flags >> 0xA) & 1) && ((u16)((FieldBehaviorState*)charge_state)->action_charge >= 0x41U))
                {
                    charge_animation = ((FieldBehaviorAction*)view_or_action)->animation;
                    if ((charge_animation != 0xFFFF) && (charge_animation != 0))
                    {
                        ((FieldBehaviorState*)charge_state)->sequence_flags.u32_value = (u32)(sequence_flags | 0x400);
                        target_count_or_slot = func_800839F8(actor->object_index, 0);
                        if ((target_count_or_slot != -1) &&
                            (func_80083EEC(actor->object_index, target_count_or_slot, ((FieldBehaviorAction*)view_or_action)->animation) != 0))
                        {
                            field_start_actor_animation(target_count_or_slot, 0, 0);
                            ((FieldBehaviorState*)(((actor->object_index * 0x23C) + state_or_base)))->action_flags.bytes.animation_slot = target_count_or_slot;
                            charge_object_index = actor->object_index;
                            func_800A623C(charge_object_index,
                                          (0x80000000 | (charge_object_index << 0x10)) |
                                              (((FieldBehaviorState*)(((charge_object_index * 0x23C) + state_or_base)))->action_index - 4));
                            return;
                        }
                    }
                }
            }
            else
            {
                released_state = (actor->object_index * 0x23C) + &g_field_object_states;
                ((FieldBehaviorState*)released_state)->action_flags.word = (s32)(((FieldBehaviorState*)released_state)->action_flags.word & ~0x40);
                release_object_index = actor->object_index;
                if (!(((u32)ACTOR_COMMAND_STATE(release_object_index).sequence_flags >> 0xA) & 1))
                {
                    stop_actors = &g_field_actor_slots;
                    release_bindings = &g_field_actor_bindings;
                    if (release_object_index < 2U)
                    {
                        binding_offset_18 = release_object_index * 0x1C;
                    }
                    else
                    {
                        binding_offset_18 = 0x38;
                    }
                    ((FieldBehaviorActorSlot*)((stop_actors - (-(((FieldBehaviorBinding*)((release_bindings + binding_offset_18)))->animation_slot * 0x244)))))
                        ->active.s8_value = 0;
                    func_80084424(actor->object_index);
                    slot_base2 = &g_field_object_states;
                    actor->animation_state = 1U;
                    ((FieldBehaviorActorViews*)actor)->active = 1;
                    ((FieldBehaviorActorViews*)actor)->animation_frame = 0;
                    actor->animation = (u8)((actor->animation & FIELD_ANIMATION_FACING) + 0xF);
                    cancelled_state = (actor->object_index * 0x23C) + slot_base2;
                    ((FieldBehaviorState*)cancelled_state)->sequence_flags.s32_value =
                        (s32)(((FieldBehaviorState*)cancelled_state)->sequence_flags.s32_value & ~0x1800);
                    field_restart_actor_animation(actor);
                    ((FieldBehaviorActorViews*)actor)->command = 0xBDU;
                    return;
                }
                target_bindings = &g_field_actor_bindings;
                if (release_object_index < 2U)
                {
                    binding_offset_19 = release_object_index * 0x1C;
                }
                else
                {
                    binding_offset_19 = 0x38;
                }
                eligible_count = field_filter_action_targets(target_count_or_slot, scratch.targets,
                                                             ((FieldBehaviorBinding*)((target_bindings + binding_offset_19)))->animation_slot);
                actors_base = &g_field_actor_slots;
                bindings_base = &g_field_actor_bindings;
                if ((u8)actor->object_index < 2U)
                {
                    binding_offset_20 = actor->object_index * 0x1C;
                }
                else
                {
                    binding_offset_20 = 0x38;
                }
                animation_slot = actors_base + (((FieldBehaviorBinding*)((bindings_base + binding_offset_20)))->animation_slot * 0x244);
                ((FieldBehaviorActorSlot*)animation_slot)->action_flags = (s32)((((FieldBehaviorActorSlot*)animation_slot)->action_flags & ~0x1E) |
                                                                                ((((FieldBehaviorAction*)view_or_action)->flags.u8_value & 0xF) * 2));
                if (!(((FieldBehaviorAction*)view_or_action)->requirement & FIELD_ACTION_TARGETED))
                {
                    if (field_start_action_animation(actor->object_index, 0, NULL, ((FieldBehaviorAction*)view_or_action)->requirement) != 0)
                    {
                        goto execute_action;
                    }
                    else
                    {
                        goto retry_action;
                    }
                }
                if (eligible_count != 0)
                {
                    animation_target_count = eligible_count;
                    if (eligible_count >= 0xA)
                    {
                        eligible_count = 9;
                        animation_target_count = 9;
                    }
                    if (field_start_action_animation(actor->object_index, animation_target_count, scratch.targets,
                                                     ((FieldBehaviorAction*)view_or_action)->requirement) != 0)
                    {
                        target_index = 0;
                        if (eligible_count > 0)
                        {
                            do
                            {
                                target_cursor = scratch.targets + target_index;
                                target_state = (*target_cursor * 0x23C) + &g_field_object_states;
                                ((FieldBehaviorState*)target_state)->action_flags.word = (s32)(((FieldBehaviorState*)target_state)->action_flags.word | 0x80);
                                target_list_state = (actor->object_index * 0x23C) + &g_field_object_states;
                                ((FieldBehaviorState*)((target_list_state + ((FieldBehaviorState*)target_list_state)->action_flags.bytes.target_count.u)))
                                    ->targets = (u8)*target_cursor;
                                target_index += 1;
                                target_count_state = (actor->object_index * 0x23C) + &g_field_object_states;
                                ((FieldBehaviorState*)target_count_state)->action_flags.bytes.target_count.u =
                                    (u8)(((FieldBehaviorState*)target_count_state)->action_flags.bytes.target_count.u + 1);
                            } while (target_index < eligible_count);
                        }
                        goto execute_action;
                    }
                retry_action:
                    pending_state_base = &g_field_object_states;
                    retry_state = (actor->object_index * 0x23C) + pending_state_base;
                    ((FieldBehaviorState*)retry_state)->action_flags.word = (s32)(((FieldBehaviorState*)retry_state)->action_flags.word | 0x40);
                    return;
                }
                actors_base = &g_field_actor_slots;
                bindings_base = &g_field_actor_bindings;
                if ((u8)actor->object_index < 2U)
                {
                    binding_offset_21 = actor->object_index * 0x1C;
                }
                else
                {
                    binding_offset_21 = 0x38;
                }
                ((FieldBehaviorActorSlot*)((actors_base - (-(((FieldBehaviorBinding*)((bindings_base + binding_offset_21)))->animation_slot * 0x244)))))
                    ->active.s8_value = 0;
                func_80084424(actor->object_index);
                ACTOR_COMMAND_STATE(actor->object_index).action_flags.b.animation_slot = 0xFF;
                goto execute_action;
            }
        }
        else
        {
        execute_action:
            if (((FieldBehaviorAction*)view_or_action)->flags.u16_value & 0x400)
            {
                slot_base3 = &g_field_object_states;
                ((FieldBehaviorActorViews*)actor)->command = 0x87U;
                actor->animation_state = 1U;
                ((FieldBehaviorActorViews*)actor)->active = 1;
                ((FieldBehaviorActorViews*)actor)->animation_frame = 0;
                actor->animation = (u8)((actor->animation & FIELD_ANIMATION_FACING) + 0xF);
                instrument_start_state = (actor->object_index * 0x23C) + slot_base3;
                ((FieldBehaviorState*)instrument_start_state)->sequence_flags.s32_value =
                    (s32)(((FieldBehaviorState*)instrument_start_state)->sequence_flags.s32_value & ~0x1800);
                field_restart_actor_animation(actor);
                ((FieldBehaviorActorViews*)actor)->flags |= 0x800;
                return;
            }
            if (((FieldBehaviorAction*)view_or_action)->command & 0x8000)
            {
                ACTOR_COMMAND_STATE(actor->object_index).technique_gauge = 0;
                ((FieldBehaviorState*)state_or_base)->sequence_position = 0;
                ((FieldBehaviorState*)state_or_base)->sequence_id = (s32)((FieldBehaviorAction*)view_or_action)->requirement;
                ((FieldBehaviorState*)state_or_base)->action_parameter = (s32)((FieldBehaviorAction*)view_or_action)->animation;
                ((FieldBehaviorState*)state_or_base)->sequence_flags.u32_value =
                    (s32)((s32)((FieldBehaviorState*)state_or_base)->sequence_flags.u32_value & ~0x1800);
                ((FieldBehaviorState*)state_or_base)->sequence_id = (s32)((((((FieldBehaviorAction*)view_or_action)->command & 0x7FFF) + 0x88) | 0x8000) +
                                                                          (PLAYER_COMMAND_STATE(actor->object_index).weapon_type * 0x18));
                if ((u8)actor->object_index < 3U)
                {
                    technique_object_index = actor->object_index;
                    func_800A623C(technique_object_index, (((FieldBehaviorAction*)view_or_action)->command & 0x7FFF) +
                                                              (PLAYER_COMMAND_STATE(technique_object_index).weapon_type * 0x18));
                }
                field_execute_actor_sequence(actor, ((FieldBehaviorAction*)view_or_action)->command & 0x7FFF);
                ((FieldBehaviorState*)state_or_base)->sequence = (u16)(((FieldBehaviorAction*)view_or_action)->command & 0x7FFF);
                ((FieldBehaviorActorViews*)actor)->command = 0x91U;
                field_restart_actor_animation(actor);
                ((FieldBehaviorActorViews*)actor)->flags = (s32)(((FieldBehaviorActorViews*)actor)->flags | 0x800);
                return;
            }
            if ((u8)actor->object_index < 3U)
            {
                combo_object_index = actor->object_index;
                if ((u8)ACTOR_COMMAND_STATE(combo_object_index).action_index < 2U)
                {
                    PLAYER_COMMAND_STATE(combo_object_index).combo_timer = 0xA;
                }
            }
            field_apply_action_animation(actor, state_or_base, view_or_action);
            return;
        }
        break;
    default:
        scratch.displacement.vx = 0;
        scratch.displacement.vy = 0;
        scratch.displacement.vz = 0;
        field_resolve_actor_movement(actor, &scratch.displacement.vx, 0);
        ((FieldBehaviorActorViews*)actor)->command = 0U;
        return;
    }
}

/**
 * @brief Compact eligible actor indices into the front of the supplied list.
 * @param count Number of indices to inspect.
 * @param indices Input indices and destination for the eligible indices.
 * @return Number of eligible indices copied.
 * @note The original routine reserves scratch space for 16 eligible entries.
 */
s32 field_filter_action_targets(s32 count, s32* indices)
{
    /** @brief Animation entry containing eligibility state and actor slot. */
    typedef struct
    {
        u8 pad0[0x25];
        u8 state;
        u8 pad26[4];
        s16 command;
        u8 pad2C[14];
        u8 object_index;
        u8 pad3B[0x54 - 0x3B];
    } Entry;
    /** @brief Actor record containing resources, group, and eligibility flags. */
    typedef struct
    {
        s32 max_hp;
        s32 hp;
        s32 display_hp;
        s32 status_flags;
        s32 group_flags;
        u8 pad14[0x12C - 0x14];
        s32 unknown_0x12c;
        u8 pad130[0x174 - 0x130];
        s32 sequence_flags;
        s32 action_flags;
        u8 pad17C[0x23C - 0x17C];
    } Actor;

    extern Entry g_field_actors[];
    extern Actor g_field_object_states[];
    extern s32 g_field_active_group;
    extern u8 g_field_actor_bindings[];

    Entry* entry_base;
    Actor* actor_base;
    u8* slot_base;
    s32 group;
    s32 sentinel;
    s32 scratch[16];
    s16 command;
    s32* output;
    s32* input;
    s32* accepted;
    s32* copy;
    s32 owner_index;
    s32 slot;
    s32 flags;
    s32 index;
    s32 candidate_index;
    s32 action_flags;
    s32 i;
    s32 accepted_count;
    s32 binding_offset;
    s32 owner_binding_offset;
    Entry* actor;
    Actor* state;

    output = indices;
    i = 0;
    accepted_count = i;
    if (count > 0)
    {
        sentinel = 0xFF;
        entry_base = g_field_actors;
        actor_base = g_field_object_states;
        group = g_field_active_group;
        slot_base = g_field_actor_bindings;
        input = output;
        accepted = scratch;
        do
        {
            candidate_index = *input;
            if (candidate_index != sentinel)
            {
                actor = (Entry*)(candidate_index * sizeof(Entry) + (s32)entry_base);
                state = (Actor*)(candidate_index * sizeof(Actor) + (s32)actor_base);
                if ((actor->state != sentinel) && (state->hp != 0))
                {
                    flags = state->action_flags;
                    if (!(flags & 1) && ((i < 3) || ((state->group_flags & 0xF) == group)))
                    {
                        command = actor->command;
                        if ((command != 0x91) && (command != 0xAE) && (command != 0x87))
                        {
                            if (!(flags & 0x40))
                            {
                                if ((u8)actor->object_index < 2U)
                                {
                                    binding_offset = actor->object_index * 0x1C;
                                }
                                else
                                {
                                    binding_offset = 0x38;
                                }
                                slot = actor->object_index;
                                actor = (Entry*)*(s32*)(slot_base + binding_offset + 0xC);
                                owner_index = (s32)actor;
                                if (owner_index == slot)
                                {
                                    if ((u32)(owner_index & 0xFF) < 2U)
                                    {
                                        owner_binding_offset = owner_index * 0x1C;
                                    }
                                    else
                                    {
                                        owner_binding_offset = 0x38;
                                    }
                                    if (*(s32*)(slot_base + owner_binding_offset) != 0)
                                    {
                                        i += 1;
                                        input++;
                                        continue;
                                    }
                                }
                            }
                            {
                                if ((state->unknown_0x12c != 0) && !(state->status_flags & 0x2280))
                                {
                                    action_flags = state->action_flags;
                                    if (!(action_flags & 0x20) && !((u8)action_flags & 0x80) && !(state->sequence_flags & 0x8000))
                                    {
                                        accepted_count += 1;
                                        *accepted = *input;
                                        accepted++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            i += 1;
            input++;
        } while (i < count);
    }
    i = 0;
    if (accepted_count > 0)
    {
        copy = scratch;
        do
        {
            index = *copy;
            copy++;
            i += 1;
            *output = index;
            output++;
        } while (i < accepted_count);
    }
    return accepted_count;
}

/**
 * @brief Apply an animation command and its actor and sound side effects.
 * @param actor Animation entry to update.
 * @param state Actor receiving the command.
 * @param action Animation command and argument.
 * @return Unspecified.
 */
s32 field_apply_action_animation(FieldActionAnimationActor* actor, FieldActionAnimationState* state, FieldActionAnimationCommand* action)
{
    /** @brief Actor slot record containing the type byte. */
    typedef struct
    {
        u8 pad0;
        u8 weapon_type;
        u8 pad2[0x268 - 2];
    } ActorData;

    extern ActorData g_field_player_records[];
    extern u8 g_field_action_sound_ids[];
    extern void field_restart_actor_animation(FieldActionAnimationActor*);
    extern void func_800A3938(s32, s32);
    extern void func_800B61C4(s32);

    s32 animation_id;
    s32 masked_command_code;
    s32 command_code;
    u16 gauge;
    u16 next_gauge;

    state->action_parameter = (s32)action->argument;
    command_code = action->code.half;
    if (((u32)(command_code - 0x2F) < 2U) || (masked_command_code = command_code & 0xFFFF, (masked_command_code == 0x44)) || (masked_command_code == 0x45))
    {
        state->status_flags = (s32)(state->status_flags | 0x4000);
    }
    if ((action->code.half == 0x1F) && (actor->variant.half != 0) &&
        ((actor->object_index >= 2U) || (g_field_player_records[actor->object_index].weapon_type != 0xA)))
    {
        actor->animation = (u8)(actor->variant.byte[0] + (action->code.byte[0] + (actor->animation & FIELD_ANIMATION_FACING)));
    }
    else
    {
        actor->animation = (u8)(action->code.byte[0] + (actor->animation & FIELD_ANIMATION_FACING));
    }
    actor->animation_state = 1;
    actor->animation_frame = 0;
    actor->active = 1;
    state->sequence_flags = (s32)(state->sequence_flags & ~0x1800);
    field_restart_actor_animation(actor);
    animation_id = actor->animation & FIELD_ANIMATION_INDEX_MASK;
    switch (animation_id)
    {
    case 0x41:
        func_800B61C4(state->unknown_0x14);
        actor->command = 0x86;
        break;
    case 0x33:
        gauge = state->technique_gauge;
        next_gauge = gauge < 0xE0U ? gauge + 0x20 : 0xFF;
        state->technique_gauge = next_gauge;
        actor->command = 0x86;
        break;
    case 0x8:
    case 0xA:
    case 0x31:
    case 0x3D:
        actor->command = 0x86;
        actor->flags = (s32)(actor->flags | 0x800);
        break;
    default:
        actor->command = 0x86;
        break;
    }
    if (((u8)actor->object_index < 2U) && (g_field_action_sound_ids[actor->animation & FIELD_ANIMATION_INDEX_MASK] != 0xFF))
    {
        func_800A3938(g_field_action_sound_ids[actor->animation & FIELD_ANIMATION_INDEX_MASK], 0x80);
    }
}

/**
 * @brief Report whether an actor action charge is below its limit.
 * @param rec Actor record whose slot index selects the slot to inspect.
 * @return Non-zero when the action charge is below 0xFF.
 */
s32 field_actor_action_is_charging(FieldChargeActor* rec)
{
    /** @brief Per-object action charge. */
    typedef struct
    {
        u8 pad0[0x4A];
        u16 action_charge;
        u8 pad4C[0x23C - 0x4C];
    } FieldChargeState;

    extern FieldChargeState g_field_object_states[];

    return g_field_object_states[rec->object_index].action_charge < 0xFF;
}

/**
 * @brief Start an actor animation, or defer to the layered variant for banked flags.
 * @param object_index Object owning the pending slot request.
 * @param target_count Number of animation targets.
 * @param targets Animation target identifiers.
 * @param flags Layer-selection flags; other bit meanings are unknown.
 * @return One when the request was applied.
 */
s32 field_start_action_animation(s32 object_index, s32 target_count, u8* targets, s32 flags)
{
    /** @brief Active animation slot recorded by the action dispatcher. */
    typedef struct
    {
        u8 pad0[0x179];
        s8 animation_slot;
        u8 pad17A[0x23C - 0x17A];
    } FieldActionSlotState;

    extern FieldActionSlotState g_field_object_states[];

    s32 func_800839F8(s32 object_index, s32 target_count);
    s32 func_80083EEC(s32 object_index, s32 animation_slot, s32 animation);
    void field_start_actor_animation(s32 object_index, s32 target_count, u8 * targets);

    s32 animation_slot;

    if (flags & 0x8000)
    {
        return field_start_bound_action_animation(object_index, target_count, targets, flags);
    }

    animation_slot = func_800839F8(object_index, 0);
    if ((animation_slot != -1) && (func_80083EEC(object_index, animation_slot, flags & 0x3FF) != 0))
    {
        field_start_actor_animation(animation_slot, target_count, targets);
        g_field_object_states[object_index].animation_slot = animation_slot;
    }
    return 1;
}

/**
 * @brief Start the requested actor animation and initialize any extra layers.
 * @param actor_id Actor owning the pending slot request.
 * @param target_count Number of animation targets.
 * @param targets Animation target identifiers.
 * @param flags Layer-selection flags; other bit meanings are unknown.
 * @return One when the request was applied, or zero when it was not ready.
 */
s32 field_start_bound_action_animation(s32 actor_id, s32 target_count, u8* targets, s32 flags)
{
    /** @brief Animation definition view with the original 0x1C-byte stride. */
    typedef struct
    {
        u8 pad0[0x12];
        u16 unknown_0x12;
        u8 pad14[2];
        u8 animation_mode;
        u8 pad17[5];
    } AnimationDef;

    /** @brief Actor slot view with the original 0x244-byte stride. */
    typedef struct
    {
        u8 pad0[0xC];
        AnimationDef* animation;
        AnimationDef* animations;
        u8 pad14[0x10];
        u8 active;
        u8 pad25[4];
        u8 animation_index;
        u8 sequence_active;
        u8 pad2b[0x222 - 0x2B];
        u16 unknown_0x222;
        u8 pad224[0xF];
        u8 slot_index;
        u8 pad234[4];
        s16 animation_mode;
        u8 pad23a[0xA];
    } ActorSlot;

    /** @brief Pending actor-slot request, indexed with the original 0x1C-byte stride. */
    typedef struct
    {
        s32 state;
        u8 pad4[8];
        s32 object_index;
        u8 pad10[8];
        s32 animation_slot;
    } SlotRequest;

    /** @brief Actor state view containing the active slot identifier. */
    typedef struct
    {
        u8 pad0[0x179];
        u8 animation_slot;
        u8 pad17a[0x23C - 0x17A];
    } ActorState;

    void bcopy(void*, void*, s32);
    void field_start_actor_animation(s32, s32, u8*);
    s32 func_800839F8(s32, s32);
    extern SlotRequest g_field_actor_bindings[];
    extern ActorState g_field_object_states[];
    extern ActorSlot g_field_actor_slots[];

    u16 frame_value;
    s32 remaining_layers;
    s32 result;
    s32 request_index;
    s32 new_slot_id;
    s32 layer_index;
    ActorSlot* slot;
    SlotRequest* request;
    ActorSlot* base_slot;
    AnimationDef* animation;
    ActorSlot* default_slot;
    ActorSlot* selected_slot;
    ActorSlot* layer_slot;
    ActorSlot* initialized_slot;

    request_index = actor_id;
    if (actor_id >= 3)
    {
        request_index = 2;
    }
    request = &g_field_actor_bindings[request_index];
    if (request->object_index != actor_id)
    {
        return 0;
    }
    if (request->state != 2)
    {
        return 0;
    }

    {
        if (flags & 0x4000)
        {
            if (!(flags & 0x400))
            {
                g_field_actor_slots[request->animation_slot].animation_index = (s8)((flags >> 0xC) & 3);
                layer_slot = &g_field_actor_slots[request->animation_slot];
                layer_slot->unknown_0x222 = layer_slot->animations[layer_slot->animation_index].unknown_0x12;
            }
            else
            {
                g_field_actor_slots[request->animation_slot].animation_index = 0;
                base_slot = &g_field_actor_slots[request->animation_slot];
                remaining_layers = (flags >> 0xC) & 3;
                base_slot->unknown_0x222 = (u16)base_slot->animations->unknown_0x12;
                layer_index = 1;
                g_field_actor_slots[request->animation_slot].sequence_active = 1;
                if (remaining_layers != 0)
                {
                    do
                    {
                        new_slot_id = func_800839F8(actor_id, 0);
                        if (new_slot_id != -1)
                        {
                            slot = &g_field_actor_slots[new_slot_id];
                            bcopy(&g_field_actor_slots[request->animation_slot], slot, 0x244);
                            animation = slot->animations;
                            slot->slot_index = new_slot_id;
                            slot->animation_index = layer_index;
                            frame_value = animation->unknown_0x12;
                            slot->animation_mode = 0;
                            slot->animation = &animation[layer_index];
                            result = 1;
                            slot->sequence_active = result;
                            slot->active = result;
                            slot->unknown_0x222 = frame_value;
                            field_start_actor_animation(new_slot_id, target_count, targets);
                        }
                        remaining_layers -= 1;
                        layer_index += 1;
                    } while (remaining_layers != 0);
                }
            }
        }
        else
        {
            default_slot = &g_field_actor_slots[request->animation_slot];
            default_slot->unknown_0x222 = (u16)default_slot->animations->unknown_0x12;
            g_field_actor_slots[request->animation_slot].animation_index = 0;
        }
        initialized_slot = &g_field_actor_slots[request->animation_slot];
        initialized_slot->animation_mode = initialized_slot->animations[initialized_slot->animation_index].animation_mode;
        selected_slot = &g_field_actor_slots[request->animation_slot];
        selected_slot->animation = selected_slot->animations + selected_slot->animation_index;
        g_field_actor_slots[request->animation_slot].active = 1;
        field_start_actor_animation(request->animation_slot, target_count, targets);
        g_field_object_states[actor_id].animation_slot = (u8)request->animation_slot;
    }
    result = 1;
    return result;
}
