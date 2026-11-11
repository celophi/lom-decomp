/**
 * @file field_record_lookup_ops.c
 * @brief Actor record lookup and allocation, reward pickup and resource lookup.
 */

#include "common.h"

#include "../internal/field_calls.h"
#include "../internal/field_records.h"
#include "main/audio/game_audio.h"

/** @brief FieldActorRecord::pickup bits 0-7: counter index of a counter pickup. */
#define FIELD_PICKUP_INDEX_MASK 0xFF

/** @brief Event record used when an actor id has no record. */
#define FIELD_DEFAULT_EVENT_RECORD 1

/** @brief FieldActorRecord::enabled_events bit of event 8. */
#define FIELD_EVENT_8_ENABLED (1 << 8)

/** @brief field_stop_actor_script flags bit: also stop the actor itself. */
#define FIELD_STOP_ACTOR 0x1

/** @brief Diagnostic code of a failed resource lookup. */
#define DIAG_MISSING_RESOURCE 0x6B

/**
 * @brief Hand an actor's pickup to the party: an item from the battle's reward table or a counter.
 * @param recipient Unused (the reward recipient the caller passes through).
 * @param owner_id Actor whose pickup code is resolved.
 * @note JP takes the entry the search stopped at without checking for a miss, so an
 *       unknown key hands over the item past the last entry.
 */
void field_grant_actor_pickup(s32 recipient, s32 owner_id)
{
    FieldActorRecord* actor;
    s32 index;
    FieldItemRecord* found;
    FieldItemRecord* handle;

    actor = field_find_actor_record_or_default(owner_id);

    if (!(actor->pickup & FIELD_PICKUP_COUNTER))
    {
#if defined(VERSION_JP)
        for (index = 0; index < g_field_battle->rewards->count; index++)
        {
            if (g_field_battle->rewards->entries[index].key == actor->pickup)
            {
                break;
            }
        }

        found = &g_field_battle->rewards->entries[index].item;
#else
        found = NULL;

        for (index = 0; index < g_field_battle->rewards->count; index++)
        {
            if (g_field_battle->rewards->entries[index].key == actor->pickup)
            {
                found = &g_field_battle->rewards->entries[index].item;
                break;
            }
        }

        if (found == NULL)
        {
            return;
        }
#endif

        handle = field_find_free_inventory_record();

        if (handle == NULL)
        {
            return;
        }

        field_copy_inventory_record(handle, found);
        field_append_dialog_item(handle->name, 0);
    }
    else
    {
        index = actor->pickup & FIELD_PICKUP_INDEX_MASK;
        field_receive_item(index);
        field_append_dialog_item(g_field_item_name_table.bytes + g_field_item_name_table.offsets[index], 1);
    }
}

/**
 * @brief Restore a fraction of an actor's maximum capacity, capped at that maximum.
 * @param record_id Identifier of an existing runtime actor.
 * @param fraction_256 Fraction in units of 1/256; pickups use 64 or 128.
 * @note The amount comes from maximum capacity, not the current value.
 */
void field_restore_actor_capacity_fraction(s32 record_id, s32 fraction_256)
{
    FieldObjectState* state;
    s32 scaled_capacity;

    state = field_find_object_state(record_id);
    scaled_capacity = state->maximum_hp * fraction_256;
    field_heal_status(state, (u32)scaled_capacity >> 8);
}

/**
 * @brief Look up an actor record, falling back to the second event record.
 * @param id Actor id (see field_find_actor_record).
 * @return The matching record, or the fallback record when none matched.
 */
FieldActorRecord* field_find_actor_record_or_default(s32 id)
{
    FieldActorRecord* actor;

    actor = field_find_actor_record(id);

    if (actor == NULL)
    {
        actor = &g_field_runtime->events[FIELD_DEFAULT_EVENT_RECORD];
    }

    return actor;
}

/**
 * @brief Find the record for an actor id.
 * @param id Ids below FIELD_PARTY_SIZE index the party records, ids up to FIELD_EVENT_ACTOR_ID_BASE
 *           search the active records, higher ids name event records.
 * @return The record, or NULL when a searched id is not present.
 */
