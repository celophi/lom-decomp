/**
 * @file field_resource_load.c
 * @brief Battle entry sequence and the party resource reload that goes with it.
 *
 * When field_battle_start starts a battle, field_begin_battle_entry arms a
 * small state machine that field_update_battle_entry advances once per frame:
 * it waits for the
 * party's animation bindings, requests the battle resources of both players,
 * applies the actor changes queued by field_queue_battle_entry_change, waits
 * until everything has settled and then installs the new resources and sets
 * up the battle. The resources are read into per-player staging buffers and
 * installed by field_install_party_reload (also used after the battle, when
 * the field resources are read back).
 */

#include "common.h"
#include "cdrom.h"
#include "sdk/libetc.h"
#include "sdk/libgpu.h"
#include "field_actor_runtime.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_scene_transition.h"
#include "field_state_ops.h"

/** @brief Capacity of g_field_battle_entry_changes. */
#define FIELD_BATTLE_ENTRY_CHANGE_COUNT 8
/** @brief Field of a FieldBattleEntryChange left unchanged. */
#define FIELD_BATTLE_ENTRY_KEEP (-1)

/** @brief Battle entry states (D_80122B20). */
enum
{
    FIELD_BATTLE_ENTRY_IDLE = 0,
    FIELD_BATTLE_ENTRY_WAIT_BINDINGS = 1,
    FIELD_BATTLE_ENTRY_REQUEST_RESOURCES = 2,
    FIELD_BATTLE_ENTRY_APPLY_CHANGES = 3,
    FIELD_BATTLE_ENTRY_WAIT_SETTLED = 4
};

/** @brief field_get_actor_resource_id weapon_set value: the weapon-specific (battle) packages. */
#define FIELD_WEAPON_SET_BATTLE 1

/** @brief Offset of the first player's staging buffer in g_field_cd_buffer, and the size of each. */
#define FIELD_PARTY_RELOAD_BUFFER_OFFSET 0x8000
#define FIELD_PARTY_RELOAD_BUFFER_SIZE 0x18000
/** @brief Staging buffer that player @p slot's reloaded resource is read into. */
#define FIELD_PARTY_RELOAD_BUFFER(slot) (g_field_cd_buffer + FIELD_PARTY_RELOAD_BUFFER_OFFSET + (slot) * FIELD_PARTY_RELOAD_BUFFER_SIZE)

/** @brief Animation both players start once their battle resources are installed. */
#define FIELD_ANIMATION_BATTLE_ENTRY 0x11
/** @brief Tint flash length of the party members when a battle starts, in frames. */
#define FIELD_BATTLE_ENTRY_TINT_FRAMES 60

/** @brief Sound played when a battle starts. */
#define FIELD_SOUND_BATTLE_ENTRY 0x79
#define FIELD_SOUND_PAN_CENTRE 0x80

/** @brief Change applied to one actor on battle entry; FIELD_BATTLE_ENTRY_KEEP leaves a field unchanged. */
typedef struct
{
    s16 actor_index;
    /** @brief Actor animation (FieldActor::animation without the facing bit). */
    s16 animation;
    /** @brief Builtin animation started on the actor. */
    s16 builtin_animation;
    s16 sound;
} FieldBattleEntryChange;

/* Not in field_calls.h: field_dialog_screens.c calls it with an argument. */
s32 field_party_reload_reading(void);
FieldActor* field_lookup_actor(s32 key);
void field_restart_actor_animation(FieldActor* actor);
/* Defined as (void) in field_actor_runtime.c; the original call still loads 1 into $a0. */
void field_restore_default_action_animation_mappings(s32);

extern s32 g_field_battle_entry_change_count;
extern s32 g_field_party_reload_sizes[FIELD_PLAYER_COUNT];
extern s32 D_80122B20;
extern FieldBattleEntryChange g_field_battle_entry_changes[FIELD_BATTLE_ENTRY_CHANGE_COUNT];
extern s32 g_field_party_reload_resource_ids[FIELD_PLAYER_COUNT];
extern s32 g_field_active_group;
extern u8* g_field_cd_buffer;
extern void* g_field_resource_cursor;

