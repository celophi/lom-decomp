/**
 * @file field_actor_idle_ops.c
 * @brief Actor commands that wait for an animation to end and then return the actor to idle.
 */

#include "common.h"
#include "../internal/field_actor.h"
#include "../internal/field_calls.h"
#include "../internal/field_actor_sequence_runtime.h"
#include "overlays/field/field_object_state.h"

/**
 * @brief Clear the actor's command, release its sequence actor and clear its sequence movement mode.
 * @param actor Actor record.
 */
static inline void field_end_actor_command(FieldActor* actor)
{
    actor->command = FIELD_ACTOR_COMMAND_NONE;
    field_update_sequence_actor_binding(actor, 1);
    g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
}

/**
 * @brief FIELD_ACTOR_COMMAND_HIT: drop the actor to the ground, then idle it.
 *
 * When the reaction animation has finished, the actor lands, its sequence actor is
 * released, the movement mode is cleared and the animation restarts. A player
 * character also has its command history, variant and retry count cleared.
 *
 * @param actor Actor record.
 */
void field_update_hit_reaction(FieldActor* actor)
{
    if (actor->y < 0)
    {
        actor->y += FIELD_ACTOR_FALL_STEP;
        if (actor->y > 0)
        {
            actor->y = 0;
        }
    }

    if (actor->animation_state == 0)
    {
        actor->y = 0;
        field_end_actor_command(actor);
        field_restart_sequence_animation(actor);
        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            field_command_history_clear(actor->object_index);
            actor->variant = 0;
            g_field_object_states[actor->object_index].retry_count = 0;
        }
    }
}

/**
 * @brief FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION: idle the actor once its animation has finished.
 * @param actor Actor record.
 */
void field_idle_actor_after_animation(FieldActor* actor)
{
    if (actor->animation_state == 0)
    {
        field_end_actor_command(actor);
        field_restart_sequence_animation(actor);
    }
}

/**
 * @brief FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD: once the animation has finished, reload the slot and idle the actor.
 *
 * field_finish_party_slot_reload installs a pending resource for a player slot and refreshes the party
 * control modes. The actor is then idled with animation 2, keeping its facing bit.
 *
 * @param actor Actor record.
 */
void field_idle_actor_after_reload(FieldActor* actor)
{
    if (actor->animation_state == 0)
    {
        field_finish_party_slot_reload(actor->object_index);
        field_end_actor_command(actor);
        actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + 2;
        field_restart_sequence_animation(actor);
    }
}
