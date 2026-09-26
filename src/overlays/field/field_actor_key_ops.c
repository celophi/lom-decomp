/**
 * @file field_actor_key_ops.c
 * @brief Script-facing field actor operations addressed by object key.
 *
 * Scripts refer to field objects by the key word of their object state. Each
 * routine here resolves such a key to the parallel actor record and reads or
 * changes its position, facing, control mode or running script.
 */

#include "common.h"
#include "controller_internal.h"
#include "main.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "field_actor_routes.h"
#include "field_calls.h"
#include "field_actor_tables.h"

/** @brief Control modes (FieldActor::control & FIELD_CONTROL_MODE_MASK). */
#define FIELD_CONTROL_PLAYER 0
#define FIELD_CONTROL_FOLLOWER 1
#define FIELD_CONTROL_SCRIPTED 2

/** @brief Two-bit FieldActor::control field at bit 16 (meaning unknown) kept across a reset. */
#define FIELD_CONTROL_UNK16_SHIFT 16
#define FIELD_CONTROL_UNK16_BITS 3

/** @brief Coordinate value that, in all three axes, keeps the actor's old position. */
#define FIELD_KEEP_POSITION (-1)

/** @brief Positions are stored with eight fractional bits. */
#define FIELD_POSITION_SHIFT 8

/** @brief Actors face one of eight directions. */
#define FIELD_FACING_COUNT 8

/** @brief ratan2 angle units per facing direction (ONE / FIELD_FACING_COUNT). */
#define FIELD_FACING_ANGLE (ONE / FIELD_FACING_COUNT)

/** @brief Facing steps between the ratan2 zero angle and facing 0. */
#define FIELD_FACING_ANGLE_OFFSET 2

/** @brief Menu option bit 0 (the menu's vibration switch); here it lets object 1 lead. */
#define FIELD_OPTION_VIBRATION 0x01

/** @brief Object 1, the second party member. */
#define FIELD_SECOND_MEMBER 1

/** @brief Result of field_get_actor_binding_state. */
enum
{
    FIELD_BINDING_QUERY_NONE = 0,
    FIELD_BINDING_QUERY_OTHER_OWNER = 1,
    FIELD_BINDING_QUERY_LOADING = 2,
    FIELD_BINDING_QUERY_PLAYING = 3,
    FIELD_BINDING_QUERY_READY = 4
};

/** @brief Event script table: u16 offsets from the table start, then the scripts. */
extern u16* g_field_event_scripts;

FieldActor* field_lookup_actor(s32 key);
int abs(int value);
void field_restart_actor_animation(FieldActor* actor);

/**
 * @brief Replace the group bits of object @p object_index.
 * @param object_index Object whose group_flags change.
 * @param group New group.
 */
static inline void field_set_object_group(s32 object_index, s32 group)
{
    g_field_object_states[object_index].group_flags = (g_field_object_states[object_index].group_flags & ~FIELD_OBJECT_GROUP_MASK) | group;
}

/**
 * @brief Replace the group of the object with @p key.
 * @param key Object key to look up.
 * @param group New group (low four group-flag bits); nothing happens when the key is absent.
 */
void field_set_actor_group(s32 key, s32 group)
{
    FieldActor* actor;

    actor = field_lookup_actor(key);
    if (actor != FIELD_ACTOR_NONE)
    {
        field_set_object_group(actor->object_index, group);
    }
}

/**
 * @brief Reinitialize the actor with @p key at a new position and group.
 * @param key Object key to look up.
 * @param resource_entry_index Resource entry the actor record is initialized from.
 * @param group New group.
 * @param x New X position in whole units.
 * @param y New Y position in whole units.
 * @param z New Z position in whole units.
 */
void field_reset_actor_at(s32 key, s32 resource_entry_index, s32 group, s32 x, s32 y, s32 z)
{
    FieldActor* actor;
    s32 kept_bits;

    actor = field_lookup_actor(key);
    if (actor != FIELD_ACTOR_NONE)
    {
        kept_bits = actor->control.half[1] & FIELD_CONTROL_UNK16_BITS;
        field_initialize_actor_record(actor->object_index, resource_entry_index);
        actor->presence = 0;
        actor->x = x << FIELD_POSITION_SHIFT;
        actor->control.word = (actor->control.word & ~(FIELD_CONTROL_UNK16_BITS << FIELD_CONTROL_UNK16_SHIFT)) | (kept_bits << FIELD_CONTROL_UNK16_SHIFT);
        actor->y = y << FIELD_POSITION_SHIFT;
        actor->z = z << FIELD_POSITION_SHIFT;
        field_set_object_group(actor->object_index, group);
        field_restart_actor_animation(actor);
    }
}

/**
 * @brief Test whether one actor faces another, allowing one facing step either way.
 * @param looker_key Key of the actor whose facing is tested.
 * @param target_key Key of the actor looked at.
 * @return -1 when either actor is absent, otherwise 1 when facing the target and 0 when not.
 */
