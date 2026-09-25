/**
 * @file field_actor_target_queries.c
 * @brief Distance, facing and box predicates for field actors, and the target collector that applies them.
 */

#include "common.h"
#include "field_types.h"
#include "field_effect_types.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Number of field actors. */
#define FIELD_TARGET_ACTOR_COUNT 13

/** @brief Number of leading actors that belong to the party. */
#define FIELD_TARGET_PARTY_COUNT 3

/** @brief Binding slot of an actor: party members own slots 0-2, every other actor shares slot 2. */
#define FIELD_TARGET_BINDING_SLOT(index) ((index) < FIELD_TARGET_PARTY_COUNT ? (index) : 2)

/** @brief Scratchpad vector holding the scaled actor delta. */
#define FIELD_TARGET_DELTA ((VECTOR*)0x1F800000)

/** @brief Scratchpad vector receiving the squared delta components. */
#define FIELD_TARGET_SQUARE ((VECTOR*)0x1F800010)

/** @brief Selection descriptor containing the predicate table index. */
typedef struct
{
    u8 pad[2];
    u8 filter;
} FieldTargetSpec;

/** @brief Predicate for a source actor, candidate actor and caller argument. */
typedef s32 (*FieldTargetFilter)(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 argument);

/** @brief Ground attachment offsets of a field object state (the view starts 0x190 bytes into it). */
typedef struct
{
    Vec2s points[3];
    u8 pad[0x23C - 0xC];
} FieldTargetGroundPoints;

/** @brief Binding state and its owning actor in a 0x1C-byte record. */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner;
    u8 pad10[12];
} FieldTargetBinding;

extern FieldTargetGroundPoints D_80105C70[];
extern FieldTargetFilter D_800EC2D8[];
extern FieldMotionRecord g_field_actors[];
extern s32 g_field_active_group;
extern FieldTargetBinding g_field_actor_bindings[];
extern s32 g_field_duel_mode;

/**
 * @brief Test whether a candidate actor is within a distance of the source actor.
 * @param source Source actor.
 * @param candidate Candidate actor; rejected when its state is 0xFF.
 * @param max_distance Exclusive distance limit in whole units.
 * @return 1 when the candidate is closer than @p max_distance, otherwise 0.
 */
