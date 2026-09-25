/**
 * @file field_actor_target_queries.c
 * @brief Distance, facing and box predicates for field actors, and the target collector that applies them.
 */

#include "common.h"
#include "field_types.h"
#include "field_effect_types.h"
#include "field_actor.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Scratchpad vector holding the scaled actor delta. */
#define FIELD_TARGET_DELTA ((VECTOR*)0x1F800000)
/** @brief Scratchpad vector receiving the squared delta components. */
#define FIELD_TARGET_SQUARE ((VECTOR*)0x1F800010)

/** @brief Inner radius of the ring predicate, in whole units. */
#define FIELD_TARGET_RING_INNER 64
/** @brief Half depth of the box predicate (32 whole units). */
#define FIELD_TARGET_BOX_HALF_DEPTH 0x2000
/** @brief The facing limit is the angle of @p distance over this many units. */
#define FIELD_TARGET_FACING_RANGE 100
/** @brief Ground attachment points of an object. */
#define FIELD_GROUND_POINT_COUNT 3

/** @brief Action descriptor fields read by the collector. */
typedef struct
{
    u8 pad[2];
    /** @brief Entry of g_field_target_filters to apply. */
    u8 filter;
} FieldTargetSpec;

/** @brief Predicate for a source actor, candidate actor and caller argument. */
typedef s32 (*FieldTargetFilter)(FieldActor* source, FieldActor* candidate, s32 argument);

/** @brief Binding state and its owning actor in a 0x1C-byte record. */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner;
    u8 pad10[12];
} FieldTargetBinding;

extern FieldTargetFilter g_field_target_filters[];
extern s32 g_field_active_group;
extern FieldTargetBinding g_field_actor_bindings[];
extern s32 g_field_duel_mode;

/**
 * @brief Test whether a candidate actor is within a distance of the source actor.
 * @param source Source actor.
 * @param candidate Candidate actor; unused records never pass.
 * @param max_distance Exclusive distance limit in whole units.
 * @return 1 when the candidate is closer than @p max_distance, otherwise 0.
 */