/** @brief Cancel the battle entry: drop the queued changes and the pending party reloads. */
void field_reset_battle_entry(void)
{
    s32 i;

    g_field_battle_entry_change_count = 0;
    D_80122B20 = FIELD_BATTLE_ENTRY_IDLE;

    for (i = FIELD_PLAYER_COUNT - 1; i >= 0; i--)
    {
        g_field_party_reload_resource_ids[i] = 0;
    }
}

/**
 * @brief Start the battle entry sequence.
 * @return The new state, FIELD_BATTLE_ENTRY_WAIT_BINDINGS.
 */
s32 field_begin_battle_entry(void)
{
    return D_80122B20 = FIELD_BATTLE_ENTRY_WAIT_BINDINGS;
}

/** @brief Advance the battle entry sequence by one step. */
void field_update_battle_entry(void)
{
    s32 state;
    s32 i;

    state = D_80122B20;
    if (state == FIELD_BATTLE_ENTRY_IDLE)
    {
        return;
    }
    switch (state)
    {
    case FIELD_BATTLE_ENTRY_WAIT_BINDINGS:
        if ((g_field_actor_bindings[0].state | g_field_actor_bindings[1].state | g_field_actor_bindings[2].state) == FIELD_BINDING_IDLE)
        {
            D_80122B20 = FIELD_BATTLE_ENTRY_REQUEST_RESOURCES;
        }
        break;

    case FIELD_BATTLE_ENTRY_REQUEST_RESOURCES:
        field_request_party_reload(FIELD_WEAPON_SET_BATTLE);
        D_80122B20 = FIELD_BATTLE_ENTRY_APPLY_CHANGES;
        break;

    case FIELD_BATTLE_ENTRY_APPLY_CHANGES:
    {
        FieldActor* actor;
        s32 actor_slot;
        u8 animation;

        for (i = 0; i < g_field_battle_entry_change_count; i++)
        {
            actor = &g_field_actors[g_field_battle_entry_changes[i].actor_index];
            if (g_field_battle_entry_changes[i].animation != FIELD_BATTLE_ENTRY_KEEP)
            {
                actor->presence = 0;
                actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_STEP;
                animation = g_field_battle_entry_changes[i].animation | (actor->animation & FIELD_ANIMATION_FACING);
                actor->animation = animation;
                if (animation & FIELD_ANIMATION_FACING)
                {
                    actor->direction = 0;
                }
                else
                {
                    actor->direction = 0x80;
                }
                actor->animation_state = 1;
                actor->animation_active = 1;
                actor->control.word &= ~FIELD_CONTROL_PLAY_ONCE;
                actor->script_offset += 3;
                field_restart_actor_animation(actor);
            }
            if (g_field_battle_entry_changes[i].builtin_animation != FIELD_BATTLE_ENTRY_KEEP)
            {
                actor_slot = field_find_free_actor_slot(actor->object_index, 0);
                if ((actor_slot != -1) &&
                    (field_start_builtin_animation(actor->object_index, actor_slot, g_field_battle_entry_changes[i].builtin_animation) != 0))
                {
                    field_start_actor_animation(actor_slot, 0, NULL);
                    g_field_object_states[actor->object_index].contact.bytes.animation_actor_index = actor_slot;
                }
            }
            if (g_field_battle_entry_changes[i].sound != FIELD_BATTLE_ENTRY_KEEP)
            {
                field_play_sound(g_field_battle_entry_changes[i].sound, FIELD_SOUND_PAN_CENTRE);
                VSync(0);
            }
        }
        D_80122B20 = FIELD_BATTLE_ENTRY_WAIT_SETTLED;
        break;
    }

    case FIELD_BATTLE_ENTRY_WAIT_SETTLED:
    {
        s16 command;

        if (field_party_reload_reading() == 0)
        {
            for (i = 0; i < g_field_battle_entry_change_count; i++)
            {
                if (field_object_has_active_actor_tracks(g_field_battle_entry_changes[i].actor_index) != 0)
                {
                    break;
                }
            }
            if (i == g_field_battle_entry_change_count)
            {
                for (i = FIELD_PARTY_COUNT; i < FIELD_ACTOR_COUNT; i++)
                {
                    if (g_field_actors[i].presence != FIELD_ACTOR_UNUSED && g_field_actors[i].command == FIELD_ACTOR_COMMAND_SEQUENCE_STEP)
                    {
                        break;
                    }
                }
                if (i == FIELD_ACTOR_COUNT)
                {
                    for (i = 1; i < FIELD_PARTY_COUNT; i++)
                    {
                        if (g_field_player_records[i].head.bytes.flags & FIELD_PLAYER_ACTIVE)
                        {
                            command = g_field_actors[i].command;
                            if ((command == FIELD_ACTOR_COMMAND_FOLLOW_LEADER) || (command == FIELD_ACTOR_COMMAND_RUN_PATH))
                            {
                                break;
                            }
                        }
                    }
                    if (i == FIELD_PARTY_COUNT)
                    {
                        for (i = 0; i < FIELD_PARTY_COUNT; i++)
                        {
                            g_field_object_parts[i].flags &= ~FIELD_PART_IGNORE_MAP_COLLISION;
                            g_field_actors[i].command = FIELD_ACTOR_COMMAND_NONE;
                        }
                        field_play_sound(FIELD_SOUND_BATTLE_ENTRY, FIELD_SOUND_PAN_CENTRE);
                        for (i = 0; i < FIELD_PLAYER_COUNT; i++)
                        {
                            if (g_field_party_reload_resource_ids[i] != 0)
                            {
                                field_install_party_reload(FIELD_RESOURCE_HAS_ACTIONS, i);
                                g_field_actors[i].command = FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION;
                                g_field_actors[i].animation_state = 1;
                                g_field_actors[i].animation_frame = 0;
                                g_field_actors[i].animation_active = 1;
                                g_field_actors[i].animation = (g_field_actors[i].animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_BATTLE_ENTRY;
                                g_field_actors[i].control.word &= ~FIELD_CONTROL_PLAY_ONCE;
                                g_field_object_states[i].movement.word &= ~FIELD_MOVEMENT_SEQUENCE_MASK;
                                field_restart_actor_animation(&g_field_actors[i]);
                            }
                        }
                        for (i = 0; i < FIELD_PARTY_COUNT; i++)
                        {
                            if (g_field_player_records[i].head.bytes.flags & FIELD_PLAYER_ACTIVE)
                            {
                                g_field_object_states[i].tint_timer = FIELD_BATTLE_ENTRY_TINT_FRAMES;
                                g_field_object_states[i].movement.word |= FIELD_MOVEMENT_TINT_FLASH;
                            }
                        }
                        DrawSync(0);
                        field_reset_actor_resources();
                        field_restore_default_action_animation_mappings(1);
                        field_battle_setup(g_field_active_group);
                        D_80122B20 = FIELD_BATTLE_ENTRY_IDLE;
                    }
                }
            }
        }
        break;
    }
    }
}

/**
 * @brief Queue a change to an actor for the battle entry, or replace the actor's queued change.
 * @param actor_key Actor key resolved with field_lookup_actor.
 * @param animation Actor animation, or FIELD_BATTLE_ENTRY_KEEP.
 * @param builtin_animation Builtin animation to start, or FIELD_BATTLE_ENTRY_KEEP.
 * @param sound Sound to play, or FIELD_BATTLE_ENTRY_KEEP.
 * @return 0 on success, -1 when the queue is full or the actor does not exist.
 */
s32 field_queue_battle_entry_change(s32 actor_key, s32 animation, s32 builtin_animation, s32 sound)
{
    FieldActor* actor;
    s32 i;
    FieldBattleEntryChange* change;
    FieldBattleEntryChange* changes;

    if (g_field_battle_entry_change_count == FIELD_BATTLE_ENTRY_CHANGE_COUNT)
    {
        return -1;
    }
    actor = field_lookup_actor(actor_key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->presence == FIELD_ACTOR_UNUSED)
    {
        return -1;
    }
    for (i = 0; i < g_field_battle_entry_change_count; i++)
    {
        change = &g_field_battle_entry_changes[i];
        if (change->actor_index == actor->object_index)
        {
            change->actor_index = actor->object_index;
            change->animation = animation;
            change->builtin_animation = builtin_animation;
            if (builtin_animation != FIELD_BATTLE_ENTRY_KEEP)
            {
                change->animation = 0;
            }
            change->sound = sound;
            return 0;
        }
    }
    changes = g_field_battle_entry_changes;
    change = &changes[g_field_battle_entry_change_count];
    change->actor_index = actor->object_index;
    change->animation = animation;
    change->builtin_animation = builtin_animation;
    if (builtin_animation != FIELD_BATTLE_ENTRY_KEEP)
    {
        change->animation = FIELD_ANIMATION_GUARD;
    }
    changes[g_field_battle_entry_change_count].sound = sound;
    g_field_battle_entry_change_count += 1;
    return 0;
}

/**
 * @brief Check whether a party reload is waiting to be installed.
 * @return 1 if a player has a pending reload, otherwise 0.
 */
s32 field_party_reload_pending(void)
{
    s32 i;

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        if (g_field_party_reload_resource_ids[i] != 0)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Check whether a pending party reload is still being read from CD.
 * @return 1 if a read is still queued, otherwise 0.
 */
s32 field_party_reload_reading(void)
{
    s32 i;

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        if (g_field_party_reload_resource_ids[i] != 0)
        {
            /* CD resource ids are 16-bit. */
            if (cdrom_can_queue_resource((u16)g_field_party_reload_resource_ids[i]) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Install a player's reloaded resource from its staging buffer.
 * @param alternate_layout FIELD_RESOURCE_HAS_ACTIONS for a battle package, 0 for a field package.
 * @param slot Player index; nothing happens without a pending reload.
 */
void field_install_party_reload(s32 alternate_layout, s32 slot)
{
    FieldResourceEntry* entry;
    FieldResourceEntry* entries;
    u32 flags;

    if (g_field_party_reload_resource_ids[slot] != 0)
    {
        field_release_resource_entry(slot);
        entries = g_field_resource_entries;
        entry = &entries[slot];
        entry->slot_index = slot;
        entry->unk8 = 0;
        field_set_party_palettes();
        entry->unkE = 0x2F;
        flags = entry->flags;
        flags &= ~FIELD_RESOURCE_HAS_ACTIONS;
        flags |= alternate_layout & 1;
        entry->flags = flags;
        entry->start = g_field_resource_cursor;
        field_unpack_resource_package((struct FieldCdBuffer*)FIELD_PARTY_RELOAD_BUFFER(slot), g_field_party_reload_sizes[slot], slot, slot);
        entry->end = g_field_resource_cursor;
        entry->flags |= FIELD_RESOURCE_LOADED;
        g_field_party_reload_resource_ids[slot] = 0;
    }
}

/**
 * @brief Request the resources of both players for a weapon set; inactive players get none.
 * @param weapon_set Weapon set passed to field_get_actor_resource_id.
 */
void field_request_party_reload(s32 weapon_set)
{
    s32 i;

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        if (g_field_player_records[i].head.bytes.flags & FIELD_PLAYER_ACTIVE)
        {
            g_field_party_reload_resource_ids[i] = field_get_actor_resource_id(i, &g_field_player_records[i], weapon_set);
            g_field_player_records[i].resource_id = g_field_party_reload_resource_ids[i];
            g_field_party_reload_sizes[i] = cdrom_queue_read((u16)g_field_party_reload_resource_ids[i], FIELD_PARTY_RELOAD_BUFFER(i));
        }
        else
        {
            g_field_party_reload_sizes[i] = 0;
            g_field_party_reload_resource_ids[i] = 0;
        }
    }
}
