/**
 * @file field_actor_key_ops.c
 * @brief Script-facing field actor operations addressed by object key.
 *
 * Scripts refer to field objects by the key word of their object state. Each
 * routine here resolves such a key to the parallel actor record and reads or
 * changes its position, facing, control mode or running script.
 */

#include "common.h"
#include "field_actor_routes.h"
#include "field_calls.h"
#include "field_actor_tables.h"
#include "main.h"

/** @brief Position triple copied out by func_80087F44. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldActorPosition;

/** @brief Menu option bit that keeps the second party member as the leader. */
#define PAD_OPTION_SECOND_LEADER 1

extern u8* g_field_event_scripts;

FieldActor* func_80087C9C(s32 key);
long ratan2(long y, long x);
int abs(int value);
void field_restart_actor_animation(FieldActor* actor);

/**
 * @brief Replace the low four group-flag bits of the object with @p key.
 * @param key Object key to look up.
 * @param group New low four bits; the object is left unchanged when absent.
 */
void func_80087614(s32 key, s32 group)
{
    FieldActor* actor;

    actor = func_80087C9C(key);
    if (actor != FIELD_ACTOR_NONE)
    {
        g_field_object_states[actor->object_index].group_flags = (g_field_object_states[actor->object_index].group_flags & ~0xF) | group;
    }
}

/**
 * @brief Reinitialize the actor with @p key at a new position and group.
 * @param key Object key to look up.
 * @param resource_entry_index Resource entry the actor record is initialized from.
 * @param group New low four group-flag bits.
 * @param x New X position in whole units.
 * @param y New Y position in whole units.
 * @param z New Z position in whole units.
 */
void func_80087680(s32 key, s32 resource_entry_index, s32 group, s32 x, s32 y, s32 z)
{
    FieldActor* actor;
    s32 preserved;

    actor = func_80087C9C(key);
    if (actor != FIELD_ACTOR_NONE)
    {
        preserved = actor->control.half[1] & 3;
        field_initialize_actor_record(actor->object_index, resource_entry_index);
        actor->presence = 0;
        actor->x = x << 8;
        actor->control.word = (actor->control.word & 0xFFFCFFFF) | (preserved << 16);
        actor->y = y << 8;
        actor->z = z << 8;
        g_field_object_states[actor->object_index].group_flags = (g_field_object_states[actor->object_index].group_flags & ~0xF) | group;
        field_restart_actor_animation(actor);
    }
}

/**
 * @brief Test whether one actor faces another within one direction step.
 * @param first_key Key of the actor whose facing is tested.
 * @param second_key Key of the target actor.
 * @return -1 when either actor is absent, otherwise 1 when facing the target and 0 when not.
 */
