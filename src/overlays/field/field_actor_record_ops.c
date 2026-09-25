/** @file field_actor_record_ops.c
 * @brief Spawn and release dropped-item actor records, find the nearest one, toggle script-only mode.
 */

#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

/** @brief Item argument that releases the item actor's record instead of spawning it. */
#define FIELD_ITEM_RELEASE 0xFF

/** @brief Actor record whose event table every spawned item record copies. */
#define FIELD_ITEM_TEMPLATE_RECORD 4

/** @brief Event run on a freshly spawned item record, with the item id as argument. */
#define FIELD_ITEM_SPAWN_EVENT 7

/** @brief Object key whose position and facing select the nearest item (role unknown). */
#define FIELD_ITEM_SEARCH_KEY 6

/** @brief Result of field_find_nearest_faced_item when no item qualifies. */
#define FIELD_NO_ITEM_KEY 0xFF

/** @brief Upper bound of the item candidates: one per actor record. */
#define FIELD_ITEM_CANDIDATE_COUNT FIELD_ACTOR_RECORD_COUNT

/** @brief field_set_actor_record_script_only flags. */
#define FIELD_SCRIPT_ONLY_STOP_ACTOR 0x01
#define FIELD_SCRIPT_ONLY_MOVE_AWAY 0x02

/** @brief X position, in whole units, that parks an actor outside the map. */
#define FIELD_PARKED_X (-1024)

/** @brief Item key paired with its distance from the search position. */
typedef struct
{
    s32 key;
    s32 distance;
} FieldItemCandidate;

/** @brief Item candidates; func_800C1F28 sorts the entries by ascending distance. */
typedef struct
{
    s32 count;
    FieldItemCandidate entries[FIELD_ITEM_CANDIDATE_COUNT];
} FieldItemCandidateList;

FieldActorRecord* func_800C1C50(s32 id);
FieldActorRecord* func_800C1B98(s32 id);
FieldActorRecord* func_800C1B60(s32 id);
void func_800C1D14(s32 actor_id, s32 flags);
void func_800C1F28(FieldItemCandidateList* list);
s32 func_800C1FBC(Vec3i* first, Vec3i* second);
s32 field_get_actor_position(s32 key, Vec3i* position);
s32 field_set_actor_position(s32 key, s32 x, s32 y, s32 z);

extern FieldRuntimeContext* g_field_runtime;

/**
 * @brief Spawn the actor record of a dropped item and run its spawn event, or release it.
 * @param key Object key of the item actor.
 * @param item Item id passed to the spawn event, or FIELD_ITEM_RELEASE to release the record.
 */
void field_spawn_item_record(s32 key, s32 item)
{
    FieldActorRecord* record;
    s32 i;

    if (item != FIELD_ITEM_RELEASE)
    {
        record = func_800C1C50(key);
        if (record == NULL)
        {
            record_game_diagnostic(DIAG_ERROR, 1, 1, 1);
            return;
        }
        record->flags.bits.spawned = 1;
        record->enabled_events = g_field_runtime->actors[FIELD_ITEM_TEMPLATE_RECORD].enabled_events;
        for (i = 0; i < FIELD_ACTOR_SCRIPT_COUNT; i++)
        {
            record->scripts[i] = g_field_runtime->actors[FIELD_ITEM_TEMPLATE_RECORD].scripts[i];
        }
        field_run_actor_event(key, FIELD_ITEM_SPAWN_EVENT, item & 0xFF);
        return;
    }

    record = func_800C1B60(key);
    record->flags.bits.active = 0;
    record->flags.bits.spawned = 0;
}

/**
 * @brief Find the nearest spawned item that object FIELD_ITEM_SEARCH_KEY faces.
 * @param unused Script operand; not used.
 * @return Key of the nearest faced item, or FIELD_NO_ITEM_KEY when there is none.
 */
s32 field_find_nearest_faced_item(s32 unused)
{
    FieldItemCandidateList list;
    Vec3i search_position;
    Vec3i item_position;
    s32 i;
    FieldActorRecord* record;

    field_get_actor_position(FIELD_ITEM_SEARCH_KEY, &search_position);
    list.count = 0;
    for (i = FIELD_PARTY_SIZE; i < FIELD_ACTOR_RECORD_COUNT; i++)
    {
        record = &g_field_runtime->actors[i];
        /* Sign test for the active bit: a bitfield test is merged with spawned into one masked compare. */
        if (record->flags.word < 0 && record->flags.bits.spawned && field_actor_faces_actor(FIELD_ITEM_SEARCH_KEY, record->id) == 1)
        {
            field_get_actor_position(record->id, &item_position);
            list.entries[list.count].key = record->id;
            list.entries[list.count].distance = func_800C1FBC(&search_position, &item_position);
            list.count += 1;
        }
    }

    func_800C1F28(&list);
    if (list.count != 0)
    {
        return list.entries[0].key;
    }
    return FIELD_NO_ITEM_KEY;
}

/**
 * @brief Put an actor record into script-only mode and stop its script.
 * @param key Actor key.
 * @param flags FIELD_SCRIPT_ONLY_STOP_ACTOR also stops the field actor; FIELD_SCRIPT_ONLY_MOVE_AWAY parks it off the map.
 */
void field_set_actor_record_script_only(s32 key, s32 flags)
{
    FieldActorRecord* record = func_800C1B98(key);

    if (record != NULL)
    {
        record->flags.bits.script_only = 1;
        func_800C1D14(record->id, flags);
        if (flags & FIELD_SCRIPT_ONLY_MOVE_AWAY)
        {
            field_set_actor_position(key, FIELD_PARKED_X, 0, 0);
        }
    }
}

/**
 * @brief Take an actor record out of script-only mode and stop its script.
 * @param key Actor key.
 */
void field_clear_actor_record_script_only(s32 key)
{
    FieldActorRecord* record;

    record = func_800C1B60(key);
    record->flags.bits.script_only = 0;
    func_800C1D14(record->id, 0);
}