s32 field_actor_faces_actor(s32 looker_key, s32 target_key)
{
    FieldActor* looker;
    FieldActor* target;
    s32 direction;
    s32 facing;
    s32 animation;

    looker = field_lookup_actor(looker_key);
    if (looker == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    target = field_lookup_actor(target_key);
    if (target == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    animation = looker->animation;
    facing = (animation & FIELD_ANIMATION_INDEX_MASK) % FIELD_ANIMATION_DIRECTIONS;
    if (animation & FIELD_ANIMATION_FACING)
    {
        facing = FIELD_FACING_COUNT - facing;
    }
    direction = ratan2(looker->z - target->z, target->x - looker->x);
    direction = (direction + ONE / 2) / FIELD_FACING_ANGLE;
    direction += FIELD_FACING_ANGLE_OFFSET;
    direction %= FIELD_FACING_COUNT;
    if (abs(facing - direction) <= 1)
    {
        return 1;
    }
    if (abs(facing - direction + FIELD_FACING_COUNT) <= 1)
    {
        return 1;
    }
    return abs(direction - facing + FIELD_FACING_COUNT) <= 1;
}

/**
 * @brief Report the state of the animation actor bound to the actor with @p key.
 * @param key Object key to look up.
 * @return -1 when absent, otherwise a FIELD_BINDING_QUERY_* value.
 */
s32 field_get_actor_binding_state(s32 key)
{
    FieldActor* actor;
    FieldActorBinding* binding;

    actor = field_lookup_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    /* The binding is looked up again at each use; caching it changes the code. */
    if (field_actor_binding(actor)->state == FIELD_BINDING_IDLE)
    {
        return FIELD_BINDING_QUERY_NONE;
    }
    binding = field_actor_binding(actor);
    if (binding->owner != actor->object_index)
    {
        return FIELD_BINDING_QUERY_OTHER_OWNER;
    }
    if (field_object_binding(binding->owner)->state == FIELD_BINDING_LOADING)
    {
        return FIELD_BINDING_QUERY_LOADING;
    }
    if (g_field_actor_slots[field_actor_binding(actor)->slot].track_mask != 0 || g_field_actor_slots[field_actor_binding(actor)->slot].pending_track_mask != 0)
    {
        return FIELD_BINDING_QUERY_PLAYING;
    }
    return FIELD_BINDING_QUERY_READY;
}

/**
 * @brief Reload the actor with @p key from a resource entry, keeping or replacing its position.
 * @param key Object key to look up; also written back as the object's key.
 * @param resource_entry_index Resource entry to load and initialize from.
 * @param resource_slot_id Resource slot passed to the loader.
 * @param resource_base Resource base passed to the loader.
 * @param group New group.
 * @param x New X position in whole units; FIELD_KEEP_POSITION in all three keeps the old one.
 * @param y New Y position in whole units.
 * @param z New Z position in whole units.
 * @param animation New animation byte.
 * @param has_actions Non-zero marks the resource as having an action table.
 * @return -1 when no actor has @p key; the success path returns nothing.
 * @note Callers never use the result of a successful reload.
 */
s32 field_reload_actor(s32 key, s32 resource_entry_index, s32 resource_slot_id, u8* resource_base, s32 group, s32 x, s32 y, s32 z, s32 animation, s32 has_actions)
{
    Vec3i old_position;
    s32 control_mode;
    FieldActor* actor;
    FieldResourceEntry* resource;
    FieldResourceEntry* resources;

    actor = field_lookup_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    old_position.x = actor->x;
    old_position.y = actor->y;
    old_position.z = actor->z;
    control_mode = actor->control.half[0] & FIELD_CONTROL_MODE_MASK;
    field_load_resource_entry(resource_slot_id, resource_base, resource_entry_index);
    field_initialize_actor_record(actor->object_index, resource_entry_index);
    field_initialize_actor_part(actor->object_index, 0);
    /* A direct &g_field_resource_entries[i] changes the code. */
    resources = g_field_resource_entries;
    resource = &resources[resource_entry_index];
    resource->flags = (resource->flags & ~FIELD_RESOURCE_HAS_ACTIONS) | (has_actions & FIELD_RESOURCE_HAS_ACTIONS);
    g_field_object_states[actor->object_index].key = key;
    actor->presence = 0;
    if (x == FIELD_KEEP_POSITION && y == FIELD_KEEP_POSITION && z == FIELD_KEEP_POSITION)
    {
        actor->x = old_position.x;
        actor->y = old_position.y;
        actor->z = old_position.z;
    }
    else
    {
        actor->x = x << FIELD_POSITION_SHIFT;
        actor->y = y << FIELD_POSITION_SHIFT;
        actor->z = z << FIELD_POSITION_SHIFT;
    }
    field_set_object_group(actor->object_index, group);
    actor->animation = animation;
    actor->control.word = (actor->control.word & ~FIELD_CONTROL_MODE_MASK) | control_mode;
    field_restart_actor_animation(actor);
}

/**
 * @brief Find the field actor whose object state carries @p key.
 * @param key Object key to look up.
 * @return The matching actor record, or FIELD_ACTOR_NONE.
 */
FieldActor* field_lookup_actor(s32 key)
{
    return field_find_actor(key);
}

/**
 * @brief Start script @p script_index on the actor with @p key unless it is busy.
 * @param key Object key to look up.
 * @param script_index Script to run from its first command.
 * @return 0 when the script was started, or -1 when absent or in a non-interruptible command.
 */
s32 field_start_actor_script(s32 key, u8 script_index)
{
    FieldActor* actor;
    s16 command;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    command = actor->command;
    if (command == FIELD_ACTOR_COMMAND_DEFEAT_WAIT || command == FIELD_ACTOR_COMMAND_DEFEAT_END)
    {
        return -1;
    }
    if (command == FIELD_ACTOR_COMMAND_DEFEATED || command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY || command == FIELD_ACTOR_COMMAND_KNOCKED_DOWN)
    {
        return -1;
    }
    actor->script_index = script_index;
    actor->script_offset = 0;
    actor->command = FIELD_ACTOR_COMMAND_NONE;
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
s32 field_set_actor_position(s32 key, s32 x, s32 y, s32 z)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->x = x << FIELD_POSITION_SHIFT;
    actor->y = y << FIELD_POSITION_SHIFT;
    actor->z = z << FIELD_POSITION_SHIFT;
    return 0;
}

/**
 * @brief Start the object-private script @p script on the actor with @p key unless it is busy.
 * @param key Object key to look up.
 * @param script Script bytecode stored as the object's private script.
 * @return 0 when the script was started, or -1 when absent or in a non-interruptible command.
 */
s32 field_start_actor_private_script(s32 key, u8* script)
{
    FieldActor* actor;
    s16 command;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    command = actor->command;
    if (command == FIELD_ACTOR_COMMAND_DEFEAT_WAIT || command == FIELD_ACTOR_COMMAND_DEFEAT_END)
    {
        return -1;
    }
    if (command == FIELD_ACTOR_COMMAND_DEFEATED || command == FIELD_ACTOR_COMMAND_DEFEAT_DELAY || command == FIELD_ACTOR_COMMAND_KNOCKED_DOWN)
    {
        return -1;
    }
    actor->script_index = FIELD_SCRIPT_OBJECT;
    g_field_object_states[actor->object_index].script = script;
    actor->script_offset = 0;
    if (actor->command != FIELD_ACTOR_COMMAND_WALK_TO_TARGET && actor->command != FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION)
    {
        actor->command = FIELD_ACTOR_COMMAND_NONE;
    }
    return 0;
}

/**
 * @brief Resolve an event script by index in the self-relative event script table.
 * @param index Entry index in the table's leading u16 offset list.
 * @return Address of the event script.
 */
u8* field_get_event_script(s32 index)
{
    return (u8*)g_field_event_scripts + g_field_event_scripts[index];
}

/**
 * @brief Find the object state that carries @p key.
 * @param key Object key to look up.
 * @return The matching object state, or FIELD_OBJECT_STATE_NONE.
 */
FieldObjectState* field_find_object_state(s32 key)
{
    FieldObjectState* state;
    s32 i;

    state = g_field_object_states;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++, state++)
    {
        if (state->key == key)
        {
            return state;
        }
    }
    return FIELD_OBJECT_STATE_NONE;
}

/**
 * @brief Copy the position of the actor with @p key.
 * @param key Object key to look up.
 * @param position Receives the actor's fixed-point position.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 field_get_actor_position(s32 key, Vec3i* position)
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
 * @brief Set the control mode of the actor with @p key.
 * @param key Object key to look up.
 * @param mode New control mode; only the FIELD_CONTROL_MODE_MASK bits are used.
 * @return -1 when no actor has @p key, otherwise 0.
 */
s32 field_set_actor_control_mode(s32 key, s32 mode)
{
    FieldActor* actor;
    s32 control_mode;
    u8 object_index;
    ControllerState* controller = CONTROLLER_STATE;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->control.word = (actor->control.word & ~FIELD_CONTROL_MODE_MASK) | (mode & FIELD_CONTROL_MODE_MASK);
    control_mode = actor->control.half[0] & FIELD_CONTROL_MODE_MASK;
    switch (control_mode)
    {
    case FIELD_CONTROL_PLAYER:
        object_index = actor->object_index;
        actor->script_index = FIELD_SCRIPT_NONE;
        actor->unk10 = 0;
        if (object_index == FIELD_SECOND_MEMBER)
        {
            if (!(g_pad_ctx->menu_option_flags & FIELD_OPTION_VIBRATION))
            {
                controller->ports[1].actuators_enabled = 0;
            }
            else
            {
                controller->ports[1].actuators_enabled = object_index;
            }
        }
        break;
    case FIELD_CONTROL_FOLLOWER:
        actor->script_index = FIELD_SCRIPT_NONE;
        actor->unk10 = 0;
        field_refresh_party_routes();
        if (actor->object_index == FIELD_SECOND_MEMBER)
        {
            controller->ports[1].actuators_enabled = 0;
        }
        break;
    case FIELD_CONTROL_SCRIPTED:
        break;
    }
    return 0;
}
