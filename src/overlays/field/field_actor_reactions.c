/**
 * @file field_actor_reactions.c
 * @brief Field actor reactions: hit and knockback states, pending link clears,
 *        animation re-arming and depth-overlap tests between two objects.
 */

#include "common.h"
#include "field_actor_tables.h"

/** @brief Command of an actor reacting to a hit. */
#define FIELD_COMMAND_HIT_REACTION 0x82

extern u8 D_800EB068[];
extern s32 g_field_boss_hud_shake_frame;
extern s32 g_field_active_group;
extern s32 D_8010D020;

void func_8008BC5C(FieldActor* actor);
void func_8008C620(FieldActor* actor);

void field_restart_actor_animation(FieldActor* actor);
void field_restart_actor_animation_reverse(FieldActor* actor);
void field_update_sequence_actor_binding(FieldActor* actor, s32 release_actor);
void field_stop_actor_animations_for_object(FieldActor* actor, s32 force);
void func_800A2DD8(s32 object_index);
void func_80084424(s32 object_index);
void func_80083BC0(FieldActor* actor, FieldActorSlot* slot, s32 force);
void field_start_actor_animation(s32 slot_index, s32 target_count, u8* targets);
s32 func_80083EEC(s32 object_index, s32 slot_index, s32 resource_index);
s32 func_800839F8(s32 object_index, s32 require_idle);
extern s32 rand(void);

/**
 * @brief Put an actor into its hit reaction and reset its motion and animation flags.
 * @param actor Actor receiving the hit.
 * @param alternate Nonzero selects animation 0x0B; zero randomly selects 0x14 or 0x15.
 */
void func_8008B870(FieldActor* actor, s32 alternate)
{
    s16 command;
    s32 flags;
    FieldObjectState* state;
    FieldObjectState* states;

    if (actor->object_index < 3U)
    {
        g_field_player_records[actor->object_index].hit_state = 5;
    }
    else if (g_field_object_states[actor->object_index].unk8.word < 0)
    {
        g_field_boss_hud_shake_frame = 5;
    }
    if (!((g_field_object_states[actor->object_index].contact.word >> 6) & 1))
    {
        if (field_actor_binding(actor)->state != 0)
        {
            if (field_actor_binding(actor)->owner == actor->object_index)
            {
                return;
            }
        }
    }
    command = actor->command;
    if (command != 0x93 && command != 0x94 && command != 0x90)
    {
        states = g_field_object_states;
        state = &states[actor->object_index];
        flags = state->flags;
        if (!(flags & 0x200))
        {
            if ((state->contact.bytes.flags & 1) || (flags & 0x23E4))
            {
                actor->unk2E = 1;
                actor->unk27 = 0;
                actor->unk24 = 1;
                actor->animation &= 0x80;
                field_restart_actor_animation(actor);
                actor->command = FIELD_COMMAND_HIT_REACTION;
                return;
            }
            if ((actor->animation & 0x7F) != 0x44)
            {
                state->contact.word &= ~0x40;
                field_update_sequence_actor_binding(actor, 0);
                actor->command = FIELD_COMMAND_HIT_REACTION;
                actor->y -= actor->height << 8;
                g_field_object_states[actor->object_index].retry_count = 0;
                actor->unk30 = 0;
                if (alternate != 0)
                {
                    actor->animation = (actor->animation & 0x80) + 0xB;
                }
                else
                {
                    actor->animation = (actor->animation & 0x80) + 0x14;
                    actor->animation += rand() & 1;
                }
                func_8008BC5C(actor);
                actor->unk2E = 1;
                actor->unk24 = 1;
                actor->unk27 = 0;
                g_field_object_states[actor->object_index].movement.word &= ~0x1800;
                field_restart_actor_animation(actor);
                actor->y += actor->height << 8;
                if (actor->y > 0)
                {
                    actor->y = 0;
                }
                g_field_object_states[actor->object_index].flags &= ~0x4000;
                g_field_object_states[actor->object_index].flags &= 0xFFFF7FFF;
                field_stop_actor_animations_for_object(actor, 0);
                if (actor->object_index < 2U)
                {
                    func_800A2DD8(actor->object_index);
                    g_field_object_states[actor->object_index].retry_count = 0;
                    actor->unk30 = 0;
                }
            }
        }
    }
}