FieldActorRecord* field_find_actor_record(s32 id)
{
    s32 i;

    if (id < FIELD_PARTY_SIZE)
    {
        return &g_field_runtime->actors[id];
    }
    else if (id < FIELD_EVENT_ACTOR_ID_BASE)
    {
        for (i = 0; i < FIELD_ACTOR_RECORD_COUNT; i++)
        {
            if (g_field_runtime->actors[i].flags.bits.active && (g_field_runtime->actors[i].id == id))
            {
                return &g_field_runtime->actors[i];
            }
        }
    }
    else
    {
        return &g_field_runtime->events[id - FIELD_EVENT_ACTOR_ID_BASE];
    }

    return NULL;
}

/**
 * @brief Claim the first free actor record for an actor id and reset it.
 * @param id Actor id to store in the record.
 * @return The claimed record, or NULL when all FIELD_ACTOR_RECORD_COUNT records are active.
 */
FieldActorRecord* field_alloc_actor_record(s32 id)
{
    s32 i;
    s32 j;

    for (i = 0; i < FIELD_ACTOR_RECORD_COUNT; i++)
    {
        if (!g_field_runtime->actors[i].flags.bits.active)
        {
            g_field_runtime->actors[i].id = id;
            g_field_runtime->actors[i].event = FIELD_NO_EVENT;
            g_field_runtime->actors[i].enabled_events = FIELD_ALL_EVENTS;
            g_field_runtime->actors[i].flags.bits.trigger_group = 0;
            g_field_runtime->actors[i].flags.bits.spawned = 0;
            g_field_runtime->actors[i].flags.bits.script_only = 0;
            g_field_runtime->actors[i].flags.bits.active = 1;

            for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
            {
                g_field_runtime->actors[i].scripts[j] = FIELD_NO_SCRIPT;
            }

            return &g_field_runtime->actors[i];
        }
    }

    return NULL;
}

/**
 * @brief Stop an actor's script, optionally stopping the actor through field_stop_actor first.
 * @param actor_id Actor id.
 * @param flags FIELD_STOP_ACTOR to also call field_stop_actor.
 */
void field_stop_actor_script(s32 actor_id, s32 flags)
{
    FieldActorRecord* actor;

    if (flags & FIELD_STOP_ACTOR)
    {
        field_stop_actor(actor_id);
    }

    actor = field_find_actor_record_or_default(actor_id);
    actor->script.depth = 0;
    actor->script.frames[0].pc = NULL;
    actor->script.status.bits.running = 0;
}

/**
 * @brief Stop every actor that is not script-only and disable its event 8.
 */
void field_stop_non_script_actors(void)
{
    s32 i;

    for (i = 0; i < g_field_runtime->state.actor_count; i++)
    {
        if (!g_field_runtime->actors[i].flags.bits.script_only)
        {
            g_field_runtime->actors[i].enabled_events &= ~FIELD_EVENT_8_ENABLED;
            field_stop_actor_script(g_field_runtime->actors[i].id, FIELD_STOP_ACTOR);
        }
    }
}

/**
 * @brief Script command 0x28: walk the actor records without touching them.
 */
void field_actor_loop_stub(void)
{
    s32 i;

    for (i = 0; i < g_field_runtime->state.actor_count; i++)
    {
    }
}

/**
 * @brief Find the resource record whose leading halfword equals @p resource_id.
 * @param resource_id Resource id to look for.
 * @return The record, or NULL after reporting the failed lookup.
 */
void* field_find_resource(s32 resource_id)
{
    u32* base;
    u16* record;
    s32 i;
    u32 count;

    base = field_get_scene_record_table();
    count = base[0] >> 2;

    for (i = 0; i < (s32)count; i++)
    {
        record = (u16*)((u8*)base + base[i]);

        if (*record == resource_id)
        {
            return record;
        }
    }

    record_game_diagnostic(DIAG_ERROR, DIAG_MISSING_RESOURCE, resource_id, 0);
    return NULL;
}
