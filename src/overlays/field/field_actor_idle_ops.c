/**
 * @file field_actor_idle_ops.c
 * @brief Actor commands that wait for an animation to end and then return the actor to idle.
 *
 * The actor is a FieldMotionRecord here because the sequence runtime takes that view.
 * For an actor record motion_parameter holds the command (0 = idle), motion_scale the
 * animation repeat count (0 once the last loop has played) and facing_or_reward_kind the
 * animation (bit 7 mirrors it).
 */

#include "common.h"
#include "field_calls.h"
#include "field_actor_sequence_runtime.h"
#include "field_object_state.h"

/** @brief Objects 0 and 1 are the two player characters. */
#define PLAYER_OBJECT_COUNT 2

/** @brief Fall speed per frame while the actor is above the ground (up is negative y). */
#define FALL_STEP 0x800

/** @brief Animation byte bit that mirrors the animation. */
#define ANIMATION_MIRROR_BIT 0x80

/**
 * @brief Command 0x82 (hit reaction): drop the actor to the ground, then idle it.
 *
 * When the reaction animation has finished, the actor lands, its sequence actor is
 * released, the movement mode is cleared and the animation restarts. A player
 * character also has its command history, reference_index and retry count cleared.
 *
 * @param actor Actor record.
 */
void field_update_hit_reaction(FieldMotionRecord* actor)
{
    if (actor->y < 0)
    {
        actor->y += FALL_STEP;
        if (actor->y > 0)
        {
            actor->y = 0;
        }
    }

    if (actor->motion_scale == 0)
    {
        actor->y = 0;
        actor->motion_parameter = 0;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->source_object_index].movement.word &= ~FIELD_OBJECT_MOVEMENT_MODE_MASK;
        field_restart_sequence_animation(actor);
        if (actor->source_object_index < PLAYER_OBJECT_COUNT)
        {
            field_command_history_clear(actor->source_object_index);
            actor->reference_index = 0;
            g_field_object_states[actor->source_object_index].retry_count = 0;
        }
    }
}

/**
 * @brief Command 0x99: idle the actor once its animation has finished.
 * @param actor Actor record.
 */
void field_idle_actor_after_animation(FieldMotionRecord* actor)
{
    if (actor->motion_scale == 0)
    {
        actor->motion_parameter = 0;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->source_object_index].movement.word &= ~FIELD_OBJECT_MOVEMENT_MODE_MASK;
        field_restart_sequence_animation(actor);
    }
}

/**
 * @brief Command 0x9A: once the animation has finished, reload the slot and idle the actor.
 *
 * field_finish_party_slot_reload installs a pending resource for a player slot and refreshes the party
 * control modes. The actor is then idled with animation 2, keeping its mirror bit.
 *
 * @param actor Actor record.
 */
void field_idle_actor_after_reload(FieldMotionRecord* actor)
{
    if (actor->motion_scale == 0)
    {
        field_finish_party_slot_reload(actor->source_object_index);
        actor->motion_parameter = 0;
        field_update_sequence_actor_binding(actor, 1);
        g_field_object_states[actor->source_object_index].movement.word &= ~FIELD_OBJECT_MOVEMENT_MODE_MASK;
        actor->facing_or_reward_kind = (actor->facing_or_reward_kind & ANIMATION_MIRROR_BIT) + 2;
        field_restart_sequence_animation(actor);
    }
}