/**
 * @brief Clear the actor's pending link flag and the linked object's draw bit.
 * @param actor Actor whose object state is updated.
 */
void func_8008BC5C(FieldActor* actor)
{
    FieldObjectState* states = g_field_object_states;
    FieldObjectState* state;
    u32 contact;
    FieldObjectState* linked;

    state = &states[actor->object_index];
    contact = state->contact.word;
    if ((contact >> 1) & 1)
    {
        state->contact.word = contact & ~2;
        linked = &states[states[actor->object_index].linked_object_index];
        linked->flags = linked->flags & ~0x2000;
    }
}

/**
 * @brief Clear the pending link flag of every object linked to the actor.
 * @param actor Actor whose object is the link target.
 */
void func_8008BCF8(FieldActor* actor)
{
    s32 i;

    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        if ((g_field_object_states[i].contact.word >> 1) & 1)
        {
            if (g_field_object_states[i].linked_object_index == actor->object_index)
            {
                g_field_object_states[i].contact.word = g_field_object_states[i].contact.word & ~2;
                g_field_object_states[actor->object_index].flags &= ~0x2000;
            }
        }
    }
}

/**
 * @brief Stop the script and animations of the actor with @p key.
 * @param key Object key to look up.
 * @return 0 when the actor was stopped, -1 when no actor has @p key.
 */
s32 func_8008BD88(s32 key)
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
    func_80084424(actor->object_index);
    return 0;
}

/**
 * @brief Knock an actor down and release its animation actors.
 * @param actor Actor to reset.
 * @param clear_record Nonzero clears the party record's field 0x260 for party members.
 */
void func_8008BE38(FieldActor* actor, s32 clear_record)
{
    FieldObjectState* states;
    FieldObjectState* state;

    func_8008BC5C(actor);
    func_8008BCF8(actor);
    actor->command = 0x8E;
    if (clear_record != 0 && actor->object_index < 3)
    {
        g_field_player_records[actor->object_index].unk260 = 0;
    }

    actor->script_index = FIELD_SCRIPT_NONE;
    states = g_field_object_states;
    actor->unk2E = 1;
    actor->unk24 = 1;
    actor->y = 0;
    actor->unk27 = 0;
    actor->animation = (actor->animation & 0x80) + 0x1D;

    state = &states[actor->object_index];
    state->movement.word &= ~0x1800;
    field_restart_actor_animation(actor);

    field_stop_actor_animations_for_object(actor, 1);
    func_80083BC0(actor, &g_field_actor_slots[64 + actor->object_index], 1);
    actor->control.word |= 0x800;
    field_update_sequence_actor_binding(actor, 0);
    func_80084424(actor->object_index);
}

/**
 * @brief Start command 0x8F on an actor with a heading, animation and wait count.
 * @param actor Actor to update.
 * @param heading Value stored as the actor's heading byte.
 * @param animation Animation added to the actor's mirror bit.
 * @param wait Value stored in the actor's wait counter.
 */
void func_8008BF88(FieldActor* actor, s8 heading, s32 animation, s8 wait)
{
    FieldObjectState* states = g_field_object_states;
    FieldObjectState* state;

    actor->command = 0x8F;
    actor->unk2E = 1;
    actor->unk24 = 1;
    actor->unk1B = heading;
    actor->unk27 = 0;
    actor->animation = (actor->animation & 0x80) + animation;
    state = &states[actor->object_index];
    state->movement.word = state->movement.word & ~0x1800;
    field_restart_actor_animation(actor);
    actor->unk20 = wait;
}

/**
 * @brief Start command 0xAE on an actor and re-arm its object state.
 * @param actor Actor to update.
 * @param value Value stored in the object state's field 0x16C.
 */
void func_8008C024(FieldActor* actor, s8 value)
{
    if (actor->object_index < 3)
    {
        g_field_player_records[actor->object_index].unk260 = 0;
    }

    g_field_object_states[actor->object_index].flags &= 0x200;
    g_field_object_states[actor->object_index].contact.word |= 0x20;
    g_field_object_states[actor->object_index].unk16C = value;
    actor->command = 0xAE;
    actor->unk20 = 10;
}

/**
 * @brief Knock an actor down and select its recovery command and animation.
 * @param actor Actor to update.
 * @return Unspecified; callers do not use the value.
 * @note Declared s32 without a return statement: the int return type keeps the final store out of the return delay slot.
 */
