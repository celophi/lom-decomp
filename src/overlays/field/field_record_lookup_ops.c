/**
 * @file field_record_lookup_ops.c
 * @brief Actor record lookup and allocation, reward pickup and resource lookup.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief FieldActorRecord::pickup bit 15: the pickup is a counter, not a reward key. */
#define FIELD_PICKUP_COUNTER 0x8000

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
 * @brief Battle reward entry, 0x44 bytes from the table base.
 * @note Entry 0's count field holds the number of entries; each entry's item
 *       starts at offset 8 and runs into the next entry.
 */
typedef struct
{
    u16 unk0;
    u16 count;
    s32 key;
    u8 item[0x3C];
} FieldRewardEntry;

FieldStatusState* field_find_object_state(s32 actor_id);
u32* field_get_scene_record_table(void);
u8* field_find_free_inventory_record(void);
void field_append_dialog_item(s32 text, u8 quantity);
FieldActorRecord* field_find_actor_record_or_default(s32 id);
FieldActorRecord* field_find_actor_record(s32 id);

extern FieldRuntimeContext* g_field_runtime;
extern FieldBattleContext* g_field_battle;
extern u16 D_800F0E98[];

/**
 * @brief Hand an actor's pickup to the party: an item from the reward table or a counter.
 * @param unused Unused.
 * @param owner_id Actor whose pickup code is resolved.
 */
void field_grant_actor_pickup(void* unused, s32 owner_id)
{
    FieldActorRecord* actor;
    u16 code;
    s32 index;
    s32 count;
    FieldRewardEntry* table;
    FieldRewardEntry* cursor;
    s32 offset;
    s32 old_offset;
    u8* found;
    u8* handle;

    actor = field_find_actor_record_or_default(owner_id);
    code = actor->pickup;
    index = code & FIELD_PICKUP_INDEX_MASK;
    if (!(code & FIELD_PICKUP_COUNTER))
    {
        s32 key;

        table = (FieldRewardEntry*)g_field_battle->resources;
        /* The do/while(0) blocks weight the table, count and key registers for the allocator. */
        do
        {
            found = NULL;
        } while (0);
        count = table->count;
        do
        {
            index = 0;
            if (count != 0)
            {
                s32 n;
                FieldRewardEntry* base;

                key = code & 0xFFFF;
                base = table;
                n = count;
                cursor = base;
                offset = 0;
                do
                {
                    do
                    {
                        do
                        {
                            old_offset = offset;
                        } while (0);
                    } while (0);
                    if (cursor->key == key)
                    {
                        found = (u8*)(old_offset + (s32)base + 8);
                        break;
                    }
                    do
                    {
                        cursor++;
                    } while (0);
                    offset = old_offset + sizeof(FieldRewardEntry);
                    index += 1;
                } while (index < n);
            }
        } while (0);
        if (found == NULL)
        {
            return;
        }
        handle = field_find_free_inventory_record();
        if (handle == NULL)
        {
            return;
        }
        field_copy_inventory_record(handle, found);
        field_append_dialog_item((s32)handle, 0);
        return;
    }
    /* The original passes the actor as a second argument, which func_800C2138 ignores. */
    ((void (*)(s32, FieldActorRecord*))func_800C2138)(index, actor);
    field_append_dialog_item((s32)((u8*)D_800F0E98 + D_800F0E98[index]), 1);
}

/**
 * @brief Restore a fraction of an actor's maximum capacity, capped at that maximum.
 * @param record_id Identifier of an existing runtime actor.
 * @param fraction_256 Fraction in units of 1/256; pickups use 64 or 128.
 * @note The amount comes from maximum capacity, not the current value.
 */
void field_restore_actor_capacity_fraction(s32 record_id, s32 fraction_256)
{
    FieldStatusState* state;
    s32 scaled_capacity;

    state = field_find_object_state(record_id);
    scaled_capacity = state->maximum * fraction_256;
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
            g_field_runtime->actors[i].enabled_events = 0xFFFF;
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
 * @brief Loop over the actor records with an empty body (script command 0x28).
 * @note The body was compiled away in the original build.
 */
void func_800C1E08(void)
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
u16* func_800C1E40(s32 resource_id)
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