s32 func_8009CC60(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    s32 distance;

    if (candidate->state != 0xFF)
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
 * @brief Test whether a candidate actor lies in the ring between 0x40 and a distance plus 0x40.
 * @param source Source actor.
 * @param candidate Candidate actor; rejected when its state is 0xFF.
 * @param max_distance Ring width beyond the 0x40 inner radius.
 * @return 1 when 0x40 < distance < @p max_distance + 0x40, otherwise 0.
 */
s32 func_8009CD30(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    s32 distance;

    if (candidate->state != 0xFF)
    {
        delta->vx = (candidate->x - source->x) >> 8;
        delta->vy = (candidate->y - source->y) >> 8;
        delta->vz = (candidate->z - source->z) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(square);
        distance = SquareRoot0(square->vx + square->vy + square->vz);
        if (distance < max_distance + 0x40)
        {
            if (distance > 0x40)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Test whether the source actor faces the candidate within an angular limit.
 * @param source Source actor; bit 7 of its facing byte selects the facing direction.
 * @param candidate Candidate actor.
 * @param distance Value whose arctangent over 100 gives the angular limit.
 * @return 1 when the candidate is within the angular limit, otherwise 0.
 */
s32 func_8009CE10(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 distance)
{
    s32 limit;
    s32 angle;

    limit = ratan2(distance, 100);
    if (!(source->facing_or_reward_kind & 0x80))
    {
        angle = ratan2((source->z - candidate->z) >> 8, (source->x - candidate->x) >> 8);
        if (angle >= 0x801)
        {
            angle = 0x1000 - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    else
    {
        angle = ratan2((candidate->z - source->z) >> 8, (candidate->x - source->x) >> 8);
        if (angle >= 0x801)
        {
            angle = 0x1000 - angle;
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
 *
 * The box spans +-0x2000 on the Z axis and +-(@p half_width << 8) on
 * the X axis.
 *
 * @param source Source actor at the box center.
 * @param candidate Candidate actor; rejected when its state is 0xFF.
 * @param half_width Half-width of the X range in whole units.
 * @return 1 when the candidate is inside the box, otherwise 0.
 */
s32 func_8009CF1C(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 half_width)
{
    s32 dz;
    s32 dx;

    if (candidate->state == 0xFF)
    {
        return 0;
    }
    dz = candidate->z - source->z;
    if (dz >= 0x2001)
    {
        return 0;
    }
    if (dz < -0x2000)
    {
        return 0;
    }
    dx = candidate->x;
    dx = dx - source->x; /* the original loads the candidate X first and computes the delta in place */
    half_width <<= 8;
    dz = dx;
    if (half_width < dz)
    {
        return 0;
    }
    return dz >= -half_width;
}

/**
 * @brief Test the candidate against the source actor's three ground attachment points.
 * @param source Source actor; its object index selects the attachment points.
 * @param candidate Candidate actor; rejected when its state is 0xFF.
 * @param max_distance Exclusive distance limit in whole units.
 * @return 1 when any attachment point is closer than @p max_distance, otherwise 0.
 */
s32 func_8009CF84(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    Vec2s* point;
    s32 i;
    s32 distance;
    static void* const keep[] __attribute__((section(".discard"))) = {
        &&found,
    }; /* taking the label address keeps the found path a separate block, as in the original */

    if (candidate->state == 0xFF)
    {
        return 0;
    }
    point = D_80105C70[source->source_object_index].points;
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
    } while (i < 3);
    return 0;
}

/**
 * @brief Test the candidate against the source actor's first ground attachment point.
 * @param source Source actor; its object index selects the attachment point.
 * @param candidate Candidate actor; rejected when its state is 0xFF.
 * @param max_distance Exclusive distance limit in whole units.
 * @return 1 when the attachment point is closer than @p max_distance, otherwise 0.
 */
s32 func_8009D0D8(FieldMotionRecord* source, FieldMotionRecord* candidate, s32 max_distance)
{
    VECTOR* delta = FIELD_TARGET_DELTA;
    VECTOR* square = FIELD_TARGET_SQUARE;
    Vec2s* point;
    s32 distance;

    point = D_80105C70[source->source_object_index].points;
    if (candidate->state != 0xFF)
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
 * @brief Collect eligible field actor indices using a selected predicate.
 *
 * Candidates must be alive, present, in the active group (party members
 * always are), not bound to a running sequence, collidable and not flagged
 * as untargetable.
 *
 * @param source_index Actor excluded from the candidate list.
 * @param spec Descriptor selecting a predicate from D_800EC2D8.
 * @param group_mode Zero selects the opposite group; one selects the same group.
 * @param filter_arg Additional predicate argument.
 * @param output Destination for the accepted indices.
 * @return Number of indices written to output.
 */
s32 func_8009D1E4(s32 source_index, FieldTargetSpec* spec, s32 group_mode, s32 filter_arg, s32* output)
{
    s32 index;
    s32 count;
    s32 end;
    s32 start;
    s32 flags;
    s32 prior;
    s16 motion;
    FieldMotionRecord* actor;
    FieldObjectRuntime* state;

    if (g_field_duel_mode != 0)
    {
        start = 0;
        end = FIELD_TARGET_ACTOR_COUNT;
    }
    else
    {
        switch (group_mode)
        {
        case 0:
            if (source_index < FIELD_TARGET_PARTY_COUNT)
            {
                start = FIELD_TARGET_PARTY_COUNT;
                end = FIELD_TARGET_ACTOR_COUNT;
            }
            else
            {
                start = 0;
                end = FIELD_TARGET_PARTY_COUNT;
            }
            break;
        case 1:
            if (source_index < FIELD_TARGET_PARTY_COUNT)
            {
                start = 0;
                end = FIELD_TARGET_PARTY_COUNT;
            }
            else
            {
                start = FIELD_TARGET_PARTY_COUNT;
                end = FIELD_TARGET_ACTOR_COUNT;
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
        if (index == source_index || actor->state == 0xFF || state->current_hp == 0)
        {
            continue;
        }
        flags = state->contact.flags;
        if (flags & 1)
        {
            continue;
        }
        if (g_field_active_group != (state->group_flags & 0xF) && index >= FIELD_TARGET_PARTY_COUNT)
        {
            continue;
        }
        if (flags & 0x20)
        {
            continue;
        }
        motion = actor->motion_parameter;
        if (motion == 0x91 || motion == 0xAE || motion == 0x87)
        {
            continue;
        }
        if (!(flags & 0x40))
        {
            if (g_field_actor_bindings[FIELD_TARGET_BINDING_SLOT(index)].owner == index && g_field_actor_bindings[FIELD_TARGET_BINDING_SLOT(index)].state != 0)
            {
                continue;
            }
        }
        if ((state->object_flags & 0x2280) || state->collision.word == 0 || (state->movement.word & 0x8000) || (state->contact.flags & 0x80))
        {
            continue;
        }
        if (D_800EC2D8[spec->filter](&g_field_actors[source_index], &g_field_actors[index], filter_arg) != 0)
        {
            for (prior = 0; prior < count; prior++)
            {
                /* the original walks the accepted indices without testing them */
            }
            output[count] = index;
            count += 1;
        }
    }
    return count;
}