s32 func_8008C104(FieldActor* actor)
{
    FieldObjectState* states;
    FieldObjectState* state;
    FieldResourceEntry* resource;
    FieldResourceEntry* resources;
    s32 object_index;

    func_8008BC5C(actor);
    func_8008BCF8(actor);
    actor->command = 0x8E;
    actor->script_index = FIELD_SCRIPT_NONE;
    actor->unk2E = 1;
    actor->unk24 = 1;
    actor->y = 0;
    actor->unk27 = 0;
    actor->animation = (actor->animation & 0x80) + 0x1D;
    states = g_field_object_states;
    state = &states[actor->object_index];
    state->movement.word &= ~0x1800;
    field_restart_actor_animation(actor);
    field_stop_actor_animations_for_object(actor, 1);
    func_80083BC0(actor, &g_field_actor_slots[64 + actor->object_index], 1);
    actor->control.word |= 0x800;
    field_update_sequence_actor_binding(actor, 0);
    func_80084424(actor->object_index);
    object_index = actor->object_index;
    if (!(states[object_index].contact.bytes.flags & 1))
    {
        resources = g_field_resource_entries;
        resource = &resources[actor->resource_index];
        if (resource->unkE & 0x8000)
        {
            actor->command = 0x92;
        }
        else
        {
            func_80083EEC(object_index, object_index + 0x40, resource->unkE);
            field_start_actor_animation(actor->object_index + 0x40, 0, 0);
        }
    }
    if (!(g_field_object_states[actor->object_index].flags & 0x200))
    {
        if (actor->command == 0x92)
        {
            actor->command = 0x93;
        }
        else
        {
            actor->command = 0x90;
        }
    }
}

/**
 * @brief Test whether two objects can interact and overlap in depth.
 * @param first_key Key of the object whose position is the centre of the test.
 * @param second_key Key of the object whose eligibility and position are tested.
 * @return -1 when either object is absent or the second is unused, 1 for an eligible overlap, 0 otherwise.
 */
s32 func_8008C2EC(s32 first_key, s32 second_key)
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
    if (second_object >= 3U && (second_state->group_flags & 15) != g_field_active_group)
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
    if (D_8010D020 == 0)
    {
        if (first_index < 3U)
        {
            if (second_index < 3U)
            {
                return 0;
            }
        }
        else if (second_index >= 3U)
        {
            return 0;
        }
    }
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
 * @brief Count a retry for an object and, at the party member's limit, restart its recovery.
 * @param object_index Object whose retry counter advances.
 */
void func_8008C4A8(s32 object_index)
{
    FieldObjectState* states;
    FieldObjectState* state;
    s32 retry_count;
    FieldActor* actor;
    s32 animation_slot;

    states = g_field_object_states;
    state = &states[object_index];
    if (state->flags & 0x8000)
    {
        retry_count = state->retry_count + 1;
        state->retry_count = retry_count;
        if ((object_index < 2) && ((u8)retry_count >= D_800EB068[g_field_player_records[object_index].type]))
        {
            actor = field_find_actor(state->key);
            if (actor != FIELD_ACTOR_NONE)
            {
                animation_slot = func_800839F8(0, 0);
                if ((animation_slot != -1) && (func_80083EEC(actor->object_index, animation_slot, 0x21) != 0))
                {
                    field_start_actor_animation(animation_slot, 0, 0);
                }
            }
            func_8008C620(&g_field_actors[object_index]);
        }
    }
}

/**
 * @brief Start the recovery command 0xB7 on an actor and clear its hit flags.
 * @param actor Actor to update.
 */
void func_8008C620(FieldActor* actor)
{
    g_field_object_states[actor->object_index].retry_count = 0;
    g_field_object_states[actor->object_index].flags &= ~0x8000;
    g_field_object_states[actor->object_index].flags &= ~0x4000;
    actor->command = 0xB7;
    actor->unk20 = 0x1E;
    actor->unk2E = 1;
    actor->unk30 = 0;
    actor->y = 0;
    actor->unk27 = 0;
    actor->animation = (actor->animation & 0x80) + 0x13;
    actor->unk24 = 1;
    g_field_object_states[actor->object_index].movement.word &= ~0x1800;
    field_restart_actor_animation_reverse(actor);
}
