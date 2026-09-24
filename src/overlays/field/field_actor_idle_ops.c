/**
 * @file field_actor_idle_ops.c
 * @brief Return field actors to their idle animation.
 */

#include "common.h"
#include "field_actor_sequence_runtime.h"
#include "field_object_state.h"

/** @brief Movement flag bits cleared when an actor goes idle. */
#define FIELD_IDLE_CLEARED_MOVEMENT_FLAGS 0x1800

void func_8006AA7C(s32 actor_slot);
void func_800A2DD8(s32 player_index);

/**
 * @brief Let a falling actor settle, then idle it when its motion scale is zero.
 *
 * A negative vertical position rises by 0x800 per call and is clamped at zero.
 * Idling zeroes the height and motion parameter, releases the sequence binding,
 * clears the movement flags, restarts the animation and, for the two party
 * members, also clears their command-history idle counter.
 *
 * @param actor Actor motion record.
 */
void func_800923F0(FieldMotionRecord* actor)
{
    s32 y;
    FieldObjectRuntime* states;
    FieldObjectRuntime* state;

    y = actor->y;
    if (y < 0)
    {
        y += 0x800;
        actor->y = y;
        if (y > 0)
        {
            actor->y = 0;
        }
    }

    if (actor->motion_scale == 0)
    {
        actor->y = 0;
        actor->motion_parameter = 0;
        field_update_sequence_actor_binding(actor, 1);
        states = g_field_object_states;
        state = &states[actor->source_object_index];
        state->movement.word &= ~FIELD_IDLE_CLEARED_MOVEMENT_FLAGS;
        field_restart_sequence_animation(actor);
        if (actor->source_object_index < 2)
        {
            func_800A2DD8(actor->source_object_index);
            actor->reference_index = 0;
            states[actor->source_object_index].targets[13] = 0;
        }
    }
}

/**
 * @brief Idle an actor when its motion scale is zero.
 * @param actor Actor motion record.
 */
void func_800924D8(FieldMotionRecord* actor)
{
    if (actor->motion_scale == 0)
    {
        actor->motion_parameter = 0;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->source_object_index].movement.word &= ~FIELD_IDLE_CLEARED_MOVEMENT_FLAGS;
        field_restart_sequence_animation(actor);
    }
}

/**
 * @brief Refresh the controller flags of an actor and idle it when its motion scale is zero.
 *
 * Also sets the facing state to 2 while keeping its high bit.
 *
 * @param actor Actor motion record.
 * @see decomp.me (100%) TODO
 */
void func_80092550(FieldMotionRecord* actor)
{
    if (actor->motion_scale == 0)
    {
        func_8006AA7C(actor->source_object_index);
        actor->motion_parameter = 0;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->source_object_index].movement.word =
            g_field_object_states[actor->source_object_index].movement.word & ~FIELD_IDLE_CLEARED_MOVEMENT_FLAGS;
        actor->facing_or_reward_kind = (u8)((actor->facing_or_reward_kind & 0x80) + 2);
        field_restart_sequence_animation(actor);
    }
}
