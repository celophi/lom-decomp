/**
 * @file field_actor_target_queries.c
 * @brief Target selection inside an action's ground effect.
 *
 * Each ground effect kind (GroundEffectKind) has a predicate in
 * g_field_target_filters that tests whether a candidate stands inside the
 * effect's area at its current radius; field_collect_action_targets applies
 * the predicate of an action to every eligible object.
 */

#include "common.h"
#include "overlays/field/field_types.h"
#include "../internal/field_effect_types.h"
#include "../internal/field_actor.h"
#include "../internal/field_calls.h"
#include "../internal/field_resource_actions.h"
#include <libgte.h>
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Test whether a candidate stands inside a ground effect.
 * @param source Object that owns the effect.
 * @param candidate Candidate actor.
 * @param radius Current effect radius (FIELD_MOVEMENT_SCALE_MASK bits of the source's movement word).
 * @return Non-zero when the candidate is inside the effect.
 */
typedef s32 (*FieldTargetFilter)(FieldActor* source, FieldActor* candidate, s32 radius);

/** @brief Target predicate of each ground effect kind, indexed by GroundEffectKind. */
extern FieldTargetFilter g_field_target_filters[];

/**
 * @brief Dome predicate: test whether a candidate is within the dome around the source.
 * @param source Object at the dome centre.
 * @param candidate Candidate actor; unused records never pass.
 * @param radius Dome radius in whole units (exclusive).
 * @return 1 when the candidate is inside the dome, otherwise 0.
 */
s32 field_actor_within_distance(FieldActor* source, FieldActor* candidate, s32 radius)
{
    VECTOR* delta = FIELD_DISTANCE_DELTA;
    VECTOR* squares = FIELD_DISTANCE_SQUARES;
    s32 distance;

    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        delta->vx = (candidate->x - source->x) >> FIELD_POSITION_SHIFT;
        delta->vy = (candidate->y - source->y) >> FIELD_POSITION_SHIFT;
        delta->vz = (candidate->z - source->z) >> FIELD_POSITION_SHIFT;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squares);
        distance = SquareRoot0(squares->vx + squares->vy + squares->vz);

        if (distance < radius)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Strips predicate: test whether a candidate is under the strip arches around the source.
 * @param source Object at the centre of the strips.
 * @param candidate Candidate actor; unused records never pass.
 * @param radius Effect radius; the arches span GROUND_EFFECT_STRIPS_INNER_RADIUS to that plus @p radius.
 * @return 1 when the candidate is inside the ring the arches cover, otherwise 0.
 */
s32 field_actor_within_ring(FieldActor* source, FieldActor* candidate, s32 radius)
{
    VECTOR* delta = FIELD_DISTANCE_DELTA;
    VECTOR* squares = FIELD_DISTANCE_SQUARES;
    s32 distance;

    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        delta->vx = (candidate->x - source->x) >> FIELD_POSITION_SHIFT;
        delta->vy = (candidate->y - source->y) >> FIELD_POSITION_SHIFT;
        delta->vz = (candidate->z - source->z) >> FIELD_POSITION_SHIFT;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squares);
        distance = SquareRoot0(squares->vx + squares->vy + squares->vz);

        if (distance < radius + GROUND_EFFECT_STRIPS_INNER_RADIUS && distance > GROUND_EFFECT_STRIPS_INNER_RADIUS)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Spiral predicate: test whether a candidate lies within the spiral's angle of the source's facing direction.
 * @param source Object that owns the spiral; its facing bit selects +X (set) or -X (clear).
 * @param candidate Candidate actor.
 * @param radius Effect radius; the limit is GROUND_EFFECT_SPIRAL_ANGLE(@p radius) on either side.
 * @return 1 when the candidate is within the angle, otherwise 0.
 */
s32 field_actor_faces_target(FieldActor* source, FieldActor* candidate, s32 radius)
{
    s32 limit;
    s32 angle;

    limit = GROUND_EFFECT_SPIRAL_ANGLE(radius);

    if (!(source->animation & FIELD_ANIMATION_FACING))
    {
        angle = ratan2((source->z - candidate->z) >> FIELD_POSITION_SHIFT, (source->x - candidate->x) >> FIELD_POSITION_SHIFT);

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
        angle = ratan2((candidate->z - source->z) >> FIELD_POSITION_SHIFT, (candidate->x - source->x) >> FIELD_POSITION_SHIFT);

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
 * @brief Curves predicate: test whether a candidate is inside the box the curve strips cover.
 * @param source Object at the centre of the curves.
 * @param candidate Candidate actor; unused records never pass.
 * @param radius Effect radius: how far the curves run out along X on each side, in whole units.
 * @return 1 when the candidate is inside the box, otherwise 0.
 * @note The box reaches GROUND_EFFECT_CURVE_RADIUS along Z on either side.
 */
s32 field_actor_within_box(FieldActor* source, FieldActor* candidate, s32 radius)
{
    s32 dz;
    s32 dx;

    if (candidate->presence == FIELD_ACTOR_UNUSED)
    {
        return 0;
    }

    dz = candidate->z - source->z;

    if (dz > GROUND_EFFECT_CURVE_RADIUS)
    {
        return 0;
    }

    if (dz < -GROUND_EFFECT_CURVE_RADIUS)
    {
        return 0;
    }

    dx = candidate->x - source->x;

    if (dx > (radius << FIELD_POSITION_SHIFT))
    {
        return 0;
    }

    return dx >= -(radius << FIELD_POSITION_SHIFT);
}

/**
 * @brief Scattered-domes predicate: test the candidate against the dome on each ground effect point.
 * @param source Object that owns the domes; its object state holds the points.
 * @param candidate Candidate actor; unused records never pass.
 * @param radius Dome radius in whole units (exclusive).
 * @return 1 when the candidate is inside any dome, otherwise 0.
 */
s32 field_actor_near_ground_effect_points(FieldActor* source, FieldActor* candidate, s32 radius)
{
    VECTOR* delta = FIELD_DISTANCE_DELTA;
    VECTOR* squares = FIELD_DISTANCE_SQUARES;
    Vec2s* point;
    s32 point_index;
    s32 distance;

    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        point = g_field_object_states[source->object_index].ground_effect_points;

        for (point_index = 0; point_index < FIELD_GROUND_EFFECT_POINT_COUNT; point_index++, point++)
        {
            delta->vx = ((candidate->x - source->x) - (point->x << FIELD_POSITION_SHIFT)) >> FIELD_POSITION_SHIFT;
            delta->vy = (candidate->y - source->y) >> FIELD_POSITION_SHIFT;
            delta->vz = ((candidate->z - source->z) - (point->y << FIELD_POSITION_SHIFT)) >> FIELD_POSITION_SHIFT;
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(squares);
            distance = SquareRoot0(squares->vx + squares->vy + squares->vz);

            if (distance < radius)
            {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Drifting and aimed dome predicate: test the candidate against the dome on the first ground effect point.
 * @param source Object that owns the dome; its object state holds the point.
 * @param candidate Candidate actor; unused records never pass.
 * @param radius Dome radius in whole units (exclusive).
 * @return 1 when the candidate is inside the dome, otherwise 0.
 */
s32 field_actor_near_first_ground_effect_point(FieldActor* source, FieldActor* candidate, s32 radius)
{
    VECTOR* delta = FIELD_DISTANCE_DELTA;
    VECTOR* squares = FIELD_DISTANCE_SQUARES;
    Vec2s* point;
    s32 distance;

    point = g_field_object_states[source->object_index].ground_effect_points;

    if (candidate->presence != FIELD_ACTOR_UNUSED)
    {
        delta->vx = ((candidate->x - source->x) - (point->x << FIELD_POSITION_SHIFT)) >> FIELD_POSITION_SHIFT;
        delta->vy = (candidate->y - source->y) >> FIELD_POSITION_SHIFT;
        delta->vz = ((candidate->z - source->z) - (point->y << FIELD_POSITION_SHIFT)) >> FIELD_POSITION_SHIFT;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squares);
        distance = SquareRoot0(squares->vx + squares->vy + squares->vz);

        if (distance < radius)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Collect the objects inside an action's ground effect.
 *
 * Candidates must be alive, present, in the active group (party members
 * always are), not bound to a running animation, collidable and not flagged
 * as untargetable.
 *
 * @param source_index Acting object, never a candidate itself.
 * @param action Action whose ground effect (FieldActionFlags::target_filter) selects the predicate in g_field_target_filters.
 * @param group_mode 0 targets the other side (party or enemies), 1 the same side.
 * @param effect_radius Current radius of the source object's ground effect, passed to the predicate.
 * @param output Receives the accepted object indices.
 * @return Number of indices written to @p output.
 */
s32 field_collect_action_targets(s32 source_index, FieldActionSlot* action, s32 group_mode, s32 effect_radius, s32* output)
{
    s32 index;
    s32 count;
    s32 end;
    s32 start;
    s32 flags;
    s32 prior;
    FieldActor* actor;
    FieldObjectState* state;

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
    actor = &g_field_actors[start];
    state = &g_field_object_states[start];

    for (index = start; index < end; index++, actor++, state++)
    {
        if (index == source_index || actor->presence == FIELD_ACTOR_UNUSED || state->current_hp.word == 0)
        {
            continue;
        }

        flags = state->contact.word;

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

        if (actor->command == FIELD_ACTOR_COMMAND_TECHNIQUE || actor->command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY ||
            actor->command == FIELD_ACTOR_COMMAND_INSTRUMENT)
        {
            continue;
        }

        if (!(flags & FIELD_CONTACT_ACTION_PENDING))
        {
            if (g_field_actor_bindings[FIELD_BINDING_INDEX(index)].owner == index && g_field_actor_bindings[FIELD_BINDING_INDEX(index)].state != 0)
            {
                continue;
            }
        }

        if ((state->flags & FIELD_OBJECT_UNTARGETABLE_FLAGS) || state->collision.word == 0 || (state->movement.word & FIELD_MOVEMENT_INVULNERABLE) ||
            (state->contact.word & FIELD_CONTACT_TARGETED))
        {
            continue;
        }

        if (g_field_target_filters[action->flags.target_filter](&g_field_actors[source_index], &g_field_actors[index], effect_radius) != 0)
        {
            for (prior = 0; prior < count; prior++)
            {
            }

            output[count] = index;
            count++;
        }
    }

    return count;
}
