/**
 * @file field_actor_reactions.c
 * @brief Field actor reactions: hit reactions, knockdowns, defeat, action
 *        chain limits, object links and the depth-overlap test between two objects.
 */

#include "common.h"
#include "field_actor_tables.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "sdk/rand.h"


/** @brief First frame of the HUD panel shake started by a hit (counts down to 0). */
#define FIELD_HUD_SHAKE_START 5

/** @brief FieldObjectState.flags bits; the numbered ones have unknown meanings. */
#define FIELD_OBJECT_FLAG_0004 0x0004
#define FIELD_OBJECT_FLAG_0020 0x0020
#define FIELD_OBJECT_FLAG_0040 0x0040
#define FIELD_OBJECT_FLAG_0080 0x0080
#define FIELD_OBJECT_FLAG_0100 0x0100
/** @brief Another object holds a link to this object (FieldContactWord linked bit). */
#define FIELD_OBJECT_FLAG_LINK_TARGET 0x2000
#define FIELD_OBJECT_FLAG_4000 0x4000
/** @brief An action chain is running; its hits are counted in retry_count. */
#define FIELD_OBJECT_CHAINING 0x8000

/** @brief Flags that keep an object in place: it takes no knockback from a hit. */
#define FIELD_OBJECT_IMMOBILE_FLAGS                                                                                                                            \
    (FIELD_OBJECT_FLAG_0004 | FIELD_OBJECT_FLAG_0020 | FIELD_OBJECT_FLAG_0040 | FIELD_OBJECT_FLAG_0080 | FIELD_OBJECT_FLAG_0100 |                              \
     FIELD_OBJECT_FLAG_KNOCKED_OUT | FIELD_OBJECT_FLAG_LINK_TARGET)


/** @brief FieldResourceEntry.unkE flag: the defeat animation plays on the bound animation actor. */
#define FIELD_REQUEST_BOUND 0x8000

#define FIELD_ANIMATION_KNOCKED_DOWN 29
/** @brief An actor playing this animation ignores hits. */
#define FIELD_ANIMATION_44 0x44

/** @brief Effect resource played when an action chain reaches the weapon's limit. */
#define FIELD_CHAIN_LIMIT_EFFECT 0x21

/** @brief Frames between the start of a defeat and the collapse. */
#define FIELD_DEFEAT_DELAY 10

/** @brief Frames an actor waits after its action chain ended. */
#define FIELD_CHAIN_END_WAIT 30

/** @brief Number of hits an action chain may land, indexed by weapon type. */
extern u8 g_field_weapon_chain_limits[];
extern s32 g_field_boss_hud_shake_frame;
extern s32 g_field_active_group;
extern s32 g_field_duel_mode;

void field_release_object_link(FieldActor* actor);
static void field_release_links_to_actor(FieldActor* actor);
static void field_end_actor_chain(FieldActor* actor);

void field_restart_actor_animation(FieldActor* actor);
void field_restart_actor_animation_reverse(FieldActor* actor);
void field_update_sequence_actor_binding(FieldActor* actor, s32 release_actor);
void field_stop_actor_animations_for_object(FieldActor* actor, s32 force);

/**
 * @brief Put an actor into its hit reaction and reset its motion and animation flags.
 * @param actor Actor receiving the hit.
 * @param guard Nonzero plays the guard animation; zero picks one of the two hit animations.
 */