s32 func_80087770(s32 first_key, s32 second_key)
{
    FieldActor* first;
    FieldActor* second;
    s32 direction;
    s32 facing;
    s32 animation;

    first = func_80087C9C(first_key);
    if (first == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    second = func_80087C9C(second_key);
    if (second == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    animation = first->animation;
    facing = (animation & 0x7F) % 5;
    if (animation & 0x80)
    {
        facing = 8 - facing;
    }
    direction = ratan2(first->z - second->z, second->x - first->x);
    direction = (direction + 0x800) / 0x200;
    direction += 2;
    direction %= 8;
    if (abs(facing - direction) < 2)
    {
        return 1;
    }
    if (abs(facing - direction + 8) < 2)
    {
        return 1;
    }
    return abs(direction - facing + 8) < 2;
}

/**
 * @brief Classify the animation binding state of the actor with @p key.
 * @param key Object key to look up.
 * @return -1 when absent; 0 unbound; 1 bound to another object; 2 loading; 3 or 4 by the slot's track state.
 */
s32 func_800878B4(s32 key)
{
    FieldActor* actor;
    FieldActorBinding* binding;

    actor = func_80087C9C(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (field_actor_binding(actor)->state == 0)
    {
        return 0;
    }
    binding = field_actor_binding(actor);
    if (binding->owner != actor->object_index)
    {
        return 1;
    }
    if (field_object_binding(binding->owner)->state == 1)
    {
        return 2;
    }
    if (g_field_actor_slots[field_actor_binding(actor)->slot].track_mask != 0 || g_field_actor_slots[field_actor_binding(actor)->slot].pending_track_mask != 0)
    {
        return 3;
    }
    return 4;
}

/**
 * @brief Reload the actor with @p key from a resource entry, keeping or replacing its position.
 * @param key Object key to look up; also written back as the object's key.
 * @param resource_entry_index Resource entry to load and initialize from.
 * @param resource_slot_id Resource slot passed to the loader.
 * @param resource_base Resource base passed to the loader.
 * @param group New low four group-flag bits.
 * @param x New X position in whole units; -1 in all three coordinates keeps the old position.
 * @param y New Y position in whole units.
 * @param z New Z position in whole units.
 * @param animation New animation byte.
 * @param resource_flag Low bit stored in the resource entry's flags.
 * @return -1 when no actor has @p key; otherwise the value left in the return register by the animation restart.
 * @note The success path has no return statement; callers never use its value.
 */
s32 func_80087A9C(s32 key, s32 resource_entry_index, s32 resource_slot_id, u8* resource_base, s32 group, s32 x, s32 y, s32 z, s32 animation, s32 resource_flag)
{
    s32 position[3];
    s32 control_mode;
    FieldActor* actor;
    FieldObjectState* state;
    FieldResourceEntry* resource;
    FieldResourceEntry* resources;
    FieldObjectState* states;

    actor = func_80087C9C(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    position[0] = actor->x;
    position[1] = actor->y;
    position[2] = actor->z;
    control_mode = actor->control.half[0] & 0x1FF;
    field_load_resource_entry(resource_slot_id, resource_base, resource_entry_index);
    field_initialize_actor_record(actor->object_index, resource_entry_index);
    field_initialize_actor_part(actor->object_index, 0);
    resources = g_field_resource_entries;
    resource = &resources[resource_entry_index];
    resource->flags = (resource->flags & ~1) | (resource_flag & 1);
    g_field_object_states[actor->object_index].key = key;
    actor->presence = 0;
    if (x == -1 && y == -1 && z == -1)
    {
        actor->x = position[0];
        actor->y = position[1];
        actor->z = position[2];
    }
    else
    {
        actor->x = x << 8;
        actor->y = y << 8;
        actor->z = z << 8;
    }
    states = g_field_object_states;
    state = &states[actor->object_index];
    state->group_flags = (state->group_flags & ~0xF) | group;
    actor->animation = animation;
    actor->control.word = (actor->control.word & ~0x1FF) | control_mode;
    field_restart_actor_animation(actor);
}

/**
 * @brief Find the field actor whose object state carries @p key.
 * @param key Object key to look up.
 * @return The matching actor record, or FIELD_ACTOR_NONE.
 */
FieldActor* func_80087C9C(s32 key)
{
    return field_find_actor(key);
}

/**
 * @brief Start script @p script_index on the actor with @p key unless it is busy.
 * @param key Object key to look up.
 * @param script_index Script to run from its first command.
 * @return 0 when the script was started, or -1 when absent or in a non-interruptible command.
 */
s32 func_80087CE0(s32 key, u8 script_index)
{
    FieldActor* actor;
    s16 command;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    command = actor->command;
    if (command == 0x93 || command == 0x94)
    {
        return -1;
    }
    if (command == 0x90 || command == 0xAE || command == 0x8E)
    {
        return -1;
    }
    actor->script_index = script_index;
    actor->script_offset = 0;
    actor->command = 0;
    return 0;
}

/**
 * @brief Move the actor with @p key to a new position.
 * @param key Object key to look up.
 * @param x New X position in whole units.
 * @param y New Y position in whole units.
 * @param z New Z position in whole units.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 func_80087D8C(s32 key, s32 x, s32 y, s32 z)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->x = x << 8;
    actor->y = y << 8;
    actor->z = z << 8;
    return 0;
}

/**
 * @brief Start the object-private script @p script on the actor with @p key unless it is busy.
 * @param key Object key to look up.
 * @param script Script bytecode stored as the object's private script.
 * @return 0 when the script was started, or -1 when absent or in a non-interruptible command.
 */
s32 func_80087E00(s32 key, u8* script)
{
    FieldActor* actor;
    s16 command;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    command = actor->command;
    if (command == 0x93 || command == 0x94)
    {
        return -1;
    }
    if (command == 0x90 || command == 0xAE || command == 0x8E)
    {
        return -1;
    }
    actor->script_index = FIELD_SCRIPT_OBJECT;
    g_field_object_states[actor->object_index].script = script;
    actor->script_offset = 0;
    if (actor->command != 0x8B && actor->command != 0x99)
    {
        actor->command = 0;
    }
    return 0;
}

/**
 * @brief Resolve an event script by index in the self-relative event script table.
 * @param index Entry index in the table's leading u16 offset list.
 * @return Address of the event script.
 */
u8* func_80087EF0(s32 index)
{
    return g_field_event_scripts + ((u16*)g_field_event_scripts)[index];
}

/**
 * @brief Find the object state that carries @p key.
 * @param key Object key to look up.
 * @return The matching object state, or (FieldObjectState*)-1.
 */
FieldObjectState* func_80087F0C(s32 key)
{
    FieldObjectState* state;
    s32 i;

    state = g_field_object_states;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++)
    {
        if (state->key == key)
        {
            return state;
        }
        state++;
    }
    return (FieldObjectState*)-1;
}

/**
 * @brief Copy the position of the actor with @p key.
 * @param key Object key to look up.
 * @param position Receives the actor's X, Y and Z position.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 func_80087F44(s32 key, FieldActorPosition* position)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    position->x = actor->x;
    position->y = actor->y;
    position->z = actor->z;
    return 0;
}

/**
 * @brief Set the nine-bit control mode of the actor with @p key.
 * @param key Object key to look up.
 * @param mode New control mode; only its low nine bits are used.
 * @return -1 when no actor has @p key, otherwise 0.
 */
s32 func_80087FC0(s32 key, s32 mode)
{
    FieldActor* actor;
    s32 control_mode;
    u8 object_index;
    FieldRenderState* render_state = FIELD_RENDER_STATE;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->control.word = (actor->control.word & ~0x1FF) | (mode & 0x1FF);
    control_mode = actor->control.half[0] & 0x1FF;
    if (control_mode != 1)
    {
        if (control_mode < 2)
        {
            if (control_mode == 0)
            {
                object_index = actor->object_index;
                actor->script_index = FIELD_SCRIPT_NONE;
                actor->unk10 = 0;
                if (object_index == 1)
                {
                    if (!(g_pad_ctx->menu_option_flags & PAD_OPTION_SECOND_LEADER))
                    {
                        render_state->leader_object_index = 0;
                    }
                    else
                    {
                        render_state->leader_object_index = object_index;
                    }
                }
            }
        }
    }
    else
    {
        actor->script_index = FIELD_SCRIPT_NONE;
        actor->unk10 = 0;
        field_refresh_party_routes();
        if (actor->object_index == control_mode)
        {
            render_state->leader_object_index = 0;
        }
    }
    return 0;
}
