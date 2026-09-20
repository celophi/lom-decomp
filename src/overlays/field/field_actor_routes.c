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
    void func_8008EF0C(FieldRouteActor*);
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
        func_8008EF0C(actor);
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