s32 field_actor_within_distance(FieldActor* source, FieldActor* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    s32 distance;

    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        delta->vx = (candidate->x - source->x) >> 8;
        delta->vy = (candidate->y - source->y) >> 8;
        delta->vz = (candidate->z - source->z) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(square);
        distance = SquareRoot0(square->vx + square->vy + square->vz);
        if (distance < max_distance)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Test whether a candidate actor lies in the ring around the source actor.
 * @param source Source actor.
 * @param candidate Candidate actor; unused records never pass.
 * @param max_distance Ring width beyond FIELD_TARGET_RING_INNER.
 * @return 1 when the candidate is inside the ring, otherwise 0.
 */
s32 field_actor_within_ring(FieldActor* source, FieldActor* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    s32 distance;

    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        delta->vx = (candidate->x - source->x) >> 8;
        delta->vy = (candidate->y - source->y) >> 8;
        delta->vz = (candidate->z - source->z) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(square);
        distance = SquareRoot0(square->vx + square->vy + square->vz);
        if (distance < max_distance + FIELD_TARGET_RING_INNER)
        {
            if (distance > FIELD_TARGET_RING_INNER)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Test whether the source actor faces the candidate within an angular limit.
 * @param source Source actor; its facing bit selects the direction it looks in.
 * @param candidate Candidate actor.
 * @param distance The limit is the angle of @p distance over FIELD_TARGET_FACING_RANGE.
 * @return 1 when the candidate is within the angular limit, otherwise 0.
 */
s32 field_actor_faces_target(FieldActor* source, FieldActor* candidate, s32 distance)
{
    s32 limit;
    s32 angle;

    limit = ratan2(distance, FIELD_TARGET_FACING_RANGE);
    if (!(source->animation & FIELD_ANIMATION_FACING))
    {
        angle = ratan2((source->z - candidate->z) >> 8, (source->x - candidate->x) >> 8);
        if (angle > FIELD_ANGLE_HALF_TURN)
        {
            angle = FIELD_ANGLE_TURN - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    else
    {
        angle = ratan2((candidate->z - source->z) >> 8, (candidate->x - source->x) >> 8);
        if (angle > FIELD_ANGLE_HALF_TURN)
        {
            angle = FIELD_ANGLE_TURN - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Test whether a candidate actor lies in a box around the source actor.
 * @param source Source actor at the box center.
 * @param candidate Candidate actor; unused records never pass.
 * @param half_width Half-width of the box along X in whole units; the depth is fixed.
 * @return 1 when the candidate is inside the box, otherwise 0.
 */
s32 field_actor_within_box(FieldActor* source, FieldActor* candidate, s32 half_width)
{
    s32 dz;
    s32 dx;

    if (candidate->presence == FIELD_ACTOR_UNUSED)
    {
        return 0;
    }
    dz = candidate->z - source->z;
    if (dz > FIELD_TARGET_BOX_HALF_DEPTH)
    {
        return 0;
    }
    if (dz < -FIELD_TARGET_BOX_HALF_DEPTH)
    {
        return 0;
    }
    dx = candidate->x;
    dx = dx - source->x;
    half_width <<= 8;
    dz = dx;
    if (half_width < dz)
    {
        return 0;
    }
    return dz >= -half_width;
}

/**
 * @brief Test the candidate against the source actor's ground attachment points.
 * @param source Source actor; its object index selects the points.
 * @param candidate Candidate actor; unused records never pass.
 * @param max_distance Exclusive distance limit in whole units.
 * @return 1 when any point is closer than @p max_distance, otherwise 0.
 */
s32 field_actor_near_ground_points(FieldActor* source, FieldActor* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    Vec2s* point;
    s32 i;
    s32 distance;
    /* Taking the label's address keeps the found path a block of its own. */
    static void* const keep[] __attribute__((section(".discard"))) = {
        &&found,
    };

    if (candidate->presence == FIELD_ACTOR_UNUSED)
    {
        return 0;
    }
    point = g_field_object_states[source->object_index].ground_attachment_points;
    i = 0;
    do
    {
        delta->vx = ((candidate->x - source->x) - (point->x << 8)) >> 8;
        delta->vy = (candidate->y - source->y) >> 8;
        delta->vz = ((candidate->z - source->z) - (point->y << 8)) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(square);
        distance = SquareRoot0(square->vx + square->vy + square->vz);
        if (distance < max_distance)
        {
        found:
            return 1;
        }
        i++;
        point++;
    } while (i < FIELD_GROUND_POINT_COUNT);
    return 0;
}

/**
 * @brief Test the candidate against the source actor's first ground attachment point.
 * @param source Source actor; its object index selects the point.
 * @param candidate Candidate actor; unused records never pass.
 * @param max_distance Exclusive distance limit in whole units.
 * @return 1 when the point is closer than @p max_distance, otherwise 0.
 */
s32 field_actor_near_first_ground_point(FieldActor* source, FieldActor* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    Vec2s* point;
    s32 distance;

    point = g_field_object_states[source->object_index].ground_attachment_points;
    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        delta->vx = ((candidate->x - source->x) - (point->x << 8)) >> 8;
        delta->vy = (candidate->y - source->y) >> 8;
        delta->vz = ((candidate->z - source->z) - (point->y << 8)) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(square);
        distance = SquareRoot0(square->vx + square->vy + square->vz);
        if (distance < max_distance)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Collect the actors an action can target, using the action's predicate.
 *
 * Candidates must be alive, present, in the active group (party members
 * always are), not bound to a running animation, collidable and not flagged
 * as untargetable.
 *
 * @param source_index Acting object, never a candidate itself.
 * @param spec Action descriptor selecting a predicate from g_field_target_filters.
 * @param group_mode 0 targets the other side (party or enemies), 1 the same side.
 * @param filter_arg Argument passed to the predicate.
 * @param output Receives the accepted object indices.
 * @return Number of indices written to @p output.
 */
s32 field_collect_action_targets(s32 source_index, FieldTargetSpec* spec, s32 group_mode, s32 filter_arg, s32* output)
{
    s32 index;
    s32 count;
    s32 end;
    s32 start;
    s32 flags;
    s32 prior;
    s16 command;
    FieldActor* actor;
    FieldObjectRuntime* state;

    if (g_field_duel_mode != 0)
    {
        start = 0;
        end = FIELD_ACTOR_COUNT;
    }
    else
    {
        switch (group_mode)
        {
        case 0:
            if (source_index < FIELD_PARTY_COUNT)
            {
                start = FIELD_PARTY_COUNT;
                end = FIELD_ACTOR_COUNT;
            }
            else
            {
                start = 0;
                end = FIELD_PARTY_COUNT;
            }
            break;
        case 1:
            if (source_index < FIELD_PARTY_COUNT)
            {
                start = 0;
                end = FIELD_PARTY_COUNT;
            }
            else
            {
                start = FIELD_PARTY_COUNT;
                end = FIELD_ACTOR_COUNT;
            }
            break;
        }
    }
    count = 0;
    index = start;
    actor = &g_field_actors[index];
    state = &g_field_object_states[index];
    for (; index < end; index++, actor++, state++)
    {
        if (index == source_index || actor->presence == FIELD_ACTOR_UNUSED || state->current_hp == 0)
        {
            continue;
        }
        flags = state->contact.flags;
        if (flags & FIELD_CONTACT_ANIMATION_HIDDEN)
        {
            continue;
        }
        if (g_field_active_group != (state->group_flags & FIELD_OBJECT_GROUP_MASK) && index >= FIELD_PARTY_COUNT)
        {
            continue;
        }
        if (flags & FIELD_CONTACT_NO_HIT_TEST)
        {
            continue;
        }
        command = actor->command;
        if (command == FIELD_ACTOR_COMMAND_TECHNIQUE || command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY || command == FIELD_ACTOR_COMMAND_INSTRUMENT)
        {
            continue;
        }
        if (!(flags & FIELD_CONTACT_IGNORE_BINDING))
        {
            if (g_field_actor_bindings[FIELD_BINDING_INDEX(index)].owner == index && g_field_actor_bindings[FIELD_BINDING_INDEX(index)].state != 0)
            {
                continue;
            }
        }
        if ((state->object_flags & FIELD_OBJECT_UNTARGETABLE_FLAGS) || state->collision.word == 0 || (state->movement.word & FIELD_MOVEMENT_TINT_FLASH) ||
            (state->contact.flags & FIELD_CONTACT_TARGETED))
        {
            continue;
        }
        if (g_field_target_filters[spec->filter](&g_field_actors[source_index], &g_field_actors[index], filter_arg) != 0)
        {
            for (prior = 0; prior < count; prior++)
            {
                /* The original walks the accepted indices without testing them. */
            }
            output[count] = index;
            count += 1;
        }
    }
    return count;
}