void field_start_actor_hit_reaction(FieldActor* actor, s32 guard)
{
    s16 command;
    s32 flags;
    FieldObjectState* state;
    FieldObjectState* states;

    if (actor->object_index < FIELD_PARTY_COUNT)
    {
        g_field_player_records[actor->object_index].hit_state = FIELD_HUD_SHAKE_START;
    }
    else if (g_field_object_states[actor->object_index].unk8.word < 0)
    {
        g_field_boss_hud_shake_frame = FIELD_HUD_SHAKE_START;
    }
    if (!g_field_object_states[actor->object_index].contact.bits.flag6)
    {
        if (field_actor_binding(actor)->state != FIELD_BINDING_IDLE)
        {
            if (field_actor_binding(actor)->owner == actor->object_index)
            {
                return;
            }
        }
    }
    command = actor->command;
    if (command != FIELD_ACTOR_COMMAND_DEFEAT_WAIT && command != FIELD_ACTOR_COMMAND_DEFEAT_END && command != FIELD_ACTOR_COMMAND_DEFEATED)
    {
        states = g_field_object_states;
        state = &states[actor->object_index];
        flags = state->flags;
        if (!(flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
        {
            if ((state->contact.bytes.flags & FIELD_CONTACT_ANIMATION_HIDDEN) || (flags & FIELD_OBJECT_IMMOBILE_FLAGS))
            {
                actor->animation_state = 1;
                actor->animation_frame = 0;
                actor->animation_active = 1;
                actor->animation &= FIELD_ANIMATION_FACING;
                field_restart_actor_animation(actor);
                actor->command = FIELD_ACTOR_COMMAND_HIT;
                return;
            }
            if ((actor->animation & FIELD_ANIMATION_INDEX_MASK) != FIELD_ANIMATION_44)
            {
                state->contact.word &= ~FIELD_CONTACT_IGNORE_BINDING;
                field_update_sequence_actor_binding(actor, 0);
                actor->command = FIELD_ACTOR_COMMAND_HIT;
                actor->y -= actor->height << 8;
                g_field_object_states[actor->object_index].retry_count = 0;
                actor->variant = 0;
                if (guard != 0)
                {
                    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_GUARD_ALT;
                }
                else
                {
                    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_HIT;
                    actor->animation += rand() & 1;
                }
                field_release_object_link(actor);
                actor->animation_state = 1;
                actor->animation_active = 1;
                actor->animation_frame = 0;
                g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
                field_restart_actor_animation(actor);
                actor->y += actor->height << 8;
                if (actor->y > 0)
                {
                    actor->y = 0;
                }
                g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_FLAG_4000;
                g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
                field_stop_actor_animations_for_object(actor, 0);
                if (actor->object_index < FIELD_PLAYER_COUNT)
                {
                    field_command_history_clear(actor->object_index);
                    g_field_object_states[actor->object_index].retry_count = 0;
                    actor->variant = 0;
                }
            }
        }
    }
}

/**
 * @brief Release the actor's link and clear the link-target flag of the linked object.
 * @param actor Actor whose object may hold a link.
 */
void field_release_object_link(FieldActor* actor)
{
    FieldObjectState* states = g_field_object_states;
    FieldObjectState* state;
    FieldObjectState* linked;

    state = &states[actor->object_index];
    if (state->contact.bits.linked)
    {
        state->contact.bits.linked = 0;
        linked = &states[states[actor->object_index].linked_object_index];
        linked->flags &= ~FIELD_OBJECT_FLAG_LINK_TARGET;
    }
}

/**
 * @brief Release every link that points at the actor's object.
 * @param actor Actor whose object is the link target.
 */
static void field_release_links_to_actor(FieldActor* actor)
{
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        if (g_field_object_states[i].contact.bits.linked)
        {
            if (g_field_object_states[i].linked_object_index == actor->object_index)
            {
                g_field_object_states[i].contact.bits.linked = 0;
                g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_FLAG_LINK_TARGET;
            }
        }
    }
}

/**
 * @brief Stop the script and animations of the actor with @p key.
 * @param key Object key to look up.
 * @return 0 when the actor was stopped, -1 when no actor has @p key.
 */
s32 field_stop_actor(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->script_index = FIELD_SCRIPT_NONE;
    actor->unk10 = 0;
    actor->script_offset++;
    field_update_sequence_actor_binding(actor, 0);
    field_stop_actor_animations_for_object(actor, 1);
    field_release_actor_binding(actor->object_index);
    return 0;
}

/**
 * @brief Knock an actor down, release its links and stop its animation actors.
 * @param actor Actor to knock down.
 * @param clear_recovery Nonzero clears a party member's recovery time limit.
 */
void field_knock_down_actor(FieldActor* actor, s32 clear_recovery)
{
    FieldObjectState* states;
    FieldObjectState* state;

    field_release_object_link(actor);
    field_release_links_to_actor(actor);
    actor->command = FIELD_ACTOR_COMMAND_KNOCKED_DOWN;
    if (clear_recovery != 0 && actor->object_index < FIELD_PARTY_COUNT)
    {
        g_field_player_records[actor->object_index].revive_delay = 0;
    }

    actor->script_index = FIELD_SCRIPT_NONE;
    states = g_field_object_states;
    actor->animation_state = 1;
    actor->animation_active = 1;
    actor->y = 0;
    actor->animation_frame = 0;
    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_KNOCKED_DOWN;

    state = &states[actor->object_index];
    state->movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
    field_restart_actor_animation(actor);

    field_stop_actor_animations_for_object(actor, 1);
    field_stop_actor_slot(actor, &g_field_actor_slots[FIELD_OBJECT_EFFECT_SLOT_BASE + actor->object_index], 1);
    actor->control.word |= FIELD_CONTROL_PLAY_ONCE;
    field_update_sequence_actor_binding(actor, 0);
    field_release_actor_binding(actor->object_index);
}

/**
 * @brief Start the jump command on an actor.
 * @param actor Actor to update.
 * @param direction Facing angle (256 steps) the actor jumps towards.
 * @param animation Animation to play, added to the actor's mirror bit.
 * @param command_param Parameter of the jump command.
 */
void field_start_actor_jump(FieldActor* actor, s8 direction, s32 animation, s8 command_param)
{
    FieldObjectState* states = g_field_object_states;
    FieldObjectState* state;

    actor->command = FIELD_ACTOR_COMMAND_JUMP;
    actor->animation_state = 1;
    actor->animation_active = 1;
    actor->direction = direction;
    actor->animation_frame = 0;
    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + animation;
    state = &states[actor->object_index];
    state->movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
    field_restart_actor_animation(actor);
    actor->command_param = command_param;
}

/**
 * @brief Start an actor's defeat: drop its flags and collapse it after a short delay.
 * @param actor Actor that was defeated.
 * @param value Value stored in the object state's unk16C.
 */
void field_start_actor_defeat(FieldActor* actor, s8 value)
{
    if (actor->object_index < FIELD_PARTY_COUNT)
    {
        g_field_player_records[actor->object_index].revive_delay = 0;
    }

    g_field_object_states[actor->object_index].flags &= FIELD_OBJECT_FLAG_KNOCKED_OUT;
    g_field_object_states[actor->object_index].contact.word |= FIELD_CONTACT_NO_HIT_TEST;
    g_field_object_states[actor->object_index].unk16C = value;
    actor->command = FIELD_ACTOR_COMMAND_DEFEAT_DELAY;
    actor->command_param = FIELD_DEFEAT_DELAY;
}

/**
 * @brief Knock a defeated actor down and start its defeat animation.
 * @param actor Actor to update.
 * @return Unspecified; callers do not use the value.
 * @note Declared s32 without a return statement: a void function fills the final branch delay slot differently.
 */
s32 field_collapse_defeated_actor(FieldActor* actor)
{
    FieldObjectState* states;
    FieldObjectState* state;
    FieldResourceEntry* resource;
    FieldResourceEntry* resources;
    s32 object_index;

    field_release_object_link(actor);
    field_release_links_to_actor(actor);
    actor->command = FIELD_ACTOR_COMMAND_KNOCKED_DOWN;
    actor->script_index = FIELD_SCRIPT_NONE;
    actor->animation_state = 1;
    actor->animation_active = 1;
    actor->y = 0;
    actor->animation_frame = 0;
    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_KNOCKED_DOWN;
    states = g_field_object_states;
    state = &states[actor->object_index];
    state->movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
    field_restart_actor_animation(actor);
    field_stop_actor_animations_for_object(actor, 1);
    field_stop_actor_slot(actor, &g_field_actor_slots[FIELD_OBJECT_EFFECT_SLOT_BASE + actor->object_index], 1);
    actor->control.word |= FIELD_CONTROL_PLAY_ONCE;
    field_update_sequence_actor_binding(actor, 0);
    field_release_actor_binding(actor->object_index);
    object_index = actor->object_index;
    if (!(states[object_index].contact.bytes.flags & FIELD_CONTACT_ANIMATION_HIDDEN))
    {
        resources = g_field_resource_entries;
        resource = &resources[actor->resource_index];
        if (resource->unkE & FIELD_REQUEST_BOUND)
        {
            actor->command = FIELD_ACTOR_COMMAND_DEFEAT_BOUND;
        }
        else
        {
            field_start_builtin_animation(object_index, object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, resource->unkE);
            field_start_actor_animation(actor->object_index + FIELD_OBJECT_EFFECT_SLOT_BASE, 0, 0);
        }
    }
    if (!(g_field_object_states[actor->object_index].flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
    {
        if (actor->command == FIELD_ACTOR_COMMAND_DEFEAT_BOUND)
        {
            actor->command = FIELD_ACTOR_COMMAND_DEFEAT_WAIT;
        }
        else
        {
            actor->command = FIELD_ACTOR_COMMAND_DEFEATED;
        }
    }
}

/**
 * @brief Test whether two objects are opponents that overlap in depth.
 * @param first_key Key of the object whose position is the centre of the test.
 * @param second_key Key of the object whose eligibility and position are tested.
 * @return -1 when either object is absent or the second is unused, 1 for an eligible overlap, 0 otherwise.
 */
s32 field_test_actor_depth_overlap(s32 first_key, s32 second_key)
{
    FieldActor* first;
    FieldActor* second;
    FieldObjectState* second_state;
    s32 second_z;
    s32 first_z;
    s32 first_extent;
    s16 second_extent;
    u8 second_object;
    u8 first_index;
    u8 second_index;

    first = field_find_actor(first_key);
    if (first == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    second = field_find_actor(second_key);
    if (second == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    second_object = second->object_index;
    second_state = &g_field_object_states[second_object];
    if (second->presence == FIELD_ACTOR_UNUSED)
    {
        return -1;
    }
    if (second_object >= FIELD_PARTY_COUNT && (second_state->group_flags & FIELD_OBJECT_GROUP_MASK) != g_field_active_group)
    {
        return 0;
    }
    if (second_state->unk4.word == 0)
    {
        return 0;
    }
    second_index = second->object_index;
    first_index = first->object_index;
    if (second_index == first_index)
    {
        return 0;
    }
    if (g_field_duel_mode == 0)
    {
        /* Outside a duel, party members never overlap each other, nor enemies each other. */
        if (first_index < FIELD_PARTY_COUNT)
        {
            if (second_index < FIELD_PARTY_COUNT)
            {
                return 0;
            }
        }
        else if (second_index >= FIELD_PARTY_COUNT)
        {
            return 0;
        }
    }
    /* The extents are whole units; half of each, in 24.8 fixed point, is added to the range. */
    first_z = first->z;
    second_z = second->z;
    first_extent = ((s16)g_field_object_states[first->object_index].collision.half.extent >> 1) << 8;
    second_extent = (s16)second_state->collision.half.extent;
    if (second_z < first_z - first_extent - (second_extent << 7))
    {
        return 0;
    }
    return first_z + first_extent + (second_extent << 7) >= second_z;
}

/**
 * @brief Count a hit of an object's action chain and end the chain at the weapon's limit.
 * @param object_index Object that landed the hit.
 */
void field_count_chain_hit(s32 object_index)
{
    FieldObjectState* states;
    FieldObjectState* state;
    s32 hit_count;
    FieldActor* actor;
    s32 effect_slot;

    states = g_field_object_states;
    state = &states[object_index];
    if (state->flags & FIELD_OBJECT_CHAINING)
    {
        hit_count = state->retry_count + 1;
        state->retry_count = hit_count;
        if ((object_index < FIELD_PLAYER_COUNT) && ((u8)hit_count >= g_field_weapon_chain_limits[g_field_player_records[object_index].head.bytes.weapon_type]))
        {
            actor = field_find_actor(state->key);
            if (actor != FIELD_ACTOR_NONE)
            {
                effect_slot = field_find_free_actor_slot(0, 0);
                if ((effect_slot != -1) && (field_start_builtin_animation(actor->object_index, effect_slot, FIELD_CHAIN_LIMIT_EFFECT) != 0))
                {
                    field_start_actor_animation(effect_slot, 0, 0);
                }
            }
            field_end_actor_chain(&g_field_actors[object_index]);
        }
    }
}

/**
 * @brief End an actor's action chain and make it wait in the chain-end pose.
 * @param actor Actor to update.
 */
static void field_end_actor_chain(FieldActor* actor)
{
    g_field_object_states[actor->object_index].retry_count = 0;
    g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
    g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_FLAG_4000;
    actor->command = FIELD_ACTOR_COMMAND_WAIT;
    actor->command_param = FIELD_CHAIN_END_WAIT;
    actor->animation_state = 1;
    actor->variant = 0;
    actor->y = 0;
    actor->animation_frame = 0;
    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_13;
    actor->animation_active = 1;
    g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
    field_restart_actor_animation_reverse(actor);
}
